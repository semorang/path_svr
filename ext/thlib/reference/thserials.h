/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THSERIAL_
#define _THSERIAL_

#include <termio.h>
#include <unistd.h>

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

namespace thlib {
class thSerial {
 protected:
  static const char thPORT[4][15];
 public:
  thSerial():_file_dsc(-1), _total_read_size(0) {}
  ~thSerial() {
    if (isOpen()) 
      close();
  }

 public:
  int open(int port);
  int setConfig(int baud_rate = B4800);
  int close() { 
    // 초기 setting 값으로 복귀
    if (tcsetattr(_file_dsc, TCSANOW, &_old_term) == -1) return -1;    
    if (::close(_file_dsc) == -1) return -1; 
    return 1;
  }
  int read(char* buff, uint32_t size);
  int write(char* buff, uint32_t size);
  int isOpen() { return _file_dsc != -1 ? 1 : 0; }
  int baudRateCheck();
 private:
  int _file_dsc;
  struct termios _old_term;
  // read size를 관리 하기 위해서 .
  char _char_pool[256];
  int _total_read_size;
};

inline int thSerial::open(int port) {
  int flags;
  flags = O_RDWR;
  flags |= O_NOCTTY;
  _file_dsc = ::open(thPORT[port], flags);
  if (_file_dsc == -1) return -1;
  return 0;
}

inline int thSerial::setConfig(int baud_rate) {
  if (!isOpen()) return -1;

  int flags = fcntl(_file_dsc, F_GETFL, 0);
  if (fcntl(_file_dsc, F_SETFL, flags | O_NONBLOCK) == -1)
    return -1;
  // default setting
  // cgsetispeed 로 baudrate setting 가능 하다. 
  struct termios term_set;

  if (tcgetattr(_file_dsc, &term_set) == -1)
    return -1;
  if (tcgetattr(_file_dsc, &_old_term) == -1)
    return -1;

  // flush out any garbage left....
  if (tcflush(_file_dsc, TCIOFLUSH) == -1)
    return -1;

  switch (baud_rate) {
    case 2400:
      term_set.c_cflag = B2400;
      break;
    case 4800:
      term_set.c_cflag = B4800;
      break;
    case 9600:
      term_set.c_cflag = B9600;
      break;
    case 19200:
      term_set.c_cflag = B19200;
      break;
    case 38400:
      term_set.c_cflag = B38400;
      break;
    case 57600:
      term_set.c_cflag = B57600;
      break;
    case 115200:
      term_set.c_cflag = B115200;
      break;
    default:
      return -1;
  };
  // 흐름재어를 함, 8 bit, 모뎀제어를 하지 않음, 문자 순신 가능 모드  
  //term_set.c_cflag |= CRTSCTS | CS8 | CLOCAL | CREAD;
  term_set.c_cflag |= CS8 | CLOCAL | HUPCL | CREAD;
#if 0
  term_set.c_cc[VTIME] = 0; // 문자 사이의 timer disable
  term_set.c_cc[VMIN] = 2; // 최소 5문자 
#endif

  term_set.c_iflag = IGNPAR | ICRNL;
  
  if (tcsetattr(_file_dsc, TCSANOW, &term_set) == -1)
    return -1;

  return 0;
}

#if 0
inline int thSerial::read(char* buff, uint32_t size) {
  if (_file_dsc == -1) return 0;
  const int ITEM_SIZE = 80;
  char tmp[ITEM_SIZE*2];
  memset(tmp, 0x00, ITEM_SIZE*2);
  int read_size = 0;
  int item_size = 0;
  for (int i = 0; i < 20; ++i) {
    item_size = ::read(_file_dsc, tmp, ITEM_SIZE);
    memcpy(&buff[read_size], tmp, item_size);
    read_size += item_size;
    if (read_size > ITEM_SIZE) {
      break;
    }
  }
  return read_size;
}
#endif

inline int thSerial::write(char* buff, uint32_t size) {
  if (_file_dsc == -1) return 0;
  return ::write(_file_dsc, buff, size);
}

#if 0
inline bool thSerial::setConfig(int baud_rate) {
  if (isOpen()) return false;
  
  struct termios term_set;
  if (tcgetattr(_file_dsc, &term_set) == -1) 
    return -1;
  // baud rate setting 
  cfsetispeed(&term_set, baud_rate);
  cfsetospeed(&term_set, baud_rate);

  // byte bit seting 
  if (term_set
}
#endif

} // end namespace 

#endif


#if 0
int main(int argc, char** argv) {
  thSerial test;
  int ret = test.open(1); // com port 1 open;
  if (ret == -1) {
    printf("error open\n");
    return 0;
  }
  if (!test.setConfig(4800)) {
    printf("error config\n");
    return 0;
  }

  fd_set read_fs;
  FD_ZERO(&read_fs);

  char buffer[2048];
  memset(buffer, 0, sizeof(buffer));

  while (1) {
    FD_SET(ret, &read_fs);
    select(ret+1, &read_fs, NULL, NULL, NULL);
    if (FD_ISSET(ret, &read_fs)) {
      memset(buffer, 0, sizeof(buffer));
      int size = test.read(buffer, 2048);
      if (size > 0) {
        printf("%s", buffer);
      }
    }
  }
  return 0;
}
#endif

