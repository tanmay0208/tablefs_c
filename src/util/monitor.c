//#ifdef SUPPORTED
#include "util/monitor.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

//namespace tablefs {

static const char* diskmetric[11] = {
  "read_requests",        // Total number of reads completed successfully.
  "read_merged",          // Adjacent read requests merged in a single req.
  "read_sectors",         // Total number of sectors read successfully.
  "msec_read",            // Total number of ms spent by all reads.
  "write_requests",       // total number of writes completed successfully.
  "write_merged",         // Adjacent write requests merged in a single req.
  "write_sectors",        // total number of sectors written successfully.
  "msec_write",           // Total number of ms spent by all writes.
  "ios_in_progress",      // Number of actual I/O requests currently in flight
  "msec_total",           // Amount of time during which ios_in_progress >= 1.
  "msec_weighted_total",  // Measure of recent I/O completion time and backlog
};

//class IOStat: public MetricStat {
 
 //public:

  void IOStat_constructor(IOStat *iostat,const char *device_name) {
    strcpy(iostat->devname ,device_name);
   }
 
   void MetricStat_GetMetric(IOStat *iostat,hash_map *TMetList, time_t now) {
    FILE* f=fopen("/proc/diskstats", "r");
    int i;
    if (f == NULL) {
      return;
    }
    char device[20];
    TTSValue metric[11];
    int ret = 0;
    while (!feof(f)) {
      if (fscanf(f, "%s %s %s", device, device, device) < 3) {
        break;
      }
      //if (devname.compare(device) == 0) {
        if(strcmp(iostat->devname,device)==0)
        { 
        if (fscanf(f, "%lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", 
                   &metric[0], &metric[1], &metric[2], &metric[3],
                   &metric[4], &metric[5], &metric[6], &metric[7],
                   &metric[8], &metric[9], &metric[10]) < 11) {
            break;
        }
        for (i = 0; i < 11; ++i) {
          //AddMetric(metlist, devname+"."+std::string(diskmetric[i]), now , metric[i]);
        }
      } else {
        char c;
        do {
          c = getc(f);
        } while (c != '\n' && c != EOF);
      }
    }

    fclose(f);
  }

/*const static*/ int NUM_SLAB_METRIC = 3;

/*static const char* slabmetric[NUM_SLAB_METRIC] = {
  "active_objs",
  "num_objs",
  "objsize",
};

const static int NUM_SLAB_OBJ = 2;
static const char* slabobjname[NUM_SLAB_OBJ] = {
  "dentry",
  "inode_cache"
};*/

//class SlabStat: public MetricStat {

//public:
  void SlabStat_constructor(SlabStat *slabstat,const char *filesystem_name) {
    strcpy(slabstat->fsname,filesystem_name);
    slabstat->len_fsname = strlen(slabstat->fsname);
  }

  void SlabStat_GetMetric(SlabStat *slabstat,hash_map *TMetList, time_t now) {
    FILE* f=fopen("/proc/slabinfo", "r");
    int i,j;
    if (f == NULL) {
      return;
    }
    char objname[256];
    char junk[256];
    int tmp;
    TTSValue metric;
    if (fread(junk, 204, 1, f) != 1) {
      return;
    }
    while (!feof(f)) {
      if (fscanf(f, "%s", objname) < 1) {
        break;
      }
      if (strncmp(slabstat->fsname, objname, slabstat->len_fsname) == 0) {
        for (i = 0; i < NUM_SLAB_METRIC; ++i) {
          if (fscanf(f, "%lld", &metric) < 1) {
            break;
          }
          //AddMetric(metlist, std::string(objname)+"."+std::string(slabmetric[i]),
            //        now , metric);
        }
      } else {
        //for (j = 0; j < NUM_SLAB_OBJ; ++j)
          //if (strncmp(objname, slabobjname[j],
                      //strlen(slabobjname[j])) == 0)
          {
            for (i = 0; i < NUM_SLAB_METRIC; ++i) {
              if (fscanf(f, "%lld", &metric) < 1) {
                break;
              }
              //AddMetric(metlist, std::string(slabobjname[j])+"."+std::string(slabmetric[i]),
                //        now , metric);
            }
            break;
          }
      }
      char c;
      do {
        c = getc(f);
      } while (c != '\n' && c != EOF);
    }
    fclose(f);
  }
