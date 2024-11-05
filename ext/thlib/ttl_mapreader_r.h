#ifndef TTL_MAPREADER_R_H_
#define TTL_MAPREADER_R_H_

#include "ttl_mapinfo_r.h"

///////////////////////////////////////////////////////////////////////////////
// traffic index reader (using by happyway engine)
///////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32_WCE)
    /**
     * EVC3.0 EVC4.0 ���
     */
    template<class file_type = thlib::thWinFile>
#elif defined(WIN32)
    /**
     * EVC�� �ƴ� �͵��� windows 32bit�� ���
     */
    template<class file_type = thlib::thWinFile>
#elif defined(ANDROID) || defined(_LINUX_PND_)
    template<class file_type = thlib::thStdFile>
#else
    /**
     * �ϴ�, �������� ���⿡
     */
    template<class file_type = thlib::thStdMemFile>
#endif


/**
 * TTL Index Reader
  */
struct ttlIDXReader {
public:
   ttlIDXReader() {
    _last_mapid = -1;
    _last_linkid = -1;
    _map_cntl = NULL;
  }
   virtual ~ttlIDXReader() {
      destroy();
   }

   void destroy() {
    for (size_t i = 0; i < ttlIndexMapCntl::F_TR_NUM; ++i) {
      if (_file_list[i].isOpen()) {
        _file_list[i].close();
      }
    }
   }

   int init(const char* mapdir) {
    int error = 0;
    _map_cntl = ttlIndexMapCntl::instance();
    error = _map_cntl->init(mapdir);
    if (error != 0) {
      return error;
    }
    /// file list open
    thlib::thString file_path;
    for (size_t i = 0; i < ttlIndexMapCntl::F_TR_NUM; ++i) {
	  /// mapreader�� singleton��ü�� ���ܰ����...file close�ʿ�!!!
	  if (_file_list[i].isOpen()) {
        _file_list[i].close();
      }
      file_path.printf("%s/%s", mapdir, _map_cntl->_file_name[i]);
      if (_file_list[i].open(file_path, "rb") < 0) {
        return -EFILE_OPEN;
      }
    }
    return 0;
  }

  template <class TRType>
  size_t size(TRType& obj, int mapid, int rank) {
    return _map_cntl->size(obj, mapid, rank);
  }

  /**
   * cache���� �����. (rank 0, 1 index rec, rank 2 index rec)
   */
   //template <class TRType>
   //int loadblock(TRType& obj, char* data, int mapid, int rank) {
   // size_t this_size = _map_cntl->size(obj, mapid, rank);
   // size_t ds = _map_cntl->ds(obj, mapid, rank);
   // /// file list���� read
   // int file_idx = ttlIndexMapCntl::F_TR_IDX_0 + rank;
   // if (_file_list[file_idx].read(data, ds, this_size) != this_size) {
   //   return -EFILE_READ;
   // }
   // return 0;
   //}

  /**
   * cache���� �����. (rank 0, 1 match list data)
   */
  //int loadblock(thTTLLinkMatchRec& obj, char* data, int mapid, int rank) {
  //  size_t this_size = _map_cntl->size(obj, mapid, rank);
  //  size_t ds = _map_cntl->ds(obj, mapid, rank);
  //  /// file list���� read
  //  int file_idx = ttlIndexMapCntl::F_TR_ID_MATCH_0 + rank;
  //  if (_file_list[file_idx].read(data, ds, this_size) != this_size) {
  //    return -EFILE_READ;
  //  }
  //  return 0;
  //}

  /**
   * file read (rank 0, 1 index rec) 
   */
  //int read(thTTLIndexU& obj, int mapid, int id, int rank) {
  //  size_t this_size = sizeof(obj);
  //  size_t ds = _map_cntl->ds(obj, mapid, id, rank);
  //  /// file list���� read
  //  int file_idx = ttlIndexMapCntl::F_TR_IDX_0 + rank;
  //  if (_file_list[file_idx].read((char*)&obj, ds, this_size) != this_size) {
  //    return -EFILE_READ;
  //  }
  //  /// rank 0, 1 idx rec copy
  //  _last_mapid = mapid;
  //  _last_linkid = id;
  //  memcpy((char*)&_last_idx_info_u, &obj, sizeof(_last_idx_info_u));
  //  return 0;
  //}

  /**
   * file read (rank 2 index rec)
   */
  int read(thTTLIndexL& obj, int mapid, int id, int rank) {
    size_t this_size = sizeof(obj);
    size_t ds = _map_cntl->ds(obj, mapid, id, rank);
    /// file list���� read
    int file_idx = ttlIndexMapCntl::F_TR_IDX_0 + rank;
    if (_file_list[file_idx].read((char*)&obj, ds, this_size) != this_size) {
      return -EFILE_READ;
    }
    return 0;
  }

