/* Copyright 2011 <happyteam@thinkwaresys.com> */
/*****************************************************************************
 *      
 *      
 *      CopyRight (c) 2003, Thinkwaresys Navigation R&D division
 *****************************************************************************/
#ifndef  __thstring_
#define  __thstring_


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
  #include "thusewindows.h"
  #include <stdio.h>
  #pragma warning(disable : 4786 4996)
#else
  /**
   * 일단, 나머지는 여기에 
   */
  #include <stdio.h>
  #include <stdlib.h>
  #include <sstream>
#endif

#include <ctype.h>
#include <stdarg.h>

#if defined(_WIN32_WCE) || defined(WIN32)
  #include <string>
#else
  #include <string.h>
#endif
#include <vector>
#include "thdef.h"
//#include "therrno.h"
#include "thallocator.h"

namespace thlib {
  typedef std::basic_string<char, std::char_traits<char>, 
                                  std::allocator<char> > __thstring;
  /** 
   * @brief thString 
   * @note wide char, char, TCHAR에 대해서 모두 호완성이 있어야 한다. 
   *  현재는 호완성 없음. ... 
   *  string 정책은 무조건 char로 쓴다. 
   *  다만, TCHAR로 변환해야할 시점에서 변화하여 사용하도록 한다. 
   *  wide char, char, TCHAR에 대해서 모두 호완성이 있어야 한다. 
   *
   */
  class thString {
  public:
    /// default constructor.
    thString() { _str = ""; }
    thString(char *b) { //NOLINT 
      _str = "";
      _str.insert(0, b);
    }
    thString(const char *b) { //NOLINT 
      _str = ""; 
      _str.insert(0, b); 
    }
    thString(char b) { _str = b; } //NOLINT
    thString(const thString& lhs) {
      if (this != &lhs) {
        _str = lhs._str.c_str();
        if (_str.size() > 0) {
          _str[lhs._str.size()] = '\0';
        }
        return;
      }
    }
    
    /// default destructor.
    ~thString() {}
    
    /**
     * @brief 정해진 위치에서 길이만큼 문자열 복사.
     * @param src 복사할 문자열.
     * @param start 시작 값.
     * @param len 길이.
     * @return -1 실패, 0 성공
     */
    int  copy(const thString &src, size_t start, size_t len) {
      if (src._str.size() < (unsigned)(start + len) ||
        len == 0)
        return -1;
      _str.assign(src._str.c_str(), start, len);
      return 0;
    }
    
    /**
     * @brief 정해진 위치에서 길이만큼 문자열 복사.
     * @param start 시작 값.
     * @param len 길이.
     * @return 복사된 문자열.
     */
    thString copy(size_t start, size_t len) const {
      thString ret;
      if (_str.size() < (start + len) || len == 0)  
        return ret;
      __thstring ret_sub = _str.substr(start, len);
      ret = ret_sub.c_str();
      return ret;
    }
    
    /**
     * @brief 원하는 문자열 위치에서 길이만큼으로 치환.
     * @param start 시작 값.
     * @param len 길이.
     * @return 치환된 문자열.
     */
    thString substr(size_t start, size_t len) const {
      __thstring sub_ret = _str.substr(start, len);
      thString ret = sub_ret.c_str(); 
      return ret;
    }

    /**
     * @brief 원하는 문자열 위치에서 문자열 끝까지 치환.
     * @param start 시작 값.
     * @return 치환된 문자열.
     */
    thString substr(size_t start) const {
      __thstring sub_ret = _str.substr(start);
      thString ret = sub_ret.c_str();
      return ret;
    }
    
    /**
     * @brief string 객체 size 조정.
     * @param size 변경할 size.
     */
    void resize(size_t size) { _str.resize(size); }

    /**
     * @brief 문자열 추가.
     * @param str 추가할 문자열.
     * @return 최종 문자열 사이즈.
     */
    size_t append(const thString& str) {
      _str.append(str);
      return str.size();
    }

    void insert(size_t pos, const char* i) {
      _str.insert(pos, i);
    }

