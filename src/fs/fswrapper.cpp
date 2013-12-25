#ifdef SUPPORTED
#include "fs/fswrapper.h"
#include <stdio.h>
#include "fs/tablefs.h"

namespace tablefs {

TableFSWrapper::TableFSWrapper() :
  fs(NULL), tablefs_data(NULL), junkf(NULL)
{
  memset(freefd, 0, sizeof(freefd));
}

int TableFSWrapper::Setup(Properties &prop) {
  tablefs_data = new tablefs::FileSystemState();
  if (tablefs_data->Setup(prop) < 0)
    return -1;
  fs = new TableFS();
  fs->SetState(tablefs_data);
  fs->Init(NULL);
  num_fd = 0;
  if (prop.getProperty("filesystem") == std::string("tablefs_pred")) {
    for (int i = 0; i < TFS_INODE_HEADER_SIZE; ++i) {
      junk[i] = 'a';
    }
    junkf = fopen("/dev/null", "wb");
  }
  logon = prop.getPropertyBool("fswrapper.logon", false);
  return 0;
}

TableFSWrapper::~TableFSWrapper() {
  fprintf(stderr, "DESTROY tablefs\n");
  if (fs != NULL) {
    fs->Destroy(tablefs_data);
    delete fs;
  }
  if (junkf != NULL) {
    fclose(junkf);
  }
}

int TableFSWrapper::Mknod(const char* path, mode_t mode, dev_t dev) {
  struct stat statbuf;
  int ret = Stat(path, &statbuf);
  if (ret == 0) {
    errno = EEXIST;
    return -1;
  }
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = fs->MakeNode(path, mode, dev);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the file: %s\n", path);
  }
  ret = Stat(path, &statbuf);
  return ret;
}

int TableFSWrapper::Mkdir(const char* path, mode_t mode) {
  struct stat statbuf;
  int ret = Stat(path, &statbuf);
  if (ret == 0) {
    return -1;
  }
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = fs->MakeDir(path, mode);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the directory: %s\n", path);
  }
  ret = Stat(path, &statbuf);
  return ret;
}

int TableFSWrapper::Utime(const char* path, struct utimbuf *buf) {
  struct timespec tv[2];
  tv[0].tv_sec = buf->actime;
  tv[1].tv_sec = buf->modtime;
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  int ret = fs->UpdateTimens(path, tv);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the directory: %s\n", path);
  }
  return ret;
}

int TableFSWrapper::Chmod(const char* path, mode_t mode) {
  struct stat statbuf;
  int ret = Stat(path, &statbuf);
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  ret = fs->Chmod(path, mode);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to chmod the path: %s\n", path);
  }
  ret = Stat(path, &statbuf);
  return ret;
}

int TableFSWrapper::Stat(const char* path, struct stat *buf) {
  int ret = fs->GetAttr(path, buf);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to stat the path: %s\n", path);
  }
  return ret;
}

