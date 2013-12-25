/*
 * LevelDBAdaptor.cpp
 *
 *  Created on: Jul 19, 2011
 *      Author: kair
 */

#include "leveldb_adaptor.h"
//#include "leveldb/db.h"
//#include "leveldb/cache.h"
//#include "leveldb/write_batch.h"
//#include "leveldb/status.h"
//#include "leveldb/filter_policy.h"
#include <time.h>
//#ifdef SUPPORTED
//using namespace leveldb;

//namespace tablefs {
/*
void LevelDBAdaptor_contructor() :
  db_(NULL), cache_(NULL), logs(NULL), writeahead(true) {
}
*/
//LevelDBAdaptor::~LevelDBAdaptor() {
//}

/*
void LevelDBAdaptor_SetProperties(LevelDBAdaptor *leveldbadaptor, Properties *p) {
    leveldbadaptor->p_ = p;                                          //
}
*/
void LevelDBAdaptor_SetLogging(LevelDBAdaptor *leveldbadaptor,Logging *logs_) {
    leveldbadaptor->logs = logs_;
}


/*bool LevelDBAdaptor_GetStat(LevelDBAdaptor *,char *stat, char** value) {
    //return db_->GetProperty(stat, value);
}
*/

int LevelDBAdaptor_Init(LevelDBAdaptor *leveldbadaptor) {
  /*assert(db_ == NULL);
  int F_cache_size = p_.getPropertyInt("leveldb.cache.size", 16<<20);
  cache_ = (F_cache_size >= 0) ? leveldb::NewLRUCache(F_cache_size) : NULL;
  db_name = p_.getProperty("leveldb.db", "/tmp/db");
  Options options;
  options.create_if_missing =
    p_.getPropertyBool("leveldb.create.if.missing.db", true);
  options.block_cache = cache_;
  options.block_size =
    p_.getPropertyInt("leveldb.block.size", 4 << 10);
  options.write_buffer_size =
    p_.getPropertyInt("leveldb.write.buffer.size", 16<<20);
  options.max_open_files =
    p_.getPropertyInt("leveldb.max.open.files", 800);
  options.filter_policy = NewBloomFilterPolicy(12);

  if (leveldbadaptor->logs != NULL) {
    leveldbadaptor->logs->LogMsg("limit level: %d\n", options.limit_sst_file_size);
    leveldbadaptor->logs->LogMsg("limit level0: %d\n", options.limit_level_zero);
    leveldbadaptor->logs->LogMsg("factor level files: %lf\n", options.factor_level_files);
  }
  writeahead = p_.getPropertyBool("leveldb.writeahead", true);
  logon = p_.getPropertyBool("leveldb.logon", false);
  sync_time_limit = p_.getPropertyInt("leveldb.sync.time.limit", 5);
  sync_size_limit = p_.getPropertyInt("leveldb.sync.size.limit", -1);
  last_sync_time = time(NULL);
  async_data_size = 0;
  Status s = DB::Open(options, db_name, &db_);
  if (!s.ok()) {
    return -1;
  } else {
    return 0;
  }*/
}

void LevelDBAdaptor_Cleanup(LevelDBAdaptor *leveldbadaptor) {
  //delete leveldbadaptor->db_;
  //delete leveldbadaptor->cache_;
  //leveldbadaptor->db_ = NULL;
}

/*int LevelDBAdaptor_Get(LevelDBAdaptor *leveldbadaptor,const leveldb::Slice &key,
                        std::string &result) {
  ReadOptions options;
  Status s = db_->Get(options, key, &result);
  if (logon) {
    if (logs != NULL) {
      const int *data = (const int *) key.ToString().data();
      logs->LogMsg("read %s %d %x\n", db_name.c_str(), data[0], data[1]);
    }
  }
  if (!s.ok()) {
    result = s.ToString();
    return -1;
  } else {
    return (s.IsNotFound()) ? 0 : 1;
  }
}
*/LevelDBIterator* LevelDBAdaptor_GetNewIterator(LevelDBAdaptor *leveldbadaptor) {
    //ReadOptions read_options;
    if (leveldbadaptor->logon) {
      if (leveldbadaptor->logs != NULL)
        Logging_LogMsg(leveldbadaptor->logs,"iterator\n");
  }
  //Iterator* iter = db_->NewIterator(read_options);
  //return new LevelDBIterator(iter);
}

int LevelDBAdaptor_Sync(LevelDBAdaptor *leveldbadaptor) {
  //WriteOptions write_options;
  //write_options.sync = true;
  //leveldb::Status status = db_->Put(write_options, "sync", "");
  /*if (status.ok()) {
    return 0;
  } else {
    return -1;
  }*/
}
/*
int LevelDBAdaptor::Put(const leveldb::Slice &key,
                        const leveldb::Slice &value) {
  if (logon) {
    if (logs != NULL) {
      const int *data = (const int *) key.ToString().data();
      logs->LogMsg("Put %d %x\n", data[0], data[1]);
    }
  }
  WriteOptions write_options;
  if (sync_size_limit > 0) {
    async_data_size += key.size() + value.size();
    if (async_data_size > sync_size_limit) {
      write_options.sync = true;
      async_data_size = 0;
    }
  } else
  if (sync_time_limit > 0) {
    time_t now = time(NULL);
    if (now - last_sync_time > sync_time_limit) {
      write_options.sync = true;
      last_sync_time = now;
    }
  }
  write_options.writeahead = writeahead;
  leveldb::Status status = db_->Put(write_options, key, value);
  if (status.ok()) {
    return 0;
  } else {
    if (logon) {
      if (logs != NULL) {
        logs->LogMsg("Put Error: %s\n", status.ToString().c_str());
      }
    }
    return -1;
  }
}

int LevelDBAdaptor::Delete(const leveldb::Slice &key) {
  if (logon) {
    if (logs != NULL) {
      const int *data = (const int *) key.ToString().data();
      logs->LogMsg("Delete %d %x\n", data[0], data[1]);
    }
  }
  WriteOptions write_options;
  db_->Delete(write_options, key);
  return 0;
}

int LevelDBAdaptor::Write(LevelDBAdaptor *leveldbadaptor,WriteBatch &batch) {
  WriteOptions write_options;
  Status s = db_->Write(write_options, &batch);
  if (!s.ok()) {
    return -1;
  }
  return 0;
}
*/

/*
void LevelDBAdaptor_Report(LevelDBAdaptor *leveldbadaptor) {
  char *result;
  leveldbadaptor->db_->GetProperty(leveldb::Slice("leveldb.stats"), &result);
  leveldbadaptor->logs->LogMsg("\n%s\n", result.c_str());
}

void LevelDBAdaptor_Compact(LevelDBAdaptor *leveldbadaptor) {
  leveldbadaptor->db_->CompactRange(NULL, NULL);
}

bool LevelDBAdaptor_GetMetric(LevelDBAdaptor *leveldbadaptor,std::string* value) {
  return leveldbadaptor->db_->GetProperty(Slice("leveldb.stats"), value);
}
*/
//#endif
