#pragma once

#include "MapBase.h"

class MapCourse : public MapBase
{
public:
	MapCourse();
	~MapCourse();

	virtual bool Initialize(void);
	virtual void Release(void);

	bool AddCourseDataByLink(IN const uint64_t linkId, IN const uint32_t courseId);
	bool AddLinkDataByCourse(IN const uint32_t courseId, IN const uint64_t linkId);

	set<uint32_t>* GetCourseByLink(IN const uint64_t linkId);
	set<uint64_t>* GetLinkByCourse(IN const uint32_t courseId);

	const unordered_map<uint32_t, set<uint64_t>>* GetLinkDataByCourse(void) const;
	const unordered_map<uint64_t, set<uint32_t>>* GetCourseDataByLink(void) const;
	
private:
	unordered_map<uint32_t, set<uint64_t>> mapLinkByCourse;
	unordered_map<uint64_t, set<uint32_t>> mapCourseByLink;
};

