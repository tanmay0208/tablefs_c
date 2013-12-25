//#include <vector>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <utime.h>
#include <string.h>
#include <unistd.h>
#include "port/port.h"
//#include "leveldb/env.h"
#include "fs/fswrapper.h"
#include "util/monitor.h"
#include "util/allocator.h"
#include "util/traceloader.h"
#include "util/properties.h"
#include "util/command.h"
#include "util/random.h"
#include "util/mutexlock.h"
#include "util/socket.h"
//#ifdef SUPPORTED
//namespace tablefs {

const int MONITOR_INTERVAL = 10000000;
enum Mode {
    Uniform,
    Skewed,
    Default
};

typedef enum Mode Mode; 
struct RandomPicker {
//public:
  
//private
  Random rand;
  Mode mode_;
  int range_;
  int max_log_;
};

typedef struct RandomPicker RandomPicker;

void RandomPicker_constructor(RandomPicker randompicker,uint32_t seed, char *mode, int range)
{
  	  seed=rand();
  	  randompicker.range_=range;
      if (strcmp(mode,"uniform") == 0) {
        randompicker.mode_ = Uniform;                          
      } else if (strcmp(mode,"skewed") == 0) {
        randompicker.mode_ = Skewed;
      } else {
        randompicker.mode_ = Uniform;
      }
      if (randompicker.mode_ == Skewed) {
        randompicker.max_log_ = 0;
        uint32_t max_exp = 1;
        while (max_exp < randompicker.range_) {
          randompicker.max_log_ += 1;
          max_exp = max_exp << 1;
        }
      } 
      else {
        randompicker.max_log_ = 0;
      }
}

uint32_t RandomPicker_Next(RandomPicker randompicker) {
    switch (randompicker.mode_) {
      case Uniform: return /*rand.Uniform*/(randompicker.range_);
      case Skewed: return /*rand.Skewed*/(randompicker.max_log_) % randompicker.range_;
      default: return 0;
    }
}

struct SharedState {
    //leveldb::port::Mutex mu;
    //leveldb::port::CondVar cv;
    int total;

    int num_initialized;
    int num_done;
    bool start;
    //SharedState() : cv(&mu) {}                        //apan:Make constructor
};

typedef struct SharedState SharedState;

struct ThreadState {
    int tid;
    int step;
    int op_count_;
    SharedState* shared;
};

typedef struct ThreadState ThreadState;

void ThreadState_constructor(ThreadState * threadstate,int index, int numthreads){
	  threadstate->tid=index;
	  threadstate->step=numthreads;
	  threadstate->op_count_=0;
} 

void ThreadState_IncreaseCount(ThreadState * threadstate) {
      ++(threadstate->op_count_);
}

void ThreadState_IncreaseCount_num(ThreadState * threadstate,int num_ops) {
      threadstate->op_count_ += num_ops;
}

int ThreadState_GetCount(ThreadState * threadstate) {
      return threadstate->op_count_;
}

/*
void SendMetric(const char* metricString) {
  UDPSocket sock;
  try {
    sock.sendTo(metricString, strlen(metricString),
                std::string("127.0.0.1"), 10600);
  } catch (SocketException &e) {
  }
}*/

struct OperationStat{
  //:public MetricStat {
//private:
  int num_threads;
  ThreadState** targs;
  bool enabled;
  time_t last;
};

typedef struct OperationStat OperationStat;
//public:

void OperationStat_constructor(OperationStat *operationstat){
  	operationstat->enabled=false;
  	operationstat->num_threads=0;
  	operationstat->targs=NULL;
  	operationstat->last=0;
}
/*
void OperationStat_GetMetric(OperationStat *operationstat,TMetList &metlist, time_t now) {
    int i;
    if (operationstat->enabled) {
      char metname[36] = "Opcount_thread_";
      char* metname_end = metname + strlen(metname);
      int tot_count = 0;
      for (i = 0; i < operationstat->num_threads; ++i) {
        sprintf(metname_end, "%08d", i);
        int op_count = targs[i]->GetCount();
        tot_count += op_count;
        AddMetric(metlist, metname, now, op_count);
      }
      if (now - last > 5) {
        printf("opcount %d\n", tot_count);
        last = now;
        char metricString[128];
        sprintf(metricString, "fsbench.opcount %d %d\n", now, tot_count);
        SendMetric(metricString);
      }
      AddMetric(metlist, ("Opcount"), now, tot_count);
    }
}*/