int TableFSWrapper::Open(const char* path, int flags) {
  int fd = GetFileDescriptor();
  paths[fd] = std::string(path);
  offsets[fd] = 0;
  fis[fd].flags = flags;
  if (junkf != NULL) {
    fwrite(junk, sizeof(path), 1, junkf);
  }
  int ret = fs->Open(path, &(fis[fd]));
  if (ret != 0) {
    ReleaseFileDescriptor(fd);
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  return fd;
}

int TableFSWrapper::Seek(int fd, off_t offset) {
  offsets[fd] = offset;
  return 0;
}

int TableFSWrapper::Write(int fd, const char* buf, size_t size) {
  int ret = fs->Write(paths[fd].c_str(), buf, size, offsets[fd], &(fis[fd]));
  if (junkf != NULL) {
    fwrite(buf, size, 1, junkf);
  }
  if (ret < 0) {
    if (logon)
      fprintf(stderr, "Failed to write the path: %s\n", paths[fd].c_str());
  } else {
    offsets[fd] += ret;
  }
  return ret;
}

int TableFSWrapper::Read(int fd, char* buf, size_t size) {
  int ret = fs->Read(paths[fd].c_str(), buf, size, offsets[fd], &(fis[fd]));
  if (junkf != NULL) {
    fwrite(buf, size, 1, junkf);
  }
  if (ret < 0) {
    if (logon)
      fprintf(stderr, "Failed to read the path: %s\n", paths[fd].c_str());
  } else {
    offsets[fd] += ret;
  }
  return ret;
}

static int list_dir_count = 0;
int fuse_filler_dir(void* buf, const char* name, const struct stat *stbuf, off_t off) {
  ++list_dir_count;
}

int TableFSWrapper::Listdir(const char* path) {
  fuse_file_info fi;
  int ret = fs->OpenDir(path, &fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  list_dir_count = 0;
  if (fs->ReadDir(path, NULL, fuse_filler_dir, 0, &fi) != 0) {
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
  lsstat_fs->Stat(lsstat_fpath, &statbuf);
  ++lsstat_count;
}

int TableFSWrapper::Lsstat(const char* path) {
  fuse_file_info fi;
  int ret = fs->OpenDir(path, &fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }

  sprintf(lsstat_fpath, "%s/", path);
  lsstat_end_fpath = lsstat_fpath + strlen(lsstat_fpath);
  lsstat_fs = this;
  lsstat_count = 0;
  if (fs->ReadDir(path, NULL, lsstat_filler, 0, &fi) != 0) {
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
  scanfile_fs->Stat(scanfile_fpath, &statbuf);
  int fd = scanfile_fs->Open(scanfile_fpath, O_RDONLY);
  int read_size = 0;
  while (read_size < statbuf.st_size) {
    int ret = scanfile_fs->Read(fd, scanfile_content, 4096);
    if (ret == 0) break;
      read_size += ret;
  }
  scanfile_fs->Close(fd);
  ++scanfile_count;
}

int TableFSWrapper::Scanfile(const char* path) {
  fuse_file_info fi;
  int ret = fs->OpenDir(path, &fi);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to open the path: %s\n", path);
    return -1;
  }
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }

  sprintf(scanfile_fpath, "%s/", path);
  scanfile_end_fpath = scanfile_fpath + strlen(scanfile_fpath);
  scanfile_fs = this;
  scanfile_count = 0;
  if (fs->ReadDir(path, NULL, scanfile_filler, 0, &fi) != 0) {
    if (logon)
      fprintf(stderr, "Failed to listdir the path: %s\n", path);
    return -1;
  }
  return scanfile_count;
}

int TableFSWrapper::Rename(const char* old_path,
                           const char* new_path) {
  if (junkf != NULL) {
    fwrite(old_path, strlen(old_path), 1, junkf);
    fwrite(new_path, strlen(new_path), 1, junkf);
  }

  int ret = fs->Rename(old_path, new_path);
  if (ret != 0) {
    if (logon)
      fprintf(stderr,
              "Failed to rename %s to %s\n",
              old_path, new_path);
    return -1;
  }
  return 0;
}

int TableFSWrapper::Unlink(const char* path) {
  if (junkf != NULL) {
    fwrite(path, strlen(path), 1, junkf);
  }

  int ret = fs->Unlink(path);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to unlink %s\n", path);
    return -1;
  }
  return 0;
}

int TableFSWrapper::Close(int fd) {
  int ret = fs->Release(paths[fd].c_str(), &(fis[fd]));
  ReleaseFileDescriptor(fd);
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to close the path: %s\n", paths[fd].c_str());
  }
  return ret;
}

bool TableFSWrapper::GetStat(std::string stat, std::string* value) {
  if (junkf != NULL) {
    fwrite(junk, sizeof(junk), 1, junkf);
  }
  return fs->GetStat(stat, value);
}

int TableFSWrapper::GetFileDescriptor() {
  for (int i = 0; i < FDLIMIT; ++i)
    if (freefd[i] == 0) {
      freefd[i] = 1;
      return i;
    }
  return -1;
}

void TableFSWrapper::ReleaseFileDescriptor(int fd) {
  freefd[fd] = 0;
}

int TableFSTestWrapper::Mknod(const char* path, mode_t mode, dev_t dev) {
  struct stat statbuf;
  leveldb::Slice key(path, strlen(path));
  int ret = Stat(path, &statbuf);
  if (ret == 0) {
    errno = EEXIST;
    return -1;
  }
  ret = fs->metadb->Put(key, std::string(""));
  if (ret != 0) {
    if (logon)
      fprintf(stderr, "Failed to create the file: %s\n", path);
  }
  ret = Stat(path, &statbuf);
  return ret;
}

int TableFSTestWrapper::Stat(const char* path, struct stat *buf) {
  std::string result;
  leveldb::Slice key(path, strlen(path));
  int ret = 0;
  if (fs->metadb->Get(key, result) > 0) {
    lstat("/tmp", buf);
  } else {
    ret = -1;
    if (logon)
      fprintf(stderr, "Failed to stat the path: %s\n", path);
  }
  return ret;
}

}
#endif
