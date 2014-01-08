//#ifdef SUPPORTED
#include "fs/fswrapper.h"
#include <stdio.h>
#include "fs/tablefs.h"

//namespace tablefs {

/*void TableFSWrapper_constructor() :
  tfs(NULL), tablefs_data(NULL), junkf(NULL)
{
  memset(freefd, 0, sizeof(freefd));
}*/ 
  TableFS *tablefs;
int TableFSWrapper_Setup(TableFSWrapper *tablefswrapper,Properties *prop) {
  //tablefs_data = new tablefs::FileSystemState();
  //if (tablefs_data->Setup(prop) < 0)
    //return -1;
  //tfs = new TableFS();
  //tfs->SetState(tablefs_data);
  //tfs->Init(NULL);
   num_fd = 0;
  //if (prop.getProperty("TableFSWrapper_filesystem") == std::string("tablefs_pred")) {
    int i;
    for ( i = 0; i < TFS_INODE_HEADER_SIZE; ++i) {
     // junk[i] = 'a';
    }
    junkf = fopen("/dev/null", "wb");
  //}
  logon = Properties_getPropertyBool(prop,"fswrapper.logon", false);
  return 0;
}

void TableFSWrapper_destructor(TableFSWrapper *tablefswrapper) {
  fprintf(stderr, "DESTROY tablefs\n");
  //if (tfs != NULL) {
    //tfs->Destroy(tablefs_data);
    //delete tfs;
  //}
  if (junkf != NULL) {
    fclose(junkf);
  }
}

int TableFSWrapper_Mknod(TableFSWrapper *tablefswrapper,const char* path, mode_t mode, dev_t dev) {
  struct stat statbuf;
  int ret = TableFSWrapper_Stat(tablefswrapper,path,&statbuf);
  if (ret == 0) {
    errno = EEXIST;
    return -1;
  }
  if (junkf != NULL) {//junk
    //fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = TableFS_MakeNode(tfs,path, mode, dev);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the file: %s\n", path);
  }
  ret = TableFSWrapper_Stat(tablefswrapper,path, &statbuf);
  return ret;
}

int TableFSWrapper_Mkdir(TableFSWrapper *tablefswrapper,const char* path, mode_t mode) {
  struct stat statbuf;
  int ret = TableFSWrapper_Stat(tablefswrapper,path, &statbuf);
  if (ret == 0) {
    return -1;
  }
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = TableFS_MakeDir(tfs,path, mode);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the directory: %s\n", path);
  }
  ret = TableFSWrapper_Stat(tablefswrapper,path, &statbuf);
  return ret;
}

int TableFSWrapper_Utime(TableFSWrapper *tablefswrapper,const char* path, struct utimbuf *buf) {
  struct timespec tv[2];
  tv[0].tv_sec = buf->actime;
  tv[1].tv_sec = buf->modtime;
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }
  int ret = TableFS_UpdateTimens(tfs,path, tv);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the directory: %s\n", path);
  }
  return ret;
}

int TableFSWrapper_Chmod(TableFSWrapper *tablefswrapper,const char* path, mode_t mode) {
  struct stat statbuf;
  int ret = TableFSWrapper_Stat(tablefswrapper,path, &statbuf);//junk
  if (junkf != NULL) {
  //  fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = TableFS_Chmod(tfs,path, mode);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to chmod the path: %s\n", path);
  }
  ret = TableFSWrapper_Stat(tablefswrapper,path, &statbuf);
  return ret;
}

int TableFSWrapper_Stat(TableFSWrapper *tablefswrapper,const char* path, struct stat *buf) {
  int ret = TableFS_GetAttr(tfs,path, buf);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to stat the path: %s\n", path);
  }
  return ret;
}

