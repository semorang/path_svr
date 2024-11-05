/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THDIR_H__
#define _THDIR_H__


/** 
 * afx.h���� time.h�� include�ϴµ� STL�� time.h�� ���� include�� 
 * �Ǿ�� �� �� �մϴ�. �׷��� �Լ��� ���������� ã�� ����
 */
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
  #include <errno.h>
  #include <direct.h>
  #include "thusewindows.h"
  #pragma warning(disable : 4786)
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
  #include <errno.h>
  #include <direct.h>
#endif


#include "../../thstring.h"
#include "../../thfile.h"


namespace thlib {
class thFS {
 public:
  enum read_method { THFS_NORECUR = 0, THFS_RECUR = 1 };

 public:
  static int makedir(thString& path, uint16_t mode = 0) {
#if defined(_WIN32_WCE)
    /**
     * EVC3.0 EVC4.0 ��� 
     */
    wchar_t conv_string[1024] = { L"", };
    uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
    ::MultiByteToWideChar(acp, 0, path(), path.size()+1
      , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));          
    if (::CreateDirectory(conv_string, NULL) == 0) {
      DWORD lerr = ::GetLastError();
      if (lerr != ERROR_ALREADY_EXISTS) {
        return -1;
      }
    }
#elif defined(WIN32)
    /**
     * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
     */
    if (_mkdir(path()) != 0) {
      if (errno != EEXIST) {
        return -1;
      }
    }
#endif
    
    return 0;
  }


  static int modeopen(thFile<>& file, thString& path
          , char* opt, int mode = 0) {
    int ret = file.open(path(), opt);
    return ret;
  }

  static int list_in_dir(std::vector<thString>& list
        , thString& ext, int recur = THFS_NORECUR) {
    int ret = 0;
    WIN32_FIND_DATA fdata = {0, };
#ifdef UNICODE
    wchar_t conv_string[2048] = { L"", };
    uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
    ::MultiByteToWideChar(acp, 0, ext(), (int)ext.size()+1
      , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
    char conv_string[1024] = { 0, };
    snprintf(conv_string, sizeof(conv_string), "%s", ext());
#endif

    HANDLE f_hdl = ::FindFirstFile(conv_string, &fdata);
    if (f_hdl == INVALID_HANDLE_VALUE) {
      return 0;
    }

    thString fname;
    while (true) {
      if (fdata.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
        ret = list_in_dir(list, ext, recur);
        if (ret) {
          return ret;
        }
      } else {
#ifdef UNICODE
        char conv_string[512] = { 0, };
        uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
        WideCharToMultiByte(CP_ACP, 0, fdata.cFileName, -1, 
          conv_string, 512, NULL, NULL);
#else
        char conv_string[512] = { 0, };
        snprintf(conv_string, sizeof(conv_string), "%s", fdata.cFileName);
#endif
        fname.printf("%s", fdata.cFileName);
        list.push_back(fname);
      }
      if (!::FindNextFile(f_hdl, &fdata)) {
        break;
      }
    }

    ::FindClose(f_hdl);

    return 0;
  }
};
};


#endif

