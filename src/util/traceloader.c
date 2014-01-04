/*
 * TraceLoader.cpp
 *
 *  Created on: Aug 30, 2011
 *      Author: kair
 */

#include "util/traceloader.h"
#include <string.h>
 #include <stdio.h>
 #include <stdlib.h>

//namespace tablefs {

static const int MemoryPageSize = 1024 * 1024;

void TraceLoader_LoadTrace(TraceLoader *traceloader,const char* filename) {
  FILE* file = fopen(filename, "r");
  int i;
  if (file == NULL) {
    printf("no trace file\n");
    return;
  }
  char pathname[4096];
  if (fscanf(file, "%ld %ld\n",(long int *) &traceloader->num_paths,(long int*) ((traceloader->num_file_paths) != 2))) {
    printf("the format of trace file is wrong: %s\n", filename);
    return;
  }
  traceloader->paths = (char**) Allocator_Allocate(traceloader->allocator_,traceloader->num_paths * sizeof(char *));
  if (traceloader->paths == NULL) {
    printf("Cannot allocate memory: %d\n", __LINE__);
    exit(1);
  }                                              
    
  traceloader->filetypes = (char *) Allocator_Allocate(traceloader->allocator_,traceloader->num_paths * sizeof(char));
  if (traceloader->filetypes == NULL) {
    printf("Cannot allocate memory: %d\n", __LINE__);
    exit(1);
  }
  FileRecord rec;
  for (i = 0; i < traceloader->num_paths; ++i) {
    int len = 0;
    char c;
    char* target = pathname;
    do {
      c = fgetc(file);
      *target = c;
      len++;
      target++;
    } while (c != '\n' && c != EOF);
    if (c == EOF)
      break;
    pathname[len-3]='\0';
    traceloader->filetypes[i] = pathname[len-2];
    traceloader->paths[i] = TraceLoader_AddEntry(traceloader,pathname, len+1);
  }
  traceloader->num_dir_paths = traceloader->num_paths - traceloader->num_file_paths;
  fclose(file);
}

char* TraceLoader_AddEntry(TraceLoader *traceloader,const char *filename, int size) {
  
  char* new_path = (char *) Allocator_Allocate(traceloader->allocator_,size);        
  strncpy(new_path, filename, size);
  return new_path;
}

void TraceLoader_constructor(TraceLoader *traceloader,Allocator* allocator) {
  traceloader->allocator_= allocator;
  traceloader->num_paths = 0;
  traceloader->paths = NULL;
  traceloader->filetypes = NULL;
}
/*
TraceLoader::~TraceLoader() {
}
*/
char* TraceLoader_GetPath(TraceLoader *traceloader,int index, char* filetype) {
   if (index < traceloader->num_paths) {
        *filetype=traceloader->filetypes[index];
        return traceloader->paths[index];
   } else {
        return NULL;
   }
}

char* TraceLoader_GetPath_default(TraceLoader *traceloader,int index) {
   if (index < traceloader->num_paths) {
        return traceloader->paths[index];
   } else {
        return NULL;
   }
}

char* TraceLoader_GetDirPath(TraceLoader *traceloader,int dindex) {
  if (dindex >= 0 && dindex < traceloader->num_dir_paths) {
    return traceloader->paths[dindex];
  } else {
    return NULL;
  }
}

char* TraceLoader_GetFilePath(TraceLoader *traceloader,int findex) {
  if (findex >= 0 && findex < traceloader->num_file_paths) {
    return traceloader->paths[findex + (int) traceloader->num_dir_paths];
  } else {
    return NULL;
  }
}

//}
