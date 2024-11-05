/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THFILE_
#define _THFILE_


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #pragma warning(disable : 4786)
  #include "thwinfile.h"
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #pragma warning(disable : 4786)
  #include "thWinfile.h"
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif


//#include "thcfile.h"
#include "thstdfile.h"
#include "thmemfile.h"

namespace thlib {

// 지저분 하게 여러개의 맴버 함수를 쓰지 않는다. 
// 단지 필요한 것들만 쓰고 최소한 맴버로 처리하도록 한다. 
// 즉, 클래스가 atomic하게 만들고 싶은 거지...(말이 되진 않지만)

class thString;


#if defined(_WIN32_WCE) || defined(WIN32)
#define THSEEK_SET FILE_BEGIN
#define THSEEK_CUR FILE_CURRENT
#define THSEEK_END FILE_END
#else
#define THSEEK_SET SEEK_SET
#define THSEEK_CUR SEEK_CUR
#define THSEEK_END SEEK_END
#endif


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  template<class FileType = thWinFile>

#elif defined(WIN32)
  /**
   *  windows 32bit
   */

  template<class FileType = thWinFile>
#else
  /**
   * 일단, 나머지는 여기에 
   */
  /** 
   * @brief thStdFile 제한 사항 
   * unicode를 지원하지 않으니 multi bytes로 쓸것 
   */
  template<class FileType = thStdFile>
#endif
class thFile {
 public:
  FileType operator()() { return _file_obj; }
//  static size_t thnpos;

  public:
  thFile() {}
  ~thFile() { if (isOpen()) close(); }
  int open(const char* name, const char *mode="rb", const int bs=-1) {
    return _file_obj.open(name, mode, bs);
  }
  int open(char* name, const char *mode="rb", const int bs=-1) {
    return _file_obj.open(name, mode, bs);
  }
  int open(thString& name, const char * mode="rb", const int bs=-1) {
    return _file_obj.open(name, mode, bs);
  }
  int  close() {
    flush();
    return _file_obj.close();
  }
  size_t  seek(size_t lpoint) {
    return _file_obj.seek(lpoint);
  }
  /** 
   * 아래 interface때문에 seek의 return 값이 size_t or long형이 될 수 
   * 없는 것인가??
   */
  size_t seek(size_t lpoint, int where) {
    return _file_obj.seek(lpoint, where);
  }
  int  flush(bool sync = false) {
    return _file_obj.flush(sync);
  }

  int eof() {
    return _file_obj.eof();
  }
/* // 문제 있는 코드 잘못 구현되어 있음//lint 수정 중 발견
  int  printf(const char *str, ...) {
    return _file_obj.printf(str);
  }
*/
  size_t size() { 
    return _file_obj.size();
  }

  size_t position() {
    return _file_obj.position();
  }

  int isOpen() { 
    return _file_obj.isOpen(); 
  }

  template<class Type>
  size_t read(Type *buff, size_t pos, size_t size) {
    thassert(pos < this->size());
    seek(pos);
    return _file_obj.read(buff, size);
  }

  template<class Type>
  size_t read(Type *buff, size_t size) {
    if (_file_obj.read(reinterpret_cast<char*>(buff), size) != size)
      return UINT_MAX;
    return size;
  }

  template<class Type>
  size_t read(Type& buff) {
    size_t size = sizeof(Type);
    char* cnvt = reinterpret_cast<char*>(&buff);
    if (_file_obj.read(cnvt, size) != size) 
      return UINT_MAX;
    return size;
  }

  size_t readline(char *buff, size_t size = 512) {
    if (_file_obj.readline(buff, size) == UINT_MAX)
      return UINT_MAX;
    return size;
  }


  template<class Type>
  size_t write(Type *buff, size_t size) {
    if (buff == NULL)  return UINT_MAX;
    if (_file_obj.write(buff, size) != size)
      return UINT_MAX;
    return size;
  }

  template<class Type>
  size_t write(Type *buff, size_t pos, size_t size) {
    thassert(buff != NULL);
    thassert(pos < this->size());
    seek(pos);
    if (_file_obj.write(buff, size) != size)
      return UINT_MAX;
    return size;
  }

  size_t write(const char* buff, size_t size) {
    if (buff == NULL)  return UINT_MAX;
    if (_file_obj.write(buff, size) != size)
      return UINT_MAX;
    return size;
  }

  size_t writeline(const char *buff) {
    if (_file_obj.writeline(buff) == UINT_MAX)
      return UINT_MAX;
    return 0;
  }


#if 1
  template<class Type>
  size_t write(Type &buff) {
    size_t size = sizeof(Type);
    if (_file_obj.write((char*)&buff, sizeof(buff)) != size)
      return UINT_MAX;
    return size;
  }
#endif

  size_t write(thlib::thString& buff) { 
    return _file_obj.write(buff.pointer(), buff.size()); 
  }

  char* address() { return _file_obj.address(); }

 private:
  FileType  _file_obj;
};
} // thlib



#endif

