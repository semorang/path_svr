/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thsync.h"

#include "../../thstring.h"


namespace thlib {
#if defined(WIN32) && !defined(_WIN32_WCE)
int thSem::init(int key, int nsems, int mode) {
  int ret = 0;
  HANDLE ret_h;
  thString key_str;
  key_str.printf("%d%d", thSyncDef::TH_SYNC_PREFIX, key);
#ifdef UNICODE
  wchar_t conv_string[512] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, key_str(), key_str.size()+1
    , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", key_str());
#endif
  switch (mode) {
  case thSyncDef::TH_SYNC_CREATE:
    ret_h = CreateSemaphore(NULL, 1, nsems, conv_string);
    if (ret_h == NULL) {
      return thSyncErr::TH_SYNC_CFAIL;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      return thSyncErr::TH_SYNC_ALREADY;
    }
    break;
  case thSyncDef::TH_SYNC_USE:
    ret_h = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, conv_string);
    if (ret_h == NULL) {
      return thSyncErr::TH_SYNC_CFAIL;
    }
    break;
  default:
    return thSyncErr::TH_SYNC_INVARG;
  }
  _handle = ret_h;

  return thSyncErr::TH_SYNC_SUCCESS;
}


int thSem::release() {
  if (_handle != NULL) {
    CloseHandle(_handle);
    _handle = NULL;
  }
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thSem::enter() {
  int ret = ::WaitForSingleObjectEx(_handle, INFINITE, false);
  if (ret == WAIT_OBJECT_0) {
    return thSyncErr::TH_SYNC_SUCCESS;
  }
  return thSyncErr::TH_SYNC_FAIL;
}


int thSem::leave() {
  LPLONG tmpcnt = 0; // lint땜시 long을 사용하지 못함 
  int ret = ::ReleaseSemaphore(_handle, 1, tmpcnt);
  if (ret) {
    return thSyncErr::TH_SYNC_SUCCESS;
  }
  return thSyncErr::TH_SYNC_FAIL;
}
#endif


};  //  end of namespace thlib
