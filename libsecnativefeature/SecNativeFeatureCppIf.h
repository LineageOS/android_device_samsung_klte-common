#ifndef SEC_NATIVE_FEATURE_CPPIF_H
#define SEC_NATIVE_FEATURE_CPPIF_H

#include <string>
#include <map>

class SecNativeFeature {
public:
    bool getEnableStatus(const char* tag);
    bool getEnableStatus(const char* tag, bool defaultValue);
    const char* getString(const char* tag);
    const char* getString(const char* tag, char* defaultValue);
    int getInteger(const char* tag);
    int getInteger(const char* tag, int defaultValue);

    static SecNativeFeature* getInstance();
private:
    static SecNativeFeature* _instance;
    SecNativeFeature();
    ~SecNativeFeature();
    int _loadFeature();
    int _loadDefault();
    std::map<std::string,std::string> _features;
};

#endif // SEC_NATIVE_FEATURE_CPPIF_H

