// CoordinateParser.h  (single-header, no std::optional, VS2015-safe)
// 목적:
//   - 어떤 텍스트든 WGS84 좌표를 최대한 robust하게 찾아서 (lng=x, lat=y)로 반환
//   - 지역(기본 한국) 옵션으로 오탐을 줄임
//
// 특징:
//   1) 키 기반 우선: lng/lat, lon/lat, longitude/latitude, x/y
//   2) N/S/E/W 표기 지원: "37.5N 127.0E", "E127.0 N37.5"
//   3) 도분초(DMS) 지원: 37°30'00"N 127°00'00"E (가능한 범위 내에서)
//   4) 일반 숫자 쌍 추출 fallback: "128.123 34.234", "{128.123,34.234}" 등
//   5) 후보가 여러 개면 스코어링으로 가장 그럴듯한 쌍 선택
//
// 사용 예:
//   #include "CoordinateParser.h"
//   SPoint pt;
//   if (getCoordinate(text, pt)) { ... }
//
//   ParseOptions opt; opt.preset = RegionPreset::World;
//   ParseResult r = ParseCoordinate(text, opt);
//
// 주의:
//   - “모든 텍스트”에서 숫자 2개는 흔하므로, RegionPreset::Korea(기본)로 오탐을 낮춤
//   - 정말 전세계/로그/버전 문자열이 섞이면 World는 오탐이 조금 늘 수 있음

#pragma once

#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <algorithm>

#ifndef COORDPARSER_NO_NAMESPACE
namespace coordparser
{
#endif

	// -------------------- Public Types --------------------
	enum class RegionPreset
	{
		Korea,   // default
		World,
		Custom
	};

	struct ParseOptions
	{
		RegionPreset preset = RegionPreset::Korea;
		SBox customBox;// = SBox();     // preset==Custom 일 때 사용
		bool allowSwap = true;        // lat/lng 뒤집힘 자동 교정
		bool enableKeyed = true;      // lng=lat= 같은 키 기반 추출
		bool enableDMS = false;// true;        // 도분초 지원
		bool enableCardinal = false;// true;   // N/S/E/W 표기 지원
	};

	struct ParseResult
	{
		bool ok = false;
		SPoint pt;
		size_t matchPos = (size_t)-1; // 원문에서 매칭 시작 위치(대략)
		size_t matchLen = 0;
		const char* reason = "";      // "keyed", "dms", "cardinal", "generic", "no candidates" 등
	};

	// -------------------- Internal Helpers --------------------

	static inline bool cp_isfinite(double v)
	{
#if defined(_MSC_VER)
		return _finite(v) != 0;
#else
		return std::isfinite(v);
#endif
	}

	static inline bool cp_islat(double v) { return cp_isfinite(v) && v >= -90.0 && v <= 90.0; }
	static inline bool cp_islng(double v) { return cp_isfinite(v) && v >= -180.0 && v <= 180.0; }

	static inline std::string cp_tolower(std::string s)
	{
		for (size_t i = 0; i < s.size(); ++i) {
			s[i] = (char)std::tolower((unsigned char)s[i]);
		}
		return s;
	}

	static inline bool cp_is_keychar(char c)
	{
		return std::isalnum((unsigned char)c) || c == '_' || c == '-';
	}

	static inline bool cp_is_numstart(char c)
	{
		return std::isdigit((unsigned char)c) || c == '+' || c == '-' || c == '.';
	}

	static inline SBox cp_preset_box(RegionPreset p, const SBox& custom)
	{
		if (p == RegionPreset::World) return SBox{ -180, -90, 180, 90 };
		if (p == RegionPreset::Custom) return custom;
		// Korea rough box with buffer
		return SBox{ 123.0, 31.0, 133.5, 39.8 };
	}

