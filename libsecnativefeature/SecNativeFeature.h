#ifndef SEC_NATIVE_FEATURE_H
#define SEC_NATIVE_FEATURE_H

#include "SecNativeFeatureTagAll.h"

// define TAG for default value
/*
#define Str_NoTag               ""
#define Bool_NoTag              false
#define Int_NoTag               (0)

#define TAG_BOOLEAN_TEST_TRUE   "CscFeature_BooleanTestTrue"
#define TAG_BOOLEAN_TEST_FALSE  "CscFeature_BooleanTestFalse"
#define TAG_BOOLEAN_TEST_NULL   "CscFeature_BooleanTestNull"
#define TAG_STRING_TEST         "CscFeature_StringTest"
#define TAG_STRING_TEST_NULL    "CscFeature_StringTestNull"
#define TAG_INTEGER_TEST        "CscFeature_IntegerTest"
#define TAG_INTEGER_TEST_NULL   "CscFeature_IntegerTestNull"

#define TAG_FMRADIO_RTPLUS      "CscFeature_FMRadioRTPlus"
#define TAG_FMRADIO_SEASETTING  "CscFeature_FMRadioSEASetting"
*/

#ifdef __cplusplus
#include "SecNativeFeatureCppIf.h"
#else
#include "SecNativeFeatureCIf.h"
#endif

#endif // SEC_NATIVE_FEATURE_H

