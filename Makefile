NAME = led_blinker

PART = up5k
PACKAGE = sg48

MKDIR = mkdir -p
RM = rm -rf

.PRECIOUS: tmp/%.json tmp/%.asc tmp/%.bin

all: tmp/$(NAME).bin

bin: tmp/$(NAME).bin

tmp/%.json: projects/%/top.v
	$(MKDIR) $(@D)
	yosys --commands "synth_ice40 -top top -json $@ -dsp" $< modules/*.v

tmp/%.asc: tmp/%.json
	$(MKDIR) $(@D)
	nextpnr-ice40 --placer sa --starttemp 1e9 --json $< --$(PART) --package $(PACKAGE) --pcf cfg/ports.pcf --asc $@

tmp/%.bin: tmp/%.asc
	$(MKDIR) $(@D)
	icepack $< $@

clean:
	$(RM) tmp
