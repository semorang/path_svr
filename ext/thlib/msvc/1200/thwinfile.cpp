/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thwinfile.h"
//#include <winbase.h>


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등
   */
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include <io.h>
#else
  /**
  * 일단, 나머지는 여기에
   */
#endif

namespace thlib {

int thWinFile::open(const char* name, const char *mode, const int) {
#ifdef UNICODE
  wchar_t conv_string[512] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, name, strlen(name)+1
    , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", name);
#endif
  int is_file_exist = 1;
  if (0xffffffff == ::GetFileAttributes(conv_string)) {
    is_file_exist = 0;
  }
  _hfile = ::CreateFile(conv_string, cvtMode1(mode), 
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
    cvtMode2(mode, is_file_exist), FILE_ATTRIBUTE_NORMAL, NULL);
  if (_hfile == INVALID_HANDLE_VALUE)
    return -1;
  _cvtmode(mode);

  ///< append mode
  if (_cvtmode.find('a') != std::string::npos) {
    SetFilePointer(_hfile, 0, NULL, FILE_END);
  }
  return 0;
}

int thWinFile::open(char* name, const char *mode, const int) {
#ifdef UNICODE
  wchar_t conv_string[512] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, name, strlen(name)+1
    , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", name);
#endif
  int is_file_exist = 1;
  if (0xffffffff == ::GetFileAttributes(conv_string)) {
    is_file_exist = 0;
  }
  _hfile = ::CreateFile(conv_string, cvtMode1(mode), 
    FILE_SHARE_READ | FILE_SHARE_WRITE,  NULL, 
    cvtMode2(mode, is_file_exist), FILE_ATTRIBUTE_NORMAL, NULL);
  int ret = GetLastError();
  if (_hfile == INVALID_HANDLE_VALUE)
    return -1;
  _cvtmode(mode);
  ///< append mode
  if (_cvtmode.find('a') != std::string::npos) {
    SetFilePointer(_hfile, 0, NULL, FILE_END);
  }
  return 0;
}

int thWinFile::close() {
  ::CloseHandle(_hfile);
  _hfile = INVALID_HANDLE_VALUE;
  return 0;
}

size_t thWinFile::seek(size_t lPoint) {
  return ::SetFilePointer(_hfile, (DWORD)lPoint, NULL, FILE_BEGIN);
}

size_t thWinFile::seek(size_t lPoint, int where) {
  return ::SetFilePointer(_hfile, (DWORD)lPoint, NULL, where);
}

int thWinFile::flush(bool sync) {
  ::FlushFileBuffers(_hfile);
  ///  @todo
  /*
  if (sync) {
    return _commit(_fileno(_hfile));
  }
  */
  return 0;
}

int thWinFile::eof() {
  return position() == size() ? 1 : 0;
}

int thWinFile::printf(const char* fmt, ...) {
  return 0;
}

size_t thWinFile::size() {
  return (size_t)::GetFileSize(_hfile, NULL);
}

size_t thWinFile::position() {
  return seek(0, FILE_CURRENT);
}

int thWinFile::isOpen() {
  return (_hfile == INVALID_HANDLE_VALUE) ? 0 : 1;
}

size_t thWinFile::read(char *buff, size_t size) {
  DWORD byte_read;
  ::ReadFile(_hfile, buff, (uint32_t)size, &byte_read, NULL);
  return byte_read;
}


// line_max_size를 작게하면 newline으로 discharge되는 양을 줄일 수 있다.
// 256권장 
size_t thWinFile::readline(char *buff, size_t size) {
  if (!isOpen())
    return 0xffffffff;
  if (thWinFile::position() >= thWinFile::size())
    return 0xffffffff;
// keypos는 리턴할 버퍼(축적), bufpos는 읽기 버퍼 
  DWORD byte_read = 0;
  size_t keypos = 0, bufpos = 0; 
  thassert(size != 0);
  char *mem = MemPolicy<char>::alloc(size);
  memset(mem, '\0', size);
  memset(buff, '\0', size);
  do {
    if (bufpos >= byte_read) {
      byte_read = 0;
      bufpos = 0;
      if (::ReadFile(_hfile, mem, size, &byte_read, NULL) == FALSE)
        return 0xffffffff;
    }
    buff[keypos] = mem[bufpos];
    ++keypos; 
    ++bufpos;
  }
  while ((mem[keypos] != '\r') && (mem[keypos] != '\n') && (mem[keypos] != '\0'));
  buff[keypos] = '\0';

  int add = 1;
  if (mem[keypos] == '\r') {
    add += 1;
  }
  int discharge = byte_read - (bufpos + add);
  /** 
   * what is it ??
   * -discharge??
   */
  // EOF이면 수행안함.
  if (seek(-discharge, FILE_CURRENT) >= thWinFile::size()) {
    seek(0, FILE_END);
  }
  MemPolicy<char>::memfree(mem);
  mem = NULL;

  return 0;
}

size_t thWinFile::write(char *buff, size_t size) {
  DWORD byte_write;
//  if (_cvtmode.find('a') != std::string::npos)
//    ::SetEndOfFile(_hfile);
  ::WriteFile(_hfile, buff, (uint32_t)size, &byte_write, NULL);
  return byte_write;
}

size_t thWinFile::write(const char* buff, size_t size) {
  DWORD byte_write;
//  if (_cvtmode.find('a') != std::string::npos)
//    ::SetEndOfFile(_hfile);
  ::WriteFile(_hfile, buff, (uint32_t)size, &byte_write, NULL);
  return byte_write;
}
// write()와 다르지 않다.
size_t thWinFile::writeline(const char *buff) {
  return write(buff, sizeof(buff));
}

// thMMFile

int thMMFile::open(const char* name, const char *mode, const int) {
#ifdef UNICODE
  wchar_t conv_string[512] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, name, strlen(name)+1
    , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", name);
#endif
  int is_file_exist = 1;
  if (0xffffffff == ::GetFileAttributes(conv_string)) {
    is_file_exist = 0;
  }
  _hfile = ::CreateFile(conv_string, cvtMode1(mode), 
    FILE_SHARE_READ | FILE_SHARE_WRITE,  NULL, 
    cvtMode2(mode, is_file_exist), FILE_ATTRIBUTE_NORMAL, NULL);
  int ret = GetLastError();
  if (_hfile == INVALID_HANDLE_VALUE)                                    
    return -1;
  _cvtmode(mode);
  ///< append mode
  if (_cvtmode.find('a') != std::string::npos) {
    SetFilePointer(_hfile, 0, NULL, FILE_END);
  }
  
  _hMap = ::CreateFileMapping(_hfile, NULL, mapMode1(mode), 0, 0, NULL);
  if (_hMap == NULL) {
    close();
    return false;
  }
  
  _pMapView = (BYTE *)::MapViewOfFile(_hMap, mapviewMode1(mode), 0, 0, 0);
  if (_pMapView == NULL) {
    close();
    return false;
  }
  _nFilePointer = 0;
  _nFileSize = ::GetFileSize(_hfile, NULL);
  return 0;
}

int thMMFile::open(char* name, const char *mode, const int) {
#ifdef UNICODE
  wchar_t conv_string[512] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, name, strlen(name)+1
    , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", name);
#endif
  int is_file_exist = 1;
  if (0xffffffff == ::GetFileAttributes(conv_string)) {
    is_file_exist = 0;
  }
  _hfile = ::CreateFile(conv_string, cvtMode1(mode), 
    FILE_SHARE_READ | FILE_SHARE_WRITE,  NULL, 
    cvtMode2(mode, is_file_exist), FILE_ATTRIBUTE_NORMAL, NULL);
  int ret = GetLastError();
  if (_hfile == INVALID_HANDLE_VALUE)
    return -1;

  _nFileSize = ::GetFileSize(_hfile, NULL);

  _cvtmode(mode);
  ///< append mode
  if (_cvtmode.find('a') != std::string::npos) {
    SetFilePointer(_hfile, 0, NULL, FILE_END);
  }

  _hMap = ::CreateFileMapping(_hfile, NULL, mapMode1(mode), 0, 0, NULL);
  if (_hMap == NULL) {
    close();
    return false;
  }

  _pMapView = (BYTE *)::MapViewOfFile(_hMap, mapviewMode1(mode), 0, 0, 0);
  if (_pMapView == NULL) {
    close();
    return false;
  }

  _nFilePointer = 0;
  _nFileSize = ::GetFileSize(_hfile, NULL);
  return 0;
}

int thMMFile::close() {
  if (_pMapView != NULL) {
    ::UnmapViewOfFile(_pMapView);
    _pMapView = NULL;
  }
  
  if (_hMap != NULL) {
    ::CloseHandle(_hMap);
    _hMap = NULL;
  }
  
  if (_hfile != INVALID_HANDLE_VALUE) {
    ::CloseHandle(_hfile);
    _hfile = INVALID_HANDLE_VALUE;
  }

  return 0;
}

size_t thMMFile::seek(size_t lPoint) {
  _nFilePointer = lPoint;
  return _nFilePointer;
//  return ::SetFilePointer(_hMMFile, (long)lPoint, NULL, FILE_BEGIN);
}

size_t thMMFile::seek(size_t lPoint, int where) {
  _nFilePointer = where+lPoint;
  return _nFilePointer;
//  return ::SetFilePointer(_hMMFile, (long)lPoint, NULL, where);
}

int thMMFile::flush(bool sync) {
//  ::FlushFileBuffers(_hMMFile);
  ///  @todo
  /*
  if (sync) {
    return _commit(_fileno(_hfile));
  }
  */
  return 0;
}

int thMMFile::eof() {
  return position() == size() ? 1 : 0;
}

int thMMFile::printf(const char* fmt, ...) {
  return 0;
}

size_t thMMFile::size() {
  return (size_t)::GetFileSize(_hfile, NULL);
}

size_t thMMFile::position() {
//  return seek(0, FILE_CURRENT);
  return (size_t)_nFilePointer;
}

int thMMFile::isOpen() {
  return (_hfile == INVALID_HANDLE_VALUE) ? 0 : 1;
}

size_t thMMFile::read(char *buff, size_t size) {
  if (_pMapView == NULL)
    return 0;
  
  
  if (_nFilePointer >= _nFileSize)
    return 0;
  
  if (int(_nFilePointer + size) > _nFileSize) {
    size = _nFileSize - _nFilePointer;
  }

  ::CopyMemory(buff, &_pMapView[_nFilePointer], size);
  _nFilePointer += size;
//  ::ReadFile(_hMMFile, buff, (uint32_t)size, &byte_read, NULL);
  return size;
}

// line_max_size를 작게하면 newline으로 discharge되는 양을 줄일 수 있다. 
// 256권장 
size_t thMMFile::readline(char *buff, size_t size) {
  if (!isOpen())
    return 0xffffffff;
  if (thMMFile::position() >= thMMFile::size())
    return 0xffffffff;

  if (_pMapView == NULL)
    return 0;
 /* 
  if (_nFilePointer >= _nFileSize)
    return 0;
  
  if ( int(_nFilePointer + size) > _nFileSize) {
    size = _nFileSize - _nFilePointer;
  }
*/
  uint32_t byte_read = 0;
  // keypos는 리턴할 버퍼(축적), bufpos는 읽기 버퍼
  size_t keypos = 0, bufpos = 0;  
  thassert(size != 0);
  char *mem = MemPolicy<char>::alloc(size);
  memset(mem, '\0', size);
  memset(buff, '\0', size);
  do {
    if (bufpos >= byte_read) {
      byte_read = 0;
      bufpos = 0;
//      if (::ReadFile(_hMMFile, mem, size, &byte_read, NULL)==FALSE)
      if (::CopyMemory(mem, &_pMapView[_nFilePointer], size) == FALSE)
        return 0xffffffff;
      byte_read = size;
      _nFilePointer += size;
    }
    buff[keypos] = mem[bufpos];
    ++keypos; 
    ++bufpos;
  } while ((mem[keypos] != '\r') && (mem[keypos] != '\n') && (mem[keypos] != '\0'));
  buff[keypos] = '\0';
  
  int add = 1;
  if (mem[keypos] == '\r') {
    add += 1;
  }
  int discharge = byte_read - (bufpos + add); 
  /** 
   * what is it ??
   * -discharge??
   */
  // EOF이면 수행안함.
  if (seek(-discharge, FILE_CURRENT) >= thMMFile::size()) {
    seek(0, FILE_END);
  }
  MemPolicy<char>::memfree(mem);
  mem = NULL;
  return 0;
}

size_t thMMFile::write(char *buff, size_t size) {
  if (_pMapView == NULL)
    return 0;
  
  if ( int(_nFilePointer) >= _nFileSize)
    return 0;
  
  if (int(_nFilePointer + size) > _nFileSize) {
    size = _nFileSize - _nFilePointer;
  }
  
  ::CopyMemory(&_pMapView[_nFilePointer], buff, size);

  _nFilePointer += size;
  return size;
}

size_t thMMFile::write(const char* buff, size_t size) {
  if (_pMapView == NULL)
    return 0;
  
  if (int(_nFilePointer) >= _nFileSize)
    return 0;
  
  if (int(_nFilePointer + size) > _nFileSize) {
    size = _nFileSize - _nFilePointer;
  }

  ::CopyMemory(&_pMapView[_nFilePointer], buff, size);
  _nFilePointer += size;
  return size;
}
// write()와 다르지 않다.
size_t thMMFile::writeline(const char *buff) {
  return write(buff, sizeof(buff));
}
} // end thlib
