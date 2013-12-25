/*tan:This file is not at all included in any file*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include "fs/tfs_state.h"
#include "fs/tablefs.h"
#include "leveldb/env.h"
#include "util/monitor.h"
#include "util/traceloader.h"
#include "util/properties.h"

//using namespace tablefs;
//using namespace tablefs::TableFS;

struct StackEntry {
  char *dirname;
  struct fuse_file_info file_info;
  struct statbuf;
};

extern bool StartsWith(const char *a, const char *b, const int len) {
  if (strlen(a) < len || strlen(b) < len) {
	return false;
  }
  for (int i = 0; i < len; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

extern void Check(int retno) {
    if (retno != 0) {
        printf("filesystem goes wrong\n");
        Destroy(NULL);
        exit(-1);
    }
}

static char *Combine(const char *par, const char *ch) {
    if (strcmp(par,"/") == 0) {
        return strcat(par,ch);
    } else {
        return strcat(par,"/",ch);
    }
}
/*
static void Chdir(std::vector<StackEntry> &stack, const std::string& dir) {
    if (dir.compare("..") == 0) {
        StackEntry ent = stack[stack.size() - 1];
        Check(ReleaseDir(ent.dirname.c_str(), &ent.file_info));
        stack.pop_back();
    } else {
        StackEntry ent;
        ent.dirname = Combine(stack.back().dirname, dir);
        if (OpenDir(ent.dirname.c_str(), &ent.file_info) == 0) {
            stack.push_back(ent);
        } else {
            printf("Cannot Open The Directory\n");
        }
    }
}*/

static int Filler(void *buf, const char *name,
                  const struct stat *stbuf, off_t off) {
    printf("%s\n", name);
    return 0;
}

/*
static void List(std::vector<StackEntry> &stack) {
    StackEntry ent = stack[stack.size() - 1];
    Check(ReadDir(ent.dirname.c_str(), NULL, Filler,
                    0, &ent.file_info));
}*/

static void Mkdir(const char *dirname) {
//   printf("%s\n", dirname.c_str());
    if (MakeDir(dirname, 0) < 0) {
       printf("Failed to create the directory: %s\n", dirname);
    }
}

static void Mknode(const char *filename) {
//    printf("%s\n", dirname.c_str());
    if (MakeNode(filename, 0, 0) < 0) {
       printf("Failed to create the file: %s\n", filename);
    }
}

static void Rmdir(const char *dirname) {
//    printf("%s\n", dirname.c_str());
    if (RemoveDir(dirname) < 0) {
//            printf("Failed to remove the directory\n");
    }
}

static void Stat(const char *path) {
    struct stat statbuf;
    if (GetAttr(path, &statbuf) < 0) {
//       printf("Failed to stat the path: %s\n", path.c_str());
    }
}

static void Test1() {
    Init(NULL);
  //  std::vector<StackEntry> stack;
    StackEntry ent;
    ent.dirname = "/";
    Check(OpenDir(ent.dirname, &ent.file_info));
    stack.push_back(ent);
    char cmd[256];
    while (true) {
        int ret = scanf("%s", cmd);
        if (strcmp(cmd, "exit") == 0) {
                break;
        }
        else if (StartsWith(cmd, "cd", 2)) {
                scanf("%s", cmd);
                Chdir(stack, cmd);
        }
        else if (StartsWith(cmd, "mkdir", 5)) {
                scanf("%s", cmd);
                Mkdir(Combine(stack.back().dirname,cmd));
        }
        else if (StartsWith(cmd, "rmdir", 5)) {
                scanf("%s", cmd);
                Rmdir(Combine(stack.back().dirname,cmd));
        }
        else if (strcmp(cmd, "ls") == 0) {
                List(stack);
        }
    }
}

