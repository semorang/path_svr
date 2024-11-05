/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thSerialS.h"

const char thlib::thSerial::thPORT[4][15] = {
  "",
  "/dev/usb/tts/0",
  "/dev/usb/tts/0",
  "/dev/ttyS1"
};

int thlib::thSerial::read(char* buff, uint32_t size) {
  if (_file_dsc == -1) return 0;
#if 0 
  const int SZ = 128;
  char tmp[SZ];
  memset(tmp, 0x00, SZ);
  int item_size = ::read(_file_dsc, tmp, SZ);
  if (item_size < 1)  item_size = 0;
  if (_total_read_size + item_size > SZ) {
    memcpy(&_char_pool[_total_read_size], tmp, item_size);
    int ret_size = _total_read_size + item_size;
    memcpy(buff, _char_pool, ret_size);
    _total_read_size = 0;
    return ret_size;
  } 
  memcpy(&_char_pool[_total_read_size], tmp, item_size);
  _total_read_size += item_size;
  return 0;
#endif
#if 1
  _total_read_size = 0;
  const int SZ = 2045;
  char pool[SZ];
  memset(pool, 0x00, SZ);
  char tmp[1024];
  memset(tmp, 0x00, 1024);
  int item_size = ::read(_file_dsc, tmp, 1024);
  if (item_size < 1) {
    return 0;
  }
//  pool(&pool[_total_read_size], tmp, item_size);
//  _total_read_size += item_size;
  while (item_size > 1) {
    memcpy(&pool[_total_read_size], tmp, item_size);
    _total_read_size += item_size;
    memset(tmp, 0x00, 1024);
    item_size = ::read(_file_dsc, tmp, 1024);
  }

  memcpy(buff, pool, _total_read_size);
  return _total_read_size;

#endif
#if 0 
  return ::read(_file_dsc, buff, 2045);
#endif
}



