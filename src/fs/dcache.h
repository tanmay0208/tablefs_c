
  /*
 * icache.h
 *
 *  Created on: Aug 15, 2011
 *      Author: kair
 */

#ifndef _DCACHE_H_
#define _DCACHE_H_

#include "fs/tfs_inode.h"
#include "port/port.h"
#include <stdlib.h>
#include <stdbool.h>
//#include <unordered_map>
//#include <list>

//namespace tablefs {
struct DentryCacheComp {
#ifdef UNSUPPORTED
  bool operator() (const tfs_meta_key_t &lhs, const tfs_meta_key_t &rhs) const {
    return (lhs.hash_id == rhs.hash_id) && (lhs.inode_id == rhs.inode_id);
  }
#endif
};

struct DentryCacheHash {
#ifdef UNSUPPORTED
  std::size_t operator()(const tfs_meta_key_t& x) const
  {
    std::size_t seed = 0;
    seed ^= x.inode_id + 0x9e3779b9 + (seed<<6) + (seed>>2);
    seed ^= x.hash_id + 0x9e3779b9 + (seed<<6) + (seed>>2);
    return seed;
  }
#endif
};
//class DentryCache {
//public:
#ifdef UNSUPPORTED
  typedef std::pair<tfs_meta_key_t, tfs_inode_t> Entry;
  typedef std::list<Entry> CacheList;
  typedef std::unordered_map<tfs_meta_key_t, std::list<Entry>::iterator,
            DentryCacheHash, DentryCacheComp> CacheMap;
#endif

struct DentryCache{
  size_t maxsize;
};
typedef struct DentryCache DentryCache;
  
void tfs_DentryCache_constructor(DentryCache *,size_t);

bool tfs_DentryCache_Find (DentryCache *,tfs_meta_key_t *, tfs_inode_t *);

void tfs_DentryCache_Insert (DentryCache *,tfs_meta_key_t *, const tfs_inode_t *);

void tfs_DentryCache_Evict(DentryCache *,tfs_meta_key_t *);


  //virtual ~DentryCache();

/*private:
  size_t maxsize;
  CacheList cache;
  CacheMap lookup;

  leveldb::port::Mutex cache_mutex;
};
*/
//}
#endif /* ICACHE_H_ */
