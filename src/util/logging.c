/*
 * Logging.cpp
 *
 *  Created on: Jul 24, 2011
 *      Author: kair
 */

#include "logging.h"
#include <sys/stat.h>
#include<string.h>

//namespace tablefs {

void Logging_constructor_default(Logging *logging){
  strcpy(logging->logging_filename,"/tmp/tmp.log");
}

void Logging_constructor(Logging *logging,const char *path){
  strcpy(logging->logging_filename,path);
}



void Logging_Destructor(Logging *logging) {
  if (logging->logfile != NULL) {
    fflush(logging->logfile);
    fclose(logging->logfile);
  }
}

void Logging_Open(Logging *logging) {
  if (sizeof(logging->logging_filename) > 0) {
    logging->logfile = fopen(logging->logging_filename, "w");
  } else {
    logging->logfile = NULL;
  }
}

void Logging_LogMsg(Logging *logging,const char *format, ...)
{
  va_list ap;
  if (logging->logfile != NULL) {
    va_start(ap, format);
    vfprintf(logging->logfile, format, ap);
    fflush(logging->logfile);
  }
}

void Logging_LogStat(Logging *logging,const char *path, const struct stat *statbuf)
{
  if (logging->logfile != NULL) {
    fprintf(logging->logfile, "Stat of [%s]:\n", path);
    fprintf(logging->logfile, "  inode[%d] mode[%d], uid[%d], gid[%d], size[%d]\n",
            (int)statbuf->st_ino,
            (int)statbuf->st_mode,(int)statbuf->st_uid,
            (int)statbuf->st_gid, (int)statbuf->st_size);
    fprintf(logging->logfile, "  atime[%d] mtime[%d] ctime[%d]\n",
            (int)statbuf->st_atime, (int)statbuf->st_mtime,
            (int)statbuf->st_ctime);
    fflush(logging->logfile);
  }
}


static Logging* log_ = NULL;

Logging* Logging_Default() {
  return log_;
}

void Logging_SetDefault(Logging *logging,Logging* log) {
  log_ = log;
}

//} 
