/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef THHOLIDAY_H_
#define THHOLIDAY_H_

#include "thfile.h"
#include "thtime.h"
#include <set>

namespace thlib {

#define MAX_HOLIDAY_YEAR_CNT  8
#define MAX_HOLIDAY_CNT       24
/**
 *  @brief  ���� ����
 */
typedef struct __HolidayTblHdr {
  int16_t   _first_year;      ///< ���Ͽ� ����ִ� ù��° �⵵
  int16_t   _year_cnt;        ///< �⵵ ����
} HolidayTblHdr;

typedef struct __HolidayInfo {
  uint16_t  _ds;
  uint16_t  _cnt;
} HolidayInfo;

typedef struct __Holiday {
  char    _month;
  char    _day;
} Holiday;



class thHolidayManager {
  /**
   *  @brief  Kind of Pattern
   */
  enum __PATTERN_TYPE {
    PTN_NORMAL = 0,         ///< ����
    PTN_FRIDAY_EFFECT = 1,  ///< ���� ����
    PTN_HOLIDAY = 2         ///< ����
  }; /// sti_maprecord.h�� ���ǵǾ� ������ include�� ���ϱ� ���� �ߺ���.

  static const char*        F_HOLIDAY;
  static thHolidayManager*  _instance;
  HolidayInfo               _info[MAX_HOLIDAY_YEAR_CNT];
  Holiday                   (*_data)[MAX_HOLIDAY_CNT];
  std::set<uint32_t>        _holiday_set;

  int                       _is_init;
  int                       _is_old;
  /// header�� ��������...
  int                       _first_year;
  int                       _year_cnt;

///////////////////////////////////////////////////////////////////////////////
 public:
  thHolidayManager();
  ~thHolidayManager();
 private :
  static void destroy_bridge();
  void  destroy();
  void  next_holiday(struct tm* tm_current, struct tm* tm_next);

 public :
  static thHolidayManager*  instance();
  int   init(const char* map_path);
  int   pattern(time_t timer);
  int   is_legal_holiday(struct tm* timer);
  int   is_legal_holiday(uint32_t time) {
    time = time + 32400;
    time = time - (time % 86400);
    if (_holiday_set.find(time) != _holiday_set.end()) return 1;
    else return 0;
  }

  void  holiday_year(int* first, int* cnt) {
    *first = _first_year;
    *cnt   = _year_cnt;
  }

  void  use_old_pattern(int val) {
    _is_old = val;
  }
};

}; // namespace thlib

#endif // THHOLIDAY_H_
