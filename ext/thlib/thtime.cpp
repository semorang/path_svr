/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thtime.h"

#if defined(_WIN32_WCE)

/******************************************************************************
  Constants and macros used internally
******************************************************************************/
/////////////////////////////////////////////////
// localtime.c
/////////////////////////////////////////////////
#define SECS_PER_MIN  60
#define MINS_PER_HOUR  60
#define HOURS_PER_DAY  24
#define DAYS_PER_WEEK  7
#define DAYS_PER_NYEAR  365
#define DAYS_PER_LYEAR  366
#define SECS_PER_HOUR  (SECS_PER_MIN * MINS_PER_HOUR)
#define SECS_PER_DAY  ((int32_t) SECS_PER_HOUR * HOURS_PER_DAY)
#define MONS_PER_YEAR  12

#define TM_SUNDAY  0
#define TM_MONDAY  1
#define TM_TUESDAY  2
#define TM_WEDNESDAY  3
#define TM_THURSDAY  4
#define TM_FRIDAY  5
#define TM_SATURDAY  6

#define TM_JANUARY  0
#define TM_FEBRUARY  1
#define TM_MARCH  2
#define TM_APRIL  3
#define TM_MAY    4
#define TM_JUNE    5
#define TM_JULY    6
#define TM_AUGUST  7
#define TM_SEPTEMBER  8
#define TM_OCTOBER  9
#define TM_NOVEBER  10
#define TM_DECEMBER  11
#define TM_SUNDAY  0

#define TM_YEAR_BASE  1900

#define EPOCH_YEAR  1970
#define EPOCH_WDAY  TM_THURSDAY

#define isleap(y) (((y) % 4) == 0 && ((y) % 100) != 0 || ((y) % 400) == 0)
/////////////////////////////
static int mon_lengths[2][MONS_PER_YEAR] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
static const int year_lengths[2] = { DAYS_PER_NYEAR, DAYS_PER_LYEAR };

/////////////////////////////////////////////////
// mktime.c
/////////////////////////////////////////////////
#define MONTHS_NUMBER 12
static const int MONTHDAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30,
                                                            31, 30, 31 };





/******************************************************************************
  internal functions
******************************************************************************/
/////////////////////////////////////////////////
// localtime.c
/////////////////////////////////////////////////
static struct tm * __wceex_offtime(const time_t *timer, int32_t tzoffset) {
    register struct tm *tmp;
    register int32_t    days;
    register int32_t    rem;
    register int    y;
    register int    yleap;
    register int       *ip;
    static struct tm    tm;

    tmp = &tm;
    days = *timer / SECS_PER_DAY;
    rem = *timer % SECS_PER_DAY;
    rem += tzoffset;
    while (rem < 0) {
        rem += SECS_PER_DAY;
        --days;
    }
    
    while (rem >= SECS_PER_DAY) {
        rem -= SECS_PER_DAY;
        ++days;
    }
    tmp->tm_hour = (int) (rem / SECS_PER_HOUR);

    rem = rem % SECS_PER_HOUR;
    tmp->tm_min = (int) (rem / SECS_PER_MIN);
    tmp->tm_sec = (int) (rem % SECS_PER_MIN);
    tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYS_PER_WEEK);
    
    if (tmp->tm_wday < 0)
        tmp->tm_wday += DAYS_PER_WEEK;
    
    y = EPOCH_YEAR;
    
    if (days >= 0) {
        for ( ; ; ) {
            yleap = isleap(y);
            if (days < (int32_t) year_lengths[yleap])
                break;

            ++y;
            days = days - (int32_t) year_lengths[yleap];
        }
    } else {
        do {
            --y;
            yleap = isleap(y);
            days = days + (int32_t) year_lengths[yleap];
        } while (days < 0);
    }

    tmp->tm_year = y - TM_YEAR_BASE;
    tmp->tm_yday = (int) days;
    ip = mon_lengths[yleap];

    for (tmp->tm_mon = 0; days >= (int32_t) ip[tmp->tm_mon]; ++(tmp->tm_mon)) {
        days = days - (int32_t) ip[tmp->tm_mon];
    }

    tmp->tm_mday = (int) (days + 1);
    tmp->tm_isdst = 0;

    return tmp;
}

/////////////////////////////////////////////////
// time.c
/////////////////////////////////////////////////
static time_t __wceex_tm_to_time_t(const struct tm *tmbuff) {
    time_t timer;

    /* If the year is <1970 or the value is negative, 
       the relationship is undefined */ 
    if (tmbuff->tm_year < 70) {
        return (time_t)-1;
    }

    /* If the year is >=1970 */
    /* Each and every day shall be accounted for by exactly 86400 seconds */

    timer = tmbuff->tm_sec
        + tmbuff->tm_min * 60         /* convert minutes to seconds */
        + tmbuff->tm_hour * 3600      /* convert hours to seconds */
        + tmbuff->tm_yday * 86400     /* convert day of year to seconds */
        + (tmbuff->tm_year - 70) * 31536000       /* convert year to seconds */
        /* add a day (seconds) every 4 years starting in 1973 */
        + ((tmbuff->tm_year - 69) / 4) * 86400
        /* subtract a day back out every 100 years starting in 2001 */
        - ((tmbuff->tm_year - 1) / 100) * 86400 
        /* add a day back in every 400 years starting in 2001 */
        + ((tmbuff->tm_year + 299) / 400) * 86400;

    return timer;
}