//};

//class MemStat: public MetricStat {
struct MemStat
{
  MetricStat *metricstat;
  char *fsname;/* data */
};
typedef struct MemStat MemStat;  

//public:
  void MemStat_GetMetric(MemStat *memstat,hash_map *TMetList, time_t now) {
    int i;
    FILE* f=fopen("/proc/meminfo", "r");
    if (f == NULL) {
      return;
    }
    char metname[256];
    char junk[256];
    int tmp;
    TTSValue metric;
    for (i = 0; i < 30; ++i) {
      if (fscanf(f, "%s %lld %s", metname, &metric, junk) < 3) {
        break;
      }
      //AddMetric(metlist, std::string(metnMemame, strlen(metname)), now , metric);
    }
    fclose(f);
  }

//class ProcStat: public MetricStat {
//private:
  struct ProcStat{
  MetricStat *metricstat;
  int pid;
  int page_size;
  char *name;
  };
  typedef struct ProcStat ProcStat;

  int ProcStat_FindPid(ProcStat *procstat,char *cmdline) {
    char shellcmd[1024];
   // sprintf(shellcmd, "ps -ef | grep /'%s/' | cut -c 9-15", cmdline.c_str());
    FILE* outf = popen(shellcmd, "r");
    int pid = -1;
    if (outf != NULL) {
      if (fscanf(outf, "%d", &pid) != 1) {
        pid = -1;
      }
      pclose(outf);
    }
    printf("FindPid: %s, %d\n", cmdline, pid);
    return pid;
  }

//public:

  void ProcStat_constructor(ProcStat *procstat, char *cmdline) {
    if (strlen(cmdline) > 0) {
      procstat->name = cmdline;
      procstat->pid = ProcStat_FindPid(procstat,cmdline);
    } else {
      strcpy(procstat->name,"self");
      procstat->pid = getpid();
    }
    procstat->page_size = sysconf(_SC_PAGESIZE);
  }

   void ProcStat_GetMetric(ProcStat *procstat,hash_map *TMetList, time_t now) {
    int i;
    if (procstat->pid == -1) {
      return;
    }
    char tmp[256];
    sprintf(tmp, "/proc/%d/stat", procstat->pid);
    FILE* f=fopen(tmp, "r");
    if (f == NULL) {
      return;
    }
    for (i = 0; i < 13; ++i)
      if (fscanf(f, "%s", tmp) < 1)
        return;
    double proctime;
    if (fscanf(f, "%lf", &proctime) < 1)
      return;
    //AddMetric(metlist, name+std::string(".utime"), now, TTSValue(proctime*100));
    if (fscanf(f, "%lf", &proctime) < 1)
      return;
    //AddMetric(metlist, name+std::string(".stime"), now, TTSValue(proctime*100));
    for (i = 0; i < 7; ++i)
      if (fscanf(f, "%s", tmp) < 1)
        return;
    long long memory;
    if (fscanf(f, "%lld", &memory) < 1)
      return;
    //AddMetric(metlist, name+std::string(".vmsize"), now, memory);
    if (fscanf(f, "%lld", &memory) < 1)
      return;
    //AddMetric(metlist, name+std::string(".rss"), now, memory * page_size);
    fclose(f);
  }


//class LatencyStat: public MetricStat {
struct LatencyStat{
  MetricStat *metricstat;
  char *metname;
  time_t lasttime;
};
typedef struct LatencyStat LatencyStat;
//public:
  void LatencyStat_constructor(LatencyStat *latencystat) {
    latencystat->lasttime = time(NULL);
    strcpy(latencystat->metname,"latency");
  }

 /* virtual void GetMetric(TMetList &metlist, time_t now) {
    AddMetric(metlist, metname, now, TTSValue(now-lasttime));
  }*/
