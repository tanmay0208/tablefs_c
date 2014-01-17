/*************************************************************************
* Author: Kai Ren
* Created Time: 2012-09-10 17:04:58
* File Name: inodemutex.cpp
* Description:
 ************************************************************************/
#include "fs/inodemutex.h"
#include "util/hash.h"
#include "util/logging.h"

//namespace tablefs {

void RWLock_constructor(RWLock *rwlock) { 
	pthread_rwlock_init(&rwlock->rw_, NULL); 
}

void RWLock_destructor(RWLock *rwlock) { 
	pthread_rwlock_destroy(&rwlock->rw_); 
}

void RWLock_ReadLock(RWLock *rwlock) { 
	pthread_rwlock_rdlock(&rwlock->rw_); 
}

void RWLock_WriteLock(RWLock *rwlock) { 
	pthread_rwlock_wrlock(&rwlock->rw_); 
}

void RWLock_Unlock(RWLock *rwlock) { 
	pthread_rwlock_unlock(&rwlock->rw_); 
}

void InodeMutex_ReadLock(InodeMutex *inodemutex,const tfs_meta_key_t key) {
  uint32_t lock_id = (key.inode_id + key.hash_id) & ILOCK_BASE;
/*  uint32_t lock_id = leveldb::Hash(key.str, 16, 0) & ILOCK_BASE;
* 	ilock[lock_id].ReadLock();
* 	Logging::Default()->LogMsg("ReadLock [%d, %d]\n", key.inode_id, key.hash_id);
*	commented by Kai
*/
}

void InodeMutex_WriteLock(InodeMutex *inodemutex,const tfs_meta_key_t key) {
  uint32_t lock_id = (key.inode_id + key.hash_id) & ILOCK_BASE;
/*  uint32_t lock_id = leveldb::Hash(key.str, 16, 0) & ILOCK_BASE;
* 	ilock[lock_id].WriteLock();
*	Logging::Default()->LogMsg("WriteLock [%d, %d]\n", key.inode_id, key.hash_id);
*	commented by Kai
*/
}

void InodeMutex_Unlock(InodeMutex *inodemutex,const tfs_meta_key_t key) {
  uint32_t lock_id = (key.inode_id + key.hash_id) & ILOCK_BASE;
/*  uint32_t lock_id = leveldb::Hash(key.str, 16, 0) & ILOCK_BASE;
* 	ilock[lock_id].Unlock();
* 	Logging::Default()->LogMsg("Unlock [%d, %d]\n", key.inode_id, key.hash_id);
*	commented by Kai
*/
}

//} // namespace tablefs