	static bool cp_parse_double_at(const std::string& s, size_t pos, size_t& outNext, double& outVal)
	{
		const char* p = s.c_str() + pos;
		char* next = nullptr;
		errno = 0;
		double v = std::strtod(p, &next);
		if (next == p) return false;
		if (errno == ERANGE) return false;

		outVal = v;
		outNext = (size_t)(next - s.c_str());
		return true;
	}

	struct cp_NumToken
	{
		double v;
		size_t pos;
		size_t len;
	};

	static std::vector<cp_NumToken> cp_extract_numbers_with_pos(const std::string& s)
	{
		std::vector<cp_NumToken> out;
		size_t i = 0;
		while (i < s.size()) {
			while (i < s.size() && !cp_is_numstart(s[i])) ++i;
			if (i >= s.size()) break;

			size_t next = i;
			double v = 0.0;
			if (!cp_parse_double_at(s, i, next, v)) {
				++i;
				continue;
			}
			out.push_back(cp_NumToken{ v, i, next - i });
			i = next;
		}
		return out;
	}

	// ---- keyed number (lng=..., lat=...) ----
	static bool cp_extract_keyed_number(const std::string& lower,
		const std::vector<std::string>& keys,
		size_t& outPos,
		double& outVal)
	{
		for (size_t ki = 0; ki < keys.size(); ++ki) {
			const std::string& k = keys[ki];
			size_t pos = lower.find(k);
			while (pos != std::string::npos) {
				// ensure boundary before key (reduce false hits: e.g. "belong" contains "lon")
				if (pos > 0 && cp_is_keychar(lower[pos - 1])) {
					pos = lower.find(k, pos + 1);
					continue;
				}

				size_t i = pos + k.size();
				// allow separators
				while (i < lower.size() && (lower[i] == ' ' || lower[i] == '\t' || lower[i] == '\r' || lower[i] == '\n'
					|| lower[i] == '=' || lower[i] == ':' || lower[i] == ',')) {
					++i;
				}

				size_t next = i;
				double v = 0.0;
				if (cp_parse_double_at(lower, i, next, v)) {
					outPos = pos;
					outVal = v;
					return true;
				}

				pos = lower.find(k, pos + 1);
			}
		}
		return false;
	}

	// ---- DMS helpers ----
	static inline bool cp_is_degsym(char c) { return c == '\xB0' || c == '°' || c == 'd' || c == 'D'; }
	static inline bool cp_is_minsym(char c) { return c == '\'' || c == 'm' || c == 'M'; }
	static inline bool cp_is_secsym(char c) { return c == '"' || c == 's' || c == 'S'; }

	// Parse something like: 37°30'00"N  (cardinal required)
	static bool cp_parse_dms_near(const std::string& s, size_t start, size_t& outEnd, char& outCard, double& outVal)
	{
		size_t i = start;

		auto skip = [&](size_t& k) {
			while (k < s.size()) {
				char c = s[k];
				if (std::isspace((unsigned char)c) || c == ',' || c == ';' || c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']')
					++k;
				else
					break;
			}
		};

		skip(i);
		if (i >= s.size()) return false;

		// degrees
		double deg = 0.0;
		size_t next = i;
		if (!cp_parse_double_at(s, next, i, deg)) return false;

		skip(i);
		if (i < s.size() && cp_is_degsym(s[i])) ++i;
		skip(i);

		// minutes (optional)
		double minutes = 0.0;
		bool hasMin = false;
		size_t saveI = i;
		next = i;
		if (cp_parse_double_at(s, next, i, minutes)) {
			hasMin = true;
			skip(i);
			if (i < s.size() && cp_is_minsym(s[i])) ++i;
			skip(i);
		} else {
			i = saveI;
		}

		// seconds (optional)
		double seconds = 0.0;
		bool hasSec = false;
		saveI = i;
		next = i;
		if (cp_parse_double_at(s, next, i, seconds)) {
			hasSec = true;
			skip(i);
			if (i < s.size() && cp_is_secsym(s[i])) ++i;
			skip(i);
		} else {
			i = saveI;
		}

		// cardinal required
		if (i >= s.size()) return false;
		char c = (char)std::toupper((unsigned char)s[i]);
		if (!(c == 'N' || c == 'S' || c == 'E' || c == 'W')) return false;
		outCard = c;
		++i;

		double val = std::fabs(deg) + (hasMin ? minutes / 60.0 : 0.0) + (hasSec ? seconds / 3600.0 : 0.0);
		if (c == 'S' || c == 'W') val = -val;

		outVal = val;
		outEnd = i;
		return true;
	}

