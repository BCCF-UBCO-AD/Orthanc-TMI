#pragma once
#ifndef ORTHANC_TMI_LIBRARY_H
#define ORTHANC_TMI_LIBRARY_H
#include <ctype.h>

extern int32_t OrthancPluginInitialize(OrthancPluginContext* context); //This callback function is responsible for initializing the plugin. The context argument gives access to the API of Orthanc.
extern void OrthancPluginFinalize(); //This function is responsible for finalizing the plugin, releasing all the allocated resources.
extern const char* OrthancPluginGetName(); //This function must give a name to the plugin.
extern const char* OrthancPluginGetVersion(); //This function must provide the version of the plugin.

#endif //ORTHANC_TMI_LIBRARY_H
