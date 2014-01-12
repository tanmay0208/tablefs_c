// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// A Status encapsulates the result of an operation.  It may indicate success,
// or it may indicate an error with an associated error message.
//
// Multiple threads can invoke const methods on a Status without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Status must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_STATUS_H_
#define STORAGE_LEVELDB_INCLUDE_STATUS_H_

#include <string.h>
#include "leveldb/slice.h"

//namespace leveldb {

//class Status {
// public:

enum Code {
    kOk = 0,
    kNotFound = 1,
    kCorruption = 2,
    kNotSupported = 3,
    kInvalidArgument = 4,
    kIOError = 5,
    kNotFoundByFilter = 6
  }; 

typedef enum Code Code;

struct Status
{
  const char* state_;
  enum Code Code;
};

typedef struct Status Status;

  // Create a success status.
  void Status_constuctor(Status *status);
  void Status_destructor(Status *status);

  static const char* Status_CopyState(const char* s);
  Status *Status_constuctor_Slice(Status *status,Code code, const Slice *msg, Slice *msg2);
  Code Status_code(Status *status);


  // Copy the specified status.
  void Status_Status(Status *status,const Status *s);
  void Status_operator(Status *status,const Status *s);

  // Return a success status.
  static Status* Status_OK(Status *status);

  // Return error status of an appropriate type.
  static Status* Status_NotFound(Status *status,const Slice *msg, Slice *msg2);
  static Status* Status_Corruption(Status *status,const Slice *msg, Slice *msg2) ;
  static Status* Status_NotSupported(Status *status,const Slice *msg, Slice *msg2) ;
  static Status* Status_InvalidArgument(Status *status,const Slice *msg, Slice *msg2);
  static Status* Status_IOError(Status *status,const Slice *msg, Slice *msg2);
  static Status* Status_NotFoundByFilter(Status *status,Slice *msg,
                         Slice *msg2);

  // Returns true iff the status indicates success.
  bool Status_ok(Status *status);

  // Returns true iff the status indicates a NotFound error.
  bool Status_IsNotFound(Status *status);

  // Returns true iff the status indicates a Corruption error.
  bool Status_IsCorruption(Status *status);

  // Returns true iff the status indicates an IOError.
  bool Status_IsIOError(Status *status);

  // Returns true iff the status indicates a NotFound error found by bloom_filter.
  bool Status_IsNotFoundByFilter(Status *status) ;

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  char *Status_ToString(Status *status);

 //private:
  // OK status has a NULL state_.  Otherwise, state_ is a new[] array
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4]    == code
  //    state_[5..]  == message


  //Code Status_code(Status *status);
  //Status_constuctor_Slice(Status *status,Code code, const Slice *msg, Slice *msg2);

//};
inline void Status_Status(Status *status,const Status *s) ;
inline void Status_operator(Status *status,const Status *s);

//}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_STATUS_H_
