# Orthanc-TMI

### Table of Contents
 - [Getting Started](#getting-started)
   - [Clone](#clone)
     - [Dependencies](#dependencies)
       - [Submodules](#submodules)
   - [Building](#building)
     - [Tools](#tools)
     - [Build](#build)
     - [Install](#install)
   - [Docker](#docker)
     - [Build Image](#build-image)
     - [WSL](#windows-subsystem-for-linux)
 - [Contributing](#contributing)
   - [Style Guide](#style-guide)
   - [Special Branches](#special-branches)
   - [Branching](#branching)
 - [Testing](#testing)
 - [Configuration](#plugin-configuration-json)
   - [Hard Links](#hard-links)
   - [Date Truncation](#date-truncation)
   - [Filter](#filter---removing-dicom-elements)

This software has been developed as a plugin to run on Orthanc DICOM servers.
## Getting Started
### Clone
Don't forget to populate submodules.

<details>

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

</details>

#### Dependencies

<details>

First and foremost this project [creates an Orthanc plugin](https://book.orthanc-server.com/developers/creating-plugins.html#structure-of-the-plugins), so the Orthanc [plugin SDK](https://sdk.orthanc-server.com/index.html) is required. Which is available as [OrthancCPlugin.h](https://hg.orthanc-server.com/orthanc/file/Orthanc-1.9.7/OrthancServer/Plugins/Include/orthanc/OrthancCPlugin.h)

This repo has a copy of that file under [include/orthanc/](include/orthanc/)

##### Submodules
The submodules you need to initialize.
| Library | Purpose | URI |
|---------|---------|-----|
| [libpqxx](lib) | libpq wrapper | <ul><li>[external repo](https://github.com/jtv/libpqxx.git) <li>[docs - API](https://libpqxx.readthedocs.io/en/stable/a01382.html) |
| [nlohmann/json](lib) | json API | <ul><li>[external repo](https://github.com/nlohmann/json.git) <li>[docs - integration](https://github.com/nlohmann/json#integration) <li>[docs - API](https://nlohmann.github.io/json/api/basic_json/) |
| [googletest](lib) | unit testing | <ul><li>[external repo](https://github.com/google/googletest.git) |

</details>

### Building
The project is configured to build a plugin (dll/so) (target `'data-anonymizer'`) binary, then copy it to `docker/plugins` where the orthanc docker server can read it.

#### Tools
Some tools will be needed. Please refer to your system package manager, or each tool's website, in order to install.

<details>

* GCC - compiler
* Cmake compatible build system (eg. GNU Make, Ninja)
* Cmake 3.20 - configures build system
* Docker - local testing ([docker image](https://hub.docker.com/r/jodogne/orthanc-plugins))
* Github Actions - remote testing
* CLion (recommended)

</details>

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
 
#### Install
We don't currently provide binaries so you'll have to build and install yourself. We can't guarantee your plugin directory will be in this location, so verify its location for yourself and update the install prefix as needed.
```bash
$ mkdir build
$ cd build
$ cmake .. -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/share/orthanc/plugins
$ ninja install
```

### Docker

<details>

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

</details>

## Contributing

<details>

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

</details>

### Special Branches

<details>

| Name | Purpose |
|------|---------|
| master | stable branch |
| develop | development branch for **merging** new libraries/features/etc. |
| documents | documentation branch for independent clerical work |
| libraries | library integration branch for getting new libraries up and running |
| ci | continuous integration branch for independent updating of ci configs |
| samples | for independent updating of dicom files, should be converted into a submodule |

</details>

### Branching

<details>

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

</details>

# Plugin Configuration (json)
To configure this plugin you'll need to add a section to your json file with the key `"DataAnon" : {}` under this section different parts of the plugin can be configured. Here is an empty configuration showing the different sections. ***These are the parts of the configuration that must be present. If missing Orthanc will fail to start.***
```json
  "DataAnon": {
    "HardlinksUseHashBins" : true,
    "Hardlinks": { },
    "DateTruncation": {
      "default": "YYYYMMDD"
    },
    "Filter": {
      "blacklist": [ ],
      "whitelist": [ ]
    }
  },
  ```
**note:  changing the configuration does not (currently) affect existing DICOM files that have already been received and processed by the server.**

# Hard Links
Hard links are a special type of file on a filesystem that allow you to alias another file such that even if you rename the underlying file the hard link will still point to that file. In fact you could delete the underlying file and the hard link will take its place so to speak, which is to say the data is not lost because there is still a file referencing that spot on the storage medium. **note: not all filesystem types support hard links (eg. FAT32)**

For users of this data anonymization plugin, hard links allow us to provide a way to conveniently customize how data is organized on the filesystem. Hard links can be configured in the json using a `"/folder-name/" : "group,element"` pair.

**For example:**
Say you want to organize your DICOM files according to the patients' date of births. You would add `"/by-dob/": "0010,0030"` to the `"Hardlinks" : {}` section like so
```json
"Hardlinks" : {
   "/by-dob/": "0010,0030"
}
```
This will create a directory named `by-dob` in which dicom files will be grouped according to their date of birth. For a patient who's date of birth is recorded in an incoming DICOM file as `19880404` the relative path to that file from Orthanc's storage directory will be `by-dob/19880404/`.

You can add multiple hardlinks by separating them by commas. The directory you use can be anything, the plugin will ensure the directory is created if it does not exist, and you can find them under the Orthanc storage directory seen in the orthanc configuration json under `"StorageDirectory"` this too can be customized.

### Hash Bins
You may have more files than your file system is willing to manage under a particular group (eg. `by-dob/19880404/`) and so you can enable the use of hash bins which will use the first two characters of the DICOM file's name as part of the path. eg. `by-dob/19880404/4A/`; the file names are `uuid.DCM` where uuid is the file's rather long uuid. To enable or disable this feature you would set the `"HardlinksUseHashBins"` field accordingly (ie. `true` or `false`).

# Date Truncation
You may want to keep dates, but also anonymize them to a certain degree which you can accomplish via the date truncation feature. You can configure different dates to be truncated differently even. The date truncation code will take the configuration and simply mask the date with it.
**For example:**
```
date: 19880404
config: YYYY0101
result: 19880101
```
So in practice you could configure the date truncater to give you weird results by using a config such as `99YYM1DD`. This would result in the example date turning into `99880104` or `19881212` becoming `99881112`. Probably not what you'd want, so just be sure you set it correctly.

### Formatting
Dates in DICOM files should be in the format `YYYYMMDD` and our plugin uses this same format to configure date truncation.

### Default configuration
The date truncation requires a default truncation otherwise the plugin will not load. You can configure the truncation to do nothing by simply using `YYYYMMDD` (technically you could use any non-digits, but might as well make sense). To configure this you would just add `"default" : "YYYYMMDD"` to the `"DateTruncation": { }` section. The example at the top shows what this would look like.

### Truncating Different Dates Differently
Say you want all dates to be truncated by month, except date of birth which you want truncated by year. That is to say you only care about what year they were born, and everything else you care up to the month but not the day. To achieve this you would add an entry to the `"DateTruncation": { }` section using the DOB's `group,element` which would be `"0010,0030"` along with the truncation mask.. in the end you'd have the following.
```json
    "DateTruncation": {
      "default": "YYYYMM01",
      "0010,0030": "YYYY0101"
    },
```
1. **note: you can add a different mask for each and every type of date you can find in a DICOM file just add the appropriate tag (group,element) and the mask you want for each.**
2. **note: specific dates configured for truncation will be automatically be added to the Filter whitelist (inside the plugin, not the json).**

# Filter - Removing Dicom Elements
If you want to completely remove data, you can use the `"Filter" : { "blacklist" : [] }` section to achieve it.

### Blacklist
The blacklist can be configured in two ways, you can either remove an entire group of DICOM elements, or specific elements. To remove an entire group you simply use the group tag eg. `"0010"` and for a specific element it would be something like `"00FF,0010"`

### Whitelist
The whitelist can only be configured for specific DICOM elements because it is intended to counter-act blacklisting an entire group.
**note: dates specified under the `"DateTruncation": { }` section will automatically be added to the whitelist.**