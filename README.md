# Orthanc-TMI

### Table of Contents

 - [Dependencies](#dependencies)
   - [Tools](#tools)
   - [Submodules](#submodules)
 - [Getting Started](#getting-started)
   - [Clone](#clone)
   - [Build](#build)
   - [Docker](#docker)
     - [WSL](#windows-subsystem-for-linux)
 - [Contributing](#contributing)
   - [Style Guide](#style-guide)
   - [Special Branches](#special-branches)
   - [Branching](#branching)
 - [Testing](#testing)

This software has been developed as a plugin to run on Orthanc DICOM servers.
## Dependencies
First and foremost this project [creates an Orthanc plugin](https://book.orthanc-server.com/developers/creating-plugins.html#structure-of-the-plugins), so the Orthanc [plugin SDK](https://sdk.orthanc-server.com/index.html) is required. Which is available as [OrthancCPlugin.h](https://hg.orthanc-server.com/orthanc/file/Orthanc-1.9.7/OrthancServer/Plugins/Include/orthanc/OrthancCPlugin.h)
#### Tools
* GCC - compiler
* Cmake compatible build system (eg. GNU Make, Ninja)
* Cmake 3.20 - configures build system
* Docker - local testing ([docker image](https://hub.docker.com/r/jodogne/orthanc-plugins))
* Github Actions - remote testing
* CLion (recommended)

#### Submodules
| Library | Purpose | URI |
|---------|---------|-----|
| [libpq](lib) | PostgreSQL API | <ul><li>[external repo](https://github.com/postgres/postgres.git) <li>[docs - configure/build/install](https://www.postgresql.org/docs/14/install-procedure.html) |
| [libpqxx](lib) | libpq wrapper | <ul><li>[external repo](https://github.com/jtv/libpqxx.git) <li>[docs - API](https://libpqxx.readthedocs.io/en/stable/a01382.html) |
| [nlohmann/json](lib) | json API | <ul><li>[external repo](https://github.com/nlohmann/json.git) <li>[docs - integration](https://github.com/nlohmann/json#integration) <li>[docs - API](https://nlohmann.github.io/json/api/basic_json/) |
| [googletest](lib) | unit testing | <ul><li>[external repo](https://github.com/google/googletest.git) |

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
To test locally you'll need to launch docker with..
```bash
$ sudo docker-compose up
```
This will launch 3 docker containers with images from Docker Hub:
 - `orthanc-server`: Custom image based on Archlinux with Orthanc server, all official plugins as well as dependencies needed for our plugin
 - `postgresql`: Official PostgreSQL docker image. Database.
 - `adminer`: Official Adminer docker image, use to manage PostgreSQL database.
 
Then you can proceed to test whatever in whatever way. The docker server reads a copy of the plugin binary from `docker/plugins/` (cmake configures the copy operation).

#### Build Docker Image
To build the custom docker image instead of pulling from Docker Hub:
```bash
cd ./docker
docker image build .
```
Then update `docker-compose.yml` with the new image ID.
 
#### Windows Subsystem for Linux
When working in WSL, permissions may be an issue for docker mounting the persistent PostgreSQL data. By default Windows' file systems will be mounted under ``/mnt`` for WSL. To avoid permission issues with mounting docker's `/var/lib/postgresql/data` you'll need to change the volume for Postgres to any location that's native to your linux distribution.

You can edit `docker-compose.yml` where..
```bash
#instead of using
./docker/postgres:/var/lib/postgresql/data
#use this instead
~/docker/postgres:/var/lib/postgresql/data
```

## Contributing
### Style Guide
The C++ auto style settings can be found in CLion at `Settings -> Editor -> Code Style -> C++`, here you can import our styles from `c++styles.xml` in the project root.

Short and simple example of naming conventions and other formats to adhere to in order to maintain consistency.
```cpp
#include <iostream>

#define MY_MACRO auto x = [](){Foo();};

int main(){
    int foo_total_count = 0; 
    std::cout << ++foo_total_count;
}

class FooBar{ //Our classes should be PascalCase, unless implemented inside another class or inside a cpp file
    struct data_type{
        int example_of;
        int private_inner_class;
    };
    struct Data{ //no convention, up to preference for these
        int example_of; //these still gotta be underscore_case
        int private_inner_class;
    };
public: //variable order really only matters in terms of what is logical and or if you need memory structured a particular way
    int special_public_var_packed_at_the_front_of_object = 42;
private:
    //members
    int foo_total_count = 0;
    
    //methods
    void foo(){}
    //hard wrap at col 120
    //this can be negotiated if someone can't fit 120+ chars in their clion views (font change? proggyfonts.net/)
    //it also doesn't apply to comments, use your best judgement for comments
    void helper(std::unordered_map<size_t, std::vector<std::vector<iterator_type_with_a_long_name>> the_thing, int x){}
    void helper2(std::unordered_map<size_t, std::vector<std::vector<iterator_type_with_a_long_name>> the_thing,
                 std::unordered_map<size_t, std::vector<std::vector<iterator_type_with_a_long_name>> the_thing2){}
    void helper2(int x, /* when the signature surpasses, newline each arg */
                 int y,
                 int z,
                 std::unordered_map<size_t, std::vector<std::vector<size_t>> the_thing, 
                 std::unordered_map<size_t, std::vector<std::vector<iterator_type_with_a_long_name>> the_thing2){}
protected:
    void protected_special_helper(){}
public:
    bool public_flag = false;
    void Bar(){}
};

void Foo(int foo_bar){
}
```

### Special Branches
| Name | Purpose |
|------|---------|
| master | stable branch |
| develop | development branch for **merging** new libraries/features/etc. |
| documents | documentation branch for independent updating of documents |
| readme | readme update branch for standalone updates to the readme file |
| libraries | library integration branch for getting new libraries up and running


### Branching
Our strategy is simple: 
 1. develop code in a branch
    - test
    - fix
    - test
 2. merge to * *(if and when needed)*
 3. merge to develop
      - test
      - fix
      - test
 4. merge to master.

As a general guide to naming branches:

| Prefix | Purpose | Delete After? |
|--------|---------|----------|
| `feat-` | feature development | ok |
| `refactor-` | refactoring existing systems | no |
| `user-patch-` | fixes from github | no |
| `hotfix-` | single commit fixes | no |
| `patch-` | fixes for tracked issues | no |
| `fix-` | other fixes | no |

## The Darkest Time Line

## Testing
  - Google_Test Framework for unit Testing. [Here](https://github.com/google/googletest.git)
  - Circle CI or similar for continuous integration.
  - Circle CI with GitHub to test  pull requests to main and develop branches

# Notes:
- https://book.orthanc-server.com/users/docker.html#usage-with-plugins-enabled
