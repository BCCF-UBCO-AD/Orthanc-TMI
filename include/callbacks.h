#pragma once
#include "core.h"

// incoming callback prototype
extern int32_t FilterCallback(const OrthancPluginDicomInstance *instance);

// storage callback prototypes
extern OrthancPluginErrorCode StorageCreateCallback(const char *uuid,
                                                    const void *content,
                                                    int64_t size,
                                                    OrthancPluginContentType type);
extern OrthancPluginErrorCode StorageReadWholeCallback(OrthancPluginMemoryBuffer64 *target,
                                                       const char *uuid,
                                                       OrthancPluginContentType type);
extern OrthancPluginErrorCode StorageReadRangeCallback(OrthancPluginMemoryBuffer64 *target,
                                                       const char *uuid,
                                                       OrthancPluginContentType type,
                                                       uint64_t rangeStart);
extern OrthancPluginErrorCode StorageRemoveCallback(const char *uuid, OrthancPluginContentType type);

// after storage callback prototype
extern OrthancPluginErrorCode OnStoredInstanceCallback(const OrthancPluginDicomInstance *instance,
                                                       const char *instanceId);