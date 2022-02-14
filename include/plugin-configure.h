#pragma once
#include <core.h>
#include <dicom-anonymizer.h>

class PluginConfigurer {
private:
    static nlm::json config;
    static DicomAnonymizer filter;
protected:
public:
    static int Initialize();
    static DicomAnonymizer& GetDicomFilter() { return filter; }
    static std::string GetDateFormat(uint64_t tag_code = 0);
};