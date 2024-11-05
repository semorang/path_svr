/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thholiday.h"

namespace thlib {

thHolidayManager* thHolidayManager::_instance = NULL;
const char* thHolidayManager::F_HOLIDAY  = "mrdata/holiday.bin";


thHolidayManager::thHolidayManager() : _data(NULL), 
                                       _is_init(0), _is_old(0),
                                       _first_year(0), _year_cnt(0) {
}

thHolidayManager::~thHolidayManager() {
  destroy();
}

thHolidayManager* thHolidayManager::instance() {
  if (_instance == NULL) {
    _instance = thlib::MemPolicyNew<thHolidayManager>::alloc(1);
    atexit(destroy_bridge);
    return _instance;
  }
  return _instance;
}

void thHolidayManager::destroy_bridge() {
  if (thHolidayManager::_instance != NULL) {
    thHolidayManager::instance()->destroy();
    thlib::MemPolicyNew<thHolidayManager>::memfree(thHolidayManager::_instance);
    thHolidayManager::_instance = NULL;
  }
}

void thHolidayManager::destroy() {
  if (_data != NULL) {
    thlib::MemPolicy<Holiday>::memfree(_data);
    _data = NULL;
  }
}

int thHolidayManager::init(const char* map_path) {
  if (_is_init)
    return 0;

  size_t size = 0;
  thlib::thString path;
  thFile<>        holiday_file;
  path.printf("%s/%s", map_path, F_HOLIDAY);
  if (holiday_file.open(path, "rb") < 0)
    return -EFILE_OPEN;

  /// read header
  HolidayTblHdr  header;
  memset(&header, 0x00, sizeof(header));
  if (holiday_file.read(&header, sizeof(header)) != sizeof(header)) {
    holiday_file.close();
    return -EFILE_READ;
  }
  _first_year = header._first_year;
  _year_cnt   = header._year_cnt;

  memset(&_info, 0x00, sizeof(_info));
  size = sizeof(HolidayInfo) * _year_cnt;
  /// read info(DS & Count)
  if (holiday_file.read(&_info, size) != size) {
    holiday_file.close();
    return -EFILE_READ;
  }

  _data = (Holiday (*)[MAX_HOLIDAY_CNT])thlib::MemPolicy<Holiday>::alloc_zero(
          _year_cnt * MAX_HOLIDAY_CNT);
  if (_data == NULL) {
    return -EMEM_ALLOC;
  }
  for (int i = 0; i < _year_cnt; ++i) {
    size = sizeof(Holiday) * _info[i]._cnt;
    if (holiday_file.read((char*)_data[i], _info[i]._ds, size) != size) {
      holiday_file.close();
      return -EFILE_READ;
    }
  }

  struct tm stm;
  for (int i = 0; i < _year_cnt; ++i) {
    for (int j = 0; j < MAX_HOLIDAY_CNT; ++j) {
      if (_data[i][j]._month == 0) break;

      memset(&stm, 0x00, sizeof(stm));
      stm.tm_year = (header._first_year + i) - 1900;
      stm.tm_mon  = _data[i][j]._month - 1;
      stm.tm_mday = _data[i][j]._day;
      stm.tm_hour = 9;
      _holiday_set.insert(mktime(&stm));
    }
  }

  _is_init = 0x01;

  return 0;
}

/**
 *  @brief  입력 받은 날짜/시간을 기준으로 패턴을 구한다.
 *  @note   ret : 0이하는 error
 */
int thHolidayManager::pattern(time_t timer) {
  /// value table for performance
  int is_holiday[] = {1, 0, 0, 0, 0, 0, _is_old}; ///< 일요일~토요일
  int week_day;
  struct tm  tm_cur;
  struct tm  tm_next_holiday;

#if defined(WIN32)
  localtime_s(&tm_cur, &timer);
#elif defined(_WIN32_WCE)
#else
  localtime_r(&timer, &tm_cur);
#endif
  return tm_cur.tm_wday;
  /// 토, 일이면 그냥 휴일 패턴
  week_day = tm_cur.tm_wday;
  if (is_holiday[week_day]) {
    return PTN_HOLIDAY;
  }

  /// 현재 날짜에 가장 가까운 휴일을 얻어온다.
  next_holiday(&tm_cur, &tm_next_holiday);

  time_t timer_current = mktime(&tm_cur);
  time_t timer_next    = mktime(&tm_next_holiday);
  size_t time_diff = static_cast<size_t>(difftime(timer_next, timer_current));

  if (_is_old) {
    /// 휴일과의 시간차이를 비교
    if (time_diff >= 172800) { ///< 2day 이상
      if (week_day == 5) return PTN_FRIDAY_EFFECT; ///< 금요일
      else               return PTN_NORMAL;
    } else if (time_diff == 86400) { //< 1day
      return PTN_FRIDAY_EFFECT;
    } else { // 휴일과 일치
      return PTN_HOLIDAY;
    }
  } else {
    if (time_diff < 86400) {
      return PTN_HOLIDAY;
    } else {
      if (week_day == 6) {
        return PTN_FRIDAY_EFFECT;
      } else {
        return PTN_NORMAL;
      }
    }    
  }

  return -0xfe;
}

/**
 *  @brief  현재날짜에 가장 가까운 다음 공휴일을 찾는다.
 *          리턴값에 월과 일만 입력해 넣는다.
 */
void thHolidayManager::next_holiday(struct tm* tm_current, struct tm* tm_next) {
  int cur_month = tm_current->tm_mon + 1;
  int cur_day   = tm_current->tm_mday;
  int idx = 0;
  int year_idx  = tm_current->tm_year + 1900 - _first_year;

  if (year_idx > _year_cnt) {
    year_idx = _year_cnt - 1;
  }

  /// 일단 달부터 찾고...
  for (; idx < _info[year_idx]._cnt; ++idx) {
    if (cur_month > _data[year_idx][idx]._month) {
      continue;
    } else if (cur_month == _data[year_idx][idx]._month) {
      /// 일을 찾는다.
      if (cur_day <= _data[year_idx][idx]._day)
        break;
    } else {
      break;
    }
  }

  memset(tm_next, 0x00, sizeof(struct tm));
  tm_next->tm_year  = tm_current->tm_year;
  if (idx < _info[year_idx]._cnt) {
    tm_next->tm_mon  = _data[year_idx][idx]._month - 1;
    tm_next->tm_mday = _data[year_idx][idx]._day;
  } else {
    /// index가 끝까지 갔다믄 다음해 1/1이 next holiday다.
    ++(tm_next->tm_year);
    tm_next->tm_mon  = 0;
    tm_next->tm_mday = 1;
  }
}

/*
 *  @brief  주어진 날짜의 휴일 여부
 *  @note   ret < 0 : error
 *          ret = 0 : not holiday
 *          ret = 1 : holiday
 */
int thHolidayManager::is_legal_holiday(struct tm* timer) {
  int year_idx = timer->tm_year + 1900 - _first_year;
  if (year_idx >= _year_cnt) {
    year_idx = _year_cnt - 1;
  }

  /// 일단 달부터 찾고...
  for (int idx = 0; idx < _info[year_idx]._cnt; ++idx) {
    if (timer->tm_mon > _data[year_idx][idx]._month - 1) {
      continue;
    } else if (timer->tm_mon == _data[year_idx][idx]._month - 1) {
      /// 일을 찾는다.
      if (timer->tm_mday == _data[year_idx][idx]._day)
        return 1;
    } else {
      break;
    }
  }

  return 0;
}

}; // end of namespace
