git clone --recursive https://github.com/YosysHQ/yosys
cd yosys
make CONFIG=gcc ENABLE_PYOSYS=1 PREFIX=/opt/yosys PYTHON_DESTDIR=/opt/yosys/share/yosys/python3 install
cd -

git clone https://github.com/YosysHQ/icestorm
cd icestorm
make PREFIX=/opt/nextpnr install
cd -

git clone --recursive https://github.com/YosysHQ/nextpnr
mkdir nextpnr/build
cd nextpnr/build
cmake -DARCH=ice40 -DBUILD_GUI=ON -DCMAKE_INSTALL_PREFIX=/opt/nextpnr ..
make install
cd -