int TableFSWrapper_Open(TableFSWrapper *tablefswrapper,const char* path, int flags) {
  int fd = TableFSWrapperStat_GetFileDescriptor(tablefswrapper);
  //paths[fd] = std::string(path);
  offsets[fd] = 0;
  fis[fd].flags = flags;
  if (junkf != NULL) {
    //fwrite(junk, sizeof(path), 1, junkf);
  }
  int ret = TableFS_Open(tfs,path, &(fis[fd]));
  if (ret != 0) {
    TableFSWrapperStat_ReleaseFileDescriptor(tablefswrapper,fd);
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  return fd;
}

int TableFSWrapper_Seek(TableFSWrapper *tablefswrapper,int fd, off_t offset) {
  offsets[fd] = offset;
  return 0;
}

int TableFSWrapper_Write(TableFSWrapper *tablefswrapper,int fd, const char* buf, size_t size) {
 //mk const char *p;   //to convert paths[fd] into const
  char *p;			//addded for compiling
  strcpy(p,paths);
  //int ret = tfs->TableFS_Write(paths[fd].c_str(), buf, size, offsets[fd], &(fis[fd]));
  int ret = TableFS_Write(tfs,p, buf, size, offsets[fd], &(fis[fd]));
  if (junkf != NULL) {
    fwrite(buf, size, 1, junkf);
  }
  if (ret < 0) {
    if (logon)
      fprintf(stderr, "Failed to write the path: %s\n", p);
  } else {
    offsets[fd] += ret;
  }
  return ret;
}

int TableFSWrapper_Read(TableFSWrapper *tablefswrapper,int fd, char* buf, size_t size) {
  /*const char *p;   //to convert paths[fd] into const
  strcpy(p,paths);

  int ret = TableFS_Read(tfs,p, buf, size, offsets[fd], &(fis[fd]));
  if (junkf != NULL) {
    fwrite(buf, size, 1, junkf);
  }
  if (ret < 0) {
    if (logon)
      fprintf(stderr, "Failed to read the path: %s\n", p)junk;
  } else {
    offsets[fd] += ret;
  }
  return ret;*/
}
static int list_dir_count = 0;
int fuse_filler_dir(void* buf, const char* name, const struct stat *stbuf, off_t off) {
  ++list_dir_count;
}

int TableFSWrapper_Listdir(TableFSWrapper *tablefswrapper,const char* path) {
  struct fuse_file_info *fi;
  int ret = TableFS_OpenDir(tfs,path, fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }
  list_dir_count = 0;
  if (TableFS_ReadDir(tfs,path, NULL, fuse_filler_dir, 0, fi) != 0) {
    if (logon)
      fprintf(stderr, "Failed to listdir the path: %s\n", path);
    return -1;
  }
  return list_dir_count;
}

static int lsstat_count = 0;
static char lsstat_fpath[1024];
static char* lsstat_end_fpath;
static TableFSWrapper* lsstat_fs;
int lsstat_filler(void* buf, const char* name, const struct stat *stbuf, off_t off)
{
  strcpy(lsstat_end_fpath, name);
  struct stat statbuf;
  TableFSWrapper_Stat(lsstat_fs,lsstat_fpath, &statbuf);
  ++lsstat_count;
}

int TableFSWrapper_Lsstat(TableFSWrapper *tablefswrapper,const char* path) {
  struct fuse_file_info *fi;
  int ret = TableFS_OpenDir(tfs,path, fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }

  sprintf(lsstat_fpath, "%s/", path);
  lsstat_end_fpath = lsstat_fpath + strlen(lsstat_fpath);
  lsstat_fs = tablefswrapper;//lssta
  lsstat_count = 0;
  if (TableFS_ReadDir(tfs,path, NULL, lsstat_filler, 0, fi) != 0) {
    if (logon)
      fprintf(stderr, "Failed to listdir the path: %s\n", path);
    return -1;
  }
  return lsstat_count;
}

static int scanfile_count = 0;
static char scanfile_fpath[1024];
static char* scanfile_end_fpath;
static TableFSWrapper* scanfile_fs;
static char scanfile_content[4096];
int scanfile_filler(void* buf, const char* name,
                    const struct stat *stbuf, off_t off)
{
  strcpy(scanfile_end_fpath, name);
  struct stat statbuf;
  TableFSWrapper_Stat(scanfile_fs,scanfile_fpath, &statbuf);
  int fd = TableFSWrapper_Open(scanfile_fs,scanfile_fpath, O_RDONLY);
  int read_size = 0;
  while (read_size < statbuf.st_size) {
    int ret = TableFSWrapper_Read(scanfile_fs,fd, scanfile_content, 4096);
    if (ret == 0) break;
      read_size += ret;
  }
  TableFSWrapper_Close(scanfile_fs,fd);
  ++scanfile_count;
}

int TableFSWrapper_Scanfile(TableFSWrapper *tablefswrapper,const char* path) {
  struct fuse_file_info *fi;
  int ret = TableFS_OpenDir(tfs,path, fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }

  sprintf(scanfile_fpath, "%s/", path);
  scanfile_end_fpath = scanfile_fpath + strlen(scanfile_fpath);
  scanfile_fs = tablefswrapper;
  scanfile_count = 0;
  if (TableFS_ReadDir(tfs,path,NULL, scanfile_filler, 0, fi) != 0) {
    if (logon)
      fprintf(stderr, "Failed to listdir the path: %s\n", path);
    return -1;
  }
  return scanfile_count;
}

int TableFSWrapper_Rename(TableFSWrapper *tablefswrapper,const char* old_path,
                           const char* new_path) {
  if (junkf != NULL) {
    fwrite(old_path, strlen(old_path), 1, junkf);
    fwrite(new_path, strlen(new_path), 1, junkf);
  }

  int ret = TableFS_Rename(tfs,old_path, new_path);
  if (ret != 0) {
    if (logon)
      fprintf(stderr,
              "Failed to rename %s to %s\n",
              old_path, new_path);
    return -1;
  }
  return 0;
}

int TableFSWrapper_Unlink(TableFSWrapper *tablefswrapper,const char* path) {
  if (junkf != NULL) {
    fwrite(path, strlen(path), 1, junkf);
  }

  int ret = TableFS_Unlink(tfs,path);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to unlink %s\n", path);
    return -1;
  }
  return 0;
}

