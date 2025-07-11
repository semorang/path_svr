#pragma once

#include "../include/MapDef.h"
#include "../include/types.h"

// base64
int32_t base64fromBinary(IN const char* srcBytes, IN const int numBytes, OUT char** szBase64);
int32_t base64toBinary(IN const char* szBase64, IN const int numBytes, OUT unsigned char* dstBytes);

SPoint getCoordFromText(const char* pszCoord);