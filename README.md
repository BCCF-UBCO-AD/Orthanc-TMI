# Orthanc-TMI

This software has been developed as a plugin to run on Orthanc DICOM servers.
## Getting Started
### Build
```bash
# clone repo
git clone https://github.com/BCCF-UBCO-AD/Orthanc-TMI.git orthanc-tmi
cd orthanc-tmi
# develop is where all the action is
git checkout develop
# we need to populate submodules (googletest)
git submodule init
git submodule update
# build
mkdir build
cd build
cmake .. -G Ninja
ninja
```
The cmake configuration will ensure the plugin binary is copied to a directory where docker will see it.

### Docker
To test, whatever, on your local system just launch docker.
```bash
$ sudo docker-compose up
```
Then proceed to test whatever in whatever way.
The docker reads the a copy of the build from `docker/plugins/`.

## Contributing
### Style Guide
```cpp
#include <iostream>

int main(){
    int foo_total_count = 0;
    std::cout << ++foo_total_count;
}

class FooBar{
private:
    //members
    int foo_total_count = 0;
    
    //methods
    void foo(){}
public:
    bool public_flag = false;
    void Bar(){}
};

void Foo(){
}
```

### Branching
Be mindful if you delete branches. Other branches may want a particular fix without merging all of develop (assuming it is merged with develop).

- Feature branches: `feat-`
  - delete after or keep
- One commit fix branches: `hotfix-` or github default
  - keep after
- Other fix branches: `fix-` `patch-`
  - keep after

# Tech Stack

# Time Line

# Testing
  - Google_Test Framework for unit Testing. [Here](https://github.com/google/googletest.git)
  - Circle CI or similar for continuous integration.
  - Circle CI with GitHub to test  pull requests to main and develop branches

# Notes:
- https://hub.docker.com/r/jodogne/orthanc
- https://book.orthanc-server.com/developers/creating-plugins.html
- https://book.orthanc-server.com/users/docker.html#usage-with-plugins-enabled
- https://sdk.orthanc-server.com/
- https://www.codeproject.com/Articles/797118/Implementing-a-WADO-Server-using-Orthanc