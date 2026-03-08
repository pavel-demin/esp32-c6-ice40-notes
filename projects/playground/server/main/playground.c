#include <stdint.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#define WIFI_SSID "esp32"
#define WIFI_PASS "12345678"
#define WIFI_CHAN 11

#define PIN_NUM_MOSI GPIO_NUM_15
#define PIN_NUM_MISO GPIO_NUM_19
#define PIN_NUM_SCLK GPIO_NUM_7
#define PIN_NUM_CS GPIO_NUM_18

#define PIN_NUM_FPGA_RESET GPIO_NUM_20
#define PIN_NUM_FPGA_CDONE GPIO_NUM_21
#define PIN_NUM_FPGA_DAT1 GPIO_NUM_1

#define BUFFER_SIZE 8192

WORD_ALIGNED_ATTR uint8_t buffer[BUFFER_SIZE] = {0};
WORD_ALIGNED_ATTR uint8_t dummy[8] = {0};

spi_device_handle_t fpga_dev;

void wifi_init_softap()
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t wifi_cfg =
  {
    .ap =
    {
      .ssid = WIFI_SSID,
      .ssid_len = strlen(WIFI_SSID),
      .password = WIFI_PASS,
      .channel = WIFI_CHAN,
      .max_connection = 1,
      .authmode = WIFI_AUTH_WPA2_PSK,
    }
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg));
  ESP_ERROR_CHECK(esp_wifi_config_11b_rate(ESP_IF_WIFI_AP, true));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
  ESP_ERROR_CHECK(esp_wifi_start());
}

static void init_gpio()
{
  gpio_set_direction(PIN_NUM_FPGA_RESET, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_FPGA_RESET, 0);

  gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
  gpio_set_level(PIN_NUM_CS, 0);

  gpio_set_direction(PIN_NUM_FPGA_DAT1, GPIO_MODE_INPUT);
}

static void init_spi()
{
  spi_bus_config_t bus_cfg =
  {
    .mosi_io_num = PIN_NUM_MOSI,
    .miso_io_num = PIN_NUM_MISO,
    .sclk_io_num = PIN_NUM_SCLK,
    .max_transfer_sz = BUFFER_SIZE
  };

  spi_device_interface_config_t fpga_dev_cfg =
  {
    .clock_speed_hz = SPI_MASTER_FREQ_40M,
    .spics_io_num = -1,
    .queue_size = 1
  };

  spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
  spi_bus_add_device(SPI2_HOST, &fpga_dev_cfg, &fpga_dev);
}

static int sock_recv(int sock, void *buffer, int size)
{
  int offset, limit;
  offset = 0;
  limit = size;
  while(offset < limit)
  {
    size = recv(sock, buffer + offset, limit - offset, 0);
    if(size <= 0) return size;
    offset += size;
  }
  return offset;
}

static int sock_send(int sock, void *buffer, uint32_t size)
{
  uint32_t offset, limit;
  offset = 0;
  limit = size;
  while(offset < limit)
  {
    size = send(sock, buffer + offset, limit - offset, 0);
    if(size <= 0) return size;
    offset += size;
  }
  return offset;
}

static void spi_send(spi_device_handle_t device, uint8_t addr_size, uint32_t addr, uint32_t data_size, void *data)
{
  spi_transaction_ext_t t =
  {
    .base =
    {
      .addr = SPI_SWAP_DATA_TX(addr, addr_size),
      .length = data_size * 8,
      .tx_buffer = data,
      .flags = SPI_TRANS_VARIABLE_ADDR
    },
    .address_bits = addr_size,
  };
  spi_device_transmit(device, (spi_transaction_t *)&t);
}

static void spi_recv(spi_device_handle_t device, uint8_t addr_size, uint32_t addr, uint32_t data_size, void *data)
{
  spi_transaction_ext_t t =
  {
    .base =
    {
      .addr = SPI_SWAP_DATA_TX(addr, addr_size),
      .length = data_size * 8,
      .rx_buffer = data,
      .flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_VARIABLE_DUMMY
    },
    .address_bits = addr_size,
    .dummy_bits = 20
  };
  spi_device_transmit(device, (spi_transaction_t *)&t);
}

static void tcp_server_task(void *pvParameters)
{
  int sock_server, sock_client;
  struct sockaddr_in sock_addr = {0};
  uint32_t command, code, data, size, total, one = 1;

  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock_addr.sin_port = htons(1001);

  if((sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) goto cleanup;

  setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  if(bind(sock_server, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) goto cleanup;

  if(listen(sock_server, 1) < 0) goto cleanup;

  while(1)
  {
    if((sock_client = accept(sock_server, NULL, NULL)) < 0) break;

    while(1)
    {
      if(sock_recv(sock_client, &command, 4) <= 0) break;
      code = command >> 30 & 0x3;
      data = command & 0x3fffffff;
      switch(code)
      {
        case 0:
          // read data
          size = ((command >> 20 & 0x3ff) + 1) * 4;
          gpio_set_level(PIN_NUM_CS, 0);
          spi_recv(fpga_dev, 32, command, size, buffer);
          gpio_set_level(PIN_NUM_CS, 1);
          sock_send(sock_client, buffer, size);
          break;
        case 1:
          // write data
          size = ((command >> 20 & 0x3ff) + 1) * 4;
          if(sock_recv(sock_client, buffer, size) <= 0) continue;
          gpio_set_level(PIN_NUM_CS, 0);
          spi_send(fpga_dev, 32, command, size, buffer);
          gpio_set_level(PIN_NUM_CS, 1);
          break;
        case 2:
          // configure FPGA
          gpio_set_level(PIN_NUM_CS, 0);

          gpio_set_level(PIN_NUM_FPGA_RESET, 0);
          vTaskDelay(2);
          gpio_set_level(PIN_NUM_FPGA_RESET, 1);
          vTaskDelay(4);

          total = data;
          size = BUFFER_SIZE;

          while(total > 0)
          {
            if(size > total) size = total;
            if(sock_recv(sock_client, buffer, size) <= 0) break;
            spi_send(fpga_dev, 0, 0, size, buffer);
            total -= size;
          }

          if(total > 0) continue;

          gpio_set_level(PIN_NUM_CS, 1);
          spi_send(fpga_dev, 0, 0, 8, dummy);
          break;
        default:
          break;
      }
    }

    shutdown(sock_client, 0);
    close(sock_client);
  }

cleanup:
  if(sock_server >= 0) close(sock_server);
  vTaskDelete(NULL);
}

void app_main()
{
  esp_err_t rc = nvs_flash_init();
  if(rc == ESP_ERR_NVS_NO_FREE_PAGES || rc == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    rc = nvs_flash_init();
  }
  ESP_ERROR_CHECK(rc);

  wifi_init_softap();

  init_gpio();

  init_spi();

  xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 23, NULL);
}
