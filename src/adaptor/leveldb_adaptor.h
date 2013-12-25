//#ifdef SUPPORTED
/*
 *
 *  Created on: Jul 19, 2011
 *      Author: kair
 */

#ifndef LEVELDBADAPTOR_H_
#define LEVELDBADAPTOR_H_

//#include <vector>
#include "util/properties.h"
#include "util/logging.h"
//#include "leveldb/db.h"
//#include "leveldb/slice.h"
//#include "leveldb/iterator.h"
//#include "leveldb/write_batch.h"

//namespace tablefs {

struct LevelDBIterator {
//private:
 /*leveldb::Iterator *iter_;*/
};
typedef struct LevelDBIterator LevelDBIterator;


struct LevelDBAdaptor {
  char* db_name;
//  DB *db_;
//  leveldb::Cache *cache_;
  Logging* logs;
  Properties p_;
  bool logon;
  bool writeahead;
  time_t last_sync_time;
  time_t sync_time_limit;
  int async_data_size;
  int sync_size_limit;
};

typedef struct LevelDBAdaptor LevelDBAdaptor;
//public:
  void LevelDBAdaptor_constructor(LevelDBAdaptor *);

  void LevelDBAdaptor_destructor(LevelDBAdaptor *);

  void LevelDBAdaptor_SetProperties(LevelDBAdaptor *leveldbadaptor,Properties *p);

  void LevelDBAdaptor_SetLogging(LevelDBAdaptor *leveldbadaptor,Logging *logs_);

  int LevelDBAdaptor_Init(LevelDBAdaptor *);

  void LevelDBAdaptor_Cleanup(LevelDBAdaptor *);

  /*int LevelDBAdaptor_Get(LevelDBAdaptor *,const leveldb::Slice &key,
          std::string &result);

  int LevelDBAdaptor_Put(LevelDBAdaptor *,const leveldb::Slice &key,
          const leveldb::Slice &values);
  */
  int LevelDBAdaptor_Sync(LevelDBAdaptor *);
/*
  int LevelDBAdaptor_Write(LevelDBAdaptor *,leveldb::WriteBatch &batch);

  int LevelDBAdaptor_Delete(LevelDBAdaptor *,const leveldb::Slice &key);
*/
  LevelDBIterator* LevelDBAdaptor_GetNewIterator(LevelDBAdaptor *);

  void LevelDBAdaptor_Compact(LevelDBAdaptor *);

  void LevelDBAdaptor_Report(LevelDBAdaptor *);

  bool LevelDBAdaptor_GetStat(LevelDBAdaptor *,char *stat, char** value);

  bool LevelDBAdaptor_GetMetric(LevelDBAdaptor *,char** value);





//public:
/*
  void LevelDBIterator_constructor(LevelDBIterator *,leveldb::Iterator *iter) : iter_(iter) {}

  void LevelDBIterator_destructor() {LevelDBIterator *, delete iter_; }

  bool LevelDBIterator_Valid() const {LevelDBIterator *, return iter_->Valid(); };

  void LevelDBIterator_SeekToFirst() {LevelDBIterator *, iter_->SeekToFirst(); }

  void LevelDBIterator_SeekToLast() {LevelDBIterator *, iter_->SeekToLast(); }

 // void LevelDBIterator_Seek(LevelDBIterator *,const leveldb::Slice &target) { iter_->Seek(target); }

  void LevelDBIterator_Next() {LevelDBIterator *, iter_->Next(); }

  void LevelDBIterator_Prev() {LevelDBIterator *, iter_->Next(); }


//  leveldb::Slice key() const { return iter_->key(); }

//  leveldb::Slice value() const { return iter_->value(); }

*/


#endif /* LEVELDBADAPTOR_H_ */
//#endif
