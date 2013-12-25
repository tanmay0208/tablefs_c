//#ifdef SUPPORTED
#ifndef _META_MANAGER_H_
#define _META_MANAGER_H_

#include "adaptor/leveldb_adaptor.h"
#include "fs/inodemutex.h"
#include "fs/tfs_inode.h"
//#include "leveldb/cache.h"
#include "port/port.h"
#include <string.h>
//namespace tablefs {

enum InodeAccessMode {
  INODE_READ = 0,
  INODE_DELETE = 1,
  INODE_WRITE = 2,
};

enum InodeState {
  CLEAN = 0,
  DELETED = 1,
  DIRTY = 2,
};

struct InodeCacheHandle {
  tfs_meta_key_t key_;
  char *value_;
  enum InodeAccessMode mode_;

  //leveldb::Cache::Handle* pointer;
  int object_count;
};
typedef struct InodeCacheHandle InodeCacheHandle;


  void InodeCacheHandle_constructor(InodeCacheHandle *inodecache);

  void InodeCacheHandle_constructor_mode(InodeCacheHandle *inodecachehandle,const tfs_meta_key_t key,
                   const char *value,
                   enum InodeAccessMode mode);

  void InodeCacheHandle_destructor(InodeCacheHandle *inodecachehandle);
  
  void InodeCacheHandle_init(InodeCacheHandle *);
  

//class InodeCache {
//public:
  struct InodeCache{
   LevelDBAdaptor* metadb_;
   //leveldb::Cache* cache;
  };
  typedef struct InodeCache InodeCache;
 
  void InodeCache_constructor(InodeCache *inodecache,LevelDBAdaptor *metadb);

  void InodeCache_destructor(InodeCache *inodecache);

  InodeCacheHandle* InodeCache_Insert(InodeCache *,const tfs_meta_key_t key,
                           const char *value);

  InodeCacheHandle* InodeCache_Get(InodeCache *,const tfs_meta_key_t key,
                        const enum InodeAccessMode mode);

  void InodeCache_Release(InodeCache *,InodeCacheHandle* handle);

  void InodeCache_WriteBack(InodeCache *,InodeCacheHandle* handle);

  void InodeCache_BatchCommit(InodeCache *,InodeCacheHandle* handle1,
                   InodeCacheHandle* handle2);

  void InodeCache_Evict(InodeCache *,const tfs_meta_key_t key);

  /*friend*/ //void InodeCache_CleanInodeHandle(InodeCache *,const leveldb::Slice key, void* value);




#endif
//#endif
