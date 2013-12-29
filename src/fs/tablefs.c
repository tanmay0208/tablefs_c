//#ifdef SUPPORTED
#define FUSE_USE_VERSION 26

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <sys/time.h>
//#include <vector>
//#include <algorithm>
#include <pthread.h>
//#include <sstream>
#include "tablefs.h"
#include "util/myhash.h"
#include "util/mutexlock.h"
#include "util/socket.h"
//#include "leveldb/env.h"
//#include "leveldb/db.h"
//#include "leveldb/cache.h"
//#include "leveldb/write_batch.h"

//namespace tablefs {

struct tfs_file_handle_t {
  InodeCacheHandle* handle_;
  int flags_;
  int fd_;
};

typedef struct tfs_file_handle_t tfs_file_handle_t;  
void tfs_file_handle_t_constructor(tfs_file_handle_t *tfs_file_handle) {				//Struct constructor has to be called
  tfs_file_handle->handle_=NULL;
  tfs_file_handle->fd_=-1;
}

inline static void BuildMetaKey_tablefs(TableFS *tablefs,const tfs_inode_t inode_id,
                                const tfs_hash_t hash_id,
                                tfs_meta_key_t *key) {
  key->inode_id = inode_id;
  key->hash_id = hash_id;
}

inline static void BuildMetaKey_path(const char *path,
                                const int len,
                                const tfs_inode_t inode_id,
                                tfs_meta_key_t *key) {
  BuildMetaKey(inode_id, murmur64(path, len, 123), key);
}

/*inline static bool IsKeyInDir(const leveldb::Slice &key,
                              const tfs_meta_key_t &dirkey) {
  const tfs_meta_key_t* rkey = (const tfs_meta_key_t *) key.data();
  return rkey->inode_id == dirkey.inode_id;							//levelDB
}*/ 

const tfs_inode_header *GetInodeHeader(const char *value) {
  //return reinterpret_cast<const tfs_inode_header*> (value.data());		//return type handle
  return (const tfs_inode_header*) (value);		//return type handle
}

/*const tfs_stat_t *GetAttribute(std::string &value) {					//return type handle
  return reinterpret_cast<const tfs_stat_t*> (value.data());
}*/

size_t GetInlineData(char *value, char* buf, size_t offset, size_t size) {
  const tfs_inode_header* header = GetInodeHeader(value);		//typecast for compiling delete afterward
  size_t realoffset = TFS_INODE_HEADER_SIZE + header->namelen + 1 + offset;
  if (realoffset < strlen(value)) {
    if (realoffset + size > strlen(value)) {
      size = strlen(value) - realoffset;
    }
    memcpy(buf, value + realoffset , size);
    return size;
  } else {
    return 0;
  }
}

void UpdateIhandleValue(char *value,
                        const char* buf, size_t offset, size_t size) {
  if (offset > strlen(value)) {
    //value.resize(offset);				resize handle
  }
  //value.replace(offset, size, buf, size);			replace handle
}

void UpdateInodeHeader(char *value,
                       tfs_inode_header *new_header) {
  UpdateIhandleValue(value, (const char *) &new_header,0, TFS_INODE_HEADER_SIZE);
}

void UpdateAttribute(char *value,
                     const tfs_stat_t *new_fstat) {
  UpdateIhandleValue(value, (const char *) &new_fstat,
                     0, TFS_INODE_ATTR_SIZE);
}

void UpdateInlineData(char *value,
                      const char* buf, size_t offset, size_t size) {
  const tfs_inode_header* header = GetInodeHeader(value);
  size_t realoffset = TFS_INODE_HEADER_SIZE + header->namelen + 1 + offset;
  UpdateIhandleValue(value, buf, realoffset, size);
}

void TruncateInlineData(char *value, size_t new_size) {
  const tfs_inode_header* header = GetInodeHeader(value);
  size_t target_size = TFS_INODE_HEADER_SIZE + header->namelen + new_size + 1;
  //value.resize(target_size);
}

void DropInlineData(char *value) {
  const tfs_inode_header* header = GetInodeHeader(value);
  size_t target_size = TFS_INODE_HEADER_SIZE + header->namelen + 1;
  //value.resize(target_size);
}

void TableFS_SetState(TableFS *tablefs,FileSystemState* state) {
  tablefs->state_ = state;
}

int TableFS_FSError(TableFS *tablefs,const char *err_msg) {
  int retv = -errno;
#ifdef TABLEFS_DEBUG
  state_->GetLog()->LogMsg(err_msg);
#endif
  return retv;
}

void TableFS_InitStat(TableFS *tablefs,tfs_stat_t *statbuf,
                       tfs_inode_t inode,
                       mode_t mode,
                       dev_t dev) {
  statbuf->st_ino = inode;
  statbuf->st_mode = mode;
  statbuf->st_dev = dev;

  if (tablefs->flag_fuse_enabled) {
    statbuf->st_gid = fuse_get_context()->gid;
    statbuf->st_uid = fuse_get_context()->uid;
  } else {
    statbuf->st_gid = 0;
    statbuf->st_uid = 0;
  }

  statbuf->st_size = 0;
  statbuf->st_blksize = 0;
  statbuf->st_blocks = 0;
  if S_ISREG(mode) {
    statbuf->st_nlink = 1;
  } else {
    statbuf->st_nlink = 2;
  }
  time_t now = time(NULL);
  statbuf->st_atim.tv_sec = now;
  statbuf->st_ctim.tv_sec = now;
  statbuf->st_mtim.tv_sec = now;
}

