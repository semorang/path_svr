/* Copyright 2011 <happyteam@thinkwaresys.com> */
/** 
 * STL�� time.h�� ���� include�Ѵٰ� �����ؾ� �Ѵ�. 
 * STL�� ���ٸ� ���̾�. 
 */
#ifndef _THTIMEDEF_
#define _THTIMEDEF_


#if defined(_WIN32_WCE)
  #include "wce_time.h"
#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */
  #undef _INC_TIME
  #include <time.h>
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
  #undef _INC_TIME
  #include <time.h>
#endif


#include "thconfig.h"
#include "thstring.h"


namespace thlib {


class thTime {
 public:
  static void sec2time(uint32_t t, uint32_t& h, uint32_t& m, uint32_t& s) {
    h = t / 3600;
    t -= (h * 3600);
    m = t / 60;
    t -= (m * 60);
    s = t;
  }

  /**
   * ���μ��� �ð����� ������ clock���� ���ϰ�, 
   * second�� ȯ���Ѵ�. 
   * @return ���� 1000���� 1�η� �����. 
   */
  static unsigned int get_ticcount() {
#if defined(WIN32) || defined(_WIN32_WCE)
    return GetTickCount();
#else
    if (CLOCKS_PER_SEC == 1000) {
      return clock();
    } else {
      unsigned int ret_clock = clock();
      ret_clock /= (CLOCKS_PER_SEC / 1000);
      return ret_clock;
    }
#endif
  }

  /* interface ���� 
  static time_t thtime(time_t* timer) {
#if defined(_WIN32_WCE)
    return wceex_time(timer);
#else
    return time(timer);
#endif
  }

  static struct tm* thlocaltime(const time_t* timer) {
#if defined(_WIN32_WCE)
    return wceex_localtime(timer);
#else
    return localtime(timer);
#endif
  }
  */

  /**
   * diff second�� ���Ѵ�. 
   * utctime1 - utctime0 ���Ѵ�.
   * �ܿ��⼭ ������ ���� ��� ����� �������� �����Ѵ�.
   */
  //static int get_diff_second(const uint32_t utctime1, 
  //              const uint32_t utctime0 = 0) {
  //  time_t basetime = utctime0; /// �Էµ� �ð�����.
  //  if (utctime0 == 0) {    /// ���� �ð�����.
  //    basetime = time(NULL);
  //  } 
  //  
  //  /// �ʴ��� diff���� �����Ѵ�. 
  //  double diff_time = difftime(utctime1, basetime);
  //  return static_cast<int>(diff_time);
  //}
  
  /**
   * diff minute�� ���Ѵ�.
   */
  //static int get_diff_minute(const uint32_t utctime1,
  //              const uint32_t utctime0 = 0) {
  //  int diff_sec = get_diff_second(utctime1, utctime0);
  //  return static_cast<int>(diff_sec/60.) + 1;
  //}

  /**
   * YYYY-MM-DD hh:mm:ss ������ time���� 
   */
  //static void get_standard_datestr(const uint32_t utctime,
  //                thString& timestr) {
  //  struct tm* curtm = NULL;
  //  time_t input_time = time(NULL);  /// ���� �ð�����.
  //  if (utctime != 0) {                /// �Էµ� �ð�����.    
  //    input_time = utctime;
  //  }
  //  curtm = localtime(&input_time);

  //  if (curtm == NULL)    return;
  //  
  //  timestr.printf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", 
  //    curtm->tm_year+1900, curtm->tm_mon + 1, 
  //    curtm->tm_mday, curtm->tm_hour,
  //    curtm->tm_min, curtm->tm_sec);
  //}
  /**
   * TIME ������ ���ϱ�.
   */
  //static void get_time_value(const uint32_t utctime, 
  //              int16_t& year, int16_t& month, int16_t& day, 
  //              int16_t& hour, int16_t& minute, int16_t& sec) {
  //  year = month = day = 0;
  //  hour = minute = sec = 0;
  //  struct tm* curtm = NULL;
  //  time_t input_time = time(NULL); /// ���� �ð�����.
  //  if (utctime != 0) {    
  //    input_time = static_cast<time_t>(utctime); /// �Էµ� �ð�����.
  //  } 
  //  curtm = localtime(&input_time);

  //  year = curtm->tm_year+1900;
  //  month = curtm->tm_mon + 1;
  //  day = curtm->tm_mday;
  //  hour = curtm->tm_hour;
  //  minute = curtm->tm_min;
  //  sec = curtm->tm_sec;
  //}

  //static int nextfixtime(time_t& next, int interval) {
  //  time_t curtt = time(NULL);
  //  next = curtt + interval;
  //  return THE_SUCCESS;
  //}

 public:
};
};  //  end of namespace



#endif // end _THTIMEDEF_