static void Test(Properties &prop) {
    FileSystemState *tablefs_data;
    tablefs_data = new tablefs::FileSystemState(prop.getPropertyInt("threshold"),
                                          prop.getProperty("metadir").c_str(),
                                          prop.getProperty("datadir").c_str(),
                                          prop.getProperty("logfile").c_str(),
                                          prop.getPropertyInt("sync.time.limit",-1));

    SetState(tablefs_data);

    Init(NULL);

    int mem_lock_size = 400;
    mem_lock_size = prop.getPropertyInt("mem.lock.size", 400);
    TraceLoader loader(mem_lock_size);
    loader.LoadTrace(prop.getProperty("tracefile").c_str());

    Monitor mon = Monitor(prop.getProperty("mon.partition"), 
                          prop.getProperty("mon.filesystem"));
    Monitor tmon = Monitor(prop.getProperty("mon.partition"), 
                           prop.getProperty("mon.filesystem"));
    mon.DoMonitor();
    tmon.DoMonitor();

    sync();

    srand(100);
    time_t time1 = time(NULL);
    int cnt = 0;
    for (int i = 0; i < loader.getNumPaths(); ++i) {
    	char filetype;
    	char* path = loader.getPath(i, filetype);
        Stat(path);
        if (filetype == 'f') {
            Mknode(path);
        } else
        if (filetype == 'd') {
            Mkdir(path);
        }
        Stat(path);
        cnt ++;
        if (cnt >= 1000) {
          tmon.DoMonitor();
          cnt = 0;
        }
    }

    time_t time2 = time(NULL);
    mon.DoMonitor();
    tmon.DoMonitor();
    printf("create directory tree , %d\n", (int) (time2 - time1));
    int nquery = prop.getPropertyInt("num.query");
    int npath = loader.getNumPaths();
    printf("nquery %d npath %d\n", nquery, npath);
    cnt = 0;
    for (int i = 0; i < nquery; ++i) {
      int pi = rand() % npath;
      char* path = loader.getPath(pi);
      if ((i & 3) == 0) {
        Chmod(path, 777);
      } else 
      if ((i & 3) == 1) {
        struct timespec tv[2];
        tv[0].tv_sec = time(NULL);
        tv[1].tv_sec = time(NULL);
        UpdateTimens(path, tv);
      } else {
        Stat(path);
      }
      cnt++;
      if (cnt >= 1000) {
        tmon.DoMonitor();
        cnt = 0;
      }
    }

    time_t time3 = time(NULL);
    mon.DoMonitor();
    tmon.DoMonitor();
    printf("update and query , %d\n", (int) (time3 - time2));

    char tpath[256];
    sprintf(tpath, "%s/test.txt", prop.getProperty("metadir").c_str()); 
    int fd = open(tpath, O_WRONLY); 
    fsync(fd);
    close(fd);
    sync();
    
    for (int i = 0; i < 20; ++i) {
      sleep(1);
      tmon.DoMonitor();
    }

    time_t time4 = time(NULL);
    mon.DoMonitor();
    tmon.DoMonitor();
    printf("sync , %d\n", (int) (time4 - time3));

    mon.Report();
    tmon.Report();

    Destroy(tablefs_data);
}

static void usage()
{
  fprintf(stderr,
          "USAGE:  tablefs_lib <TRACEFILE> <threshold> <METADIR> <DATADIR> <LOGFILE>\n");
  abort();
}

static void parseOpts(int argc, char *argv[], Properties &prop) {
  for (int i = 1; i < argc; ++i) 
    if (argv[i][0] == '-') {
      prop.setProperty(std::string(argv[i]+1),
                       std::string(argv[i+1]));
      printf("%s, %s\n", argv[i]+1, argv[i+1]);
      ++i; 
    }
}

//PangooFS MigrateThreshold METADIR DATADIR FuseMount LogFile
int main(int argc, char *argv[]) {
    if (argc < 2) {
      usage(); 
    }
    Properties prop;
    parseOpts(argc, argv, prop);
    Test(prop);
    return 0;
}

