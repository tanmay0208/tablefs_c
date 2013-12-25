/*
 *  Created on: Aug 15, 2011
 *      Author: kair
 */

#ifndef TFS_INODE_H_
#define TFS_INODE_H_

#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include "leveldb/slice.h"

//namespace tablefs {

typedef uint64_t tfs_inode_t;
typedef uint64_t tfs_hash_t;
typedef struct stat tfs_stat_t;

static const char PATH_DELIMITER = '/';
static const int INODE_PADDING = 104;   
static const int MAX_PATH_LEN = 256;
static const tfs_inode_t ROOT_INODE_ID = 0;
static const int NUM_FILES_IN_DATADIR_BITS = 14;
static const int NUM_FILES_IN_DATADIR = 16384;
static const int MAX_OPEN_FILES = 512;
static const char* ROOT_INODE_STAT = "/tmp/";

struct tfs_meta_key_t {
  tfs_inode_t inode_id;
  tfs_hash_t hash_id;

  /*
  char str[17];
  leveldb::Slice ToSlice() const {
    return leveldb::Slice(str, 16);
  }
  * commented bt Kai
  */
  
  #ifdef UNSUPPORTED
  const std::string ToString() const {
    return std::string((const char *) this, sizeof(tfs_meta_key_t));
  }

  leveldb::Slice ToSlice() const {
    return leveldb::Slice((const char *) this, sizeof(tfs_meta_key_t));
  }
  #endif
};
typedef struct tfs_meta_key_t tfs_meta_key_t; 

struct tfs_inode_header {
  tfs_stat_t fstat;
  char padding [104];
  //char padding [INODE_PADDING];       Changes are made ..
  uint32_t has_blob;
  uint32_t namelen;
};
typedef struct tfs_inode_header tfs_inode_header;

static const size_t TFS_INODE_HEADER_SIZE = sizeof(tfs_inode_header);
static const size_t TFS_INODE_ATTR_SIZE = sizeof(struct stat);

struct tfs_inode_val_t {
  size_t size;
  char* value;
  #ifdef UNSUPPORTED
  tfs_inode_val_t() : value(NULL), size(0) {}

  leveldb::Slice ToSlice() const {
    return leveldb::Slice((const char *) value, size);
  }

  std::string ToString() const {
    return std::string(value, size);
  }
  #endif
};
typedef struct tfs_inode_val_t tfs_inode_val_t;
//}

#endif
