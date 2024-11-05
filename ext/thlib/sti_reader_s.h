/* copyright <happyteam@thinkware.co.kr> */
#ifndef  STI_READER_S_H_
#define  STI_READER_S_H_

//#include "therrno.h"
#include "thfile.h"
#include "thstring.h"
#include "sti_datast_strf.h"
#include <assert.h>

class sTrfReader {
 public :
   static const char* STRF_KS_PATH;
   static const char* STRF_TTL_PATH;

   enum STRF_TYPE { STRF_TYPE_KS = 0, STRF_TYPE_TTL , STRF_TYPE_TTL_PARTIAL };
 public :
  ~sTrfReader() {
    if (_strf_file.isOpen()) {
      _strf_file.close();
    }
  }

  int init(const char* map_path, int type) {
    thlib::thString path;
    if (type == STRF_TYPE_TTL) {
      path.printf("%s/%s", map_path, STRF_TTL_PATH);
    } else if (type == STRF_TYPE_TTL_PARTIAL) {
      path.printf("%s/%s.bin", map_path, PARTIAL_STRF_FILENAME_TTL);
    } else {
      path.printf("%s/%s", map_path, STRF_KS_PATH);
    }
    if (_strf_file.open(path, "rb") < 0)
      return -EFILE_OPEN;
    return 0;
  }

  size_t size() {
    return _strf_file.size();
  }

  template<class ObjType>
  int read(ObjType* data, size_t ds, size_t size) {
    if (_strf_file.read((char*)data, ds, size) != size)
      return -EFILE_READ;
    return 0;
  }
 public:
  thlib::thFile<> _strf_file;
};

#endif
