// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "port/port.h"
#include "leveldb/status.h"

//namespace leveldb {

static const char* Status_CopyState(const char* state) {
  uint32_t size;
  memcpy(&size, state, sizeof(size));
  char* result = (char*)malloc(sizeof(char)*(size+5));
  memcpy(result, state, size + 5);
  return result;
}


Status *Status_constuctor_Slice(Status *status,Code code, const Slice* msg, Slice* msg2) {
  assert(code != kOk);
  const uint32_t len1 = Slice_size(msg);
  const uint32_t len2 = Slice_size(msg2);
  const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
  char* result = (char*)malloc(sizeof(char)*(size+5));
  memcpy(result, &size, sizeof(size));
  //result[4] = static_cast<char>(code);
  memcpy(result + 5, Slice_data(msg), len1);
  if (len2) {
    result[5 + len1] = ':';
    result[6 + len1] = ' ';
    memcpy(result + 7 + len1, Slice_data(msg2), len2);
  }
  status->state_ = result;
  return status;
}

char* Status_ToString(Status *status) {
  if (status->state_ == NULL) {
    return (char*)"OK";
  } else {
    char tmp[30];
    const char* type;
    switch (Status_code(status)) {
      case kOk:
        type = "OK";
        break;
      case kNotFound:
        type = "NotFound: ";
        break;
      case kCorruption:
        type = "Corruption: ";
        break;
      case kNotSupported:
        type = "Not implemented: ";
        break;
      case kInvalidArgument:
        type = "Invalid argument: ";
        break;
      case kIOError:
        type = "IO error: ";
        break;
      default:
     /*   snprintf(tmp, sizeof(tmp), "Unknown code(%d): ",
                 static_cast<int>(code()));*/
        type = tmp;
        break;
    }
    char* result;
    strcpy(result,type);
    uint32_t length;
    memcpy(&length, status->state_, sizeof(length));
    //result.append(state_ + 5, length);
    return result;
  }
}

  void Status_constuctor(Status *status) {
      status->state_=NULL;
  }
  void Status_destructor(Status *status) {
   free (status); 
   //delete[] state_; 
 }
  static Status* Status_OK(Status *status) { 
    Status_constuctor(status);
    return status; 
  }

  // Return error status of an appropriate type.
  static Status* Status_NotFound(Status *status,const Slice *msg, Slice *msg2) {
    Slice_init(msg2);
    return Status_constuctor_Slice(status,kNotFound, msg, msg2);
  }
  static Status* Status_Corruption(Status *status,const Slice *msg, Slice *msg2) {
    Slice_init(msg2);
    return Status_constuctor_Slice(status,kCorruption, msg, msg2);
  }
  static Status* Status_NotSupported(Status *status,const Slice *msg, Slice *msg2) {
    Slice_init(msg2);
    return Status_constuctor_Slice(status,kNotSupported, msg, msg2);
  }
  static Status* Status_InvalidArgument(Status *status,const Slice *msg, Slice *msg2) {
    Slice_init(msg2);
    return Status_constuctor_Slice(status,kInvalidArgument, msg, msg2);
  }
  static Status* Status_IOError(Status *status,const Slice *msg, Slice *msg2) {
    Slice_init(msg2);
    return Status_constuctor_Slice(status,kIOError, msg, msg2);
  }
  static Status* Status_NotFoundByFilter(Status *status,Slice *msg,
                         Slice *msg2) {
    Slice_init(msg2);
    Slice_init(msg);
    return Status_constuctor_Slice(status,kNotFoundByFilter, msg, msg2);
  }

  // Returns true iff the status indicates success.
  bool Status_ok(Status *status) { return (status->state_ == NULL); }

  // Returns true iff the status indicates a NotFound error.
  bool Status_IsNotFound(Status *status){ return Status_code(status) == kNotFound; }

  // Returns true iff the status indicates a Corruption error.
  bool Status_IsCorruption(Status *status) { return Status_code(status) == kCorruption; }

  // Returns true iff the status indicates an IOError.
  bool Status_IsIOError(Status *status)  { return Status_code(status) == kIOError; }

  // Returns true iff the status indicates a NotFound error found by bloom_filter.
  bool Status_IsNotFoundByFilter(Status *status) { return Status_code(status) == kNotFoundByFilter; }

inline void Status_Status(Status *status,const Status *s) {
  status->state_ = (s->state_ == NULL) ? NULL : Status_CopyState(s->state_);
}
inline void Status_operator(Status *status,const Status *s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  if (status->state_ != s->state_) {
    //delete[] status->state_;
    status->state_ = (s->state_ == NULL) ? NULL : Status_CopyState(s->state_);
  }
}


  Code Status_code(Status *status) {
    //return (status->state_ == NULL) ? kOk : static_cast<Code>(state_[4]);
  }
//}  // namespace leveldb

