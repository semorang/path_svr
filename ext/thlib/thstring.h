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
   * EVC3.0 EVC4.0 ��� 
   */
  #include "thusewindows.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */
  #include "thusewindows.h"
  #include <stdio.h>
  #pragma warning(disable : 4786 4996)
#else
  /**
   * �ϴ�, �������� ���⿡ 
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
   * @note wide char, char, TCHAR�� ���ؼ� ��� ȣ�ϼ��� �־�� �Ѵ�. 
   *  ����� ȣ�ϼ� ����. ... 
   *  string ��å�� ������ char�� ����. 
   *  �ٸ�, TCHAR�� ��ȯ�ؾ��� �������� ��ȭ�Ͽ� ����ϵ��� �Ѵ�. 
   *  wide char, char, TCHAR�� ���ؼ� ��� ȣ�ϼ��� �־�� �Ѵ�. 
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
     * @brief ������ ��ġ���� ���̸�ŭ ���ڿ� ����.
     * @param src ������ ���ڿ�.
     * @param start ���� ��.
     * @param len ����.
     * @return -1 ����, 0 ����
     */
    int  copy(const thString &src, size_t start, size_t len) {
      if (src._str.size() < (unsigned)(start + len) ||
        len == 0)
        return -1;
      _str.assign(src._str.c_str(), start, len);
      return 0;
    }
    
    /**
     * @brief ������ ��ġ���� ���̸�ŭ ���ڿ� ����.
     * @param start ���� ��.
     * @param len ����.
     * @return ����� ���ڿ�.
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
     * @brief ���ϴ� ���ڿ� ��ġ���� ���̸�ŭ���� ġȯ.
     * @param start ���� ��.
     * @param len ����.
     * @return ġȯ�� ���ڿ�.
     */
    thString substr(size_t start, size_t len) const {
      __thstring sub_ret = _str.substr(start, len);
      thString ret = sub_ret.c_str(); 
      return ret;
    }

    /**
     * @brief ���ϴ� ���ڿ� ��ġ���� ���ڿ� ������ ġȯ.
     * @param start ���� ��.
     * @return ġȯ�� ���ڿ�.
     */
    thString substr(size_t start) const {
      __thstring sub_ret = _str.substr(start);
      thString ret = sub_ret.c_str();
      return ret;
    }
    
    /**
     * @brief string ��ü size ����.
     * @param size ������ size.
     */
    void resize(size_t size) { _str.resize(size); }

    /**
     * @brief ���ڿ� �߰�.
     * @param str �߰��� ���ڿ�.
     * @return ���� ���ڿ� ������.
     */
    size_t append(const thString& str) {
      _str.append(str);
      return str.size();
    }

    void insert(size_t pos, const char* i) {
      _str.insert(pos, i);
    }

    /**
     * @brief ���ڿ� ġȯ.
     * @param d1 ã�� ���ڿ�.
     * @param d2 ġȯ �� ���ڿ�.
     * @param start ���ڿ� ã�⸦ ������ ��ġ.
     * @return ġȯ Ƚ��.
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
     * @brief �ش� ���� �����
     * @param pattern ���ڿ� ����
     * @param start ã�� ������ ��ġ
     * @return ���� ���� ����
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
    /// ���ڿ� ������ ����. 
    size_t  size() const { return _str.size(); }
    /// ���ڿ� ������ ����.
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
     * @brief char token�� �����Ͽ� �ش� token�� ��Ÿ�� �� ������ ���ڿ� ����.
     * @param cseperator token ������.
     * @return token�� ��Ÿ���� �������� ���ڿ�.
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
     * @brief token���ڿ��� �����Ͽ� �ش� token�� ��Ÿ�� �� ������ ���ڿ� ����.
     * @param cseperator token ������ ���ڿ�.
     * @return token�� ��Ÿ���� �������� ���ڿ�.
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
     * @brief string�� �Ϻθ� ������ �о� ���������� ����.
     * @param start ���� ��ġ.
     * @param len ����.
     * @return �о���� string�� ������.
     * @note ������ atoi�� �ᵵ ������ ��. ;;;
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
     * @brief string�� token���� parser. 
     * @param tokens string���� �о token vector.
     * @param delimiter ��ū ������.
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
    
    /// string�� ù��° ���ڸ� �빮�ڷ�.
    void makeUpper() {
      strupr(&(this->_str[0]));
    }
    
    /// string�� ù��° ���ڸ� �ҹ��ڷ�.
    void makeLower() {
      strlwr(&(this->_str[0]));
    }
    
    /**
     * @brief ���ڿ��� �������� �ش� ���ڿ��� �ִٸ� �߶󳽴�.
     * @param ch �߶� ���ڿ�.
     * @return �߶� ���ڿ�.
     */
    thString&  trimLeft(thString ch = thString(' ')) {
      if (_str.size() > 0)
        _str = _str.substr(_str.find_first_not_of(ch.pointer()));
      else 
        _str = "";
      return *this; 
    }

    /**
     * @brief ���ڿ��� ���������� �ش� ���ڿ��� �ִٸ� �߶󳽴�.
     * @param ch �߶� ���ڿ�.
     * @return �߶� ���ڿ�.
     */
    thString&  trimRight(thString ch = thString(' ')) {
      if (_str.size() > 0)
        _str = _str.substr(0, _str.find_last_not_of(ch.pointer()) + 1);
      else 
        _str = "";
      return *this;
    }

    /**
     * @brief ���ڿ��� ��/���������� �ش� ���ڿ��� �ִٸ� �߶󳽴�.
     * @param ch �߶� ���ڿ�.
     * @return �߶� ���ڿ�.
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
