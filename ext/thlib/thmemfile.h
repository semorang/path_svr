/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thmemfile_h_
#define _thmemfile_h_



#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #include "thusewindows.h"
  #include "thwinfile.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "thusewindows.h"
  #include "thwinfile.h"
  #pragma warning(disable : 4786)
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif

#include <map>
#include "thstring.h"
#include "thstdfile.h"
#include "thmemalloc.h"

namespace thlib {
  template<class T> class thMemFile;
  typedef thlib::thMemFile<thlib::thStdFile> thStdMemFile;

#if defined(_WIN32_WCE)
  typedef thlib::thMemFile<thlib::thWinFile> thWinMemFile;


#elif defined(WIN32)
  
  typedef thlib::thMemFile<thlib::thWinFile> thWinMemFile;
#endif

  /**
   * 일단 전체 cache하는 것만 구현 하도록 하자. 
   * 누군가 나머지는 구현 하겠지. . . 
   */
  class IdNSize { 
    public: 
      int _id; 
      size_t _size; 
  };
  typedef std::map<thString, IdNSize> OpenFileList;
  /// reading 하는 것만 지원 함. 
  template<class file_type = thStdFile>
  class thMemFile {
  public:
    thMemFile() {
      _total_size = 0;
      _cur_pointer = 0;
      //_buf_size = 512 * 1024;
      _buf_size = -1;
      _buf_start_offset = 0;
      _open_id = -1;
    }

    ~thMemFile() {
      _total_size = 0;
      _cur_pointer = 0;
      _buf_size = -1;
      _buf_start_offset = 0;
      _open_id = -1;
    }

    static void mem_clear() {
      OpenFileList::iterator it;
      it = _open_list.begin();
      while (it != _open_list.end()) {
        int id = (*it).second._id;
#if 0
        thString msg;
        msg.printf("delete %s\n", (*it).first.c_str());
        perror(msg.c_str());
#endif
//        delete[] _mem_buf[id];
        MemPolicy<char>::memfree(_mem_buf[id]);
        _mem_buf[id] = NULL;
        ++it;
      }
      _open_list.clear();
    }