/*tfs_inode_val_t TableFS_InitInodeValue(tfs_inode_t inum,
                                        mode_t mode,
                                        dev_t dev,
                                        leveldb::Slice filename) {
  tfs_inode_val_t ival;
  ival.size = TFS_INODE_HEADER_SIZE + filename.size() + 1;
  ival.value = new char[ival.size];
  tfs_inode_header* header = reinterpret_cast<tfs_inode_header*>(ival.value);
  InitStat(header->fstat, inum, mode, dev);
  header->has_blob = 0;
  header->namelen = filename.size();
  char* name_buffer = ival.value + TFS_INODE_HEADER_SIZE;
  memcpy(name_buffer, filename.data(), filename.size());
  name_buffer[header->namelen] = '\0';
  return ival;
}
*/
/*char *TableFS_InitInodeValue(const char *old_value,
                                    leveldb::Slice filename) {
  //TODO: Optimize avoid too many copies
  std::string new_value = old_value;
  tfs_inode_header header = *GetInodeHeader(old_value);
  new_value.replace(TFS_INODE_HEADER_SIZE, header.namelen+1,
                    filename.data(), filename.size()+1);
  header.namelen = filename.size();
  UpdateInodeHeader(new_value, header);

  return new_value;
}
*/

/*void TableFS_FreeInodeValue(tfs_inode_val_t &ival) {
  if (ival.value != NULL) {
    //delete [] ival.value;
    ival.value = NULL;
  }
}*/ 

/*bool TableFS_ParentPathLookup(const char *path,
                               tfs_meta_key_t &key,
                               tfs_inode_t &inode_in_search,
                               const char* &lastdelimiter) {
  const char* lpos;
  const char* rpos;
  bool flag_found = true;
  char* item;
  strcpy(lpos,path);
  inode_in_search = ROOT_INODE_ID;
  while ((rpos = strchr(lpos+1, PATH_DELIMITER)) != NULL) {
    if (rpos - lpos > 0) {
      BuildMetaKey(lpos+1, rpos-lpos-1, inode_in_search, key);
      if (!dentry_cache->Find(key, inode_in_search)) {
        {
          fstree_lock.ReadLock(key);
          char* result;
          int ret = metadb->Get(key.ToSlice(), result);
          if (ret == 1) {
            inode_in_search = GetAttribute(result)->st_ino;
            dentry_cache->Insert(key, inode_in_search);
          } else {
            errno = ENOENT;
            flag_found = false;
          }
          fstree_lock.Unlock(key);
          if (!flag_found) {
            return false;
          }
        }
      }
    }
    lpos = rpos;
  }
  if (lpos == path) {
    BuildMetaKey(NULL, 0, ROOT_INODE_ID, key);
  }
  lastdelimiter = lpos;
  return flag_found;
}
*/
bool TableFS_PathLookup(TableFS *tablefs,const char *path,
                         tfs_meta_key_t *key) {
  const char* lpos;
  tfs_inode_t inode_in_search;
/*  if (ParentPathLookup(path, key, inode_in_search, lpos)) {
    const char* rpos = strchr(lpos, '\0');
    if (rpos != NULL && rpos-lpos > 1) {
      BuildMetaKey(lpos+1, rpos-lpos-1, inode_in_search, key);
    }
    return true;
  } else {
    errno = ENOENT;
    return false;
  }*/
}

/*
bool TableFS_PathLookup(const char *path,
                         tfs_meta_key_t &key,
                         leveldb::Slice &filename) {
  const char* lpos;
  tfs_inode_t inode_in_search;
  if (ParentPathLookup(path, key, inode_in_search, lpos)) {
    const char* rpos = strchr(lpos, '\0');
    if (rpos != NULL && rpos-lpos > 1) {
      BuildMetaKey(lpos+1, rpos-lpos-1, inode_in_search, key);
      filename = leveldb::Slice(lpos+1, rpos-lpos-1);
    } else {
      filename = leveldb::Slice(lpos, 1);
    }
    return true;
  } else {
    errno = ENOENT;
    return false;
  }
}
*/
//#ifdef SUPPORTED

const int DEFAULT_SYNC_INTERVAL = 5;
volatile int stop_monitor_thread;
volatile int flag_monitor_thread_finish;
pthread_mutex_t     mtx_monitor;
pthread_cond_t      cv_monitor;

void do_monitor(LevelDBAdaptor* metadb) {
/*  std::string metric;
  if (metadb->GetMetric(&metric)) {
    const int metric_cnt = 13;
    int r[metric_cnt];
    std::stringstream ssmetric(metric);
    for (int i = 0; i < metric_cnt; ++i)
      ssmetric >> r[i];
    const char* metname[metric_cnt] = {
        "num_files", "num_size", "num_compact",
        "tot_comp_time", "tot_comp_read", "tot_comp_write",
        "num_write_op", "num_get_op", "filterhit",
        "minor_count", "log_writes", "log_syncs", "desc_syncs"
    };
    int now_time = (int) time(NULL);
    for (int ri = 0; ri < metric_cnt; ++ri) {
      char metricString[128];
      sprintf(metricString,
             "tablefs.%s %d %d\n", metname[ri], now_time, r[ri]);
      UDPSocket sock;
      try {
        sock.sendTo(metricString, strlen(metricString),
                    std::string("127.0.0.1"), 10600);
      } catch (SocketException &e) {
      }
    }
  }*/
}