//};

//void Monitor_constructor(const std::string &part, const std::string &fs) : logfile(NULL)
void Monitor_constructor_part_fs(Monitor *monitor,const char *part, const char *fs)     
{
  //statlist.push_back(new IOStat(part));
  IOStat *iostat;
  IOStat_constructor(iostat,part);
  vector_add(monitor->statlist,iostat);

  //statlist.push_back(new SlabStat(fs));
  SlabStat *slabstat;
  SlabStat_constructor(slabstat,fs);
  vector_add(monitor->statlist,slabstat);
  
  //statlist.push_back(new MemStat());
  MemStat *memstat;
  vector_add(monitor->statlist,memstat);
  
   //statlist.push_back(new ProcStat());
  ProcStat *procstat;
  char *n;   //NULL string;
  n[0]='\0';
  ProcStat_constructor(procstat,n);
  vector_add(monitor->statlist,procstat);
  
  //statlist.push_back(new ProcStat(std::string("tablefs")));
  strcpy(n,"tablefs");
  ProcStat_constructor(procstat,n);
  vector_add(monitor->statlist,procstat);
  
  //statlist.push_back(new LatencyStat());
  LatencyStat *latency;
  LatencyStat_constructor(latency);
  vector_add(monitor->statlist,latency);
}

void Monitor_constructor_part(Monitor *monitor,const char *part) 
{
  //statlist.push_back(new IOStat(part));
  IOStat *iostat;
  IOStat_constructor(iostat,part);
  vector_add(monitor->statlist,iostat);

}

/*Monitor::Monitor(Monitor *monitor) : logfile(NULL) {
}*/

void Monitor_AddMetricStat(Monitor *monitor,MetricStat* mstat) {
  if (mstat != NULL)
    vector_add(monitor->statlist,mstat);
    //statlist.push_back(mstat);
}

/*Monitor::~Monitor() {
  std::vector<MetricStat*>::iterator it;
  for (it = statlist.begin(); it != statlist.end(); it++) {
    delete (*it);
  }
  if (logfile != NULL) {
    fclose(logfile);
  }
}

void Monitor_SetLogFile(Monitor *monitor,const char *logfilename) {
  monitor->logfile = fopen(logfilename, "w");
}*/

/*void Monitor_DoMonitor(Monitor *monitor) {
  std::vector<MetricStat*>::iterator it;
  time_t now = time(NULL);
  int i = 0;
  for (it = statlist.begin(); it != statlist.end(); it++) {
    ++i;
    (*it)->GetMetric(metlist, now);
  }
}

void Monitor_AddMetric(Monitor *monitor,char *name, TTSValue ts, TTSValue val) {
  TMetList::iterator it = metlist.find(name);
  if (it == metlist.end()) {
    metlist[name] = TSeries();
    it = metlist.find(name);
  }
  it->second.push_back(TSEntry(ts, val));
}

void Monitor_Report(Monitor *monitor) {
  for (TMetList::iterator it = metlist.begin(); it != metlist.end(); it++) {
    printf("%s", it->first.c_str());
    for (TSeries::iterator jt = it->second.begin();
         jt != it->second.end(); jt++) {
      printf(" %ld %lld", jt->first, jt->second);
    }
    printf("\n");
  }
}*/

/*void Monitor_Report_logf(Monitor *monitor,FILE* logf) {
  for (TMetList::iterator it = metlist.begin(); it != metlist.end(); it++) {
    fprintf(logf, "%s", it->first.c_str());
    for (TSeries::iterator jt = it->second.begin();
         jt != it->second.end(); jt++) {
      fprintf(logf, " %ld %lld", jt->first, jt->second);
    }
    fprintf(logf, "\n");
  }
}*/

/*void Monitor_ReportToFile(Monitor *monitor) {
  Monitor_Report_logf(monitor,monitor->logfile);
}*/

//}
//#endif
