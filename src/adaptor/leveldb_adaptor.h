
/*
 *
 *  Created on: Jul 19, 2011
 *      Author: kair
 */

#ifndef LEVELDBADAPTOR_H_
#define LEVELDBADAPTOR_H_

#include <stdlib.h>
#include "fs/vector.h"
#include "util/properties.h"
#include "util/logging.h"
#include "leveldb/c.h"
#include "leveldb/db.h"
#include "leveldb/slice.h"
#include "leveldb/iterator.h"
#include "leveldb/write_batch.h"

//namespace tablefs {
//char** errptr;          //Ashay ::added for leveldb_put as sixth argument

struct LevelDBIterator {
	leveldb_iterator_t *iter_;
};
typedef struct LevelDBIterator LevelDBIterator;


struct LevelDBAdaptor {
  char* db_name;
  leveldb_t *db_;
  leveldb_cache_t *cache_;
  Logging* logs;
  Properties *p_;
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

  int LevelDBAdaptor_Get(LevelDBAdaptor *,Slice *key,
          Slice *result);

  int LevelDBAdaptor_Put(LevelDBAdaptor *,Slice *key,
          char *values);
  
  int LevelDBAdaptor_Sync(LevelDBAdaptor *);

  int LevelDBAdaptor_Write(LevelDBAdaptor *,leveldb_writebatch_t *batch);

  int LevelDBAdaptor_Delete(LevelDBAdaptor *,Slice *key);

  LevelDBIterator* LevelDBAdaptor_GetNewIterator(LevelDBAdaptor *);

  void LevelDBAdaptor_Compact(LevelDBAdaptor *);

  void LevelDBAdaptor_Report(LevelDBAdaptor *);

  bool LevelDBAdaptor_GetStat(LevelDBAdaptor *,char *stat, char** value);

  bool LevelDBAdaptor_GetMetric(LevelDBAdaptor *,char** value);





  void LevelDBIterator_constructor(LevelDBIterator *leveldbiterator,leveldb_iterator_t *iter); 

  void LevelDBIterator_destructor(LevelDBIterator *leveldbiterator);

  bool LevelDBIterator_Valid(LevelDBIterator *leveldbiterator);

  void LevelDBIterator_SeekToFirst(LevelDBIterator *leveldbiterator);

  void LevelDBIterator_SeekToLast(LevelDBIterator *leveldbiterator);

  void LevelDBIterator_Seek(LevelDBIterator *leveldbiterator,Slice *target) ;

  void LevelDBIterator_Next(LevelDBIterator *leveldbiterator) ;

  void LevelDBIterator_Prev(LevelDBIterator *leveldbiterator) ;
                                //it was leveldb_iter_next bt changed it

  Slice LevelDBAdaptor_key(LevelDBAdaptor *,LevelDBIterator *leveldbiterator);
  Slice LevelDBAdaptor_value(LevelDBAdaptor *);




#endif /* LEVELDBADAPTOR_H_ */
//#endif
