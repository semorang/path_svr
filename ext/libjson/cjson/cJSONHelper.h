#pragma once

//#include "../../include/types.h"
#include "../../include/MapDef.h"

#if defined(USE_CJSON)

#include "cJSON.h"

// ---- helper: 안전한 숫자/문자 파싱 ----
static bool JsonGetInt32(const cJSON* obj, const char* key, int32_t& out)
{
	if (!obj || !key) return false;
	const cJSON* it = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(obj), key);
	if (!it) return false;

	if (cJSON_IsNumber(it)) {
		out = static_cast<int32_t>(it->valuedouble);
		return true;
	}
	if (cJSON_IsString(it) && it->valuestring) {
		out = static_cast<int32_t>(std::atoi(it->valuestring));
		return true;
	}
	return false;
}

static bool JsonGetUInt32(const cJSON* obj, const char* key, uint32_t& out)
{
	int32_t tmp = 0;
	if (!JsonGetInt32(obj, key, tmp)) return false;
	if (tmp < 0) tmp = 0;
	out = static_cast<uint32_t>(tmp);
	return true;
}

static bool JsonGetDouble(const cJSON* obj, const char* key, double& out)
{
	if (!obj || !key) return false;
	const cJSON* it = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(obj), key);
	if (!it) return false;

	if (cJSON_IsNumber(it)) {
		out = it->valuedouble;
		return true;
	}
	if (cJSON_IsString(it) && it->valuestring) {
		out = std::strtod(it->valuestring, nullptr);
		return true;
	}
	return false;
}

static bool JsonGetString(const cJSON* obj, const char* key, std::string& out)
{
	if (!obj || !key) return false;
	const cJSON* it = cJSON_GetObjectItemCaseSensitive(const_cast<cJSON*>(obj), key);
	if (!it) return false;

	if (cJSON_IsString(it) && it->valuestring) {
		out = it->valuestring;
		return true;
	}
	return false;
}

// arr == [x,y] 형태 좌표 파싱
static bool JsonParsePointArray(const cJSON* arr2, SPoint& out)
{
	if (!arr2 || !cJSON_IsArray(arr2) || cJSON_GetArraySize(arr2) != 2) return false;

	const cJSON* jx = cJSON_GetArrayItem(const_cast<cJSON*>(arr2), 0);
	const cJSON* jy = cJSON_GetArrayItem(const_cast<cJSON*>(arr2), 1);
	if (!jx || !jy) return false;

	double x = 0.0, y = 0.0;

	if (cJSON_IsNumber(jx)) x = jx->valuedouble;
	else if (cJSON_IsString(jx) && jx->valuestring) x = std::strtod(jx->valuestring, nullptr);
	else return false;

	if (cJSON_IsNumber(jy)) y = jy->valuedouble;
	else if (cJSON_IsString(jy) && jy->valuestring) y = std::strtod(jy->valuestring, nullptr);
	else return false;

	out.x = x;
	out.y = y;
	return true;
}


// str =>"origins", "destinations", ...
static int JsonParsePointDoubleArray(const cJSON* root, const char* str, std::vector<SPoint>& vtPosition, std::string* err = nullptr)
{
	if (!root || !cJSON_IsObject(root)) {
		if (err) *err = "Root JSON is null or not an object";
		return -1;
	}

	if (!str || strlen(str) <= 0) {
		if (err) *err = "Object String is null";
		return -1;
	}

	// get origin
	cJSON* origins = cJSON_GetObjectItemCaseSensitive(root, str);
	if (!origins || !cJSON_IsArray(origins)) {
		if (err) *err = std::string(str) + " is null";
		return -1;
	}

	const int cnt = cJSON_GetArraySize(origins);
	vtPosition.reserve(cnt);
	for (int i = 0; i < cnt; ++i) {
		cJSON* position = cJSON_GetArrayItem(origins, i);
		SPoint coord;
		if (JsonParsePointArray(position, coord)) {
			vtPosition.emplace_back(coord);
		}
	}

	return cnt;
}


