#!/bin/bash

SUDO=''
if [[ $EUID != 0 ]]
then
	SUDO='sudo'
fi

# run uninstall target
if [ -d "build" ]; then
	cd build
	$SUDO make uninstall
	cd ..
	$SUDO rm -rf build
fi

# manually remove rogue shared objects (not
# doing this step will not affect the build,
# but there may be runtime Python import issues)
$SUDO rm -f /usr/local/lib/libgnuradio-shbb60.so*

# build and install
mkdir build
cd build
cmake ..
make -j$(nproc)
$SUDO make install
$SUDO ldconfig