void report_get_count(LevelDBAdaptor* metadb) {
/*  std::string metric;
  if (metadb->GetStat("leveldb.get_count_by_num_files", &metric)) {
    const int metric_cnt = 20;
    int r[metric_cnt];
    std::stringstream ssmetric(metric);
    for (int i = 0; i < metric_cnt; ++i)
      ssmetric >> r[i];
    for (int i = 0; i < metric_cnt; ++i) {
      int now_time = (int) time(NULL);
      char metricString[128];
      sprintf(metricString,
             "tablefs.get_count_%d %d %d\n", i, now_time, r[i]);
      UDPSocket sock;
      try {
        sock.sendTo(metricString, strlen(metricString),
                    std::string("127.0.0.1"), 10600);
      } catch (SocketException &e) {
      }
    }
  }
  if (metadb->GetStat("leveldb.false_get_count_by_num_files", &metric)) {
    const int metric_cnt = 20;
    int r[metric_cnt];
    std::stringstream ssmetric(metric);
    for (int i = 0; i < metric_cnt; ++i)
      ssmetric >> r[i];
    for (int i = 0; i < metric_cnt; ++i) {
      int now_time = (int) time(NULL);
      char metricString[128];
      sprintf(metricString,
             "tablefs.false_get_count_%d %d %d\n", i, now_time, r[i]);
      UDPSocket sock;
      try {
        sock.sendTo(metricString, strlen(metricString),
                    std::string("127.0.0.1"), 10600);
      } catch (SocketException &e) {
      }
    }
  }*/
}


/*void* monitor_thread(void *v) {
    LevelDBAdaptor* metadb= (LevelDBAdaptor*) v;

    struct timespec wait;
    memset(&wait, 0, sizeof(wait)); // ensure it's initialized  kai
    struct timeval now;
    char* err;

    pthread_mutex_lock(&(mtx_monitor));
    int ret = ETIMEDOUT;
    while (stop_monitor_thread == 0) {
        if (ret == ETIMEDOUT) {
          do_monitor(metadb);
        } else {
          if (stop_monitor_thread == 0) {
            fprintf(stderr, "Unexpected interrupt for monitor thread\n");
            stop_monitor_thread = 1;
          }
        }
        gettimeofday(&now, NULL);
        wait.tv_sec = now.tv_sec + DEFAULT_SYNC_INTERVAL;
        pthread_cond_timedwait(&(cv_monitor), &(mtx_monitor), &wait);
    }
    flag_monitor_thread_finish = 1;
    pthread_cond_broadcast(&(cv_monitor));
    pthread_mutex_unlock(&(mtx_monitor));
    return NULL;
}
*/

/*
void monitor_init(LevelDBAdaptor *mdb) {
    stop_monitor_thread = 0;
    flag_monitor_thread_finish = 0;
    pthread_mutex_init(&(mtx_monitor), NULL);
    pthread_cond_init(&(cv_monitor), NULL);

    int ret;
    pthread_t tid;
    if ((ret = pthread_create(&tid, NULL, monitor_thread, mdb))) {
        fprintf(stderr, "pthread_create() error: %d", ret);
        exit(1);
    }
    if ((ret = pthread_detach(tid))) {
        fprintf(stderr, "pthread_detach() error: %d", ret);
        exit(1);
    }
}
*/

void monitor_destroy() {
/*  pthread_mutex_lock(&(mtx_monitor));
  stop_monitor_thread = 1;
  pthread_cond_signal(&(cv_monitor));
  while (flag_monitor_thread_finish == 0) {
    pthread_cond_wait(&(cv_monitor), &(mtx_monitor));
  }
  pthread_mutex_unlock(&(mtx_monitor));

  pthread_mutex_destroy(&(mtx_monitor));
  pthread_cond_destroy(&(cv_monitor));*/
}

 void* TableFS_Init(TableFS *tablefs,struct fuse_conn_info *conn) {
  Logging_LogMsg(FileSystemState_GetLog(tablefs->state_),"TableFS initialized.\n");
  if (conn != NULL) {
    tablefs->flag_fuse_enabled = true;
  } else {
    tablefs->flag_fuse_enabled = false;
  }
  tablefs->metadb = FileSystemState_GetMetaDB(tablefs->state_);
  if (FileSystemState_IsEmpty(tablefs->state_)) {
    Logging_LogMsg(FileSystemState_GetLog(tablefs->state_),"TableFS create root inode.\n");
//    state_->GetLog()->LogMsg("TableFS create root inode.\n");
    tfs_meta_key_t key;
    //BuildMetaKey(NULL, 0, ROOT_INODE_ID, key);
    struct stat statbuf;
    lstat(ROOT_INODE_STAT, &statbuf);
   /* tfs_inode_val_t value = InitInodeValue(ROOT_INODE_ID,
          statbuf.st_mode, statbuf.st_dev, leveldb::Slice("\0"));
    if (metadb->Put(key.ToSlice(), value.ToSlice()) < 0) {
      state_->GetLog()->LogMsg("TableFS create root directory failed.\n");
    }
    FreeInodeValue(value);*/
  }
   
  tablefs->inode_cache =(InodeCache*)malloc(sizeof(InodeCache));
  InodeCache_constructor(tablefs->inode_cache,tablefs->metadb);
  tablefs->dentry_cache =(DentryCache*)malloc(sizeof(DentryCache));
  tfs_DentryCache_constructor(tablefs->dentry_cache,16384);  
  //dentry_cache = new DentryCache(16384);
  //monitor_init(metadb);
  return tablefs->state_;
}

