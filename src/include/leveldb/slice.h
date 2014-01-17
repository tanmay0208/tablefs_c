// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Slice is a simple structure containing a pointer into some external
// storage and a size.  The user of a Slice must ensure that the slice
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a Slice without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Slice must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_SLICE_H_
#define STORAGE_LEVELDB_INCLUDE_SLICE_H_

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <string.h>

//namespace leveldb {

struct Slice {
//public:
  const char* data_;
  size_t size_;
 };
 typedef struct Slice Slice;  // Create an empty slice.
  
void Slice_init(Slice *slice);

  // Create a slice that refers to d[0,n-1].
void Slice_init_size(Slice *slice,const char* d, size_t n) ;

  // Create a slice that refers to the contents of "s"
void Slice_init_str(Slice *slice,const char *s);

  // Create a slice that refers to s[0,strlen(s)-1]
  //Slice(const char* s) : data_(s), size_(strlen(s)) { }

  // Return a pointer to the beginning of the referenced data
const char* Slice_data(const Slice *slice);

  // Return the length (in bytes) of the referenced data
size_t Slice_size(const Slice *slice) ;

  // Return true iff the length of the referenced data is zero
bool Slice_empty(Slice *slice) ;

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
char Slice_operator_bracket(Slice *slice,size_t n) ;

  // Change this slice to refer to an empty array
void Slice_clear(Slice *slice) ;
  // Drop the first "n" bytes from this slice.
void Slice_remove_prefix(Slice *slice,size_t n);

  // Return a string that contains the copy of the referenced data.
char *Slice_ToString(Slice *slice);
  // Three-way comparison.  Returns value:
  //   <  0 iff "*this" <  "b",
  //   == 0 iff "*this" == "b",
  //   >  0 iff "*this" >  "b"
  //int compare(const Slice& b) const;

  // Return true iff "x" is a prefix of "*this"
  bool Slice_starts_with(Slice *slice,const Slice *x) ;

 //private:
  

  // Intentionally copyable
//};

bool Slice_operator_equals(const Slice *x, const Slice *y) ;

inline bool Slice_operator_not_equals(const Slice *x, const Slice *y) ;

inline int Slice_compare(Slice *slice,const Slice *	b) ;
//}  // namespace leveldb


#endif  // STORAGE_LEVELDB_INCLUDE_SLICE_H_
