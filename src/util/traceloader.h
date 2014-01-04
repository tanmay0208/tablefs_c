/*
 * TraceLoader.h
 *
 *  Created on: Aug 30, 2011
 *      Author: kair
 */

#ifndef TRACELOADER_H_
#define TRACELOADER_H_
#include <string.h>
#include "fs/vector.h"
#include "util/allocator.h"

//namespace tablefs {

struct FileRecord {
  char* filename;
  char filetype;
};
typedef struct FileRecord FileRecord;

struct TraceLoader {
  char** paths;
  char* filetypes;
  Allocator* allocator_;
  size_t num_paths;
  size_t num_dir_paths;
  size_t num_file_paths;
};
typedef struct TraceLoader TraceLoader;
  
  char* TraceLoader_AddEntry(TraceLoader *,const char* filename, int size);

  void TraceLoader_constructor(TraceLoader *,Allocator *allocator);

  //virtual ~TraceLoader();

  void TraceLoader_LoadTrace(TraceLoader *,const char* filename);

  int TraceLoader_GetNumPaths(TraceLoader *traceloader) {
    return traceloader->num_paths;
  }

  int TraceLoader_GetNumFilePaths(TraceLoader *traceloader) {
    return traceloader->num_file_paths;
  }

  int TraceLoader_GetNumDirectoryPaths(TraceLoader *traceloader) {
    return traceloader->num_paths - traceloader->num_file_paths;
  }

  char* TraceLoader_GetPath(TraceLoader *,int index, char *filetype);

  char* TraceLoader_GetPath_default(TraceLoader *,int index);

  char* TraceLoader_GetDirPath(TraceLoader *,int dindex);

  char* TraceLoader_GetFilePath(TraceLoader *,int findex);


//}

#endif /* TRACELOADER_H_ */