    /**
     * @brief 문자열 치환.
     * @param d1 찾을 문자열.
     * @param d2 치환 할 문자열.
     * @param start 문자열 찾기를 시작할 위치.
     * @return 치환 횟수.
     */
    size_t replace(const char* d1, const char* d2, size_t start = 0) {
      size_t replaceCount = 0;
      if (d1 == NULL || d2 == NULL) return replaceCount;
      size_t d1size = strlen(d1);
      size_t d2size = strlen(d2);
      size_t pos, next = start;
      while ((pos = _str.find(d1, next)) != __thstring::npos) {
        _str.erase(pos, d1size);
        _str.insert(pos, d2);
        next = pos + d2size;
        replaceCount++;
      }
      return replaceCount;
    }
    /**
     * @brief 해당 패턴 지우기
     * @param pattern 문자열 패턴
     * @param start 찾기 시작할 위치
     * @return 지운 패턴 개수
     */
    size_t erase(const char* pattern, size_t start = 0) {
      size_t cnt = 0;
      if (pattern == NULL) return cnt;
      size_t psize = strlen(pattern);
      size_t pos, next = start;
      while ((pos = _str.find(pattern, next)) != __thstring::npos) {
        _str.erase(pos, psize);
        next = pos;
        ++cnt;
      }
      return cnt;
    }
    /// 문자열 사이즈 리턴. 
    size_t  size() const { return _str.size(); }
    /// 문자열 포인터 리턴.
    const char*  pointer() const { return _str.c_str(); }
    size_t find(char c, size_t pos = 0) { return _str.find(c, pos); }
    size_t find(char* sub, size_t pos = 0) {
         return _str.find(sub, pos);
       }
    size_t find(const char* sub, size_t pos = 0) {
         return _str.find(sub, pos);
       }
    size_t find(thString& sub, size_t pos = 0) {
         return _str.find(sub.c_str(), pos);
       }
    size_t rfind(char c, size_t pos = __thstring::npos) {
         return _str.rfind(c, pos);
       }
    size_t rfind(char* sub, size_t pos = __thstring::npos) {
         return _str.rfind(sub, pos);
       }
    size_t rfind(const char* sub, size_t pos = __thstring::npos) {
         return _str.rfind(sub, pos);
       }
    size_t rfind(thString& sub, size_t pos = __thstring::npos) {
         return _str.rfind(sub.c_str(), pos);
       }
    void     clear() { _str = ""; }
    /**
     * @brief char token을 구분하여 해당 token이 나타날 때 까지의 문자열 리턴.
     * @param cseperator token 구분자.
     * @return token이 나타나기 전까지의 문자열.
     */
    thString  element(char cseperator) {
      size_t pos = _str.find_first_of(cseperator); 
      thString tmp;

      if (pos == __thstring::npos) { 
        tmp = (_str.substr(0, _str.size())).c_str();
        _str = _str.substr(_str.size(), _str.size());
      } else {
        tmp = (_str.substr(0, pos)).c_str();
        _str = _str.substr(pos+1, _str.size());
      }
      return tmp;
    }

    /**
     * @brief token문자열을 구분하여 해당 token이 나타날 때 까지의 문자열 리턴.
     * @param cseperator token 구분자 문자열.
     * @return token이 나타나기 전까지의 문자열.
     */
    thString  element(char* cseperator) {
      thString tmp;
      if (cseperator == NULL)
        return tmp;
      size_t pos = _str.find_first_of(cseperator);
      if (pos == __thstring::npos) { 
        tmp = (_str.substr(0, _str.size())).c_str();
        _str = _str.substr(_str.size(), _str.size());
      } else {
        tmp = (_str.substr(0, pos)).c_str();
        _str = _str.substr(pos+strlen(cseperator), _str.size());
        //_str = _str.substr(pos+1, _str.size());
      }
      return tmp;
    }
    
