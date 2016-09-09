#ifndef SEC_NATIVE_FEATURE_CIF_H
#define SEC_NATIVE_FEATURE_CIF_H

#ifdef __cplusplus
#define DECLARE_BEGIN_C      extern "C" {
#define DECLARE_END_C        }
#else
#define DECLARE_BEGIN_C
#define DECLARE_END_C
#endif

DECLARE_BEGIN_C

int         SecNativeFeature_getEnableStatus(const char* tag);
int         SecNativeFeature_getEnableStatusWithDefault(const char* tag, int defaultValue);
int         SecNativeFeature_getInteger(const char* tag);
int         SecNativeFeature_getIntegerWithDefault(const char* tag, int defaultValue);
const char* SecNativeFeature_getString(const char* tag);
const char* SecNativeFeature_getStringWithDefault(const char* tag, char* defaultValue);

DECLARE_END_C

#endif // SEC_NATIVE_FEATURE_CIF_H

