#ifndef TABLE_FS_H
#define TABLE_FS_H

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "fs/tfs_state.h"
#include "fs/inodemutex.h"
#include "fs/dcache.h"
#include "fs/icache.h"

//namespace tablefs {

struct TableFS {

  FileSystemState *state_;
  LevelDBAdaptor* metadb;
  struct InodeCache *inode_cache;
  struct DentryCache *dentry_cache;
  struct InodeMutex *fstree_lock;
  bool flag_fuse_enabled;
  struct TableFSTestWrapper *tablefstestwrapper;
};

typedef struct TableFS TableFS;
/*public:
  ~TableFS() {
  }
  */

  void TableFS_SetState(TableFS *,FileSystemState* state);

  inline void* TableFS_Init(TableFS *,struct fuse_conn_info *conn);

  void TableFS_Destroy(TableFS *,void * data);

  int TableFS_GetAttr(TableFS *,const char *path, struct stat *statbuf);

  int TableFS_Open(TableFS *,const char *path, struct fuse_file_info *fi);

  int TableFS_Read(TableFS *,const char* path, char *buf, size_t size,
           off_t offset, struct fuse_file_info *fi);

  int TableFS_Write(TableFS *,const char* path, const char *buf, size_t size,
            off_t offset, struct fuse_file_info *fi);

  int TableFS_Truncate(TableFS *,const char *path, off_t offset);

  int TableFS_Fsync(TableFS *,const char *path, int datasync, struct fuse_file_info *fi);

  int TableFS_Release(TableFS *,const char *path, struct fuse_file_info *fi);

  int TableFS_Readlink(TableFS *,const char *path, char *buf, size_t size);

  int TableFS_Symlink(TableFS *,const char *target, const char *path);

  int TableFS_Unlink(TableFS *,const char *path);

  int TableFS_MakeNode(TableFS *,const char *path, mode_t mode, dev_t dev);

  int TableFS_MakeDir(TableFS *,const char *path, mode_t mode);

  int TableFS_OpenDir(TableFS *,const char *path, struct fuse_file_info *fi);

  int TableFS_ReadDir(TableFS *,const char *path, void *buf, fuse_fill_dir_t filler,
              off_t offset, struct fuse_file_info *fi);

  int TableFS_ReleaseDir(TableFS *,const char *path, struct fuse_file_info *fi);

  int TableFS_RemoveDir(TableFS *,const char *path);

  int TableFS_Rename(TableFS *,const char *new_path, const char *old_path);

  int TableFS_Access(TableFS *,const char *path, int mask);

  int TableFS_UpdateTimens(TableFS *,const char *path, const struct timespec tv[2]);

  int TableFS_Chmod(TableFS *,const char *path, mode_t mode);

  int TableFS_Chown(TableFS *,const char *path, uid_t uid, gid_t gid);

  void TableFS_Compact(TableFS *);

  bool TableFS_GetStat(TableFS *,char *stat, char ** value);


  inline int TableFS_FSError(TableFS *,const char *error_message);

  inline void TableFS_DeleteDBFile(TableFS *,tfs_inode_t inode_id, int filesize);

  inline void TableFS_GetDiskFilePath(TableFS *,char *path, tfs_inode_t inode_id);

  inline int TableFS_OpenDiskFile(TableFS *,const tfs_inode_header* iheader, int flags);

  inline int TableFS_TruncateDiskFile(TableFS *,tfs_inode_t inode_id, off_t new_size);

  inline ssize_t TableFS_MigrateDiskFileToBuffer(TableFS *,tfs_inode_t inode_it,
                                         char* buffer, size_t size);

  int TableFS_MigrateToDiskFile(TableFS *,InodeCacheHandle* handle, int *fd, int flags);

  inline void TableFS_CloseDiskFile(TableFS *,int fd_);		

  inline void TableFS_InitStat(TableFS *,struct stat *statbuf,
                       tfs_inode_t inode,
                       mode_t mode,
                       dev_t dev);
/*
  tfs_inode_val_t TableFS_InitInodeValue(TableFS *,tfs_inode_t inum,
                                 mode_t mode,
                                 dev_t dev,
                                 leveldb::Slice filename);

  char* TableFS_InitInodeValue(TableFS *,const char *old_value,
                             leveldb::Slice filename);
*/
 /* void TableFS_FreeInodeValue(TableFS *,tfs_inode_val_t *ival);
*/
 /* bool TableFS_ParentPathLookup(TableFS *,const char* path,
                        tfs_meta_key_t *key,
                        tfs_inode_t &inode_in_search,
                        const char* &lastdelimiter);
*//*
  inline bool TableFS_PathLookup_Slice(TableFS *,const char *path,
                         tfs_meta_key_t &key,
                         leveldb::Slice &filename);
*/
  inline bool TableFS_PathLookup(TableFS *,const char *path,
                         tfs_meta_key_t *key);

//  friend class TableFSTestWrapper;             added in struct



#endif
