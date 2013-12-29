//#ifdef SUPPORTED
#ifndef FSWRAPPER_H_
#define FSWRAPPER_H_

#include <stdlib.h>
#include <string.h>
//#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <utime.h>
#include <unistd.h>
#include <dirent.h>
#include "fs/tfs_state.h"
#include "fs/tablefs.h"
#include "util/properties.h"
#include "util/monitor.h"

//namespace tablefs {TableFSWrapper *tablefswrapper

//class FileSystemWrapper {
struct FileSystemWrapper
{
  /* data */
};
//TableFSWrapper *tablefswrapper;
typedef struct FileSystemWrapper FileSystemWrapper;
//public:
  void FileSystemWrapper_destructor(FileSystemWrapper *filesystemwrapper) {
  }

  int FileSystemWrapper_Setup(FileSystemWrapper *filesystemwrapper,Properties *prop) {
    return 0;
  }

  /*virtual*/ int FileSystemWrapper_Mknod(FileSystemWrapper *filesystemwrapper,const char* path, mode_t mode, dev_t dev) {
    return mknod(path, mode, dev);
  }

  /*virtual*/ int FileSystemWrapper_Mkdir(FileSystemWrapper *filesystemwrapper,const char* path, mode_t mode) {
    return mkdir(path, mode);
  }

  /*virtual*/ int FileSystemWrapper_Utime(FileSystemWrapper *filesystemwrapper,const char* path, struct utimbuf *buf) {
    return utime(path, buf);
  }

  /*virtual*/ int FileSystemWrapper_Chmod(FileSystemWrapper *filesystemwrapper,const char* path, mode_t mode) {
    return chmod(path, mode);
  }

  /*virtual*/ int FileSystemWrapper_Stat(FileSystemWrapper *filesystemwrapper,const char* path, struct stat *buf) {
    return stat(path, buf);
  }

  /*virtual*/ int FileSystemWrapper_Listdir(FileSystemWrapper *filesystemwrapper,const char* path) {
    DIR *fd;
    struct dirent *dirp;
    if (!(fd = opendir(path))) {
      return -1;
    }
    int count = 0;
    while ((dirp = readdir(fd)) != NULL) {
      ++count;
    }
    closedir(fd);
    return count;
  }

  /*virtual*/ int FileSystemWrapper_Lsstat(FileSystemWrapper *filesystemwrapper,const char* path) {
    char fpath[1024];
    sprintf(fpath, "%s/", path);
    char* end_fpath = fpath + strlen(fpath);
    DIR *fd;
    struct dirent *dirp;
    if (!(fd = opendir(path))) {
      return -1;
    }
    int count = 0;
    struct stat statbuf;
    while ((dirp = readdir(fd)) != NULL) {
      strcpy(end_fpath, dirp->d_name);
      FileSystemWrapper_Stat(filesystemwrapper,fpath, &statbuf);
      ++count;
    }
    closedir(fd);
    return count;
  }

  /*virtual*/ int FileSystemWrapper_Scanfile(FileSystemWrapper *filesystemwrapper,const char* path) {
    char fpath[1024];
    char content[4096];
    sprintf(fpath, "%s/", path);
    char* end_fpath = fpath + strlen(fpath);
    DIR *fd;
    struct dirent *dirp;
    if (!(fd = opendir(path))) {
      return -1;
    }
    int count = 0;
    struct stat statbuf;
    while ((dirp = readdir(fd)) != NULL) {
      strcpy(end_fpath, dirp->d_name);
      FileSystemWrapper_Stat(filesystemwrapper,fpath, &statbuf);
      if (S_ISREG(statbuf.st_mode)) {
        int fd = FileSystemWrapper_Open(filesystemwrapper,fpath, O_RDONLY);
        int read_size = 0;
        while (read_size < statbuf.st_size) {
          int ret = FileSystemWrapper_Read(filesystemwrapper,fd, content, 4096);
          if (ret == 0) break;
          read_size += ret;
        }
        FileSystemWrapper_Close(filesystemwrapper,fd);
      }
      ++count;
    }
    closedir(fd);
    return count;
  }

  /*virtual*/ int FileSystemWrapper_Rename(FileSystemWrapper *filesystemwrapper,const char* old_path, const char* new_path) {
    return rename(old_path, new_path);
  }

  /*virtual*/ int FileSystemWrapper_Unlink(FileSystemWrapper *filesystemwrapper,const char* path) {
    return unlink(path);
  }

  /*virtual*/ int FileSystemWrapper_Open(FileSystemWrapper *filesystemwrapper,const char* path, int flags) {
    return open(path, flags);
  }

  /*virtual*/ int FileSystemWrapper_Seek(FileSystemWrapper *filesystemwrapper,int fd, off_t offset) {
    return lseek(fd, offset, SEEK_SET);
  }

  /*virtual*/ int FileSystemWrapper_Write(FileSystemWrapper *filesystemwrapper,int fd, const char* buf, size_t size) {
    return write(fd, buf, size);
  }

  /*virtual*/ int FileSystemWrapper_Read(FileSystemWrapper *filesystemwrapper,int fd, char* buf, size_t size) {
    return read(fd, buf, size);
  }

  /*virtual*/ int FileSystemWrapper_Close(FileSystemWrapper *filesystemwrapper,int fd) {
    return close(fd);
  }

  /*virtual*/ bool FileSystemWrapper_GetStat(FileSystemWrapper *filesystemwrapper,char *stat, char **value) {
    return false;
  }

  /*virtual*/ void FileSystemWrapper_Compact(FileSystemWrapper *filesystemwrapper) {
  }

  /*virtual*/ MetricStat* GetMetricStat(FileSystemWrapper *filesystemwrapper) {
    return NULL;
  }