	// ---- candidate scoring ----
	struct cp_Candidate
	{
		double lng, lat;
		size_t pos, len;
		double score;
		const char* why;
	};

	static double cp_score(double lng, double lat,
		const SBox& region,
		bool fromKeyed,
		bool fromCardinal,
		bool fromDMS,
		size_t tokenGap,
		bool swapped)
	{
		if (!cp_islng(lng) || !cp_islat(lat)) return -1e9;

		double s = 0.0;
		s += 10.0; // base valid

		if (region.contains(lng, lat)) s += 40.0;
		else s -= 10.0;

		if (fromKeyed)    s += 30.0;
		if (fromCardinal) s += 25.0;
		if (fromDMS)      s += 25.0;

		// 가까이 붙은 숫자쌍 선호
		s += std::max(0.0, 15.0 - (double)tokenGap * 3.0);

		if (swapped) s -= 2.0;
		return s;
	}

	// -------------------- Public API --------------------

	// 권장: 외부에서 직접 호출할 메인 함수
	static ParseResult ParseCoordinate(const std::string& text, const ParseOptions& opt = ParseOptions())
	{
		ParseResult res;
		if (text.empty()) { res.reason = "empty"; return res; }

		const SBox region = cp_preset_box(opt.preset, opt.customBox);
		const std::string lower = cp_tolower(text);

		std::vector<cp_Candidate> cands;

		// 1) keyed: lng/lat ...
		if (opt.enableKeyed) {
			size_t posLng = 0, posLat = 0;
			double lng = 0.0, lat = 0.0;

			bool hasLng = cp_extract_keyed_number(lower, { "lng","lon","long","longitude","x" }, posLng, lng);
			bool hasLat = cp_extract_keyed_number(lower, { "lat","latitude","y" }, posLat, lat);

			if (hasLng && hasLat) {
				bool swapped = false;
				if (!(cp_islng(lng) && cp_islat(lat)) && opt.allowSwap && cp_islng(lat) && cp_islat(lng)) {
					std::swap(lng, lat);
					swapped = true;
				}

				cp_Candidate c;
				c.lng = lng; c.lat = lat;
				c.pos = (posLng < posLat ? posLng : posLat);
				c.len = (posLng > posLat ? posLng : posLat) - c.pos + 1;
				c.score = cp_score(lng, lat, region, true, false, false, 0, swapped);
				c.why = "keyed";
				cands.push_back(c);
			}
		}

		// 2) DMS + cardinal
		if (opt.enableDMS) {
			for (size_t i = 0; i < text.size(); ++i) {
				size_t e1 = 0, e2 = 0;
				char c1 = 0, c2 = 0;
				double v1 = 0.0, v2 = 0.0;

				if (!cp_parse_dms_near(text, i, e1, c1, v1)) continue;
				if (!cp_parse_dms_near(text, e1, e2, c2, v2)) continue;

				bool ok = false;
				double lat = 0.0, lng = 0.0;

				if ((c1 == 'N' || c1 == 'S') && (c2 == 'E' || c2 == 'W')) { lat = v1; lng = v2; ok = true; }
				if ((c1 == 'E' || c1 == 'W') && (c2 == 'N' || c2 == 'S')) { lng = v1; lat = v2; ok = true; }

				if (ok) {
					cp_Candidate c;
					c.lng = lng; c.lat = lat;
					c.pos = i; c.len = (e2 > i ? (e2 - i) : 0);
					c.score = cp_score(lng, lat, region, false, true, true, 0, false);
					c.why = "dms";
					cands.push_back(c);
				}
			}
		}

		if (opt.enableCardinal) {
			// Simple "37.5N 127.0E" style
			std::vector<cp_NumToken> nums = cp_extract_numbers_with_pos(text);

			auto cardAround = [&](size_t pos, size_t len, char& outCard) -> bool {
				size_t a = (pos > 4) ? pos - 4 : 0;
				size_t b = std::min(text.size(), pos + len + 4);
				for (size_t k = a; k < b; ++k) {
					char c = (char)std::toupper((unsigned char)text[k]);
					if (c == 'N' || c == 'S' || c == 'E' || c == 'W') { outCard = c; return true; }
				}
				return false;
			};

			auto applySign = [&](double v, char card) -> double {
				double t = std::fabs(v);
				if (card == 'S' || card == 'W') t = -t;
				return t;
			};

			for (size_t i = 0; i + 1 < nums.size(); ++i) {
				char A = 0, B = 0;
				if (!cardAround(nums[i].pos, nums[i].len, A)) continue;
				if (!cardAround(nums[i + 1].pos, nums[i + 1].len, B)) continue;

				double a = nums[i].v;
				double b = nums[i + 1].v;

				bool ok = false;
				double lat = 0.0, lng = 0.0;

				if ((A == 'N' || A == 'S') && (B == 'E' || B == 'W')) { lat = applySign(a, A); lng = applySign(b, B); ok = true; } else if ((A == 'E' || A == 'W') && (B == 'N' || B == 'S')) { lng = applySign(a, A); lat = applySign(b, B); ok = true; }

				if (ok) {
					cp_Candidate c;
					c.lng = lng; c.lat = lat;
					c.pos = nums[i].pos;
					c.len = (nums[i + 1].pos + nums[i + 1].len) - nums[i].pos;
					c.score = cp_score(lng, lat, region, false, true, false, 0, false);
					c.why = "cardinal";
					cands.push_back(c);
				}
			}
		}

		// 3) generic numeric pairing
		{
			std::vector<cp_NumToken> nums = cp_extract_numbers_with_pos(text);
			for (size_t i = 0; i + 1 < nums.size(); ++i) {
				double a = nums[i].v;
				double b = nums[i + 1].v;

				// (lng,lat)
				if (cp_islng(a) && cp_islat(b)) {
					cp_Candidate c;
					c.lng = a; c.lat = b;
					c.pos = nums[i].pos;
					c.len = (nums[i + 1].pos + nums[i + 1].len) - nums[i].pos;
					c.score = cp_score(c.lng, c.lat, region, false, false, false, 1, false);
					c.why = "generic";
					cands.push_back(c);
				}
				// (lat,lng) swap
				if (opt.allowSwap && cp_islat(a) && cp_islng(b)) {
					cp_Candidate c;
					c.lng = b; c.lat = a;
					c.pos = nums[i].pos;
					c.len = (nums[i + 1].pos + nums[i + 1].len) - nums[i].pos;
					c.score = cp_score(c.lng, c.lat, region, false, false, false, 1, true);
					c.why = "generic";
					cands.push_back(c);
				}
			}
		}

		if (cands.empty()) {
			res.reason = "no candidates";
			return res;
		}

		// choose best
		size_t best = 0;
		for (size_t i = 1; i < cands.size(); ++i) {
			if (cands[i].score > cands[best].score) best = i;
		}

		if (cands[best].score < 0.0) {
			res.reason = "low confidence";
			return res;
		}

		res.ok = true;
		res.pt = SPoint{ cands[best].lng, cands[best].lat };
		res.matchPos = cands[best].pos;
		res.matchLen = cands[best].len;
		res.reason = cands[best].why;
		return res;
	}
#ifndef COORDPARSER_NO_NAMESPACE
} // namespace coordparser
#endif
