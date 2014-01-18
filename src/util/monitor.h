//#ifdef SUPPORTED
#ifndef MONITOR_H_
#define MONITOR_H_
#include <string.h>
#include "fs/hash_map.h"
#include "fs/vector.h"
#include <time.h>
#include <stdio.h>

//namespace tablefs {

typedef time_t TTSTime;
typedef long long TTSValue;
//typedef std::pair<TTSTime, TTSValue> TSEntry;
//typedef std::vector<TSEntry> TSeries;
vector *TSeries;

//typedef std::map<std::string, TSeries> TMetList;
//hash_map *TMetList;
hash_map *TMetList;


//class MetricStat {
//public:
 struct MetricStat
 {
    /* data */
 }; 
 typedef struct MetricStat MetricStat;

 struct IOStat{
    MetricStat *metricstat; 
    char *devname;
 }; 
 typedef struct IOStat IOStat;
  //virtual ~MetricStat() {
  //}
 
  /*void AddMetric(TMetList &metlist, std::string name,
                 TTSValue ts, TTSValue val)
 void AddMetric(hash_map *TMetList, char *name,
                 TTSValue ts, TTSValue val) {
    TMetList::iterator it = metlist.find(name);
    if (it == metlist.end()) {
      metlist[name] = TSeries();
      it = metlist.find(name);
    }
    it->second.push_back(TSEntry(ts, val));
  }*/

  /*virtual*/ 
    void MetricStat_GetMetric(IOStat *iostat,hash_map *TMetList, time_t now) ;
    
//};
struct SlabStat{
  char *fsname;
  int len_fsname;
};
typedef struct SlabStat SlabStat;

void SlabStat_GetMetric(SlabStat *slabstat,hash_map *TMetList, time_t now) ;
//class Monitor {
struct Monitor
{
   FILE *logfile;
   vector *statlist;
  
};
typedef struct Monitor Monitor;
//protected:
//  TMetList metlist;
// std::vector<MetricStat*> statlist;


//public:
  void Monitor_Constructor_default(Monitor *monitor);

  void Monitor_Constructor_part(Monitor *monitor,const char *part);

  void Monitor_Constructor_part_fs(Monitor *monitor,const char *part,
          const char *fs);

  void Monitor_Destructor(Monitor *monitor);

  void Monitor_SetLogFile(Monitor *monitor,const char *logfilename);

  void Monitor_DoMonitor(Monitor *monitor);

  void Monitor_AddMetricStat(Monitor *monitor,MetricStat* mstat);

  void Monitor_AddMetric(Monitor *monitor,char *name, TTSValue ts, TTSValue val);

  void Monitor_Report(Monitor *monitor);

  void Monitor_Report_logf(Monitor *,FILE *);

  void Monitor_ReportToFile(Monitor *monitor);
//};

//}

#endif
//#endif