void TableFS_Destroy(TableFS *tablefs,void * data) {
  monitor_destroy();
  do_monitor(tablefs->metadb);
  report_get_count(tablefs->metadb);
  if (tablefs->dentry_cache != NULL)
    free(tablefs->dentry_cache);
  if (tablefs->inode_cache != NULL) {
    free(tablefs->inode_cache);
  }
    Logging_LogMsg(FileSystemState_GetLog(tablefs->state_),"Clean write back cache.\n");
  //state_->GetLog()->LogMsg("Clean write back cache.\n");
  FileSystemState_Destroy(tablefs->state_);
  free(tablefs->state_);
}

int TableFS_GetAttr(TableFS *tablefs,const char *path, struct stat *statbuf) {
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path,&key)) {
     return TableFS_FSError(tablefs,"GetAttr Path Lookup: No such file or directory: %s\n");
  }
  int ret = 0;
  InodeMutex_ReadLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_READ);
  if (handle != NULL) {
//    *statbuf = *(GetAttribute(handle->value_));		Apan: LevelDB ,InodeCacheHandle,icache madhe ahe
#ifdef TABLEFS_DEBUG
  state_->GetLog()->LogMsg("GetAttr: %s, Handle: %x HandleMode: %d\n",
                            path, handle, handle->mode_);
  state_->GetLog()->LogMsg("GetAttr DBKey: [%d,%d]\n", key.inode_id, key.hash_id);
  state_->GetLog()->LogStat(path, statbuf);
#endif
  InodeCache_Release(tablefs->inode_cache, handle);
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

void TableFS_GetDiskFilePath(TableFS *tablefs,char *path, tfs_inode_t inode_id) {
  sprintf(path, "%s/%d/%d",
          FileSystemState_GetDataDir(tablefs->state_),
          (int) inode_id >> NUM_FILES_IN_DATADIR_BITS,
          (int) inode_id % (NUM_FILES_IN_DATADIR));
}

int TableFS_OpenDiskFile(TableFS *tablefs,const tfs_inode_header* iheader, int flags) {
  char fpath[128];
  TableFS_GetDiskFilePath(tablefs,fpath, iheader->fstat.st_ino);
  int fd = open(fpath, flags | O_CREAT, iheader->fstat.st_mode);
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("TableFS_OpenDiskFile: %s InodeID: %d FD: %d\n",
                           fpath, iheader->fstat.st_ino, fd);
#endif
  return fd;
}

int TableFS_TruncateDiskFile(TableFS *tablefs,tfs_inode_t inode_id, off_t new_size) {
  char fpath[128];
  TableFS_GetDiskFilePath(tablefs,fpath, inode_id);
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("TruncateDiskFile: %s, InodeID: %d, NewSize: %d\n",
                          fpath, inode_id, new_size);
#endif
  return truncate(fpath, new_size);
}

ssize_t TableFS_MigrateDiskFileToBuffer(TableFS *tablefs,tfs_inode_t inode_id,
                                         char* buffer,
                                         size_t size) {
  char fpath[128];
  TableFS_GetDiskFilePath(tablefs,fpath, inode_id);
  int fd = open(fpath, O_RDONLY);
  ssize_t ret = pread(fd, buffer, size, 0);
  close(fd);
  unlink(fpath);
  return ret;
}

int TableFS_MigrateToDiskFile(TableFS *tablefs,InodeCacheHandle* handle, int *fd, int flags) {
     int ret = 0;				//added for compiling
  /*const tfs_inode_header* iheader = GetInodeHeader(handle->value_);
  if (fd >= 0) {
    close(fd);
  }
  fd = TableFS_OpenDiskFile(tablefs,iheader, flags);
  if (fd < 0) {
    fd = -1;
    return -errno;
  }
  int ret = 0;
  if (iheader->fstat.st_size > 0 ) {
    const char* buffer = (const char *) iheader +
                         (TFS_INODE_HEADER_SIZE + iheader->namelen + 1);
    if (pwrite(fd, buffer, iheader->fstat.st_size, 0) !=
        iheader->fstat.st_size) {
      ret = -errno;
    }
    DropInlineData(handle->value_);
  }*/
  return ret;
}

void TableFS_CloseDiskFile(TableFS *tablefs,int fd_) {		//Apan:real arg was int &fd_
  close(fd_);
  fd_ = -1;
}

int TableFS_Open(TableFS *tablefs,const char *path, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Open: %s, Flags: %d\n", path, fi->flags);
#endif

  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"Open: No such file or directory\n");
  }
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = NULL;
  if ((fi->flags & O_RDWR) > 0 ||
      (fi->flags & O_WRONLY) > 0 ||
      (fi->flags & O_TRUNC) > 0) {
    handle = InodeCache_Get(tablefs->inode_cache,key, INODE_WRITE);
  } else {
    handle = InodeCache_Get(tablefs->inode_cache,key, INODE_READ);
  }

  int ret = 0;
  if (handle != NULL) {
    tfs_file_handle_t *fh = (tfs_file_handle_t*)malloc(sizeof(tfs_file_handle_t));
    tfs_file_handle_t_constructor(fh);
    fh->handle_ = handle;
    fh->flags_ = fi->flags;
    const tfs_inode_header *iheader= GetInodeHeader(handle->value_);

    if (iheader->has_blob > 0) {
        fh->flags_ = fi->flags;
        fh->fd_ = TableFS_OpenDiskFile(tablefs,iheader, fh->flags_);
        if (fh->fd_ < 0) {
          InodeCacheRelease(tablefs->inode_cache,handle);
          ret = -errno;
        }
    }
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Open: %s, Handle: %x, HandleMode: %d FD: %d\n",
                           path, handle, handle->mode_, fh->fd_);