  /**
   * file read: rank 0, 1 
   * @note: ���� ��/������ �ι� �� �ִµ�...
   *        �׽�Ʈ �غ��� ���߿� �ѹ��⸸ �־ �ɵ�...
   */
  //int read(thTTLLinkMatchRec& obj, int mapid, int id, int rank) {
  //  /// read index ����.
  //  if (!is_lastitem(mapid, id)) {
  //     int error = read(_last_idx_info_u, mapid, id, rank);
  //     if (error != 0) {
  //       return error;
  //     }
  //  }
  //  size_t match_cnt = _last_idx_info_u._fw_cnt + _last_idx_info_u._bw_cnt;
  //  size_t this_size = sizeof(obj) * match_cnt; ///< ��/�� 
  //  size_t ds = _map_cntl->ds(obj, mapid, rank) + 
  //              (_last_idx_info_u._lindex * sizeof(obj));
  //  /// file list���� read
  //  int file_idx = ttlIndexMapCntl::F_TR_ID_MATCH_0 + rank;
  //  if (_file_list[file_idx].read((char*)&obj, ds, this_size) != this_size) {
  //    return -EFILE_READ;
  //  }
  //  return 0;
  //}

  /**
   * up link
   */
  //int set_lastitem_idxu(int mapid, int linkid, thTTLIndexU item) {
  //  _last_mapid = mapid;
  //  _last_linkid = linkid;
  //  memcpy(&_last_idx_info_u, &item, sizeof(item));
  //  return 0;
  //}
private:
  //bool is_lastitem(int mapid, int linkid) {
  //  if (_last_mapid == mapid && _last_linkid == linkid) {
  //    return true;
  //  }
  //  return false;
  //}
public:
  thlib::thFile<file_type> _file_list[ttlIndexMapCntl::F_TR_NUM];
private:
  ttlIndexMapCntl* _map_cntl; 
  int              _last_mapid;      ///< rank 0, 1
  int              _last_linkid;     ///< rank 0, 1
  thTTLIndexU      _last_idx_info_u; ///< rank 0, 1
};


///////////////////////////////////////////////////////////////////////////////
// ttl id reader (using by real traffic control)
///////////////////////////////////////////////////////////////////////////////

#if defined(_WIN32_WCE)
    /**
     * EVC3.0 EVC4.0 ���
     */
    template<class file_type = thlib::thWinFile>
#elif defined(WIN32)
    /**
     * EVC�� �ƴ� �͵��� windows 32bit�� ���
     */
    template<class file_type = thlib::thWinFile>
#elif defined(ANDROID) || defined(_LINUX_PND_)
    template<class file_type = thlib::thStdFile>
#else
    /**
     * �ϴ�, �������� ���⿡
     */
    template<class file_type = thlib::thStdMemFile>
#endif
/**
 * �������� ID Reader
  */
struct ttlIDReader {
  ttlIDReader() {
    _map_cntl = NULL;
  }
   virtual ~ttlIDReader() {
      destroy();
   }

   void destroy() {
    if (_file.isOpen()) {
      _file.close();
    }
   }

   int init(const char* mapdir) {
    int error = 0;
    _map_cntl = ttlIDMapCntl::instance();
    error = _map_cntl->init(mapdir);
    if (error != 0) {
      return error;
    }
    /// file list open
    /// mapreader�� singleton��ü�� ���ܰ����...file close�ʿ�!!!
	  if (_file.isOpen()) { 
      _file.close();
    }
    thlib::thString file_path;
    file_path.printf("%s/%s", mapdir, _map_cntl->_file_name);
    if (_file.open(file_path, "rb") < 0) {
      return -EFILE_OPEN;
    }
    return 0;
  }

  template <class TRType>
  size_t size(TRType& obj, size_t block_idx, int rank) {
    return _map_cntl->size(block_idx);
  }

  /**
   * @param block_idx cache���� ���Ǵ� slot_idx
   */
  //int loadblock(thTTLIDBlock& id_blk, char* data, 
  //              int block_idx, int rank = 2) {
  //  size_t this_size = _map_cntl->size(block_idx);
  //  size_t ds = _map_cntl->ds(block_idx);
  //  if (_file.read(data, ds, this_size) != this_size) {
  //    return -EFILE_READ;
  //  }
  //  return 0;
  // } 

  /**
   * ID Block Read. (Ȥ�� ���� ����.)
   * @param item_idx ������� ������, interface������ ���� ����.
   */
  //int read(unsigned long long* id_blk, int block_idx, int rank = 2) {
  //  size_t this_size = _map_cntl->size(block_idx);
  //  size_t ds = _map_cntl->ds(block_idx);       
  //  if (_file.read(id_blk, ds, this_size) != this_size) {
  //    return -EFILE_READ;
  //  }
  //  return 0;
  //}
  
  /**
   * ID �ϳ� �о����. 
   */
  int read(unsigned long long& ttlid, int block_idx, int item_idx, int rank = 2) {
    size_t this_size = sizeof(ttlid);
    size_t ds = _map_cntl->ds(block_idx) + item_idx * sizeof(ttlid);    
    if (_file.read((char*)&ttlid, ds, this_size) != this_size) {
      return -EFILE_READ;
    }
    return 0;
  }
  
public:
  thlib::thFile<file_type> _file;
private:
  ttlIDMapCntl*    _map_cntl; 
};

#endif
