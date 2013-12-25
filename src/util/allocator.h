#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_
#include <string.h>
//#include <vector>
#include <stdint.h>
#include <assert.h>
#include<stdbool.h>

//namespace tablefs {

//class Allocator {

struct Allocator{

  size_t remaining_bytes;
  size_t current_block;
  bool flag_using_lock_memory;

};
typedef struct Allocator Allocator;

//private:
 // std::vector<char*> blocks;
  
//public:
  void Allocator_constructor(Allocator *allocator,size_t lock_size);

  void Allocator_Destructor(Allocator *allocator);

  void* Allocator_Allocate(Allocator *allocator,size_t bytes);


//} end of namespace

#endif /* ALLOCATOR_H_ */

