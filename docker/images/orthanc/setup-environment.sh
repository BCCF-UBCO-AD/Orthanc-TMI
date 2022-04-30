#!/bin/bash

# Install all dependencies
pacman -Sy --noconfirm dos2unix libpqxx gcc ninja cmake wget patch unzip python tzdata gzip gcc-libs

# Download Orthanc core
cd /usr/bin
wget https://lsb.orthanc-server.com/orthanc/1.10.0/Orthanc
chmod +x ./Orthanc

# Download supporting libraries
cd /usr/lib
wget https://lsb.orthanc-server.com/orthanc/1.10.0/libConnectivityChecks.so
wget https://lsb.orthanc-server.com/orthanc/1.10.0/libModalityWorklists.so
wget https://lsb.orthanc-server.com/orthanc/1.10.0/libServeFolders.so

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