/////////////////////////////////////////////////
// mktime.c
/////////////////////////////////////////////////
static time_t __wceex_mktime_internal(struct tm *tmbuff, 
                                                    time_t _loctime_offset) {
    time_t tres;
    int doy;
    int i;

    /* We do allow some ill-formed dates, but we don't do anything special
    with them and our callers really shouldn't pass them to us.  Do
    explicitly disallow the ones that would cause invalid array accesses
    or other algorithm problems. */
    if (tmbuff->tm_mon < 0 || tmbuff->tm_mon > 11 || 
                         tmbuff->tm_year < (EPOCH_YEAR - TM_YEAR_BASE)) {
        return (time_t) -1;
    }

    /* Convert calender time to a time_t value. */
    tres = 0;

    /* Sum total amount of days from the Epoch with respect to leap years. */
    for (i = EPOCH_YEAR; i < tmbuff->tm_year + TM_YEAR_BASE; i++) {
        tres += 365 + IS_LEAP_YEAR(i);
    }

    /* Add days of months before current month. */
    doy = 0;
    for (i = 0; i < tmbuff->tm_mon; i++) {
        doy += MONTHDAYS[i];
    }
    tres += doy;
    
    /* Day of year */
    tmbuff->tm_yday = doy + tmbuff->tm_mday;

    if (tmbuff->tm_mon > 1 && IS_LEAP_YEAR(tmbuff->tm_year + TM_YEAR_BASE)) {
        tres++;
    }
    
    /* Add days of current month and convert to total to hours. */
    tres = 24 * (tres + tmbuff->tm_mday - 1) + tmbuff->tm_hour;

    /* Add minutes part and convert total to minutes. */
    tres = 60 * tres + tmbuff->tm_min;

    /* Add seconds part and convert total to seconds. */
    tres = 60 * tres + tmbuff->tm_sec;
    
    /* For offset > 0 adjust time value for timezone
    given as local to UTC time difference in seconds). */
    tres += _loctime_offset;
    
    return tres;
}





/******************************************************************************
  public functions
******************************************************************************/
/////////////////////////////////////////////////
// localtime.c
/////////////////////////////////////////////////
/**
 *  localtime
 */
struct tm* localtime(const time_t* timer) {
  register struct tm *tmp;
  int32_t tzoffset = 0;
  TIME_ZONE_INFORMATION tzi;
    
  if (GetTimeZoneInformation(&tzi) != 0xFFFFFFFF) {
    tzoffset += (tzi.Bias * 60);
    if (tzi.StandardDate.wMonth != 0) {
      tzoffset += (tzi.StandardBias * 60);
    }
  }

  tzoffset *= -1;
  tmp = __wceex_offtime(timer, tzoffset);

  return tmp;
}
/**
 *  gmtime
 */
struct tm* gmtime(const time_t* timer) {
  register struct tm* tmp;
  tmp = __wceex_offtime(timer, 0L);
  return tmp;
}

/////////////////////////////////////////////////
// time.c
/////////////////////////////////////////////////
/**
 *  time
 */
time_t time(time_t* timer) {
  time_t t;
  struct tm tmbuff;
  SYSTEMTIME st;

  /* Retrive current system date time as UTC */
  GetSystemTime(&st);

  /* Build tm struct based on SYSTEMTIME values */

  /* Date values */
  tmbuff.tm_year = st.wYear - TM_YEAR_BASE;
  tmbuff.tm_mon = st.wMonth - 1;      /* wMonth value 1-12 */
  tmbuff.tm_mday = st.wDay;

  /* Time values */
  tmbuff.tm_hour  = st.wHour;
  tmbuff.tm_min   = st.wMinute;
  tmbuff.tm_sec   = st.wSecond;
  tmbuff.tm_isdst = 0;    /* Always 0 for UTC time. */
  tmbuff.tm_wday  = st.wDayOfWeek;
  tmbuff.tm_yday  = 0;     /* Value is set by gmmktime */

  /* Convert tm struct to time_tUTC */
  t = gmmktime(&tmbuff);

  /* Assign time value. */
  if (timer != NULL) {
      *timer = t;
  }

  return t;
}

/////////////////////////////////////////////////
// mktime.c
/////////////////////////////////////////////////
/**
 *  mktime
 */
time_t mktime(struct tm* tmbuff) {
  time_t offset = 0;
  TIME_ZONE_INFORMATION tzi;

  // Retrive timezone offset in seconds
  if (GetTimeZoneInformation(&tzi) != 0xFFFFFFFF) {
    offset += (tzi.Bias * 60);
    if (tzi.StandardDate.wMonth != 0) {
      offset += (tzi.StandardBias * 60);
    }
  }

  return __wceex_mktime_internal(tmbuff, offset);
}
/**
 *  gmmktime
 */
time_t gmmktime(struct tm* tmbuff) {
  return __wceex_mktime_internal(tmbuff, 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif


