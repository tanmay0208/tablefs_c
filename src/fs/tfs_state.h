#ifndef TABLEFS_STATE_H_
#define TABLEFS_STATE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unordered_map>
#include <errno.h>
#include "fs/tfs_inode.h"
//#include "leveldb/cache.h"
//#include "leveldb/db.h"
//#include "leveldb/iterator.h"
#include "adaptor/leveldb_adaptor.h"
#include "util/properties.h"
#include "util/logging.h"

//namespace tablefs {

struct FileSystemState {
  char *metadir_;
  char *datadir_;
  char *mountdir_;

  LevelDBAdaptor* metadb;

  tfs_inode_t max_inode_num;
  int threshold_;

  Logging* logs;
};
typedef struct FileSystemState FileSystemState;
  void FileSystemState_constructor(FileSystemState *);

  //~FileSystemState();

  int FileSystemState_Setup(FileSystemState *,Properties *prop);

  void FileSystemState_Destroy(FileSystemState *);

  LevelDBAdaptor* FileSystemState_GetMetaDB(FileSystemState *) ;

  Logging* FileSystemState_GetLog(FileSystemState *) ;

  const char *FileSystemState_GetDataDir(FileSystemState *filesystemstate);

  const int FileSystemState_GetThreshold(FileSystemState *filesystemstate); 

  bool FileSystemState_IsEmpty(FileSystemState *filesystemstate);

  tfs_inode_t FileSystemState_NewInode(FileSystemState *);


//}

#endif
