#!/bin/bash

set -e
cd

URL=http://lsb.orthanc-server.com/

VERSION_AUTHORIZATION=mainline
VERSION_DICOM_WEB=mainline
VERSION_GDCM=mainline-gdcm3
VERSION_INDEXER=mainline
VERSION_MYSQL=mainline
VERSION_NEURO=mainline
VERSION_ODBC=mainline
VERSION_POSTGRESQL=mainline
VERSION_TCIA=mainline
VERSION_TRANSFERS=mainline
VERSION_WEB_VIEWER=mainline
VERSION_WSI=mainline
VERSION_STONE_WEB_VIEWER=mainline
VERSION_STONE_RT_SAMPLE=mainline

# Download binaries compiled with Linux Standard Base

# 2020-01-24: The DICOMweb and Web viewer plugins have no unit test
# anymore, as they are now built using the Holy Build Box because of
# incompatibility between GDCM 3.0 and LSB compilers

wget ${URL}/plugin-postgresql/${VERSION_POSTGRESQL}/UnitTests -O - > UnitTests-PostgreSQL
wget ${URL}/plugin-postgresql/${VERSION_POSTGRESQL}/libOrthancPostgreSQLIndex.so
wget ${URL}/plugin-postgresql/${VERSION_POSTGRESQL}/libOrthancPostgreSQLStorage.so

chmod +x ./UnitTests-PostgreSQL

# Run the unit tests
mkdir ~/UnitTests
cd ~/UnitTests
# ../UnitTests-PostgreSQL

# Recover space used by the unit tests
cd
rm -rf ./UnitTests-PostgreSQL

# Move the binaries to their final location
mv ./libOrthancPostgreSQLIndex.so      /usr/local/share/orthanc/plugins/
mv ./libOrthancPostgreSQLStorage.so    /usr/local/share/orthanc/plugins/