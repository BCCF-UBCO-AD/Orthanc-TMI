#!/bin/bash
wd=$(realpath $(dirname "${BASH_SOURCE[0]}"))
mkdir $wd/docker/orthanc-db
mkdir $wd/docker/plugins
sudo docker run -p 4242:4242 -p 8042:8042 --rm -v $wd/docker/orthanc.json:/etc/orthanc/orthanc.json:ro -v $wd/docker/orthanc-db/:/var/lib/orthanc/db/ -v $wd/docker/plugins:/usr/share/orthanc/plugins jodogne/orthanc-plugins