    /**
     * @brief string의 일부를 블럭으러 읽어 정수형으로 리턴.
     * @param start 시작 위치.
     * @param len 길이.
     * @return 읽어들인 string의 정수형.
     * @note 보통은 atoi를 써도 무관할 듯. ;;;
     */
    int tonum(size_t start = 0, size_t len = 0) const {
      if (!len) len = _str.size();
      int ret = 0;
      for (size_t i = start; i < len + start; ++i) {
        ret *= 10;
        if (isdigit(_str[i]))
          ret += _str[i] - '0';
        else
          return -1;
      }
      return ret;
    }

    int touint(size_t start = 0, size_t len = 0) const {
      if (!len) len = _str.size();
      unsigned int ret = 0;
      for (size_t i = start; i < len + start; ++i) {
        ret *= 10;
        if (isdigit(_str[i]))
          ret += _str[i] - '0';
        else
          return -1;
      }
      return ret;
    }

    double todouble(size_t start = 0, size_t len = 0) const {
      if (!len) len = _str.size();
      double ret = 0;
      for (size_t i = start; i < len + start; ++i) {
        ret *= 10;
        if (isdigit(_str[i]))
          ret += _str[i] - '0';
        else
          return -1;
      }
      return ret;
    }

    void strreverse(char* begin, char* end) {
      char aux;
      while (end > begin) {
        aux = *end, *end--=*begin, *begin++=aux;
      }
    }
    
    int tostr(int i, int base) {
     static const char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
      char rstr[11];
      char* wstr = rstr;
      int sign;
      if (base < 2 || base > 35) {
        _str.clear();
        return -1;
      }
      if ((sign = i) < 0) {
        i = -i;
      }
      do {
        *wstr++ = num[i % base];
      } while (i /= base);
      if (sign < 0) {
        *wstr++='-';
      }
      *wstr = '\0';
      strreverse(rstr, wstr - 1);
      _str = rstr;
      return 0;
    }
    /* operator << invoked segmentation fault in linux
    template <class __T>
    int tostr(__T i) {
      __thstringstream ss;
      ss << i;
      _str = ss.str();
      return 0;
    }
    */

  
    /** 
     * @brief string을 token으로 parser. 
     * @param tokens string에서 읽어낸 token vector.
     * @param delimiter 토큰 구분자.
     */
    void tokenize(std::vector<
        thlib::thString/*, thallocator<thlib::thString>*/ >& tokens,
            const char delimiter = '|') {
      thString token;
      while (_str.size() > 0) {
        token.clear();
        token = element(delimiter);
        tokens.push_back(token);
      }
    }    
    
    /// string의 첫번째 문자를 대문자로.
    void makeUpper() {
      strupr(&(this->_str[0]));
    }
    
    /// string의 첫번째 문자를 소문자로.
    void makeLower() {
      strlwr(&(this->_str[0]));
    }
    
    /**
     * @brief 문자열의 왼쪽편을 해당 문자열이 있다면 잘라낸다.
     * @param ch 잘라낼 문자열.
     * @return 잘라낸 문자열.
     */
    thString&  trimLeft(thString ch = thString(' ')) {
      if (_str.size() > 0)
        _str = _str.substr(_str.find_first_not_of(ch.pointer()));
      else 
        _str = "";
      return *this; 
    }

    /**
     * @brief 문자열의 오른쪽편을 해당 문자열이 있다면 잘라낸다.
     * @param ch 잘라낼 문자열.
     * @return 잘라낸 문자열.
     */
    thString&  trimRight(thString ch = thString(' ')) {
      if (_str.size() > 0)
        _str = _str.substr(0, _str.find_last_not_of(ch.pointer()) + 1);
      else 
        _str = "";
      return *this;
    }

    /**
     * @brief 문자열의 왼/오른쪽편을 해당 문자열이 있다면 잘라낸다.
     * @param ch 잘라낼 문자열.
     * @return 잘라낸 문자열.
     */
    thString&  trim(thString ch = thString(' ')) {
      trimLeft(ch);  
      trimRight(ch);
      return *this;
    }

