#include "fs/tfs_state.h"
#include <unistd.h>
#include <string.h>

//namespace tablefs {

LevelDBAdaptor* FileSystemState_GetMetaDB(FileSystemState *filesystemstate) {
      return filesystemstate->metadb;
}

Logging* FileSystemState_GetLog(FileSystemState *filesystemstate) {
      return filesystemstate->logs;
}


const char *FileSystemState_GetDataDir(FileSystemState *filesystemstate) {
  return filesystemstate->datadir_;
}

const int FileSystemState_GetThreshold(FileSystemState *filesystemstate) {
  return filesystemstate->threshold_;
}

bool FileSystemState_IsEmpty(FileSystemState *filesystemstate) {
  return (filesystemstate->max_inode_num == 0);
}



void FileSystemState_constructor(FileSystemState *filesystemstate) {
 filesystemstate->metadb=NULL;
 filesystemstate->max_inode_num=0;      //max_inode_num is object of tfs_inode_t
 filesystemstate->threshold_=0;
 filesystemstate->logs=NULL ;

}

/*FileSystemState::~FileSystemState() {
}*/

int FileSystemState_Setup(FileSystemState *filesystemstate,Properties *prop) {
  char resolved_path[4096];
  char* ret;
  ret = realpath(Properties_getProperty(prop,"metadir"), resolved_path);
  strcpy(filesystemstate->metadir_ ,resolved_path);
  ret = realpath(Properties_getProperty(prop,"datadir"), resolved_path);
  strcpy(filesystemstate->datadir_ ,resolved_path);
  ret = realpath(Properties_getProperty(prop,"mountdir"), resolved_path);
  strcpy(filesystemstate->mountdir_, resolved_path);

  filesystemstate->threshold_ = Properties_getPropertyInt(prop,"threshold", 4096);
  if (access(filesystemstate->datadir_, W_OK)>0 || access(filesystemstate->metadir_, W_OK)>0) {
     fprintf(stderr, "cannot open directory!\n");
     exit(1);
  }
  filesystemstate->logs=(Logging*)malloc(sizeof(Logging));
  Logging_constructor(filesystemstate->logs,Properties_getProperty_default(prop,"logfile",(char*) ""));
  //Logging_SetDefault()        seems jhol 2 arguments are same
  Logging_Open(filesystemstate->logs);

  /*logs = new Logging(prop.getProperty("logfile", ""));
  logs->SetDefault(logs); 		//Logging is class 
  logs->Open();*/

  Properties *prop_ = prop;
  Properties_setProperty(prop_,"leveldb.db", strcat(filesystemstate->metadir_,"/meta"));
  Properties_setProperty(prop_,"leveldb.create.if.missing.db",(char*)"true");

  filesystemstate->metadb =(LevelDBAdaptor*)malloc(sizeof(LevelDBAdaptor));
  LevelDBAdaptor_constructor(filesystemstate->metadb);
  //LevelDBAdaptor_SetProperties(filesystemstate->metadb,prop_);      prop_ is pointer & we need struct
  LevelDBAdaptor_SetLogging(filesystemstate->metadb,filesystemstate->logs);					//LevelDB madhe handle karne
  if (LevelDBAdaptor_Init(filesystemstate->metadb) < 0) {
    printf("failed to open metadb %s\n", Properties_getProperty(prop_,"leveldb.db"));
    return -1;
  } else {
    printf("open metadb successfully %s\n",filesystemstate->metadir_);
  }

  Logging_LogMsg(filesystemstate->logs,"Initialized two databases.\n");

  char fpath[256];
  sprintf(fpath, "%s/root.dat", filesystemstate->datadir_);
  FILE *f = fopen(fpath, "r");
  if (f == NULL) {
    f = fopen(fpath, "w");
    filesystemstate->max_inode_num = 0;
    fprintf(f, "%u\n", (int)filesystemstate->max_inode_num);
    fclose(f);
    char fpath[512];
    sprintf(fpath, "%s/%d", filesystemstate->datadir_, 0);
    mkdir(fpath, 0777);
  } else {
    if (fscanf(f, "%u", (int *)&filesystemstate->max_inode_num) == 0) {
      filesystemstate->max_inode_num = 0;
    }
    fclose(f);
  }

  return 0;
}

void FileSystemState_Destroy(FileSystemState *filesystemstate) {
  char fpath[256];
  sprintf(fpath, "%s/root.dat",filesystemstate->datadir_);
  FILE* f = fopen(fpath, "w");
  if (f != NULL) {
    //fprintf(f, "%u\n", filesystemstate->max_inode_num);   unsigned interger expected
    fclose(f);
    Logging_LogMsg(filesystemstate->logs,"fpath: %s\n", fpath);
  } else {
    Logging_LogMsg(filesystemstate->logs,"Cannot write the max inode num: %s %s\n", 
                 fpath, strerror(errno));
  }
  if (filesystemstate->metadb != NULL) {
    LevelDBAdaptor_Cleanup(filesystemstate->metadb);
    free (filesystemstate->metadb);
  }
  if (filesystemstate->logs != NULL)			
    free (filesystemstate->logs);
}

tfs_inode_t FileSystemState_NewInode(FileSystemState *filesystemstate) {
  filesystemstate->max_inode_num++;
  if (filesystemstate->max_inode_num % (NUM_FILES_IN_DATADIR) == 0) {
    char fpath[512];
    sprintf(fpath, "%s/%d", filesystemstate->datadir_,
            (int) filesystemstate->max_inode_num >> 14);
    mkdir(fpath, 0777);
  }
  return filesystemstate->max_inode_num; 
}

//}