// origins
static int JsonParseOriginArray(const cJSON* root, std::vector<stWaypoint>& vtOrigin, std::string* err = nullptr)
{
	if (!root || !cJSON_IsObject(root)) {
		if (err) *err = "Root JSON is null or not an object";
		return -1;
	}

	// get origins
	cJSON* origins = cJSON_GetObjectItemCaseSensitive(root, "origins");
	if (!origins || !cJSON_IsArray(origins)) {
		if (err) *err = std::string("origins is null or not an array");
		return -1;
	}

	const int cnt = cJSON_GetArraySize(origins);
	vtOrigin.reserve(cnt);

	for (int i = 0; i < cnt; ++i) {
		cJSON* item = cJSON_GetArrayItem(origins, i);
		if (!item || !cJSON_IsObject(item)) {
			continue;
		}

		stWaypoint origin;

		// position (SPoint)
		cJSON* pos = cJSON_GetObjectItemCaseSensitive(item, "position");
		if (pos && cJSON_IsArray(pos)) {
			SPoint coord;
			if (JsonParsePointArray(pos, coord)) {
				origin.position = coord;
			}
		}

		// layoverTime
		cJSON* layover = cJSON_GetObjectItemCaseSensitive(item, "layoverTime");
		if (cJSON_IsNumber(layover)) {
			origin.layoverTime = layover->valueint;
		}

		// cargoVolume
		cJSON* cargo = cJSON_GetObjectItemCaseSensitive(item, "cargoVolume");
		if (cJSON_IsNumber(cargo)) {
			origin.cargoVolume = cargo->valueint;
		}

		// weight
		cJSON* weight = cJSON_GetObjectItemCaseSensitive(item, "weight");
		if (cJSON_IsNumber(weight)) {
			origin.weight = weight->valueint;
		}

		// count
		cJSON* count = cJSON_GetObjectItemCaseSensitive(item, "count");
		if (cJSON_IsNumber(count)) {
			origin.count = count->valueint;
		}

		// size
		cJSON* size = cJSON_GetObjectItemCaseSensitive(item, "size");
		if (cJSON_IsNumber(size)) {
			origin.size = size->valueint;
		}

		vtOrigin.emplace_back(origin);
	}

	return cnt;
}


static bool JsonObjectReplaceOrAdd(cJSON* root, cJSON* obj, const char* str, std::string* err = nullptr, int precision = 10)
{
	if (!root || !cJSON_IsObject(root)) {
		if (err) *err = "Root JSON is null or not an object";
		return false;
	}

	if (!obj || cJSON_IsInvalid(obj)) {
		if (err) *err = "Object JSON is null or invalid";
		return false;
	}

	if (!str || strlen(str) <= 0) {
		if (err) *err = "Object String is null";
		return false;
	}

	// If "origins" exists -> replace, else -> add
	if (cJSON_HasObjectItem(root, str)) {
		// ReplaceItem takes ownership of newOrigins if success.
		if (!cJSON_ReplaceItemInObjectCaseSensitive(root, str, obj)) {
			if (err) *err = "cJSON_ReplaceItemInObjectCaseSensitive failed";
			cJSON_Delete(obj); // prevent leak on failure
			return false;
		}
	} else {
		// AddItem takes ownership
		cJSON_AddItemToObject(root, str, obj);
	}

	return true;
}


static cJSON* JsonBuildOriginsArrayObject(const std::vector<SPoint>& vtPosition, int precision = 10)
{
	cJSON* origins = cJSON_CreateArray();
	if (!origins) return nullptr;

	for (const auto& pt : vtPosition) {
		// 각 점을 표현할 배열 [x, y]
		cJSON* coord = cJSON_CreateArray();
		if (!coord) { cJSON_Delete(origins); return nullptr; }

		// 숫자 값 추가 (소수점 precision 적용)
		cJSON_AddItemToArray(coord, cJSON_CreateNumber(
			std::round(pt.x * std::pow(10, precision)) / std::pow(10, precision)));
		cJSON_AddItemToArray(coord, cJSON_CreateNumber(
			std::round(pt.y * std::pow(10, precision)) / std::pow(10, precision)));

		// 최상위 origins 배열에 추가
		cJSON_AddItemToArray(origins, coord);
	}

	return origins;
}

static bool JsonReplaceOrigins(cJSON* root, const std::vector<SPoint>& vtPosition, std::string* err = nullptr, int precision = 10)
{
	if (!root || !cJSON_IsObject(root)) {
		if (err) *err = "Root JSON is null or not an object";
		return false;
	}
	if (vtPosition.empty()) {
		if (err) *err = "vtNewOrigin is empty";
		return false;
	}

	cJSON* position = JsonBuildOriginsArrayObject(vtPosition, precision);
	if (!position) {
		if (err) *err = "Failed to build new origins array";
		return false;
	}

	// If "origins" exists -> replace, else -> add
	if (!JsonObjectReplaceOrAdd(root, position, "origins", err)) {
		return false;
	}

	return true;
}

#endif // #if defined(USE_CJSON)