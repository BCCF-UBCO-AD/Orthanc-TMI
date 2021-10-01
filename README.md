# Orthanc-TMI


## Notes:
- https://hub.docker.com/r/jodogne/orthanc
- https://book.orthanc-server.com/developers/creating-plugins.html
- https://book.orthanc-server.com/users/docker.html#usage-with-plugins-enabled
- https://sdk.orthanc-server.com/
- https://www.codeproject.com/Articles/797118/Implementing-a-WADO-Server-using-Orthanc

### Time Line

### Launching Docker container:

Generate config file: `sudo docker run --rm --entrypoint=cat jodogne/orthanc /etc/orthanc/orthanc.json > ~/orthanc/orthanc.json`

Start container: `sudo docker run -p 4242:4242 -p 8042:8042 --rm -v ~/orthanc/orthanc.json:/etc/orthanc/orthanc.json:ro -v ~/orthanc/orthanc-db/:/var/lib/orthanc/db/ jodogne/orthanc-plugins`

#### Testing

  - Google_Test Framework for unit Testing. [Here](https://github.com/google/googletest.git)
  - Circle CI or similar for continuous integration.
  - Circle CI with GitHub to test  pull requests to main and develop branches

### Tech Stack
