# Requirements and Design Milestone

## High-level description

We are tasked with developing a plugin that extends the open-source Picture Archive and Communication System (PACS) server [Orthanc](https://www.orthanc-server.com/). The plugin should have the ability to extract and de-identify incoming DICOM files, generate traceable patient information, store the corresponding data and remove any intermediate raw PHI.

## User groups

### Medical researchers

Medical researchers are expected to be the main user group for this software. The anonymized DICOM files and patient data can be used by researchers to train future Artificial Intelligence (AI) projects. The plugin must be able to automatize work that was previously done manually so as to make it as easy as possible for researchers to query large datasets of medical records.

## Overview of data flow diagram

![](assets/DFD.png)

## Milestone requirements 

WIP

## Non-functional requirements & environmental constraints

WIP

## Tech stack

### Dependencies
First and foremost this project [creates an Orthanc plugin](https://book.orthanc-server.com/developers/creating-plugins.html#structure-of-the-plugins) in C++, so the Orthanc [plugin SDK](https://sdk.orthanc-server.com/index.html) is required. Which is available as [OrthancCPlugin.h](https://hg.orthanc-server.com/orthanc/file/Orthanc-1.9.7/OrthancServer/Plugins/Include/orthanc/OrthancCPlugin.h)
#### Tools
* GCC - compiler (C/C++ languages)
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

## Testing

We are using GoogleTest, Google’s C++ testing and mocking framework, to test our plugin's features.

We are planning on using the [CircleCI](circleci.com) platform to handle continuous testing. With the help of a robust and reliable CI pipeline, we will be able to test features before merging and find issues sooner.