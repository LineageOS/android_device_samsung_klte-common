#include <string>
#include <iostream>
#include "SecNativeFeatureCppIf.h"
#include "SecNativeFeatureCIf.h"
#include <expat.h>
#include <stdlib.h>


int 
SecNativeFeature_getEnableStatus(const char* tag)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getEnableStatus(tag);
    }

    return 0;
}

int 
SecNativeFeature_getEnableStatusWithDefault(const char* tag, int defaultValue)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getEnableStatus(tag, defaultValue);
    }

    return defaultValue;
}

int 
SecNativeFeature_getInteger(const char* tag)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getInteger(tag);
    }

    return -1;
}

int 
SecNativeFeature_getIntegerWithDefault(const char* tag, int defaultValue)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getInteger(tag, defaultValue);
    }

    return defaultValue;
}

const char* 
SecNativeFeature_getString(const char* tag)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getString(tag);
    }

    return NULL;
}

const char* 
SecNativeFeature_getStringWithDefault(const char* tag, char* defaultValue)
{
    SecNativeFeature* instance = SecNativeFeature::getInstance();

    if (instance)
    {
        return instance->getString(tag, defaultValue);
    }

    return defaultValue;
}
