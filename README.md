# Orthanc-TMI

This software has been developed as a plugin to run on Orthanc DICOM servers.

# Project

## Tech Stack

## Time Line

## Contributing

### Launching Docker container:

On your local system, from the project root run:
```bash
$ run-docker.sh
```

#### Behind the scenes
```bash
# First we generated a default orthanc.json file.
sudo docker run --rm --entrypoint=cat jodogne/orthanc-plugins /etc/orthanc/orthanc.json > docker/orthanc.json

# Now we're free to start the container:
sudo docker run -p 4242:4242 -p 8042:8042 --rm -v docker/orthanc.json:/etc/orthanc/orthanc.json:ro -v docker/orthanc-db/:/var/lib/orthanc/db/ -v docker/plugins:/usr/share/orthanc/plugins jodogne/orthanc-plugins

# Unfortunately absolute paths were needed, so our bash script just figures that out for us.
#!/bin/bash
wd=$(realpath $(dirname "${BASH_SOURCE[0]}"))
mkdir $wd/docker/orthanc-db
mkdir $wd/docker/plugins
sudo docker run -p 4242:4242 -p 8042:8042 --rm -v $wd/docker/orthanc.json:/etc/orthanc/orthanc.json:ro -v $wd/docker/orthanc-db/:/var/lib/orthanc/db/ -v $wd/docker/plugins:/usr/share/orthanc/plugins jodogne/orthanc-plugins
```
The plugin's cmake file tells the build system to copy the plugin binary to `docker/plugins/` which is integrally linked to the docker container seen in the commands above.

### Testing
#### local testing
1. build the plugin
2. run docker
3. perform tests
#### notes
  - Google_Test Framework for unit Testing. [Here](https://github.com/google/googletest.git)
  - Circle CI or similar for continuous integration.
  - Circle CI with GitHub to test  pull requests to main and develop branches

# Notes:
- https://hub.docker.com/r/jodogne/orthanc
- https://book.orthanc-server.com/developers/creating-plugins.html
- https://book.orthanc-server.com/users/docker.html#usage-with-plugins-enabled
- https://sdk.orthanc-server.com/
- https://www.codeproject.com/Articles/797118/Implementing-a-WADO-Server-using-Orthanc