void OperationStat_SetThreadArgs(OperationStat *operationstat,int n, ThreadState** args) {
    operationstat->targs = args;
    operationstat->num_threads = n;
    operationstat->enabled = true;
}

void OperationStat_Disable(OperationStat *operationstat) {
    operationstat->targs = NULL;
    operationstat->num_threads = 0;
    operationstat->enabled = false;
}

struct FileSystemBenchmark {
//private:
  Properties *prop;

  int mem_lock_size;
  Allocator *allocator;
  TraceLoader *loader;
  int* path_order;

  char* mon_part;
  char* mon_fs;
  char* mon_mount;
  FileSystemWrapper *fs;

  int num_threads;
  char* benchmarks;

  OperationStat* opstat;
  Monitor* tmon;
  Monitor* emon;
  FILE* logf;
};

typedef struct FileSystemBenchmark FileSystemBenchmark; 

//public:

void FileSystemBenchmark_constructor(FileSystemBenchmark *filesystembenchmark,Properties *properties){
    int i;
    filesystembenchmark->prop=properties;
    filesystembenchmark->tmon=NULL;
    filesystembenchmark->emon=NULL;
    filesystembenchmark->fs=NULL;
    filesystembenchmark->logf =NULL;
    filesystembenchmark->allocator=NULL;
    filesystembenchmark->path_order=NULL;

    char *filename = Properties_getProperty(filesystembenchmark->prop,"monitor_logfile");
    filesystembenchmark->logf = fopen(filename, "w");
    if (filesystembenchmark->logf == NULL) {
      fprintf(stderr, "Cannot open monitor logfile!\n");
      exit(1);
    } else {
      printf("open log file: %s\n", filename);
    }
    Properties_Report(filesystembenchmark->prop,filesystembenchmark->logf);

    filesystembenchmark->mem_lock_size = Properties_getPropertyInt(filesystembenchmark->prop,"mem_lock_size", 400);
    if (filesystembenchmark->mem_lock_size < 0)
      filesystembenchmark->mem_lock_size = 0;
    if (Properties_getPropertyBool(filesystembenchmark->prop,"loadtrace", true)) {
      filesystembenchmark->allocator = (Allocator *)malloc(sizeof(Allocator)); 
      Allocator_constructor(filesystembenchmark->allocator,filesystembenchmark->mem_lock_size);
      filesystembenchmark->loader = (TraceLoader *)malloc(sizeof(TraceLoader));
      TraceLoader_constructor(filesystembenchmark->loader,filesystembenchmark->allocator);
      TraceLoader_LoadTrace(filesystembenchmark->loader,Properties_getProperty(filesystembenchmark->prop,"pathfile"));

      int npath = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",npath);
      //path_order = new int[npath];
      int path_order[npath];
      filesystembenchmark->path_order[0] = 0;
      for (i = 1; i < npath; ++i) {
        int j = rand() % i;
        filesystembenchmark->path_order[i] = filesystembenchmark->path_order[j];
        filesystembenchmark->path_order[j] = i;
      }
    } 
    else {
      filesystembenchmark->loader = NULL;
    }

    strcpy(filesystembenchmark->mon_part,Properties_getProperty(filesystembenchmark->prop,"mon_partition"));
    strcpy(filesystembenchmark->mon_fs,Properties_getProperty(filesystembenchmark->prop,"mon_filesystem"));
    strcpy(filesystembenchmark->mon_mount,Properties_getProperty(filesystembenchmark->prop,"mon_mountpoint"));
    if (strlen(filesystembenchmark->mon_part) == 0 || strlen(filesystembenchmark->mon_fs) == 0 || strlen(filesystembenchmark->mon_mount) == 0) {
      fprintf(stderr, "No partition, filesystem, or mount point for monitoring");
      exit(1);
    }
    
    filesystembenchmark->emon=(Monitor *)malloc(sizeof(Monitor));
    Monitor_constructor(filesystembenchmark->emon,filesystembenchmark->mon_part);
    filesystembenchmark->tmon=(Monitor *)malloc(sizeof(Monitor));
    Monitor_constructor(filesystembenchmark->emon);
    filesystembenchmark->opstat=(OperationStat *)malloc(sizeof(OperationStat));
    OperationStat_constructor(filesystembenchmark->opstat);
    //Monitor_AddMetricStat(filesystembenchmark->tmon,filesystembenchmark->opstat);    //Not able to understand

    filesystembenchmark->num_threads = Properties_getPropertyInt(filesystembenchmark->prop,"numthreads", 1);
    filesystembenchmark->benchmarks = Properties_getProperty(filesystembenchmark->prop,"benchmarks");
    if (strlen(filesystembenchmark->benchmarks) == 0) {
      fprintf(stderr, "No benchmark is found.");
    }

    char *benchfs = Properties_getProperty(filesystembenchmark->prop,"filesystem");
    if (strcmp(benchfs,"tablefs_user")==0 || strcmp(benchfs,"tablefs_pred")==0) {
      filesystembenchmark->fs =(TableFSWrapper *)malloc(sizeof(TableFSWrapper));
      TableFSWrapper_constructor(filesystembenchmark->fs);
      if (TableFSWrapper_Setup(filesystembenchmark->fs,filesystembenchmark->prop) < 0) {
        fprintf(stderr, "Fail to open tablefs_user\n");
        exit(1);
      }
      Properties_setProperty(filesystembenchmark->prop,"target", "/");
    } 
    else if (strcmp(benchfs,"tablefs_debug")==0) {
      filesystembenchmark->fs=(TableFSWrapper *)malloc(sizeof(TableFSWrapper));
      TableFSTestWrapper_constructor(filesystembenchmark->fs);
      if (TableFSWrapper_Setup(filesystembenchmark->fs,filesystembenchmark->prop) < 0) {
        fprintf(stderr, "Fail to open tablefs_user\n");
        exit(1);
      }
    } 
    else {
      filesystembenchmark->fs=(FileSystemWrapper *)malloc(sizeof(FileSystemWrapper));
      FileSystemWrapper_constructor(filesystembenchmark->fs);
    }
   // Monitor_AddMetricStat(filesystembenchmark->tmon,TableFSWrapperStat_GetMetricStat(filesystembenchmark->fs));  //Not able to understand
}

  void FileSystemBenchmark_destructor(FileSystemBenchmark *filesystembenchmark) {
    if (filesystembenchmark->logf != NULL) {
      if (filesystembenchmark->tmon != NULL)
        Monitor_Report_logf(filesystembenchmark->tmon,filesystembenchmark->logf);
      if (filesystembenchmark->emon != NULL)
        Monitor_Report_logf(filesystembenchmark->emon,filesystembenchmark->logf);
      fclose(filesystembenchmark->logf);
    }
    free(filesystembenchmark->fs);
    if (filesystembenchmark->path_order != NULL)
      free(filesystembenchmark->path_order);
    if (filesystembenchmark->tmon != NULL)
      free(filesystembenchmark->tmon);
    if (filesystembenchmark->emon != NULL)
      free(filesystembenchmark->emon);
    if (filesystembenchmark->loader != NULL)
      free(filesystembenchmark->loader);
    if (filesystembenchmark->allocator != NULL)
      free(filesystembenchmark->allocator);
  }

