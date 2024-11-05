/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THFILE_
#define _THFILE_


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ��� 
   */
  #pragma warning(disable : 4786)
  #include "thwinfile.h"
#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */
  #pragma warning(disable : 4786)
  #include "thWinfile.h"
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
#endif


//#include "thcfile.h"
#include "thstdfile.h"
#include "thmemfile.h"

namespace thlib {

// ������ �ϰ� �������� �ɹ� �Լ��� ���� �ʴ´�. 
// ���� �ʿ��� �͵鸸 ���� �ּ��� �ɹ��� ó���ϵ��� �Ѵ�. 
// ��, Ŭ������ atomic�ϰ� ����� ���� ����...(���� ���� ������)

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
   * EVC3.0 EVC4.0 ��� 
   */
  template<class FileType = thWinFile>

#elif defined(WIN32)
  /**
   *  windows 32bit
   */

  template<class FileType = thWinFile>
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
  /** 
   * @brief thStdFile ���� ���� 
   * unicode�� �������� ������ multi bytes�� ���� 
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
   * �Ʒ� interface������ seek�� return ���� size_t or long���� �� �� 
   * ���� ���ΰ�??
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
/* // ���� �ִ� �ڵ� �߸� �����Ǿ� ����//lint ���� �� �߰�
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