    thString left(size_t n) {
      if (n > _str.size()) { 
        return *this; 
      } else {
        for (size_t i = 0; i < n; i++) {
          if (_str[i] < 0 || _str[i] > 126) {
            n++;
            i++;
          }
        }
        return substr(0, n);
      }
    }

    thString right(size_t n) {
      if (n > _str.size()) { 
        return *this; 
      } else {
        for (size_t i = _str.size() - n; i < _str.size(); i++) {
          if (_str[i] < 0 || _str[i] > 126) {
            n++;
            i++;
          }
        }
        return substr(_str.size() - n - 1, n+1);
      }
    }

    char* strupr(char* rev) {
      char* ret = rev;
      while (*rev != '\0') {
        if (islower(*rev)) {
          *rev = toupper(*rev);
        }
        ++rev;
      }
      return ret;
    }

    char* strlwr(char* rev) {
      char* ret = rev;
      while (*rev != '\0') {
        if (isupper(*rev)) {
          *rev = tolower(*rev);
        }
        ++rev;
      }
      return ret;
    }

    int  printf(const char *fmt, ...) {
       va_list argptr;
       char  buffer[THSTRING_MAX];
       memset(buffer, 0x00, THSTRING_MAX);
       va_start(argptr, fmt);
       int cnt = vsnprintf(buffer, THSTRING_MAX, fmt, argptr);
       va_end(argptr);
       *this = buffer;
       return cnt;
    }

    uint32_t hashCode() {
      uint32_t hash_code = 0;
      for (int i = 0; i < (int)(_str.size()); ++i) {
        hash_code = 31 * hash_code + _str[i];
      }
      return hash_code;
    }

    static uint32_t hashCode(const char* str) {
      uint32_t hash_code = 0;
      int lenth = (int)strlen(str);
      for (int i = 0; i < lenth; ++i) {
        hash_code = 31 * hash_code + str[i];
      }
      return hash_code;
    }

    friend  bool  operator==(const thString &x, const thString &y) { 
      return strcmp((const char*)x, (const char*)y) == 0; 
    }
    friend  bool  operator==(const thString &x, const char* y) {
      if (y == NULL) return true;
      return strcmp((const char*)x, (const char*)y) == 0;
    }
    friend   bool   operator!=(const thString &x, const thString &y) {
      return strcmp((const char*)x, (const char*)y) != 0; 
    }
    friend   bool   operator!=(const thString &x, const char* y) {
      if (y == NULL) return true;
      return strcmp((const char*)x, (const char*)y) != 0; 
    }
    friend  bool  operator<(const thString&x, const thString&y) { 
      return strcmp((const char*)x, (const char*)y) < 0; 
    }
    thString  operator+(const thString&x) { 
      //std::printf("const thString  + operator called \n");
      thString t = *this; 
      t.append(x); 
      return t; 
    }
    thString    operator+(const char* src) {
      //std::printf("const char*  operator+ called \n");
      thString t = *this; 
      t.append(thString(src)); 
      return t;
    }
    thString&  operator+=(const thString&x) { 
      //std::printf("const thString += called \n");
      _str.append((const char*)x); 
      return *this; 
    }
    //char*    c_str() { *(&_str[0]+size())='\0'; return &_str[0]; }
    const char*    c_str() const { return _str.c_str(); }
    thString&  operator=(const char*b) { 
      //std::printf("const char = called \n");
      _str = b; 
      return *this; 
    }
    thString& operator=(const thString& b) {
      _str = b._str.c_str();
      return *this;
    }
    char  operator[](int n) { return _str[n]; }
    char  operator[](size_t n) { return _str[n]; }
    operator const char*() const { return _str.c_str(); }
    operator char*() { return &_str[0]; }
    char* operator()() { return &_str[0]; }
    thString& operator()(int in) { 
      (void)in;
      return *this; 
    }
    void operator()(thString& rev) { _str = rev; }
    void operator()(char* rev) { _str = rev;  }
    void operator()(const char* rev) { _str = rev; }
  private:
    __thstring _str;
  };
} // end thlib

#endif
