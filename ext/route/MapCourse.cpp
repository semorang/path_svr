#if defined(_WIN32)
#include "../stdafx.h"
#endif

#include "MapCourse.h"
#include "../utils/UserLog.h"

#if defined(_WIN32) && defined(_DEBUG)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MapCourse::MapCourse()
{
}


MapCourse::~MapCourse()
{
	Release();
}


bool MapCourse::Initialize(void)
{
	return MapBase::Initialize();
}


void MapCourse::Release(void)
{
	if (!mapLinkByCourse.empty())
	{
		mapLinkByCourse.clear();
		unordered_map<uint32_t, set<uint64_t>>().swap(mapLinkByCourse);
	}

	if (!mapCourseByLink.empty())
	{
		mapCourseByLink.clear();
		unordered_map<uint64_t, set<uint32_t>>().swap(mapCourseByLink);
	}

	MapBase::Release();
}


bool MapCourse::AddCourseDataByLink(IN const uint64_t linkId, IN const uint32_t courseId)
{
	unordered_map<uint64_t, set<uint32_t>>::iterator it = mapCourseByLink.find(linkId);
	if (it != mapCourseByLink.end()) {
		it->second.emplace(courseId);
	}
	else {
		set<uint32_t> setCourse;
		setCourse.emplace(courseId);
		mapCourseByLink.emplace(linkId, setCourse);
	}

	return true;
}


bool MapCourse::AddLinkDataByCourse(IN const uint32_t courseId, IN const uint64_t linkId)
{
	unordered_map<uint32_t, set<uint64_t>>::iterator it = mapLinkByCourse.find(courseId);
	if (it != mapLinkByCourse.end()) {
		it->second.emplace(linkId);
	}
	else {
		set<uint64_t> setLink;
		setLink.emplace(linkId);
		mapLinkByCourse.emplace(courseId, setLink);
	}

	return true;
}


set<uint32_t>* MapCourse::GetCourseByLink(IN const uint64_t linkId)
{
	unordered_map<uint64_t, set<uint32_t>>::iterator it = mapCourseByLink.find(linkId);
	if (it != mapCourseByLink.end()) {
		return &it->second;
	}

	return nullptr;
}


set<uint64_t>* MapCourse::GetLinkByCourse(IN const uint32_t courseId)
{
	unordered_map<uint32_t, set<uint64_t>>::iterator it = mapLinkByCourse.find(courseId);
	if (it != mapLinkByCourse.end()) {
		return &it->second;
	}

	return nullptr;
}


const unordered_map<uint32_t, set<uint64_t>>* MapCourse::GetLinkDataByCourse(void) const
{
	return &mapLinkByCourse;
}


const unordered_map<uint64_t, set<uint32_t>>* MapCourse::GetCourseDataByLink(void) const
{
	return &mapCourseByLink;
}
