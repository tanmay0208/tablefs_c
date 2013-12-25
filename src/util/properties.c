/*
 * Properties.cpp
 *
 *  Created on: Jul 19, 2011
 *      Author: kair
 */

#include "util/properties.h"
//#include <fstream>
#include <string.h>
//#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>   // added for malloc


/*Properties::Properties() {
}

Properties::~Properties() {
}*/

void Properties_setProperty(Properties *properties,const char *key, char *value) {
  //plist[key] = value;
}

void Properties_setPropertyInt(Properties *properties,const char *key, int value) {
  char buf[32];
  sprintf(buf, "%d", value);
  //plist[key] = std::string(buf);
}

char *Properties_getProperty(Properties *properties,const char *key) {
  /*std::map<std::string, std::string>::iterator it = plist.find(key);
  if (it != plist.end()) {
    return it->second;
  } else {
    return std::string("/");
  }*/
    return ("/");    // added for compiling , delete afterward
}

char *Properties_getProperty_default(Properties *properties,const char *key,
                                    char *defaultVal) {
  /*const std::map<std::string, std::string>::iterator it = plist.find(key);
  if (it != plist.end()) {
    return it->second;
  } else {
    return defaultVal;
  }*/
    return defaultVal;        // added for compiling , delete afterward
}

int Properties_getPropertyInt(Properties *properties,const char *key, int defaultVal) {
  /*const std::map<std::string, std::string>::iterator it = plist.find(key);
  defaultVal=0;
  if (it != plist.end()) {
    return atoi(it->second.data());
  } else {
    return defaultVal;
  }
  */
  return defaultVal;       // added for compiling , delete afterward
}

double Properties_getPropertyDouble(Properties *properties,const char *key,
                                     double defaultVal) {
  /*std::map<std::string, std::string>::iterator it = plist.find(key);
  defaultVal=0;
  if (it != plist.end()) {
    return atof(it->second.data());
  } else {
    return defaultVal;
  }*/
      return defaultVal;      // added for compiling , delete afterward
}

bool Properties_getPropertyBool(Properties *properties,const char *key,
                                 bool defaultVal) {
  /*std::map<std::string, std::string>::iterator it = plist.find(key);
  defaultVal=false
  if (it != plist.end()) {
    if (it->second.compare("true") == 0) {
      return true;
    } else {
      return false;
    }
  } else {
    return defaultVal;
  }*/
     return defaultVal;      // added for compiling , delete afterward
}

void Properties_load(Properties *properties,const char *filename) {
  FILE *is;	
  int ret;
  char line[1024];
  char *key,*val;   
  key=(char*)malloc(sizeof(char));
  val=(char*)malloc(sizeof(char));
  is=fopen(filename,"r");
  while (!feof(is)) {            //zhol marla ahe : while (is.good()) {
    ret=fscanf(is,"%s",line);
    if(ret==-1)
	printf("Error ///properties.c///");
    char* pch = strchr(line, '=');
    if (pch != NULL) {
      strncpy(key ,line, pch-line);
      strncpy(val,pch+1, strlen(pch+1));
      printf("%s %s\n", key, val);
      //plist[key] = val;           to be handeled 
    }
  }
  fclose(is);
}

void Properties_store(Properties *properties,const char *filename) {
  /*std::ofstream os;
  os.open(filename.data());
  if (os.good()) {
    std::map<std::string, std::string>::iterator it;
    for (it = plist.begin(); it != plist.end(); it++) {
      os << it->first << '=' << it->second << std::endl;
    }
  }
  os.close();*/
}

void Properties_parseOpts(Properties *properties,int argc, char *argv[]) {
   int i;
   for (i = 1; i < argc; ++i)
    if (argv[i][0] == '-') {
      Properties_setProperty(properties,argv[i]+1, argv[i+1]);
      printf("%s %s\n", argv[i]+1, argv[i+1]);
      ++i;
    }
}

void Properties_Report(Properties *properties,FILE* logf) {
  /*std::map<std::string, std::string>::iterator it;
  for (it = plist.begin(); it != plist.end(); it++) {
    fprintf(logf, "%s %s\n", it->first.c_str(), it->second.c_str());
  }*/
}

/*Properties& Properties::operator=(const Properties &oldp) {
  plist = oldp.plist;
  return *this;
}*/