  public:
    int open(const char* name, const char *mode="rb", const int bs=-1) {
      /// search file name 
      thString file_name(name);
      OpenFileList::iterator it;
      it = _open_list.find(name);
      if (it == _open_list.end()) {
        int err = _file_pointer.open(name, mode, bs);
        if (err == -1) return -1;
        _total_size = _file_pointer.size();
        _open_id = int(_open_list.size());
        if (_mem_buf[_open_id] != NULL) {
          /// reopen fail
          return -1;
        }
        /** 
         * bs != -1 
         */
        if (bs == -1) {
          _buf_size = _total_size;
        } else {
          _buf_size = bs;
        }
//        _mem_buf[_open_id] = new char[_buf_size];
        _mem_buf[_open_id] = MemPolicy<char>::alloc(_buf_size);
        if (_mem_buf[_open_id] == NULL) {
          /// mem alloc fail
          return -1;
        }
        _file_pointer.read(_mem_buf[_open_id], _buf_size);
        /// insert item
        IdNSize in_obj;
        in_obj._id = _open_id;
        in_obj._size = _total_size;
        _open_list[name] = in_obj;
#if 0
        thString msg;
        msg.printf("new %s\n", name);
        perror(msg.c_str());
#endif
      } else {
        _open_id = (*it).second._id;
        _total_size = (*it).second._size;
        if (bs == -1) {
          _buf_size = _total_size;
        } else {
          _buf_size = bs;
        }
#if 0
        thString msg;
        msg.printf("reopen %s\n", (*it).first.c_str());
        perror(msg.c_str());
#endif
      }
      //_buf_size = _total_size;
      _cur_pointer = 0;
      _buf_start_offset = _cur_pointer;
      return 0;
    }
    int open(char* name, const char *mode="rb", const int bs=-1) {
      return open((const char*)name, mode, bs);
    }
    int close() {
      _total_size = 0;
      _cur_pointer = 0;
      _buf_size = 0;
      _buf_start_offset = 0;
      if (_file_pointer.isOpen()) {
        _file_pointer.close();
      }
      _open_id = -1;
      return 0;
    }
    size_t seek(size_t lPoint) {
      _cur_pointer = lPoint;
      size_t max_buf_offset = _buf_start_offset + _buf_size;
      if (_cur_pointer > max_buf_offset &&
          _cur_pointer < _total_size) {
        _buf_start_offset = _cur_pointer;
        memset(_mem_buf[_open_id], 0x00, _buf_size);
        _file_pointer.seek(lPoint);
        _file_pointer.read(_mem_buf[_open_id], _buf_size);
        // read item
      } else if (_cur_pointer < _buf_start_offset) {
        _buf_start_offset = _cur_pointer;
        memset(_mem_buf[_open_id], 0x00, _buf_size);
        _file_pointer.seek(lPoint);
        _file_pointer.read(_mem_buf[_open_id], _buf_size);
      }
      return _cur_pointer;
    }
    size_t seek(size_t lPoint, int whence) {
      if (whence==SEEK_SET){
        return seek(lPoint);
      }
      if (whence==SEEK_CUR){ // SEEK_CUR 구현
        return seek(_cur_pointer+lPoint);
      }

      return 0; /// 지원하지 않음.....
//      return _file_pointer.seek(lPoint, where);
    }
    /// 지원하지 않아 
    int flush(int in) {
      (void)in;
      return 0;
    //  return _file_pointer.flush();
    }
    /**
     * @return non-zero, if success
     */
    int eof() {
      return _cur_pointer == _total_size ? 1 : 0;
    }
    int printf(const char* , ...) {
      return 0;
    }
    size_t size() {
      return _total_size;
    }
    size_t position() {
      return _cur_pointer;
      //return _file_pointer.position();
    }
    int isOpen() {
//      if (_open_id != -1) 
//        return 1;
      return _open_id != -1 ? 1 : 0;
    }
    size_t read(char *buff, size_t size) {
      size_t read_size = 0;
      size_t next_pointer = 0;
      size_t max_buf_offset = _buf_start_offset + _buf_size;

      if (_cur_pointer == _total_size) 
        return read_size;
      next_pointer = _cur_pointer + size;
#if 0
      if (next_pointer > _total_size) {
        next_pointer = _total_size;
      }
#endif
    
      if (next_pointer <= max_buf_offset) {
        /// 버퍼내에 읽으려는 놈이 있을 경우 랍니다. 
        memcpy(buff, _mem_buf[_open_id] + 
            _cur_pointer - _buf_start_offset, size);
        _cur_pointer += size;
        read_size = size;
      } else if (next_pointer > max_buf_offset) {
        size_t first_read = max_buf_offset - _cur_pointer;
        if (first_read > 0) {
          memcpy(buff, _mem_buf[_open_id] + 
              _cur_pointer - _buf_start_offset, first_read);
          _cur_pointer += first_read;
          read_size += first_read;
        }
        ///< _total_size, _buf_size가 같다면 아래 내용을 할 필요없음
        if (_total_size == _buf_size) {
          return read_size;
        }

        _buf_start_offset = _cur_pointer;
        if (size-first_read > _buf_size) {
          /// 넘어 선 경우 
          size_t left2read = size - read_size;
          size_t cnt = left2read / _buf_size;
          size_t left = left2read % _buf_size;

          size_t i = 0;
          for (; i < cnt; ++i) {
            memset(_mem_buf[_open_id], 0x00, _buf_size);
            read_size += _file_pointer.read(_mem_buf[_open_id], _buf_size);
            memcpy((char*)(buff) + first_read + 
                i*_buf_size, _mem_buf[_open_id], _buf_size);
            _cur_pointer += _buf_size;
            _buf_start_offset = _cur_pointer;
          }

          memset(_mem_buf[_open_id], 0x00, _buf_size);
          _file_pointer.read(_mem_buf[_open_id], _buf_size);
          memcpy((char*)(buff)+first_read + 
              i*_buf_size, _mem_buf[_open_id], left);
          _cur_pointer += left;
          read_size += left;
        } else {
          memset(_mem_buf[_open_id], 0x00, _buf_size);
          _file_pointer.read(_mem_buf[_open_id], _buf_size);
          memcpy((char*)(buff)+first_read, _mem_buf[_open_id] + 
              _cur_pointer-_buf_start_offset, size - first_read);
          _cur_pointer += (size-first_read);
          read_size += (size-first_read);
        }
      }
      return read_size;
    }
    size_t readline(char* buff, size_t size) {
            /// 현 위치를 잠시 기억
            size_t pos = _cur_pointer;
            /// 512를 읽어
            size_t read_byte = read(buff, size);
            if (read_byte == 0)
                return -1;
            /// loop를 돌면서 한 라인의 끝이 어딘지 검사 해
            size_t i;
            for (i = 0; i < size; ++i) {
                if (buff[i] == '\n') {
                    pos += i;
                    buff[i] = 0x00;
                    break;
                }
            }
            /// \n을 null로 변경하고 다음에 읽을때는 그 다음 부터 읽어야 한다
            _cur_pointer = pos+1;
            return i;
    }

    size_t write(char *, size_t) {
      return 0;
//      return _file_pointer.write(buff, size);
    }
    size_t write(const char* , size_t) {
      return 0;
//      return _file_pointer.Write(buff, size);
    }
    char* address() { return _mem_buf[_open_id]; }
    
  private:
    file_type       _file_pointer;
     static OpenFileList   _open_list;
    static char*    _mem_buf[512];
    int             _open_id;
    size_t           _total_size;
    size_t           _cur_pointer;
    size_t           _buf_size;
    size_t           _buf_start_offset;
  };
}


#endif

