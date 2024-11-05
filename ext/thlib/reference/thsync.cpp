/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thsync.h"

#include "thlib/thstring.h"



/**
 *  implementation of class thlib::thNoSync
 */
int thlib::thNoSync::init(size_t key, int nsems, int mode) {
  /*int ret = 0;*/
  (void) nsems; /* unused value */
  (void) mode; /* unsued value */
  thlib::thString tmpkey;
  tmpkey.printf("%d%u", thSyncDef::TH_SYNC_PREFIX, key);
  _key = atoi(tmpkey.c_str());

  return thSyncErr::TH_SYNC_SUCCESS;
}

int thlib::thNoSync::release() {
  _key = -1;
  return thSyncErr::TH_SYNC_SUCCESS;
}

int thlib::thNoSync::enter() {
  return thSyncErr::TH_SYNC_SUCCESS;
}

int thlib::thNoSync::leave() {
  return thSyncErr::TH_SYNC_SUCCESS;
}



/**
 *  implementation of class thlib::thSem
 */

int thlib::thSem::init(size_t key, int nsems, int mode) {
  int ret = 0;
  sem_t* shsem = (sem_t*)key;
  switch (mode) {
  case thSyncDef::TH_SYNC_CREATE:
    // sencond arg 0 is interprocess semaphore
    ret = sem_init(shsem, 0, nsems);
    if (ret == -1) {
      return thSyncErr::TH_SYNC_CFAIL;
    }
    _own_sem = thSyncDef::TH_SYNC_PROWN;
    break;
  case thSyncDef::TH_SYNC_USE:
    _own_sem = thSyncDef::TH_SYNC_THOWN;
    break;
  default:
    return thSyncErr::TH_SYNC_INVARG;
  }
  _sem = shsem;

  /*
  thlib::thString tmpkey;
  tmpkey.printf("%d%d", thSyncDef::TH_SYNC_PREFIX, key);
  int i_key = atoi(tmpkey());
  switch (mode) {
  case thSyncDef::TH_SYNC_CREATE:
    ret = semget(i_key, nsems, 0600 | IPC_CREAT | IPC_EXCL);
    if (ret == -1) {
      if (errno == EEXIST) {
        return thSyncErr::TH_SYNC_ALREADY;
      }
      return thSyncErr::TH_SYNC_CFAIL;
    }
    break;
  case thSyncDef::TH_SYNC_USE:
    ret = semget(i_key, nsems, 0600);
    if (ret == -1) {
      if (errno == ENOENT) {
        return thSyncErr::TH_SYNC_NOEXIST;
      }
      return thSyncErr::TH_SYNC_FAIL;
    }
    break;
  default:
    return thSyncErr::TH_SYNC_INVARG;
  }
  _key = ret;
  */

  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thSem::release() {
  int ret = 0;
  if (_sem != NULL && (_own_sem == thSyncDef::TH_SYNC_THOWN ||
        _own_sem == thSyncDef::TH_SYNC_PROWN)) {
    ret = sem_destroy(_sem);
    if (ret) {
      return thSyncErr::TH_SYNC_REMOVE;
    }
  }
  _sem = NULL;
  _key = -1;
  _own_sem = thSyncDef::TH_SYNC_NOOWN;

  /*
  semun dummy;
  int ret = semctl(_key, 0, IPC_RMID, dummy);
  if (ret) {
    return thSyncErr::TH_SYNC_REMOVE;
  }
  */
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thSem::enter() {
  int ret = sem_wait(_sem);
  if (ret) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  /*
  sembuf sb = {0, -1, SEM_UNDO};
  if (semop(_key, &sb, 1) == -1) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  */
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thSem::leave() {
  int ret = sem_post(_sem);
  if (ret) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  /*
  sembuf sb = {0, 1, SEM_UNDO};
  if (semop(_key, &sb, 1) == -1) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  */
  return thSyncErr::TH_SYNC_SUCCESS;
}




/**
 *  implementation of class thlib::thMutex
 */

int thlib::thMutex::init(size_t, int, int) {
  int ret = pthread_mutex_init(&_mutex, NULL);
  if (ret) {
    return thSyncErr::TH_SYNC_CFAIL;
  }
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thMutex::release() {
  int ret = pthread_mutex_destroy(&_mutex);
  if (ret) {
    return thSyncErr::TH_SYNC_REMOVE;
  }
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thMutex::enter() {
  int ret = pthread_mutex_lock(&_mutex);
  if (ret) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  return thSyncErr::TH_SYNC_SUCCESS;
}


int thlib::thMutex::leave() {
  int ret = pthread_mutex_unlock(&_mutex);
  if (ret) {
    return thSyncErr::TH_SYNC_FAIL;
  }
  return thSyncErr::TH_SYNC_SUCCESS;
}








