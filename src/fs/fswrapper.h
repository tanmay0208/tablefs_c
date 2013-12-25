#ifdef SUPPORTED
#ifndef FSWRAPPER_H_
#define FSWRAPPER_H_

#include <stdlib.h>
#include <string>
#include <sstream>
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

namespace tablefs {

class FileSystemWrapper {
public:
  virtual ~FileSystemWrapper() {
  }

  virtual int Setup(Properties &prop) {
    return 0;
  }

  virtual int Mknod(const char* path, mode_t mode, dev_t dev) {
    return mknod(path, mode, dev);
  }

  virtual int Mkdir(const char* path, mode_t mode) {
    return mkdir(path, mode);
  }

  virtual int Utime(const char* path, struct utimbuf *buf) {
    return utime(path, buf);
  }

  virtual int Chmod(const char* path, mode_t mode) {
    return chmod(path, mode);
  }

  virtual int Stat(const char* path, struct stat *buf) {
    return stat(path, buf);
  }

  virtual int Listdir(const char* path) {
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

  virtual int Lsstat(const char* path) {
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
      Stat(fpath, &statbuf);
      ++count;
    }
    closedir(fd);
    return count;
  }

  virtual int Scanfile(const char* path) {
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
      Stat(fpath, &statbuf);
      if (S_ISREG(statbuf.st_mode)) {
        int fd = Open(fpath, O_RDONLY);
        int read_size = 0;
        while (read_size < statbuf.st_size) {
          int ret = Read(fd, content, 4096);
          if (ret == 0) break;
          read_size += ret;
        }
        Close(fd);
      }
      ++count;
    }
    closedir(fd);
    return count;
  }

  virtual int Rename(const char* old_path, const char* new_path) {
    return rename(old_path, new_path);
  }

  virtual int Unlink(const char* path) {
    return unlink(path);
  }

  virtual int Open(const char* path, int flags) {
    return open(path, flags);
  }

  virtual int Seek(int fd, off_t offset) {
    return lseek(fd, offset, SEEK_SET);
  }

  virtual int Write(int fd, const char* buf, size_t size) {
    return write(fd, buf, size);
  }

  virtual int Read(int fd, char* buf, size_t size) {
    return read(fd, buf, size);
  }

  virtual int Close(int fd) {
    return close(fd);
  }

  virtual bool GetStat(std::string stat, std::string* value) {
    return false;
  }

  virtual void Compact() {
  }

  virtual MetricStat* GetMetricStat() {
    return NULL;
  }
};

class TableFSWrapper : public FileSystemWrapper {
public:

  TableFSWrapper();

  ~TableFSWrapper();

  int Setup(Properties &prop);

  int Mknod(const char* path, mode_t mode, dev_t dev);

  int Mkdir(const char* path, mode_t mode);

  int Utime(const char* path, struct utimbuf *buf);

  int Chmod(const char* path, mode_t mode);

  int Stat(const char* path, struct stat *buf);

  int Listdir(const char* path);

  int Lsstat(const char* path);

  int Scanfile(const char* path);

  int Rename(const char* old_path, const char* new_path);

  int Unlink(const char* path);

  int Open(const char* path, int flags);

  int Write(int fd, const char* buf, size_t size);

  int Seek(int fd, off_t offset);

  int Read(int fd, char* buf, size_t size);

  int Close(int fd);

  bool GetStat(std::string stat, std::string* value);

  void Compact() {
    fs->Compact();
  }

  class TableFSWrapperStat: public MetricStat {
    TableFSWrapper* fsw_;

  public:
    TableFSWrapperStat(TableFSWrapper* fsw): fsw_(fsw) {
    }

    virtual void GetMetric(TMetList &metlist, time_t now) {
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
    }
  };

  MetricStat* GetMetricStat() {
    return new TableFSWrapperStat(this);
  }

protected:
  FileSystemState *tablefs_data;
  TableFS *fs;
  enum { FDLIMIT = 128 };
  struct fuse_file_info fis[FDLIMIT];
  std::string paths[FDLIMIT];
  off_t offsets[FDLIMIT];
  int freefd[FDLIMIT];
  int num_fd;
  bool logon;
  char junk[TFS_INODE_HEADER_SIZE];
  FILE* junkf;

  int GetFileDescriptor();
  void ReleaseFileDescriptor(int fd);
};

class TableFSTestWrapper : public TableFSWrapper {
public:
  int Mknod(const char* path, mode_t mode, dev_t dev);

  int Stat(const char* path, struct stat *buf);
};

}

#endif
#endif
