/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thStdFile_h_
#define _thStdFile_h_

#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32_WCE)
  ///  can't find the function that do same work
  #define fsync(fd)  0
#elif defined(WIN32)
  #include <io.h>
  #define fsync(fd)  _commit(fd)
  #define fileno(fd)  _fileno(fd)
#else 
  #include <unistd.h>
#endif

#ifdef ANDROID
  #include <unistd.h>
#endif



#include "thconfig.h"
#include "thstring.h"

namespace thlib {
  class thStdFile  {
  public:
    FILE* operator()() { return _file_pointer; }
//    static size_t npos;

  public:
    thStdFile() { this->_file_pointer = NULL; }
    ~thStdFile() { 
      close(); 
    }

    int open(const char* name, const char *mode, const int) {
      thassert(name != "");
      if ((this->_file_pointer = fopen(name, mode)) == NULL)
        return -1;
      seek(0);
      return 0;    
    }
    
    int open(char* name, const char *mode, const int) {
      thassert(name != "");
      if ((_file_pointer = fopen(name, mode)) == NULL)
        return -1;
      seek(0);
      return 0;    
    }

    int  close()  {
      if (_file_pointer != NULL)
        fclose(_file_pointer);
      _file_pointer = NULL;
      return 0;  
    }
    
    size_t  seek(size_t lpoint) {
      thassert(_file_pointer != NULL);
      /** 
       * manual에 fseek는 return value가 0이면 성공이고 
       * 아니면 -1을 return 한다. 
       */
      if (fseek(_file_pointer, (int32_t)lpoint, SEEK_SET) != 0) { 
        return UINT_MAX; 
      }
      return 0;  
    }
    
    size_t seek(size_t lpoint, int whence) {
      thassert(_file_pointer != NULL);
      if (fseek(_file_pointer, (int32_t)lpoint, whence) != 0) { 
        return UINT_MAX; 
      }
      return lpoint;  
    }
    
    int  flush(int sync = 0) {
      thassert(_file_pointer != NULL);
      if (!sync) {
        return fflush(_file_pointer);
      } else {
        int ret = fflush(_file_pointer);
        if (ret != 0)
          return ret;
        return fsync(fileno(_file_pointer));
      }
    }
    
    int eof() {
      thassert(_file_pointer != NULL);
      return feof(_file_pointer);  
    }
    
    int  printf(const char *fmt, ...) {
      thassert(_file_pointer != NULL);
      va_list argptr;
      va_start(argptr, fmt);
      int cnt = vfprintf(_file_pointer, fmt, argptr);
      va_end(argptr);
      return cnt;
    }  
    
    size_t size() {
      size_t old_pos = position();
      fseek(_file_pointer, 0, SEEK_END);
      int ret_size = ftell(_file_pointer);
      /**
       * evc에는 rewind가 없다. 사용하지 않도록 한다. 
       */
      //rewind(_file_pointer);
      fseek(_file_pointer, (int32_t)old_pos, SEEK_SET);
      return ret_size;
    }
    
    size_t position() {
      thassert(_file_pointer != NULL);
      return ftell(_file_pointer);
    }
    
    int isOpen() { 
      return _file_pointer != NULL ? 1 : 0; 
    }
    
    size_t read(char *buff, size_t size) {
      thassert(_file_pointer != NULL);
      return fread(buff, 1, size, _file_pointer);
    }

    size_t readline(char *buff, size_t size) {
      if (_file_pointer == NULL)
        return UINT_MAX;
      if (fgets(buff, (int32_t)size, _file_pointer) == NULL)
        return UINT_MAX;
      /**
       * 상태가 메~~~롱 함
       */
      char *p;
      /// 동작이 이상하면 strchr(search forword)로 바꿔라
      if ((p = strrchr(buff, '\n')) != NULL)
        *p = '\0';    // thString에서 '\0'를 붙인다.
      return 0;
    }

    size_t write(char *buff, size_t size) {
      thassert(_file_pointer != NULL);
      return fwrite(buff, 1, size, _file_pointer);
    }

    size_t write(const char *buff, size_t size) {
      thassert(_file_pointer != NULL);
      return fwrite(buff, 1, size, _file_pointer);
    }

    size_t writeline(const char *buff) {
      if (_file_pointer == NULL)
        return UINT_MAX;
      if (fputs(buff, _file_pointer) == EOF)
        return UINT_MAX;
      return 0;
    }
    char* address() { return NULL; }

  private:
    FILE  *_file_pointer;
  };
} // thlib

#endif