#endif
    if (ret == 0) {
      fi->fh = (int)fh;			//Apan: Khali zhol distoy
      //fi->fh = (uint64_t)fh;
    } else {
      free(fh);
    }
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);


  return ret;
}

int TableFS_Read(TableFS *tablefs,const char* path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Read: %s\n", path);
#endif
  //tfs_file_handle_t* fh = reinterpret_cast<tfs_file_handle_t*>(fi->fh);
  tfs_file_handle_t* fh;				//Apan : Added for compiling ,delete afterwards
  InodeCacheHandle* handle = fh->handle_;
  InodeMutex_ReadLock(tablefs->fstree_lock,&handle->key_);
  const tfs_inode_header* iheader = GetInodeHeader(handle->value_);
  int ret;
  if (iheader->has_blob > 0) {
    if (fh->fd_ < 0) {
      fh->fd_ = TableFS_OpenDiskFile(tablefs,iheader, fh->flags_);
      if (fh->fd_ < 0)
        ret = -EBADF;
    }
    if (fh->fd_ >= 0) {
      ret = pread(fh->fd_, buf, size, offset);
    }
  } else {
    ret = GetInlineData(handle->value_, buf, offset, size);
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&handle->key_);
  return ret;
}

int TableFS_Write(TableFS *tablefs,const char* path, const char *buf, size_t size,
                   off_t offset, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Write: %s %lld %d\n", path, offset, size);
#endif

//  tfs_file_handle_t* fh = reinterpret_cast<tfs_file_handle_t*>(fi->fh);
  tfs_file_handle_t* fh;				//Apan : Added for compiling ,delete afterwards
  InodeCacheHandle* handle = fh->handle_;
  handle->mode_ = INODE_WRITE;
  InodeMutex_ReadLock(tablefs->fstree_lock,&handle->key_);
  const tfs_inode_header* iheader = GetInodeHeader(handle->value_);
  int ret = 0, has_imgrated = 0;
  int has_larger_size = (iheader->fstat.st_size < offset + size) ? 1 : 0;

#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Write: %s has_larger_size %d old: %d new: %lld\n", 
      path, has_larger_size, iheader->fstat.st_size, offset + size);
#endif

  if (iheader->has_blob > 0) {
    if (fh->fd_ < 0) {
      fh->fd_ = TableFS_OpenDiskFile(tablefs,iheader, fh->flags_);
      if (fh->fd_ < 0)
        ret = -EBADF;
    }
    if (fh->fd_ >= 0) {
      ret = pwrite(fh->fd_, buf, size, offset);
    }
  } else {
    if (offset + size > FileSystemState_GetThreshold(tablefs->state_)) {
      size_t cursize = iheader->fstat.st_size;
      ret = TableFS_MigrateToDiskFile(tablefs,handle, &fh->fd_, fi->flags);
      if (ret == 0) {
        tfs_inode_header new_iheader = *GetInodeHeader(handle->value_);
        new_iheader.fstat.st_size = offset + size;
        new_iheader.has_blob = 1;
        UpdateInodeHeader(handle->value_,&new_iheader);		// Apan :: original arg was new_iheader
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Write: UpdateInodeHeader %d\n", GetInodeHeader(handle->value_)->has_blob);
#endif
        has_imgrated = 1;
        ret = pwrite(fh->fd_, buf, size, offset);
      }
    } else {
      UpdateInlineData(handle->value_, buf, offset, size);
      ret = size;
    }
  }
  if (ret >= 0) {
    if (has_larger_size > 0 && has_imgrated == 0) {
      tfs_inode_header new_iheader = *GetInodeHeader(handle->value_);
      new_iheader.fstat.st_size = offset + size;
      UpdateInodeHeader(handle->value_, &new_iheader);			// Apan :: original arg was new_iheader
    }
  }
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Write: %s, Handle: %x HandleMode: %d\n",
                           path, handle, handle->mode_);
#endif
  InodeMutex_Unlock(tablefs->fstree_lock,&handle->key_);
  return ret;
}


int TableFS_Fsync(TableFS *tablefs,const char *path, int datasync, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Fsync: %s\n", path);
#endif
  tfs_file_handle_t* fh;
  //tfs_file_handle_t* fh = reinterpret_cast<tfs_file_handle_t*>(fi->fh); 				//Apan : Added for compiling ,delete afterwards
  InodeCacheHandle* handle = fh->handle_;
  InodeMutex_WriteLock(tablefs->fstree_lock,&handle->key_);

  const tfs_inode_header* iheader = GetInodeHeader(handle->value_);
  int ret = 0;
  if (handle->mode_ == INODE_WRITE) {
    if (iheader->has_blob > 0) {
      ret = fsync(fh->fd_);
    }
    if (datasync == 0) {
      ret = LevelDBAdaptor_Sync(tablefs->metadb);
    }
  }

  InodeMutex_Unlock(tablefs->fstree_lock,&handle->key_);
  return -ret;
}

int TableFS_Release(TableFS *tablefs,const char *path, struct fuse_file_info *fi) {
  
  tfs_file_handle_t* fh;  		//Apan : Added for compiling ,delete afterwards
  //tfs_file_handle_t* fh = reinterpret_cast<tfs_file_handle_t*>(fi->fh);
  InodeCacheHandle* handle = fh->handle_;
  InodeMutex_WriteLock(tablefs->fstree_lock,&handle->key_);

  if (handle->mode_ == INODE_WRITE) {
    const tfs_stat_t *value;              	//Apan : Added for compiling ,delete afterwards
    //const tfs_stat_t *value = GetAttribute(handle->value_); Apan: LevelDB ,InodeCacheHandle,icache madhe ahe
    tfs_stat_t new_value = *value;
    new_value.st_atim.tv_sec = time(NULL);
    new_value.st_mtim.tv_sec = time(NULL);
    UpdateAttribute(handle->value_, &new_value);  //// Apan :: original arg was new_iheader
  }

#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Release: %s, FD: %d\n",
                           path, fh->fd_);
