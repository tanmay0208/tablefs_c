#ifndef _INODE_LOCK_H_
#define _INODE_LOCK_H_

#include <stdint.h>
#include <pthread.h>
#include "fs/tfs_inode.h"

//namespace tablefs {
#define NUM_ILOCK 1024

const static unsigned int ILOCK_BASE = 0x3ff;

struct RWLock{
/*  public:
*  RWLock();
*  ~RWLock();
*
* void ReadLock();
* void WriteLock();
* void Unlock();
*
* private:
*/
  pthread_rwlock_t rw_;

  // No copying
  //RWLock(const RWLock&);
  //void operator=(const RWLock&);
};
typedef struct RWLock RWLock;

  void RWLock_constructor(RWLock *);
  void RWLock_destructor(RWLock *);

  void RWLock_ReadLock(RWLock *);
  void RWLock_WriteLock(RWLock *);
  void RWLock_Unlock(RWLock *);

struct InodeMutex {
  
  RWLock ilock[NUM_ILOCK];

  //no copying
 // InodeMutex(const InodeMutex &);       Handle Operator Overloading
//  void operator=(const InodeMutex &);
};
typedef struct InodeMutex InodeMutex;

/*InodeMutex() {}     Ashay:Dont need it
 * ~InodeMutex() {}
 */

  void InodeMutex_ReadLock(InodeMutex *,const tfs_meta_key_t *);

  void InodeMutex_WriteLock(InodeMutex *,const tfs_meta_key_t *);

  void InodeMutex_Unlock(InodeMutex *,const tfs_meta_key_t *);

//}

#endif
