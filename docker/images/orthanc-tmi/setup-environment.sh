#!/bin/bash

# Install all dependencies
pacman -Sy --noconfirm libpqxx gcc ninja cmake wget patch unzip python tzdata gzip

# Download and decompress Orthanc source
cd ~
wget https://www.orthanc-server.com/downloads/get.php?path=/orthanc/Orthanc-1.9.7.tar.gz -O orthanc-1.9.7.tar.gz
tar -xvf orthanc-1.9.7.tar.gz
cd Orthanc-1.9.7

# Build and install Orthanc
mkdir build
cd build
cmake -G Ninja -DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE=Release ../OrthancServer/
ninja install

# Link localtime
ln -sf /usr/share/zoneinfo/Canada/Pacific /etc/localtime

# Create the various directories
mkdir /etc/orthanc
mkdir -p /var/lib/orthanc/db
mkdir -p /usr/share/orthanc/plugins
mkdir -p /usr/local/share/orthanc/plugins

# Auto-generate, then patch the main configuration file
CONFIG=/etc/orthanc/orthanc.json
Orthanc --config=$CONFIG

sed 's/\("Name" : \)".*"/\1"Orthanc inside Docker"/' -i $CONFIG
sed 's/\("IndexDirectory" : \)".*"/\1"\/var\/lib\/orthanc\/db"/' -i $CONFIG
sed 's/\("StorageDirectory" : \)".*"/\1"\/var\/lib\/orthanc\/db"/' -i $CONFIG
sed 's/\("Plugins" : \[\)/\1 \n    "\/usr\/share\/orthanc\/plugins", "\/usr\/local\/share\/orthanc\/plugins"/' -i $CONFIG
sed 's/"RemoteAccessAllowed" : false/"RemoteAccessAllowed" : true/' -i $CONFIG
sed 's/"AuthenticationEnabled" : false/"AuthenticationEnabled" : true/' -i $CONFIG

# New since jodogne/orthanc:1.7.3
sed 's/\("HttpsCACertificates" : \)".*"/\1"\/etc\/ssl\/certs\/ca-certificates.crt"/' -i $CONFIG

# Cleanup
cd ~
rm orthanc-1.9.7.tar.gz
rm -rf Orthanc-1.9.7