#endif

  int ret = 0;
  if (fh->fd_ != -1) {
    ret = close(fh->fd_);
  }
  InodeCache_WriteBack(tablefs->inode_cache,handle);
  tfs_meta_key_t key = handle->key_;
  InodeCache_Release(tablefs->inode_cache,handle);
  InodeCache_Evict(tablefs->inode_cache,key);
  free(fh);

  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  if (ret != 0) {
    return -errno;
  } else {
    return 0;
  }
}

int TableFS_Truncate(TableFS *tablefs,const char *path, off_t new_size) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Truncate: %s\n", path);
#endif

  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"Open: No such file or directory\n");
  }
  InodeCacheHandle* handle =
    InodeCache_Get(tablefs->inode_cache,key, INODE_WRITE);
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);

  int ret = 0;
  if (handle != NULL) {
    const tfs_inode_header *iheader = GetInodeHeader(handle->value_);

    if (iheader->has_blob > 0) {
      if (new_size > FileSystemState_GetThreshold(tablefs->state_)) {
        TruncateDiskFile(iheader->fstat.st_ino, new_size);
      } else {
	
        char* buffer;/* = new char[new_size];*/                                 // tan::See for "for loop" me,mory allocation 
        buffer=(char *)malloc(sizeof(new_size));
        MigrateDiskFileToBuffer(iheader->fstat.st_ino, buffer, new_size);
        UpdateInlineData(handle->value_, buffer, 0, new_size);
        free(buffer);                                 // tan::See for "for loop" me,mory deallocation
      }
    } else {
      if (new_size > FileSystemState_GetThreshold(tablefs->state_)) {
        int fd;
        if (TableFS_MigrateToDiskFile(tablefs,handle, &fd, O_TRUNC|O_WRONLY) == 0) {
          if ((ret = ftruncate(fd, new_size)) == 0) {
            fsync(fd);
          }
          close(fd);
        }
      } else {
        TruncateInlineData(handle->value_, new_size);
      }
    }
    if (new_size != iheader->fstat.st_size) {
      tfs_inode_header new_iheader = *GetInodeHeader(handle->value_);
      new_iheader.fstat.st_size = new_size;
      if (new_size > FileSystemState_GetThreshold(tablefs->state_)) {
        new_iheader.has_blob = 1;
      } else {
        new_iheader.has_blob = 0;
      }
      UpdateInodeHeader(handle->value_, &new_iheader);       //// Apan :: original arg was new_iheader
    }
    InodeCache_WriteBack(tablefs->inode_cache,handle);
    InodeCache_Release(tablefs->inode_cache,handle);
  } else {
    ret = -ENOENT;
  }

  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

int TableFS_Readlink(TableFS *tablefs,const char *path, char *buf, size_t size) {
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"Open: No such file or directory\n");
  }
  InodeMutex_ReadLock(tablefs->fstree_lock,&key);
  char* result;
  int ret = 0;
  /*if (LevelDBAdaptor_Get(table->metadb,key.ToSlice(), result) > 0) {
    size_t data_size = GetInlineData(result, buf, 0, size-1);
    buf[data_size] = '\0';
  } else {                            
    errno = ENOENT;
    ret = -1;
  }*/
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  if (ret < 0) {
    return TableFS_FSError(tablefs,"Open: No such file or directory\n");
  } else {
    return 0;
  }
}

int TableFS_Symlink(TableFS *tablefs,const char *target, const char *path) {
  tfs_meta_key_t key;
  //leveldb::Slice filename;
  /*if (!PathLookup(path, key, filename)) {
#ifdef  TABLEFS_DEBUG
    state_->GetLog()->LogMsg("Symlink: %s %s\n", path, target);
#endif
    return TableFS_FSError(tablefs,"Symlink: No such parent file or directory\n");
  }*/
  size_t val_size;                                        	//Apan : Added for compiling ,delete afterwards
  //size_t val_size = TFS_INODE_HEADER_SIZE+filename.size()+1+strlen(target);
  char* value;                                           // tan::See for "for loop" me,mory
  value =(char *)malloc(sizeof(val_size));
  tfs_inode_header* header;                              	//Apan : Added for compiling ,delete afterwards
  //tfs_inode_header* header = reinterpret_cast<tfs_inode_header*>(value);
  TableFS_InitStat(tablefs,&header->fstat, FileSystemState_NewInode(tablefs->state_), S_IFLNK, 0);
  header->has_blob = 0;
  //header->namelen = filename.size();
  char* name_buffer = value + TFS_INODE_HEADER_SIZE;
  //memcpy(name_buffer, filename.data(), filename.size());
  name_buffer[header->namelen] = '\0';
  //strncpy(name_buffer+filename.size()+1, target, strlen(target));

  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  //metadb->Put(key.ToSlice(), std::string(value, val_size));
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  free(value);
  //delete [] value;                                       // tan::See for "for loop" me,mory
  return 0;
}

