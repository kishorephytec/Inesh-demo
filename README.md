# Inesh-demo

A Wi-SUN Border Router is a key component in a Wi-SUN network that acts as a gateway between the wireless mesh network (Field Area Network) and the HES(Head End System)

It connects Wi-SUN-enabled nodes/devices (like smart meters, sensors, or streetlights) to external networks, enabling two-way communication for monitoring, control, and data collection.
🔑 Key Roles:

    Translates between IPv6 mesh and standard IP networks

    Maintains and advertises PAN (Personal Area Network) parameters

    Handles routing, security, and mesh network formation

    Often interfaces with cloud or backend systems via Ethernet or cellular

Follow Below Preocedure to prepare wisun border router:
*******************************************************
Required Packages:
******************
sudo apt-get install libnl-3-dev libnl-route-3-dev libcap-dev libsystemd-dev cmake ninja-build libcjson-dev libmosquitto-dev pkg-config lrzsz libdbus-1-dev libxml2-dev

Clone the source and Follow Below steps:
****************************************
mkdir wisun_gateway
cd wisun_gateway

git clone --branch=v3.6.2 --recurse-submodules https://github.com/ARMmbed/mbedtls
cd mbedtls
cmake -B build -G Ninja .
ninja -C build
sudo ninja -C build install

Copy wisun border router source and follow below procedure:
***********************************************************
cd wisun-br-linux-inesh/
rm -rf build
cmake -B build -G Ninja .
ninja -C build
sudo ninja -C build install

To additionally compile wsbrd_cli:
***********************************
sudo apt-get install cargo libdbus-1-dev
cargo fetch --manifest-path=tools/wsbrd_cli/Cargo.toml

install the packages for dlms:
*******************************
1.pip install gurux-common --br
2.pip install gurux-serial --br
3.pip install gurux-net --br
4.pip install gurux-dlms --br
pip install pydbus --br

CONFIGURATION
------------------------
1. cd wisun-br-linux-inesh/examples/
2. changes these parameters in wsbrd.conf
	network_name = Test_1
	size= SMALL
	enable_lfn=false
	domain= IN
	regional_regulation = wpc
	allowed_channels = 6-7
	class= 2
	mode=3
	enable_ffn10=yes
	
Note : comment phy_mod_id and phy_operating_modes