//private:
  void FileSystemBenchmark_MetadataCreate(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    int i;
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int cnt = 0;
    int numdirs = TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    printf("number dir %d\n", numdirs);
    for (i = 0; i < numdirs; ++i) {
      char* path = TraceLoader_GetDirPath(filesystembenchmark->loader,i);
      if (path != NULL) {
        strcpy(end_fpath, path);
        if (FileSystemWrapper_Mkdir(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
          perror(fpath);
        }
        ThreadState_IncreaseCount(thread);
      }
    }
    int numpaths = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",numpaths);
    
    for (i = thread->tid; i < numpaths; i += thread->step) {
      char* path;
      strcpy(path,TraceLoader_GetFilePath(filesystembenchmark->loader,i));
      if (path != NULL) {
        strcpy(end_fpath, path);
        if (FileSystemWrapper_Mknod(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG | S_IRWXO, 0) < 0) {
          perror(fpath);
          ++cnt;
        }
        ThreadState_IncreaseCount(thread);
      }
    }
    printf("Bad count = %d\n", cnt);
  }

  void FileSystemBenchmark_MetadataCreateWithCompact(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    MetadataCreate(thread);
    FileSystemWrapper_Compact(filesystembenchmark->fs);
  }

  void FileSystemBenchmark_MetadataQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int nquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",nquery);
    int ratio = Properties_getPropertyInt(filesystembenchmark->prop,"readwrite_readratio",ratio);
    int npath = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",npath)
               + TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);

    struct stat statbuf;
    srand(100);
    int i;
    for (i = 0; i < nquery; ++i) {
      int pi = rand() % npath;
      char* path;
      strcpy(path,TraceLoader_GetPath_default(filesystembenchmark->loader,pi));

      if (path != NULL) {
        strcpy(end_fpath, path);
        int qi = rand() % 100;
        int ret;
        if ((qi % 100) < ratio/2) {
          ret = FileSystemWrapper_Chmod(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG );
        } else
        if ((qi % 100) < ratio) {
          struct utimbuf timbuf;
          timbuf.actime = time(NULL);
          timbuf.modtime = time(NULL);
          ret = FileSystemWrapper_Utime(filesystembenchmark->fs,fpath, &timbuf);
        } 
        else {
          ret = FileSystemWrapper_Stat(filesystembenchmark->fs,fpath, &statbuf);
        }
        if (ret < 0) {
          printf("Error %s\n", fpath);
        }
        ThreadState_IncreaseCount(thread);
      }
    }
  }

  void FileSystemBenchmark_OnedirCreate(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int numpaths = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",numpaths);
    srand(100);
    int i,bad_count = 0;
    for (i = thread->tid; i < numpaths; i += thread->step) {
      sprintf(end_fpath, "/f%016d", rand() % numpaths);
      if (FileSystemWrapper_Mknod(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG | S_IRWXO, 0) < 0) {
        bad_count ++;
      }
      ThreadState_IncreaseCount(thread);
    }
    printf("BadCount = %d\n", bad_count);
  }

  void FileSystemBenchmark_OnedirQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    int i;
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int nquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",nquery);
    int ratio = Properties_getPropertyInt(filesystembenchmark->prop,"readwrite_readratio",ratio);
    int npath = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",npath);

    struct stat statbuf;
    srand(100);
    int bad_count = 0;
    for (i = 0; i < nquery; ++i) {
      int pi = rand() % npath;
      sprintf(end_fpath, "/f%016d", pi);
      int qi = rand() % 100;
      int ret;
      if ((qi % 100) < ratio/2) {
        ret = FileSystemWrapper_Chmod(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG );
      } else
      if ((qi % 100) < ratio) {
        struct utimbuf timbuf;
        timbuf.actime = time(NULL);
        timbuf.modtime = time(NULL);
        ret = FileSystemWrapper_Utime(filesystembenchmark->fs,fpath, &timbuf);
      } else {
        ret = FileSystemWrapper_Stat(filesystembenchmark->fs,fpath, &statbuf);
      }
      if (ret < 0) {
        bad_count ++;
      }
      ThreadState_IncreaseCount(thread);
    }
    printf("BadCount = %d\n", bad_count);
  }

  void FileSystemBenchmark_SmallFileCreate(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    int i;
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int len_fpath = strlen(fpath);
    int numpaths = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",numpaths) +
                   TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    int content_size = Properties_getPropertyInt(filesystembenchmark->prop,"filesize",content_size);
    char content[content_size];
    for (i = 0; i < content_size; ++i)
      content[i] = rand() % 26 + 97;
    for (i = thread->tid; i < numpaths; i += thread->step) {
      char filetype;
      char* path = TraceLoader_GetPath(filesystembenchmark->loader,i, filetype);
      if (path != NULL) {
        strcpy(end_fpath, path);
        if (filetype == 'f') {
          FileSystemWrapper_Mknod(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG | S_IRWXO, 0);
          int fd = FileSystemWrapper_Open(filesystembenchmark->fs,fpath, O_WRONLY);
          if (fd >= 0) {
            FileSystemWrapper_Write(filesystembenchmark->fs,fd, content, content_size);
            FileSystemWrapper_Close(filesystembenchmark->fs,fd);
          } else {
            perror(fpath);
            return;
          }
        } else
        if (filetype == 'd') {
            if (FileSystemWrapper_Mkdir(filesystembenchmark->fs,fpath, S_IRWXU | S_IRWXG | S_IRWXO) < 0)  {
              perror(fpath);
            }
        }
        ThreadState_IncreaseCount(thread);
      }
    }
  }

  void FileSystemBenchmark_SmallFileQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    size_t content_size = Properties_getPropertyInt(filesystembenchmark->prop,"filesize",content_size);
    char content[content_size];
    int i,j;
    for (i = 0; i < content_size; ++i)
      content[i] = rand() % 26 + 97;

    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    int len_fpath = strlen(fpath);

    int n = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",n);
    int nfilepath = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",nfilepath);
    int ratio = Properties_getPropertyInt(filesystembenchmark->prop,"readwrite_readratio",0);

    srand(100);
    for (j = 0; j < n; ++j) {
      int pi = rand() % nfilepath;
      char* path;
      path=(char *)malloc(sizeof(char));
      strcpy(path,TraceLoader_GetFilePath(filesystembenchmark->loader,pi));
      sprintf(fpath+len_fpath, "%s\0", path);

      int qi = rand() % 100;
      if (qi % 100 < ratio) {
        int fd = FileSystemWrapper_Open(filesystembenchmark->fs,fpath, O_RDONLY);
        if (fd >= 0) {
          FileSystemWrapper_Read(filesystembenchmark->fs,fd, content, content_size);
          FileSystemWrapper_Close(filesystembenchmark->fs,fd);
        }
      } else {
        int fd = FileSystemWrapper_Open(filesystembenchmark->fs,fpath, O_WRONLY);
        if (fd >= 0) {
          FileSystemWrapper_Write(filesystembenchmark->fs,fd, content, content_size);
          FileSystemWrapper_Close(filesystembenchmark->fs,fd);
        }
      }
      ThreadState_IncreaseCount(thread);
    }
    free(content);
  }

  void FileSystemBenchmark_DeleteQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);

    int i,err_cnt = 0;
    int numquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",numquery);
    int numfiles = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",numfiles);
    RandomPicker picker;
    RandomPicker_constructor(picker,thread->tid+1, "uniform", numfiles);

    for (i = thread->tid; i < numquery; i += thread->step) {
      char* path;
      path=(char *)malloc(sizeof(char));
      strcpy(path,TraceLoader_GetFilePath(filesystembenchmark->loader,filesystembenchmark->path_order[i % numfiles]));
      if (path != NULL) {
        strcpy(end_fpath, path);
        if (FileSystemWrapper_Unlink(filesystembenchmark->fs,fpath) < 0)  {
          ++err_cnt;
        } else {
          ThreadState_IncreaseCount(thread);
        }
      }
    }
    printf("Error count = %d\n", err_cnt);
  }

  void FileSystemBenchmark_ScanQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    int i;
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int err_cnt = 0;
    int numquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",numquery);
    int numdirs = TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    RandomPicker picker;
    RandomPicker_constructor(picker,thread->tid+1, "uniform", numdirs);
    for (i = thread->tid; i < numquery; i += thread->step) {
      char* path = TraceLoader_GetDirPath(filesystembenchmark->loader,RandomPicker_Next(picker));
      if (path != NULL) {
        strcpy(end_fpath, path);
        int ops;
        if ((ops = FileSystemWrapper_Listdir(filesystembenchmark->fs,fpath)) < 0)  {
          ++err_cnt;
        } 
        else {
          ThreadState_IncreaseCount_num(thread,ops);
        }
      }
    }
    printf("Error count = %d\n", err_cnt);
  }

  void FileSystemBenchmark_LsstatQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int err_cnt = 0;
    int numquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",numquery);
    int numdirs = TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    RandomPicker picker;
    RandomPicker_constructor(picker,thread->tid+1, "uniform", numdirs);
    int i;
    for (i = thread->tid; i < numquery; i += thread->step) {
      char* path;
      strcpy(path,TraceLoader_GetDirPath(filesystembenchmark->loader,RandomPicker_Next(picker)));
      if (path != NULL) {
        strcpy(end_fpath, path);
        int ops;
        if ((ops = FileSystemWrapper_Lsstat(filesystembenchmark->fs,fpath)) < 0)  {
          ++err_cnt;
        } else {
          ThreadState_IncreaseCount_num(thread,ops);
        }
      }
    }
    printf("Error count = %d\n", err_cnt);
  }

  void FileSystemBenchmark_ScanfileQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    int i;
    char fpath[4096];
    sprintf(fpath, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_fpath = fpath + strlen(fpath);
    int err_cnt = 0;
    int numquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",numquery);
    int numdirs = TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    RandomPicker picker;
    RandomPicker_constructor(picker,thread->tid+1, "uniform", numdirs);
    for (i = thread->tid; i < numquery; i += thread->step) {
      char* path = TraceLoader_GetDirPath(filesystembenchmark->loader,RandomPicker_Next(picker));
      if (path != NULL) {
        strcpy(end_fpath, path);
        int ops;
        if ((ops = FileSystemWrapper_Scanfile(filesystembenchmark->fs,fpath)) < 0)  {
          ++err_cnt;
        } else {
          ThreadState_IncreaseCount_num (thread,ops);
        }
      }
    }
    printf("Error count = %d\n", err_cnt);
  }

  void FileSystemBenchmark_RenameQueryTest(FileSystemBenchmark *filesystembenchmark,ThreadState* thread) {
    char src_path[4096];
    char dst_path[4096];
    sprintf(src_path, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    sprintf(dst_path, "%s/", Properties_getProperty(filesystembenchmark->prop,"target"));
    char* end_src_path = src_path + strlen(src_path);
    char* end_dst_path = dst_path + strlen(dst_path);

    int err_cnt = 0;
    int numquery = Properties_getPropertyInt(filesystembenchmark->prop,"query_number",numquery);
    int numdirs = TraceLoader_GetNumDirectoryPaths(filesystembenchmark->loader);
    int numfiles = Properties_getPropertyInt(filesystembenchmark->prop,"create_numpaths",numfiles);
    int rename_type = Properties_getPropertyInt(filesystembenchmark->prop,"rename_type", 0);

    RandomPicker dir_picker;
    RandomPicker_constructor(dir_picker,thread->tid+1, "uniform", numdirs);
    int i;
    for ( i = thread->tid; i < numquery; i += thread->step) {
      char* path;
      strcpy(path,TraceLoader_GetFilePath(filesystembenchmark->loader,filesystembenchmark->path_order[i % numfiles]));
      if (path != NULL) {
        strcpy(end_src_path, path);
        if (rename_type == 0) {
          strcpy(end_dst_path, path);
          strcpy(dst_path + strlen(dst_path), ".rename");
        } else {
          path = TraceLoader_GetDirPath(filesystembenchmark->loader,RandomPicker_Next(dir_picker));
          strcpy(end_dst_path, path);
          sprintf(dst_path + strlen(dst_path), "%016d", i);
        }

        if (FileSystemWrapper_Rename(filesystembenchmark->fs,src_path, dst_path) < 0)  {
          ++err_cnt;
        } else {
          ThreadState_IncreaseCount(thread);
        }
      }
    }
    printf("Error count = %d\n", err_cnt);
  }


#ifdef SUPPORTED                                        //tan:Remaining todo
  struct ThreadArg {
      FileSystemBenchmark* fsb;
      SharedState* shared;
      ThreadState* thread;
      void (FileSystemBenchmark::*method) (ThreadState*);
  };

                                       

  static void ThreadBody(void* v) {
    ThreadArg* arg = reinterpret_cast<ThreadArg*>(v);
    SharedState* shared = arg->shared;
    ThreadState* thread = arg->thread;
    {
      leveldb::MutexLock l(&shared->mu);
      shared->num_initialized++;
      if (shared->num_initialized >= shared->total) {
        shared->cv.SignalAll();
      }
      while (!shared->start) {
        shared->cv.Wait();
      }
    }

    (arg->fsb->*(arg->method))(thread);

     {
      leveldb::MutexLock l(&shared->mu);
      shared->num_done++;
      if (shared->num_done >= shared->total) {
        shared->cv.SignalAll();
      }
    }
  }

  static void MonitorThreadBody(void* v) {
    ThreadArg* arg = reinterpret_cast<ThreadArg*>(v);
    SharedState* shared = arg->shared;
    {
      leveldb::MutexLock l(&shared->mu);
      shared->num_initialized++;
      if (shared->num_initialized >= shared->total) {
        shared->cv.SignalAll();
      }
      while (!shared->start) {
        shared->cv.Wait();
      }
    }

    int count = 0;
    int alpha = 1;
    while (shared->num_done < shared->total - 1) {
      arg->fsb->tmon->DoMonitor();
      count ++;
      if (count > 10000) {
        alpha ++;
        count = 0;
      }
      leveldb::Env::Default()->SleepForMicroseconds(MONITOR_INTERVAL * alpha);
    }
    arg->fsb->tmon->DoMonitor();
    shared->num_done ++;
    shared->cv.SignalAll();
  }

  void RunBenchmark(int n,
                    void (FileSystemBenchmark::*method)(ThreadState*)) {
    SharedState shared;
    shared.total = n+1;
    shared.num_initialized = 0;
    shared.num_done = 0;
    shared.start = false;

    ThreadArg* arg = new ThreadArg[n+1];
    ThreadState** thread_states = new ThreadState*[n];
    for (int i = 0; i < n; i++) {
      arg[i].fsb = this;
      arg[i].method = method;
      arg[i].shared = &shared;
      arg[i].thread = new ThreadState(i, n);
      thread_states[i] = arg[i].thread;
      arg[i].thread->shared = &shared;
    }
    arg[n].fsb = this;
    arg[n].shared = &shared;
    opstat->SetThreadArgs(n, thread_states);

    for (int i = 0; i < n; i++) {
      leveldb::Env::Default()->StartThread(ThreadBody, &arg[i]);
    }
    //TODO: Add monitor thread (change n to n + 1, < n to <=n )
    leveldb::Env::Default()->StartThread(MonitorThreadBody, &arg[n]);

    shared.mu.Lock();
    while (shared.num_initialized <= n) {
      shared.cv.Wait();
    }


    emon->DoMonitor();
    tmon->DoMonitor();

    char metricString[128];
    sprintf(metricString, "fsbench.stage %d 100\n", time(NULL));
    SendMetric(metricString);

    shared.start = true;
    shared.cv.SignalAll();
    while (shared.num_done <= n) {
      shared.cv.Wait();
    }
    shared.mu.Unlock();

    sprintf(metricString, "fsbench.stage %d 200\n", time(NULL));
    SendMetric(metricString);

    emon->DoMonitor();
    tmon->DoMonitor();

    opstat->Disable();
    for (int i = 0; i < n; i++) {
      delete arg[i].thread;
    }
    delete[] arg;
    delete[] thread_states;
  }

public:
  void Run() {
    size_t lastbench = 0;
    while (lastbench < benchmarks.size()) {
      size_t sep = benchmarks.find(',', lastbench);
      std::string bench;
      if (sep == std::string::npos) {
        bench = benchmarks.substr(lastbench, benchmarks.size() - lastbench);
        lastbench = benchmarks.size();
      } else {
        bench = benchmarks.substr(lastbench, sep - lastbench);
        lastbench = sep + 1;
      }

      void (FileSystemBenchmark::*method)(ThreadState*) = NULL;
      if (bench == std::string("metadatacreate")) {
        method = &FileSystemBenchmark::MetadataCreate;
      } else if (bench == std::string("metadatacreatecompact")) {
        method = &FileSystemBenchmark::MetadataCreateWithCompact;
      } else if (bench == std::string("metadataquery")) {
        method = &FileSystemBenchmark::MetadataQueryTest;
      } else if (bench == std::string("onedircreate")) {
        method = &FileSystemBenchmark::OnedirCreate;
      } else if (bench == std::string("onedirquery")) {
        method = &FileSystemBenchmark::OnedirQueryTest;
      }else if (bench == std::string("smallfilecreate")) {
        method = &FileSystemBenchmark::SmallFileCreate;
      } else if (bench == std::string("smallfilequery")) {
        method = &FileSystemBenchmark::SmallFileQueryTest;
      } else if (bench == std::string("scanquery")) {
        method = &FileSystemBenchmark::ScanQueryTest;
      } else if (bench == std::string("lsstatquery")) {
        method = &FileSystemBenchmark::LsstatQueryTest;
      } else if (bench == std::string("scanfilequery")) {
        method = &FileSystemBenchmark::ScanfileQueryTest;
      } else if (bench == std::string("renamequery")) {
        method = &FileSystemBenchmark::RenameQueryTest;
      } else if (bench == std::string("deletequery")) {
        method = &FileSystemBenchmark::DeleteQueryTest;
      } else {
        fprintf(stderr, "Cannot find the benchmark %s", bench.c_str());
        continue;
      }

      time_t start = time(NULL);
      RunBenchmark(num_threads, method);
      time_t end = time(NULL);

      fprintf(logf, "%s: %ld\n", bench.c_str(), end-start);

      sync();
      command::DropBufferCache();
    }
  }
};

}  // namespace leveldb

int main(int argc, char *argv[]) {
  tablefs::Properties prop;
  prop.parseOpts(argc, argv);
  std::string config_filename = prop.getProperty("configfile");
  if (config_filename.size() > 0) {
    prop.load(config_filename);
  }
  tablefs::FileSystemBenchmark bench = tablefs::FileSystemBenchmark(prop);
  bench.Run();
  return 0;
}
#endif