int TableFS_Unlink(TableFS *tablefs,const char *path) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Unlink: %s\n", path);
#endif
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"Unlink: No such file or directory\n");
  }

  int ret = 0;
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_DELETE);
  if (handle != NULL) {
    const tfs_inode_header *value = GetInodeHeader(handle->value_);
    if (value->fstat.st_size > FileSystemState_GetThreshold(tablefs->state_)) {
      char fpath[128];
      TableFS_GetDiskFilePath(tablefs,fpath, value->fstat.st_ino);
      unlink(fpath);
    }
    tfs_DentryCache_Evict(tablefs->dentry_cache,&key);
    InodeCache_Release(tablefs->inode_cache,handle);
    InodeCache_Evict(tablefs->inode_cache,key);
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);

  return ret;
}


int TableFS_MakeNode(TableFS *tablefs,const char *path, mode_t mode, dev_t dev) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("MakeNode: %s\n", path);
#endif
  tfs_meta_key_t key;
  /*leveldb::Slice filename;
  if (!PathLookup(path, key, filename)) {
    return TableFS_FSError(tablefs,"MakeNode: No such parent file or directory\n");
  }

  tfs_inode_val_t value =
    InitInodeValue(state_->NewInode(), mode | S_IFREG, dev, filename);
*/
  int ret = 0;
  {
    InodeMutex_WriteLock(tablefs->fstree_lock,&key);
    //ret=metadb->Put(key.ToSlice(), value.ToSlice());
    InodeMutex_Unlock(tablefs->fstree_lock,&key);

  }

  //FreeInodeValue(value);

  if (ret == 0) {
    return 0;
  } else {
    errno = ENOENT;
    return TableFS_FSError(tablefs,"MakeNode failed\n");
  }
}

int TableFS_MakeDir(TableFS *tablefs,const char *path, mode_t mode) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("MakeDir: %s\n", path);
#endif
  tfs_meta_key_t key;
  /*leveldb::Slice filename;
  if (!PathLookup(path, key, filename)) {
    return TableFS_FSError(tablefs,"MakeDir: No such parent file or directory\n");
  }

  tfs_inode_val_t value =
    InitInodeValue(state_->NewInode(), mode | S_IFDIR, 0, filename);
*/
  int ret = 0;
  {
    InodeMutex_WriteLock(tablefs->fstree_lock,&key);
//    ret=metadb->Put(key.ToSlice(), value.ToSlice());
    InodeMutex_Unlock(tablefs->fstree_lock,&key);
  }

//  FreeInodeValue(value);

  if (ret == 0) {
    return 0;
  } else {
    errno = ENOENT;
    return TableFS_FSError(tablefs,"MakeDir failed\n");
  }
}

int TableFS_OpenDir(TableFS *tablefs,const char *path, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("OpenDir: %s\n", path);
#endif
  tfs_meta_key_t key;
  char *inode;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"OpenDir: No such file or directory\n");
  }
  InodeMutex_ReadLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle =
    InodeCache_Get(tablefs->inode_cache,key, INODE_READ);
  if (handle != NULL) {
    //fi->fh = (uint64_t) handle;
    fi->fh = (int) handle;				// tan::Var Bagha
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
    return 0;
  } else {
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
    return TableFS_FSError(tablefs,"OpenDir: No such file or directory\n");
  }
}

int TableFS_ReadDir(TableFS *tablefs,const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("ReadDir: %s\n", path);
#endif
  InodeCacheHandle* phandle = (InodeCacheHandle *)(int)(fi->fh);	//tan::int add kelay	
  tfs_meta_key_t childkey;
  int ret = 0;
/*  tfs_inode_t child_inumber = GetAttribute(phandle->value_)->st_ino;	//Apan: LevelDB ,InodeCacheHandle,icache madhe ahe
  BuildMetaKey(child_inumber,
              (child_inumber == ROOT_INODE_ID) ? 1 : 0,
              childkey);*/
  LevelDBIterator* iter = LevelDBAdaptor_GetNewIterator(tablefs->metadb);
  if (filler(buf, ".", NULL, 0) < 0) {
    return TableFS_FSError(tablefs,"Cannot read a directory");
  }
  if (filler(buf, "..", NULL, 0) < 0) {
    return TableFS_FSError(tablefs,"Cannot read a directory");
  }
  /*for (iter->Seek(childkey.ToSlice());
       iter->Valid() && IsKeyInDir(iter->key(), childkey);
       iter->Next()) {
    const char* name_buffer = iter->value().data() + TFS_INODE_HEADER_SIZE;
    if (filler(buf, name_buffer, NULL, 0) < 0) {
      ret = -1;
    }
    if (ret < 0) {
      break;
    }
  }*/
  free (iter);
  return ret;
}

int TableFS_ReleaseDir(TableFS *tablefs,const char *path, struct fuse_file_info *fi) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("ReleaseDir: %s\n", path);
#endif
  InodeCacheHandle* handle = (InodeCacheHandle *)(int) fi->fh;			//tan::int add kelay	
  tfs_meta_key_t key = handle->key_;
  InodeCache_Release(tablefs->inode_cache,handle);
  return 0;
}

