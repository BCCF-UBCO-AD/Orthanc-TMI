# Orthanc-TMI

This software has been developed as a plugin to run on Orthanc DICOM servers.
## Getting Started
### Clone
Don't forget to populate submodules.
```bash
# clone repo
git clone https://github.com/BCCF-UBCO-AD/Orthanc-TMI.git orthanc-tmi
cd orthanc-tmi
# develop is where all the action is
git checkout develop
# we need to populate submodules (googletest)
git submodule init
git submodule update
```

### Build
The cmake configuration will ensure the plugin binary is copied to `docker/plugins` where the docker server can read it.
```bash
mkdir build
cd build
# configure build
# on linux
cmake ..
# on windows, pretty sure they've got VS as the default
cmake .. -G "Unix Makefiles"

# perform build
make
```
or if you're a cool ninja
```bash
mkdir build
cd build
cmake .. -G Ninja
ninja
```

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
### Libraries
| Library | Purpose | URI |
|---------|---------|-----|
| libpqxx | Postgres API | <ul><li>https://github.com/jtv/libpqxx.git <li>https://libpqxx.readthedocs.io/en/stable/a01382.html |
| nlohmann/json | json API | <ul><li>https://github.com/nlohmann/json.git <li>https://github.com/nlohmann/json#integration |
| googletest | unit testing | <ul><li>https://github.com/google/googletest.git |

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