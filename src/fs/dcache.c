/*
 * InodeCache.cpp
 *
 *  Created on: Aug 15, 2011
 *      Author: kair
 */

#include "dcache.h"
#include "util/mutexlock.h"


/*DentryCache::~DentryCache() {         // it is default destructor 
}*/ 
bool tfs_DentryCache_Find(DentryCache *dentry_cache,tfs_meta_key_t key, tfs_inode_t value) {
  //leveldb::MutexLock lock_cache_mutex(&cache_mutex);

  /*CacheMap::iterator it = lookup.find(key);
  if (it == lookup.end()) {
    return false;
  } else {
    value = it->second->second;
    cache.push_front(*(it->second));
    cache.erase(it->second);
    lookup[key] = cache.begin();
    return true;
  }*/
    return true;  // Added to for the sake of compiling ..
}

void tfs_DentryCache_Insert(DentryCache *dentry_cache,tfs_meta_key_t key, const tfs_inode_t value) {	
	/* Ashay: void tfs_DentryCache_Insert(Dentrycache *dentry_cache,tfs_meta_key_t &key, const tfs_inode_t &value) {	
	*/
  //leveldb::MutexLock lock_cache_mutex(&cache_mutex);

  /*Entry ent(key, value);
  cache.push_front(ent);
  lookup[key] = cache.begin();
  if (cache.size() > maxsize) {
    lookup.erase(cache.back().first);
    cache.pop_back();
  }*/
}

void tfs_DentryCache_Evict(DentryCache *dentry_cache,tfs_meta_key_t key) {
	/* Ashay:void tfs_DentryCache_Evict(Dentrycache *dentry_cache,tfs_meta_key_t &key) {
	*/
  //leveldb::MutexLock lock_cache_mutex(&cache_mutex);

  /*CacheMap::iterator it = lookup.find(key);
  if (it != lookup.end()) {
    lookup.erase(it);
  }*/
}

void tfs_DentryCache_GetSize(DentryCache *dentry_cache,size_t *size_cache_list, size_t *size_cache_map) {
	/*void tfs_DentryCache_GetSize(Dentrycache *dentry_cache,size_t &size_cache_list, size_t &size_cache_map) {*/
    #ifdef UNSUPPORTED
    size_cache_map = cache.size();
    size_cache_list = lookup.size();
    #endif
  }
  
void tfs_DentryCache_constructor(DentryCache *dentry_cache,size_t size) {
    dentry_cache->maxsize = size;
  }