int TableFS_RemoveDir(TableFS *tablefs,const char *path) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("RemoveDir: %s\n", path);
#endif

  //TODO: Do we need to scan the whole table and delete the item?
  tfs_meta_key_t key;
  char *inode;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"RemoveDir: No such file or directory\n");
  }
  int ret = 0;
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_DELETE);
  if (handle != NULL) {
    tfs_DentryCache_Evict(tablefs->dentry_cache,&key);
    InodeCache_Release(tablefs->inode_cache,handle);
    InodeCache_Evict(tablefs->inode_cache,key);
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

int TableFS_Rename(TableFS *tablefs,const char *old_path, const char *new_path) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Rename: %s %s\n", old_path, new_path);
#endif
  tfs_meta_key_t old_key;
  if (!TableFS_PathLookup(tablefs,old_path, &old_key)) {
    return TableFS_FSError(tablefs,"No such file or directory\n");
  }
  tfs_meta_key_t new_key;
  /*leveldb::Slice filename;
  if (!PathLookup(new_path, new_key, filename)) {
    return TableFS_FSError(tablefs,"No such file or directory\n");
  }
*/
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Rename old_key: [%08x, %08x]\n", old_key.inode_id, old_key.hash_id);
  state_->GetLog()->LogMsg("Rename new_key: [%08x, %08x]\n", new_key.inode_id, new_key.hash_id);
#endif

  tfs_meta_key_t large_key;
  tfs_meta_key_t small_key;
  if (old_key.inode_id > new_key.inode_id ||
      (old_key.inode_id == new_key.inode_id) &&
      (old_key.hash_id > new_key.hash_id)) {
    large_key = old_key;
    small_key = new_key;
  } else {
    large_key = new_key;
    small_key = old_key;
  }
  InodeMutex_WriteLock(tablefs->fstree_lock,&large_key);
  InodeMutex_WriteLock(tablefs->fstree_lock,&small_key);

  int ret = 0;
  InodeCacheHandle* old_handle = InodeCache_Get(tablefs->inode_cache,old_key, INODE_DELETE);
  if (old_handle != NULL) {
    const tfs_inode_header* old_iheader = GetInodeHeader(old_handle->value_);

    //char *new_value = InitInodeValue(old_handle->value_, filename);
    char *new_value;					//added for comiling
    InodeCacheHandle* new_handle = InodeCache_Insert(tablefs->inode_cache,new_key, new_value);
    InodeCache_BatchCommit(tablefs->inode_cache,old_handle, new_handle);
//    inode_cache->BatchCommit(old_handle, new_handle);
    
    InodeCache_Release(tablefs->inode_cache,new_handle);
    InodeCache_Release(tablefs->inode_cache,old_handle);
    InodeCache_Evict(tablefs->inode_cache,old_key);
    tfs_DentryCache_Evict(tablefs->dentry_cache,&old_key);
    tfs_DentryCache_Evict(tablefs->dentry_cache,&new_key);
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&small_key);
  InodeMutex_Unlock(tablefs->fstree_lock,&large_key);
  return ret;
}

int TableFS_Access(TableFS *tablefs,const char *path, int mask) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("Access: %s %08x\n", path, mask);
#endif
  //TODO: Implement Access
  return 0;
}

int TableFS_UpdateTimens(TableFS *tablefs,const char *path, const struct timespec tv[2]) {
#ifdef  TABLEFS_DEBUG
  state_->GetLog()->LogMsg("UpdateTimens: %s\n", path);
#endif
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"No such file or directory\n");
  }
  int ret = 0;
  InodeMutex_Writelock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_WRITE);
  if (handle != NULL) {
    {
      //const tfs_stat_t *value = GetAttribute(handle->value_);			//leveldb
      const tfs_stat_t *value;
      tfs_stat_t new_value = *value;
      new_value.st_atim.tv_sec = tv[0].tv_sec;
      new_value.st_mtim.tv_sec = tv[1].tv_sec;
      UpdateAttribute(handle->value_, &new_value);		//& takla
      InodeCache_WriteBack(tablefs->inode_cache,handle);
      InodeCache_Release(tablefs->inode_cache,handle);
    }
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

int TableFS_Chmod(TableFS *tablefs,const char *path, mode_t mode) {
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"No such file or directory\n");
  }
  int ret = 0;
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_WRITE);
  if (handle != NULL) {
    //const tfs_stat_t *value = GetAttribute(handle->value_);    //leveldb
    const tfs_stat_t *value;
    tfs_stat_t new_value = *value;
    new_value.st_mode = mode;
    UpdateAttribute(handle->value_, &new_value);		//& takla
    InodeCache_WriteBack(tablefs->inode_cache,handle);
    InodeCache_Release(tablefs->inode_cache,handle);
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

int TableFS_Chown(TableFS *tablefs,const char *path, uid_t uid, gid_t gid) {
  tfs_meta_key_t key;
  if (!TableFS_PathLookup(tablefs,path, &key)) {
    return TableFS_FSError(tablefs,"No such file or directory\n");
  }
  int ret = 0;
  InodeMutex_WriteLock(tablefs->fstree_lock,&key);
  InodeCacheHandle* handle = InodeCache_Get(tablefs->inode_cache,key, INODE_WRITE);
  if (handle != NULL) {
    //const tfs_stat_t *value = GetAttribute(handle->value_);    //leveldb
    const tfs_stat_t *value;
    tfs_stat_t new_value = *value;
    new_value.st_uid = uid;
    new_value.st_gid = gid;
    UpdateAttribute(handle->value_, &new_value);	//&takla
 InodeCache_WriteBack(tablefs->inode_cache,handle);
    InodeCache_Release(tablefs->inode_cache,handle);
    return 0;
  } else {
    ret = -ENOENT;
  }
  InodeMutex_Unlock(tablefs->fstree_lock,&key);
  return ret;
}

bool TableFS_GetStat(TableFS *tablefs,char *stat, char** value) {
  //return state_->GetMetaDB()->GetStat(stat, value);
}
/*
void TableFS_Compact(TableFS *tablefs) {
  LevelDBAdaptor_Compact(FileSystemState_GetMetaDB(tablefs->state_));
}
*/
//}
