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

### Branches
| Name | Purpose |
|------|---------|
| master | stable branch |
| develop | development branch for **merging** new libraries/features/etc. |
| documents | documentation branch for independent updating of documents |
| readme | readme update branch for standalone updates to the readme file |
| libraries | library integration branch for getting new libraries up and running


### Branching
We may want to merge branches to things other than develop, use best judgement. As a general guide:

| Prefix | Purpose | Delete After? |
|--------|---------|----------|
| `feat-` | feature development | ok |
| `refactor-` | refactoring existing systems | no |
| `user-patch-` | fixes from github | no |
| `hotfix-` | single commit fixes | no |
| `patch-` | fixes for tracked issues | no |
| `fix-` | other fixes | no |

# Tech Stack
### Libraries
| Library | Purpose | URI |
|---------|---------|-----|
| libpqxx | Postgres API | <ul><li>https://github.com/jtv/libpqxx.git <li>https://libpqxx.readthedocs.io/en/stable/a01382.html |
| nlohmann/json | json API | <ul><li>https://github.com/nlohmann/json.git <li>https://github.com/nlohmann/json#integration <li>https://nlohmann.github.io/json/api/basic_json/|
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
