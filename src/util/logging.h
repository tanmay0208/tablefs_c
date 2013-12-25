
/*
 * Logging.h
 *
 *  Created on: Jul 24, 2011
 *      Author: kair
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>

//namespace tablefs {

struct Logging {
  char logging_filename[1024];     //Size can be changed
  FILE *logfile;
};

typedef struct Logging Logging;

//public:

  void Logging_constructor_default(Logging *logging);

  void Logging_constructor(Logging *logging,const char *path);


  //Logging(const std::string &path) : logging_filename(path) {}

  void Logging_Open(Logging *);

  void Logging_LogMsg(Logging *,const char *format, ...);

  void Logging_LogStat(Logging *,const char* path, const struct stat* statbuf);

  void Logging_Destructor(Logging *);

  static Logging* Logging_Default();

  void Logging_SetDefault(Logging *,Logging* log);

//private:
//  std::string logging_filename;
//  FILE *logfile;
//};

//}

#endif /* LOGGING_H_ */
