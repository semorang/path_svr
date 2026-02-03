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

	std::set<uint32_t>* GetCourseByLink(IN const uint64_t linkId);
	std::set<uint64_t>* GetLinkByCourse(IN const uint32_t courseId);

	const std::unordered_map<uint32_t, std::set<uint64_t>>* GetLinkDataByCourse(void) const;
	const std::unordered_map<uint64_t, std::set<uint32_t>>* GetCourseDataByLink(void) const;
	
private:
	std::unordered_map<uint32_t, std::set<uint64_t>> mapLinkByCourse;
	std::unordered_map<uint64_t, std::set<uint32_t>> mapCourseByLink;
};

