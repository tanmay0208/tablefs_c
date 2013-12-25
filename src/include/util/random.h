//#ifdef SUPPORTED
// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_UTIL_RANDOM_H_
#define STORAGE_LEVELDB_UTIL_RANDOM_H_

#include <stdint.h>

//namespace leveldb {

struct Random{
  uint32_t seed_;
};

typedef struct Random Random;
  
// A very simple random number generator.  Not especially good at
// generating truly random bits, but good enough for our needs in this
// package.
//class Random {
// private:
//  uint32_t seed_;
// public:

void Random_constructor(Random *random,uint32_t s){
  random->seed_=s & 0x7fffffffu;
}
  
  uint32_t Random_Next(Random *random) {
    static const uint32_t M = 2147483647L;   // 2^31-1
    static const uint64_t A = 16807;  // bits 14, 8, 7, 5, 2, 1, 0
    
    // We are computing
    //       seed_ = (seed_ * A) % M,    where M = 2^31-1
    //
    // seed_ must not be zero or M, or else all subsequent computed values
    // will be zero or M respectively.  For all other values, seed_ will end
    // up cycling through every number in [1,M-1]
    uint64_t product = random->seed_ * A;

    // Compute (product % M) using the fact that ((x << 31) % M) == x.
    //random->seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
    // The first reduction may overflow by 1 bit, so we may need to
    // repeat.  mod == M is not possible; using > allows the faster
    // sign-bit-based test.
    if (random->seed_ > M) {
      random->seed_ -= M;
    }
    return random->seed_;
  }

  // Returns a uniformly distributed value in the range [0..n-1]
  // REQUIRES: n > 0
  uint32_t Random_Uniform(Random *random,int n) { return Random_Next(random) % n; }

  // Randomly returns true ~"1/n" of the time, and false otherwise.
  // REQUIRES: n > 0
  bool Random_OneIn(Random *random,int n) { return (Random_Next(random) % n) == 0; }

  // Skewed: pick "base" uniformly from range [0,max_log] and then
  // return "base" random bits.  The effect is to pick a number in the
  // range [0,2^max_log-1] with exponential bias towards smaller numbers.
  uint32_t Random_Skewed(Random *random,int max_log) {
    return Random_Uniform(random,1 << Random_Uniform(random,max_log + 1));
  }


#endif  // STORAGE_LEVELDB_UTIL_RANDOM_H_
//#endif
