/*
 * Properties.h
 *
 *  Created on: Jul 19, 2011
 *      Author: kair
*/

#ifndef PROPERTIES_H_
#define PROPERTIES_H_
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
//#include <map>      c++ map

//namespace tablefs {

struct Properties {
  //std::map<std::string, std::string> plist;       std::map handle  std::plist new structure
};
typedef struct Properties Properties;
 /* explicit Properties();      We dont need it
  virtual ~Properties();*/

  //Properties & operator=(const Properties &oldp);   Operator Overloading

  char *Properties_getProperty(Properties *,const char *key);

  char *Properties_getProperty_default(Properties *,const char *key, char *defaultValue);

  int Properties_getPropertyInt(Properties *,const char *key, int defaultValue);

  double Properties_getPropertyDouble(Properties *,const char *key, double defaultValue);

  bool Properties_getPropertyBool(Properties *,const char *key, bool defaultValue);

  void Properties_setProperty(Properties *,const char *key, char *value);

  void Properties_setPropertyInt(Properties *,const char *key, int value);

  void Properties_load(Properties *,const char *filename);

  void Properties_store(Properties *,const char *filename);

  void Properties_parseOpts(Properties *,int argc, char *argv[]);

  void Properties_Report(Properties *,FILE* logf);
//}

#endif /* PROPERTIES_H_ */
