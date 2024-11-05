/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thwinfile_h_
#define _thwinfile_h_


/** 
 * stdafx.h를 못 찾는 경우가 있어서 파일 추가 해 두었음 
 */
/** 
 * wince4.0 이상, MSVC 이상 
 */

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

//#include <tchar.h>
#include "../../thstring.h"
#include "../../thmemalloc.h"

namespace thlib {
  /** 
   * \brief Get windows current directory
   * get current directory 
   */
  class thCurDir {
  public:
    thlib::thString& operator()() {
      TCHAR full_dir[256];
      ::GetModuleFileName(NULL, full_dir, 256);
      //USES_CONVERSION;
#ifdef UNICODE
      int str_length = 256 * 2 + 1;
      _cur_path.resize(str_length);
      ::WideCharToMultiByte(
        CP_ACP,      //UINT CodePage, 
        0,        //DWORD dwFlags, 
        full_dir,    //LPCWSTR lpWideCharStr, 
        str_length,      //int cchWideChar, 
        _cur_path(),  //LPSTR lpMultiByteStr,
        256,      //int cbMultiByte, 
        NULL,      //LPCSTR lpDefaultChar, 
        NULL);      //LPBOOL lpUsedDefaultChar
#else 
      _cur_path.printf("%s", full_dir);
#endif
      _cur_path = _cur_path.substr(0, _cur_path.rfind("\\"));
      return _cur_path;
    }
    thlib::thString _cur_path;
  };

  class thWinFile {  // 표준 라이브러리를 사용하면 다른 os로의 이식이 편해진다.
  public:
    thWinFile() {
      _hfile = INVALID_HANDLE_VALUE;
    }
    ~thWinFile() {
      if (_hfile != INVALID_HANDLE_VALUE) {
        ::CloseHandle(_hfile);
        _hfile = INVALID_HANDLE_VALUE;
      }
    }
    int open(const char* name, const char *mode, const int);
    int open(char* name, const char *mode, const int);
    int close();
    size_t seek(size_t lPoint);
    size_t seek(size_t lPoint, int where);
    int flush(bool sync);
    int eof();
    int printf(const char* fmt, ...);
    size_t size();
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    size_t position();
    int isOpen();
    size_t read(char *buff, size_t size);
    size_t readline(char *buff, size_t size);
    size_t write(char *buff, size_t size);
    size_t write(const char* buff, size_t size);
    size_t writeline(const char *buff);
    char* address() { return NULL; }
  public:
    static int cvtMode1(const char* mode) {
      thlib::thString tok = mode;
      int ret_mode = GENERIC_READ;
      if (tok.find('a') != std::string::npos) {
        ret_mode |= GENERIC_WRITE;
      }
      if (tok.find('w') != std::string::npos) {
        ret_mode |= GENERIC_WRITE;
      } 
      if (tok.find('r') != std::string::npos) {
        ret_mode |= GENERIC_READ;
      } 
      return ret_mode;
    }
    
    static int cvtMode2(const char* mode, int is_file_exist) {
      thlib::thString tok = mode;
      int ret_mode = OPEN_EXISTING;
      if (tok.find('a') != std::string::npos) {
        ret_mode = OPEN_ALWAYS;
        //ret_mode = CREATE_ALWAYS;
      }
      if (tok.find('r') != std::string::npos) {
        ret_mode = OPEN_EXISTING;
      } 
      if (tok.find('w') != std::string::npos) {
        ret_mode = CREATE_NEW;
        if (is_file_exist) {
          ret_mode = 
            TRUNCATE_EXISTING;
        }
      } 
      return ret_mode;
    }
    
  private:
    HANDLE        _hfile;
    thlib::thString    _cvtmode;
  };

  class thMMFile {  // 표준 라이브러리를 사용하면 다른 os로의 이식이 편해진다.
  public:
    thMMFile() {
      _hfile = INVALID_HANDLE_VALUE;
      _nFilePointer = 0;
      _nFileSize = 0;
      _hMap = NULL;
      _pMapView = NULL;
    }
    ~thMMFile() {
      if (_hfile != INVALID_HANDLE_VALUE) {
        ::CloseHandle(_hfile);
        _hfile = INVALID_HANDLE_VALUE;
      }
    }
    int open(const char* name, const char *mode, const int);
    int open(char* name, const char *mode, const int);
    int close();
    size_t seek(size_t lPoint);
    size_t seek(size_t lPoint, int where);
    int flush(bool sync);
    int eof();
    int printf(const char* fmt, ...);
    size_t size();
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    size_t position();
    int isOpen();
    size_t read(char *buff, size_t size);
    size_t readline(char *buff, size_t size);
    size_t write(char *buff, size_t size);
    size_t write(const char* buff, size_t size);
    size_t writeline(const char *buff);
    char* address() { return NULL; }
  public:
    static int cvtMode1(const char* mode) {
      thlib::thString tok = mode;
      int ret_mode = GENERIC_READ;
      if (tok.find('a') != std::string::npos) {
        ret_mode |= GENERIC_WRITE;
      }
      if (tok.find('w') != std::string::npos) {
        ret_mode |= GENERIC_WRITE;
      } 
      if (tok.find('r') != std::string::npos) {
        ret_mode |= GENERIC_READ;
      } 
      return ret_mode;
    }

    static int mapMode1(const char* mode) {
      thlib::thString tok = mode;
      int ret_mode = PAGE_READWRITE;
      if (tok.find('a') != std::string::npos) {
        ret_mode = PAGE_READWRITE;
      }
      if (tok.find('w') != std::string::npos) {
        ret_mode = PAGE_READWRITE;
      } 
      if (tok.find('r') != std::string::npos) {
        ret_mode = PAGE_READONLY;
      } 
      return ret_mode;
    }

    static int mapviewMode1(const char* mode) {
      thlib::thString tok = mode;
      int ret_mode = FILE_MAP_READ;
      if (tok.find('a') != std::string::npos) {
        ret_mode = FILE_MAP_ALL_ACCESS;
      }
      if (tok.find('w') != std::string::npos) {
        ret_mode = FILE_MAP_WRITE;
      } 
      if (tok.find('r') != std::string::npos) {
        ret_mode = FILE_MAP_READ;
      } 
      return ret_mode;
    }
    
    static int cvtMode2(const char* mode, int is_file_exist) {
      thlib::thString tok = mode;
      int ret_mode = OPEN_EXISTING;
      if (tok.find('a') != std::string::npos) {
        ret_mode = OPEN_ALWAYS;
        //ret_mode = CREATE_ALWAYS;
      }
      if (tok.find('r') != std::string::npos) {
        ret_mode = OPEN_EXISTING;
      } 
      if (tok.find('w') != std::string::npos) {
        ret_mode = CREATE_ALWAYS;
        if (is_file_exist) {
          ret_mode = 
            OPEN_EXISTING;
        }
      } 
      return ret_mode;
    }
    
  private:
    HANDLE        _hfile;
    HANDLE        _hMap;
    BYTE*        _pMapView;
    int          _nFileSize;
    int          _nFilePointer;
    thlib::thString    _cvtmode;
    int          _mapviewmode;
  };
}


#endif

