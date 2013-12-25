#include "util/command.h"
#include <stdio.h>
#include <stdlib.h>



int MountDevice(const char* partition, const char *mountpoint) {
  char cmd[1024];
  sprintf(cmd, "mount /dev/%s %s", partition, mountpoint);
  return system(cmd);
}

int UmountDevice(const char* partition, const char *mountpoint) {
  char cmd[1024];
  sprintf(cmd, "umount /dev/%s %s", partition, mountpoint);
  return system(cmd);
}

int DropBufferCache() {
  return system("echo 3 > /proc/sys/vm/drop_caches");
}



