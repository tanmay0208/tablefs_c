// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "leveldb/filter_policy.h"

namespace leveldb {

int FilterPolicy::num_filter_hit = 0;

FilterPolicy::~FilterPolicy() { }

}  // namespace leveldb