//};

//class TableFSWrapper : public FileSystemWrapper {
//public:
struct TableFSWrapper{
   FileSystemWrapper *filesystemwrapper;
   //bool logon;
};
typedef struct TableFSWrapper TableFSWrapper;
  void TableFSWrapper_constructor(TableFSWrapper *tablefswrapper);

  void TableFSWrapper_destructor(TableFSWrapper *tablefswrapper);

  int TableFSWrapper_Setup(TableFSWrapper *tablefswrapper,Properties *prop);

  int TableFSWrapper_Mknod(TableFSWrapper *tablefswrapper,const char* path, mode_t mode, dev_t dev);

  int TableFSWrapper_Mkdir(TableFSWrapper *tablefswrapper,const char* path, mode_t mode);

  int TableFSWrapper_Utime(TableFSWrapper *tablefswrapper,const char* path, struct utimbuf *buf);

  int TableFSWrapper_Chmod(TableFSWrapper *tablefswrapper,const char* path, mode_t mode);

  int TableFSWrapper_Stat(TableFSWrapper *tablefswrapper,const char* path, struct stat *buf);

  int TableFSWrapper_Listdir(TableFSWrapper *tablefswrapper,const char* path);

  int TableFSWrapper_Lsstat(TableFSWrapper *tablefswrapper,const char* path);

  int TableFSWrapper_Scanfile(TableFSWrapper *tablefswrapper,const char* path);

  int TableFSWrapper_Rename(TableFSWrapper *tablefswrapper,const char* old_path, const char* new_path);

  int TableFSWrapper_Unlink(TableFSWrapper *tablefswrapper,const char* path);

  int TableFSWrapper_Open(TableFSWrapper *tablefswrapper,const char* path, int flags);

  int TableFSWrapper_Write(TableFSWrapper *tablefswrapper,int fd, const char* buf, size_t size);

  int TableFSWrapper_Seek(TableFSWrapper *tablefswrapper,int fd, off_t offset);

  int TableFSWrapper_Read(TableFSWrapper *tablefswrapper,int fd, char* buf, size_t size);

  int TableFSWrapper_Close(TableFSWrapper *tablefswrapper,int fd);

  bool GetStat(char *stat, char **value);

  void TableFSWrapper_Compact(TableFSWrapper *tablefswrapper) {
    //fs->Compact();
  }

  //class TableFSWrapperStat: public MetricStat {
  struct  TableFSWrapperStat
  {
    MetricStat *metricstat;
    TableFSWrapper* fsw_;/* data */
    
  };
  typedef struct TableFSWrapperStat TableFSWrapperStat;  

  //public:
    //void TableFSWrapperStat_constructor(TableFSWrapper* fsw): fsw_(fsw) {
    //}

    /*virtual void TableFSWrapperStat_GetMetric(TMetList &metlist, time_t now) {
      std::string value;
      if (fsw_->GetStat("leveldb.stats", &value)) {
        const int metric_cnt = 13;
        int r[metric_cnt];
        std::stringstream ssmetric(value);
        for (int i = 0; i < metric_cnt; ++i)
          ssmetric >> r[i];
        int now = (int) time(NULL);
        const char* metname[metric_cnt] = {
          "leveldb.num_files", "leveldb.num_size", "leveldb.num_compact",
          "leveldb.tot_comp_time", "leveldb.tot_comp_read",
          "leveldb.tot_comp_write", "leveldb.num_write_op",
          "leveldb.num_get_op", "leveldb.filterhit",
          "leveldb.minor_count", "leveldb.log_writes",
          "leveldb.log_syncs", "leveldb.desc_syncs"
        };

        for (int ri = 0; ri < metric_cnt; ++ri) {
          AddMetric(metlist, metname[ri], now, r[ri]);
        }
      }
    }*/
  //};

  MetricStat* TableFSWrapperStat_GetMetricStat() {
    //return new TableFSWrapperStat(this);
  }

//protected:
  FileSystemState *tablefs_data;
  TableFS *fs;
  enum { FDLIMIT = 128 };
  struct fuse_file_info fis[FDLIMIT];
  char paths[FDLIMIT];
  off_t offsets[FDLIMIT];
  int freefd[FDLIMIT];
  int num_fd;
  bool logon;
  //char junk[TFS_INODE_HEADER_SIZE];
  FILE* junkf;

  int TableFSWrapperStat_GetFileDescriptor();
  void TableFSWrapperStat_ReleaseFileDescriptor(int fd);
//};

//class TableFSTestWrapper : public TableFSWrapper {
//public:
  struct TableFSTestWrapper
  {
    TableFSWrapper *tablefswrapper;/* data */
  };
  typedef struct TableFSTestWrapper TableFSTestWrapper;
  int TableFSTestWrapper_Mknod(TableFSTestWrapper *,const char* path, mode_t mode, dev_t dev);

  int TableFSTestWrapper_Stat(TableFSTestWrapper *,const char* path, struct stat *buf);
//};

//}

#endif
//#endif
