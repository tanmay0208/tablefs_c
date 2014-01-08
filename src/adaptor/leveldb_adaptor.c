//#ifdef SUPPORTED
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

void LevelDBAdaptor_constructor(LevelDBAdaptor *leveldbadaptor) {
  leveldbadaptor->db_=NULL;
  leveldbadaptor->cache_=NULL; 
  leveldbadaptor->logs=NULL;
  leveldbadaptor->writeahead=true; 
}

bool LevelDBIterator_Valid(LevelDBIterator *leveldbiterator) { return leveldb_iter_valid(leveldbiterator->iter_); };

  void LevelDBIterator_SeekToFirst(LevelDBIterator *leveldbiterator) { leveldb_iter_seek_to_first(leveldbiterator->iter_); }

  void LevelDBIterator_SeekToLast(LevelDBIterator *leveldbiterator) { leveldb_iter_seek_to_last(leveldbiterator->iter_); }

  void LevelDBIterator_Seek(LevelDBIterator *leveldbiterator,Slice *target) { 
      leveldb_iter_seek(leveldbiterator->iter_,target->data_,target->size_); 
  }

  void LevelDBIterator_Next(LevelDBIterator *leveldbiterator) { leveldb_iter_next(leveldbiterator->iter_); }

  void LevelDBIterator_Prev(LevelDBIterator *leveldbiterator) { leveldb_iter_prev(leveldbiterator->iter_); } 
  

//LevelDBAdaptor::~LevelDBAdaptor() {
//}

void LevelDBIterator_destructor(LevelDBIterator *leveldbiterator) { 
	free (leveldbiterator->iter_); 
}

void LevelDBIterator_constructor(LevelDBIterator *leveldbiterator,leveldb_iterator_t *iter) { 
	leveldbiterator->iter_=iter;
} 
void LevelDBAdaptor_SetProperties(LevelDBAdaptor *leveldbadaptor, Properties p) {
    leveldbadaptor->p_ = p;                                          
}
void LevelDBAdaptor_SetLogging(LevelDBAdaptor *leveldbadaptor,Logging *logs_) {
    leveldbadaptor->logs = logs_;
}

bool LevelDBAdaptor_GetStat(LevelDBAdaptor *leveldbadaptor,char *stat, char** value) {
    //return db_->GetProperty(stat, value);
}

int LevelDBAdaptor_Init(LevelDBAdaptor *leveldbadaptor) {
  assert(db_ == NULL);
  char *db_name;
  int F_cache_size = Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.cache.size", 16<<20);
  leveldbadaptor->cache_ = (F_cache_size >= 0) ? leveldb_cache_create_lru(F_cache_size) : NULL;
  db_name = Properties_getProperty_default(&(leveldbadaptor->p_),"leveldb.db",(char*) "/tmp/db");
  leveldb_options_t *options;
  options=leveldb_options_create();
  leveldb_options_set_create_if_missing(options,Properties_getPropertyBool(&(leveldbadaptor->p_),"leveldb.create.if.missing.db", true));
  leveldb_options_set_cache(options,leveldbadaptor->cache_);
  leveldb_options_set_block_size(options,Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.block.size", 4 << 10));
  leveldb_options_set_write_buffer_size(options,Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.write.buffer.size", 16<<20));
  leveldb_options_set_max_open_files(options,Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.max.open.files", 800));
  leveldb_options_set_filter_policy(options, leveldb_filterpolicy_create_bloom(12));

  if (leveldbadaptor->logs != NULL) {
    /*Logging_LogMsg(leveldbadaptor->logs,"limit level: %d\n", options.limit_sst_file_size);
    Logging_LogMsg(leveldbadaptor->logs,"limit level0: %d\n", options.limit_level_zero);
    Logging_LogMsg(leveldbadaptor->logs,"factor level files: %lf\n", options.factor_level_files);*/
  }
  leveldbadaptor->writeahead = Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.writeahead", true);
  leveldbadaptor->logon = Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.logon", false);
  leveldbadaptor->sync_time_limit = Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.sync.time.limit", 5);
  leveldbadaptor->sync_size_limit = Properties_getPropertyInt(&(leveldbadaptor->p_),"leveldb.sync.size.limit", -1);
  leveldbadaptor->last_sync_time = time(NULL);
  leveldbadaptor->async_data_size = 0;
  /*Status s = leveldb_open(options, db_name, &db_);		//handle status first
  if (!s.ok()) {
    return -1;
  } else {
    return 0;
  }*/
    return 1;			//added for comipling
}

void LevelDBAdaptor_Cleanup(LevelDBAdaptor *leveldbadaptor) {
  free (leveldbadaptor->db_);
  free (leveldbadaptor->cache_);
  leveldbadaptor->db_ = NULL;
}

/*int LevelDBAdaptor_Get(LevelDBAdaptor *leveldbadaptor,const leveldb::Slice &key,
                        std::string &result) {
  ReadOptions options;
  Status s = db_->Get(options, key, &result);
  if (leveldbadaptor->logon) {
    if (leveldbadaptor->logs != NULL) {
      const int *data = (const int *) key.ToString().data();
      Logging_LogMsg(leveldbadaptor->logs,"read %s %d %x\n", db_name.c_str(), data[0], data[1]);
    }
  }
  if (!s.ok()) {
    result = s.ToString();
    return -1;
  } else {
    return (s.IsNotFound()) ? 0 : 1;
  }
}
*/
LevelDBIterator* LevelDBAdaptor_GetNewIterator(LevelDBAdaptor *leveldbadaptor) {
    //ReadOptions read_options;
    if (leveldbadaptor->logon) {
      if (leveldbadaptor->logs != NULL)
        Logging_LogMsg(leveldbadaptor->logs,"iterator\n");
  }
  //Iterator* iter = db_->NewIterator(read_options);
  //return new LevelDBIterator(iter);
}

int LevelDBAdaptor_Sync(LevelDBAdaptor *leveldbadaptor) {
  /*WriteOptions write_options;
  write_options.sync = true;
   leveldb::Status status = db_->Put(write_options, "sync", "");
  if (status.ok()) {
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
      Logging_LogMsg(leveldbadaptor->logs,"Put %d %x\n", data[0], data[1]);
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
        Logging_LogMsg(leveldbadaptor->logs,"Put Error: %s\n", status.ToString().c_str());
      }
    }
    return -1;
  }
}

int LevelDBAdaptor::Delete(const leveldb::Slice &key) {
  if (logon) {
    if (logs != NULL) {
      const int *data = (const int *) key.ToString().data();
      Logging_LogMsg(leveldbadaptor->logs,"Delete %d %x\n", data[0], data[1]);
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
