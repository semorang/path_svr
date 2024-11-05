/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THMEMSTATUS_
#define _THMEMSTATUS_



#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등  
   */
  #include "thusewindows.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "../../thusewindows.h"
  #pragma warning(disable : 4786)
#else
  /**
   * 일단, 나머지는 여기에
   */
#endif


namespace thlib {

#define TH_MEGA    1
#define TH_KILO    2
#define TH_BYTE    3

  template< int RET_TYPE = TH_MEGA >
  class thMemStatus {
  public:
    enum mem_type { THMS_PHYSIC, THMS_VIRTUAL, THMS_PAGE };
  public:
    double operator()() {
      double avail_mem = 0.0;
      ULARGE_INTEGER free_avail, total_bytes, free_bytes;
      ::GetDiskFreeSpaceEx(NULL, &free_avail, &total_bytes, &free_bytes);
      avail_mem += static_cast<double>(free_bytes.LowPart);
      MEMORYSTATUS mem;
      GlobalMemoryStatus(&mem);
      avail_mem += mem.dwAvailPhys; 
      switch (RET_TYPE) {
        case TH_MEGA:
          avail_mem /= 1048576.0;
          break;
        case TH_KILO:
          avail_mem /= 1024.0;
          break;
        default:
          break;
      };
      return avail_mem;
    }

    double operator()(int mem_type) {
      MEMORYSTATUS mem;
      GlobalMemoryStatus(&mem);
      double avail_mem = 0.0f;
      switch (mem_type) {
        case THMS_PHYSIC:
          avail_mem = mem.dwAvailPhys;
          break;
        case THMS_VIRTUAL:
          avail_mem = mem.dwAvailVirtual;
          break;
        case THMS_PAGE:
          return mem.dwAvailPageFile;
        default:
          break;
      };
      switch (RET_TYPE) {
        case TH_MEGA:
          avail_mem /= 1048576.0;
          break;
        case TH_KILO:
          avail_mem /= 1024.0;
          break;
        default:
          break;
      };
      return avail_mem;
    }
  };
}

#endif

