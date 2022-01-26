# Orthanc-TMI

### Table of Contents
 - [Getting Started](#getting-started)
   - [Clone](#clone)
     - [Dependencies](#dependencies)
     - [Submodules](#submodules)
   - [Building](#building)
     - [Tools](#tools)
     - [Build](#build)
   - [Docker](#docker)
     - [Build Image](#build-image)
     - [WSL](#windows-subsystem-for-linux)
 - [Contributing](#contributing)
   - [Style Guide](#style-guide)
   - [Special Branches](#special-branches)
   - [Branching](#branching)
 - [Testing](#testing)

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
#### Dependencies
First and foremost this project [creates an Orthanc plugin](https://book.orthanc-server.com/developers/creating-plugins.html#structure-of-the-plugins), so the Orthanc [plugin SDK](https://sdk.orthanc-server.com/index.html) is required. Which is available as [OrthancCPlugin.h](https://hg.orthanc-server.com/orthanc/file/Orthanc-1.9.7/OrthancServer/Plugins/Include/orthanc/OrthancCPlugin.h)

This repo has a copy of that file under [include/orthanc/](include/orthanc/)

#### Submodules
The submodules you need to initialize.
| Library | Purpose | URI |
|---------|---------|-----|
| [libpqxx](lib) | libpq wrapper | <ul><li>[external repo](https://github.com/jtv/libpqxx.git) <li>[docs - API](https://libpqxx.readthedocs.io/en/stable/a01382.html) |
| [nlohmann/json](lib) | json API | <ul><li>[external repo](https://github.com/nlohmann/json.git) <li>[docs - integration](https://github.com/nlohmann/json#integration) <li>[docs - API](https://nlohmann.github.io/json/api/basic_json/) |
| [googletest](lib) | unit testing | <ul><li>[external repo](https://github.com/google/googletest.git) |

### Building
The project is configured to build a plugin (dll/so) (target `'data-anonymizer'`) binary, then copy it to `docker/plugins` where the orthanc docker server can read it.

#### Tools
Some tools will be needed. Please refer to your system package manager, or each tool's website, in order to install.
* GCC - compiler
* Cmake compatible build system (eg. GNU Make, Ninja)
* Cmake 3.20 - configures build system
* Docker - local testing ([docker image](https://hub.docker.com/r/jodogne/orthanc-plugins))
* Github Actions - remote testing
* CLion (recommended)

#### Build
You'll need some tools, but below is all you'll need to do to build everything.
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

#### Build Image
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
    void helper3(int x, /* when the signature surpasses, newline each arg */
                 int y,
                 int z,
                 std::unordered_map<size_t, std::vector<std::vector<size_t>> the_thing, 
                 std::unordered_map<size_t, std::vector<std::vector<iterator_type_with_a_long_name>> the_thing2){}
protected:
    void protected_special_helper(){}
public:
    bool public_flag = false;
    //Our methods should also be PascalCase, might revisit this later and switch to Camel
    void Bar(){}
};

//Our functions should also be PascalCase, might revisit this later and switch to Camel
void Foo(int foo_bar){
}
```

### Special Branches
| Name | Purpose |
|------|---------|
| master | stable branch |
| develop | development branch for **merging** new libraries/features/etc. |
| documents | documentation branch for independent clerical work |
| libraries | library integration branch for getting new libraries up and running |
| ci | continuous integration branch for independent updating of ci configs |
| samples | for independent updating of dicom files, should be converted into a submodule |


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
| `*` | some special branch with an ongoing purpose | no |

## Testing
  - Google_Test Framework for unit Testing. [Here](https://github.com/google/googletest.git)
  - Github Actions for continuous integration.
  - Github Actions to test pull requests to master and develop branches
