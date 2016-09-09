#include <string>
#include <iostream>
#include "SecNativeFeatureCppIf.h"
#include <expat.h>
#include <stdlib.h>

// feature file location (which should be "/system/csc/feature.xml")
#define FEATURE_FILE "/system/csc/feature.xml"
// feature file location (which should be "/system/csc/others.xml")
#define MPS_FEATURE_FILE "/system/csc/others.xml"

// XML parsing using expat lib - handlers
typedef struct{
	std::string curr_name;
	std::map<std::string,std::string> *pFeatures;
	int depth;
} ParserUserData;

static void XMLCALL
charDataHandler(void *userData, const char *s, int len){
	ParserUserData* pData = (ParserUserData*)userData;
	std::string value(s, len);
	std::string curr_name = pData->curr_name;

	if(!curr_name.empty()){
		std::map<std::string,std::string>::iterator result = (*pData->pFeatures).find(curr_name);

		if (result != (*pData->pFeatures).end()) {
			value = result->second + value;
			(*pData->pFeatures).erase(curr_name);
		}

		std::map<std::string,std::string>::iterator begin;
		begin = (*pData->pFeatures).begin();
		std::pair<std::string,std::string> feature(curr_name,value);
		(*pData->pFeatures).insert(begin, feature);
	}
	// else{
		// // printf("no name\n");
	// }
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts __unused)
{
	ParserUserData* pData = (ParserUserData*)userData;
	pData->curr_name.assign(name);
	pData->depth += 1;
}

static void XMLCALL
endElement(void *userData, const char *name __unused)
{
	ParserUserData* pData = (ParserUserData*)userData;
	pData->curr_name.clear();
	pData->depth -= 1;
}

// SecNativeFeture class implementation
SecNativeFeature* SecNativeFeature::_instance = NULL;
SecNativeFeature::SecNativeFeature() {
	int load_result = 0;
	_features.clear();
	load_result = _loadFeature();
	if (load_result == -1){
		// todo : handle _loadFeature errors here.
	}
}

SecNativeFeature::~SecNativeFeature() {
	delete _instance;
}
SecNativeFeature* SecNativeFeature::getInstance() {
	if(_instance == NULL) {
		_instance = new SecNativeFeature();
	}
	return _instance;
}

bool SecNativeFeature::getEnableStatus(const char* tag) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);
	
	if(found == _features.end()) {
		return false;
	}

	if(found->second.compare("true") == 0 || found->second.compare("TRUE") == 0) {
		return true;
	}
	return false;
}

bool SecNativeFeature::getEnableStatus(const char* tag, bool defaultValue) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);
	
	if(found == _features.end()) {
		return defaultValue;
	}

	if(found->second.compare("true") == 0 || found->second.compare("TRUE") == 0) {
		return true;
	}
	return defaultValue;
}

const char* SecNativeFeature::getString(const char* tag) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);

	if(found == _features.end()) {
		return "";
	}
	return found->second.c_str();
}

const char* SecNativeFeature::getString(const char* tag, char* defaultValue) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);

	if(found == _features.end()) {
		return defaultValue;
	}
	return found->second.c_str();
}

int SecNativeFeature::getInteger(const char* tag) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);
	
	if(found == _features.end()) {
		return -1;
	}
	std::string raw_value = _features.find(tag)->second;
	return atoi(raw_value.c_str());
}

int SecNativeFeature::getInteger(const char* tag, int defaultValue) {
	std::map<std::string,std::string>::iterator found;
	found = _features.find(tag);
	
	if(found == _features.end()) {
		return defaultValue;
	}
	std::string raw_value = _features.find(tag)->second;
	return atoi(raw_value.c_str());
}

int SecNativeFeature::_loadFeature(){
	char buf[BUFSIZ];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	FILE * pFeatureFile = NULL;
	ParserUserData userData;
	userData.curr_name = std::string ();
	userData.pFeatures = &_features;
	userData.depth = 0;
	
	pFeatureFile = fopen(FEATURE_FILE, "r");
	if (pFeatureFile == NULL) {
		pFeatureFile = fopen(MPS_FEATURE_FILE, "r");
		if (pFeatureFile == NULL) {
			return -1;
		}
	}
	XML_SetUserData(parser, &userData);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, charDataHandler);
	do {
		size_t len = fread(buf, 1, sizeof(buf), pFeatureFile);
		if ((len != sizeof(buf)) && (ferror(pFeatureFile))){
			fclose(pFeatureFile);
			return -1;
		}
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			if(pFeatureFile) {
				fclose(pFeatureFile);
			}
			return -1;
		}
	} while (!done);
	XML_ParserFree(parser);
	fclose(pFeatureFile);
	return 0;
}
