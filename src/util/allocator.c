//#ifdef SUPPORTED
/*************************************************************************
* Author: Pratik Gaikar
* Created Time: 2012-02-27 20:41:41
* File Name: ./allocator.cpp
* Description:
 ************************************************************************/

#include "util/allocator.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

//namespace tablefs {


static const size_t MemoryPageSize = 128 * 1024 * 1024;

void Allocator_constructor(Allocator *allocator,size_t lock_size) {
  int i;
  if (lock_size > 0) {
    int num_memory_pages = (lock_size*1024*1024 - 1) / MemoryPageSize + 1;
    for (i = 0; i < num_memory_pages; ++i) {
      void* page_start;
      int ret;
      if ((ret = posix_memalign(&page_start,sysconf(_SC_PAGESIZE),
                        MemoryPageSize)) != 0) {
        printf("memory allocation failed: need %u more bytes\n",
               lock_size);
        exit(1);
      }
      if (mlock(page_start, MemoryPageSize) != 0) {
        perror("memory locking failed");
        exit(1);
      }
      vector_add(allocator->blocks,page_start);   //original arg 2 was (char *)
    }
    allocator->current_block = 0;
    allocator->remaining_bytes = MemoryPageSize;
    allocator->flag_using_lock_memory = true;
  } 
  else {
    allocator->current_block = 0;
    allocator->remaining_bytes = 0;
    allocator->flag_using_lock_memory = false;
  }
}

void Allocator_Destructor(Allocator *allocator){
  size_t i;
  if (allocator->flag_using_lock_memory) {
    for (i = 0; i < vector_count(allocator->blocks); i++) {
      vector_delete(allocator->blocks,i);
    }
  } 
  else {
  //  for (size_t i = 0; i < blocks.size(); i++) {
    //  delete [] blocks[i];
    }
}


void* Allocator_Allocate(Allocator *allocator,size_t bytes) {
  if (bytes > MemoryPageSize) {
    fprintf(stderr, "ask for too much memory\n");
    exit(1);
  }
  if (allocator->flag_using_lock_memory) {
    if (allocator->remaining_bytes < bytes) {
      if (allocator->current_block + 1 < vector_count(allocator->blocks)) {
        allocator->current_block ++;
        allocator->remaining_bytes = MemoryPageSize;
      }
     else {
        char* page_start[bytes];
        vector_add(allocator->blocks,page_start);
        allocator->remaining_bytes = 0;
        return (void *) page_start;
      }
    }
    char* alloc_addr = vector_get(allocator->blocks,(int)allocator->current_block)+    // some jhol maybe there
                       (MemoryPageSize - allocator->remaining_bytes);
    allocator->remaining_bytes -= bytes;
    return (void *) alloc_addr;
  } else {
    char* page_start[bytes];
    vector_add(allocator->blocks,page_start);
    return (void *) page_start;
  }
}

//} // namespace tablefs
//#endif