int TableFSWrapper_Close(TableFSWrapper *tablefswrapper,int fd) {
  /*const char *p;
  strcpy(p,paths);
  int ret = Tablefs_Release(tfs,p, &(fis[fd]));
  ReleaseFileDescriptor(fd);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to close the path: %s\n", p);
  }
  return ret;*/
}

bool TableFSWrapper_GetStat(TableFSWrapper *tablefswrapper,char *stat, char **value) {
  if (junkf != NULL) {
    //fwrite(junk, sizeof(junk), 1, junkf);
  }
  return TableFS_GetStat(tfs,stat, value);
}

int TableFSWrapperStat_GetFileDescriptor(TableFSWrapper *tablefswrapper) {
  int i;
  for (i = 0; i < FDLIMIT; ++i)
    if (freefd[i] == 0) {
      freefd[i] = 1;
      return i;
    }
  return -1;
}

void TableFSWrapperStat_ReleaseFileDescriptor(TableFSWrapper *tablefswrapper,int fd) {
  freefd[fd] = 0;
}

int TableFSTestWrapper_Mknod(TableFSTestWrapper *tablefstestwrapper,const char* path, mode_t mode, dev_t dev) {
  struct stat statbuf;
  //leveldb::Slice key(path, strlen(path));
  int ret = TableFSTestWrapper_Stat(tablefstestwrapper,path, &statbuf);
  if (ret == 0) {
    errno = EEXIST;
    return -1;
  }
  /*ret = tfs->metadb->Put(key, std::string(""));
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the file: %s\n", path);
  }*/
  ret = TableFSTestWrapper_Stat(tablefstestwrapper,path, &statbuf);
  return ret;
}

int TableFSTestWrapper_Stat(TableFSTestWrapper *tablefstestwrapper,const char* path, struct stat *buf) {
  char *result;
  //leveldb::Slice key(path, strlen(path));
  int ret = 0;
  /*if (tfs->metadb->Get(key, result) > 0) {
    lstat("/tmp", buf);
  } else {
    ret = -1;
    if (logon)
      fprintf(stderr, "Failed to stat the path: %s\n", path);
  }*/
  return ret;
}

//}
//#endif
