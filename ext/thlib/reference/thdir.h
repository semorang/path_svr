/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THDIR_H__
#define _THDIR_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../thstring.h"
#include "../thfile.h"


namespace thlib {


/**
 *  @class  thFS
 *  @brief  included contents about file system
 *
 */
class thFS {
 public:
  static int makedir(thString& path, mode_t mode) {
    mode_t prevmode = umask(mode);
    if (mkdir(path(), 0777) != 0) {
      if (errno != EEXIST) {
        umask(prevmode);
        return -1;
      }
    }
    umask(prevmode);

    return 0;
  }
  static int modeopen(thFile<>& file, thString& path, const char* opt, 
                      mode_t mode = 0) {
    mode_t prevmode = umask(mode);
    int ret = file.open(path(), opt);
    umask(prevmode);
    return ret;
  }
};
};



#endif
