CC = gcc
C = g++

#-----------------------------------------------
# Uncomment exactly one of the lines labelled (A), (B), and (C) below
# to switch between compilation modes.

OPT = -O2 -DNDEBUG       # (A) Production use (optimized mode)
# OPT = -g2              # (B) Debug mode, w/ full line-level debugging symbols
# OPT = -O2 -g2 -DNDEBUG # (C) Profiling mode: opt, but w/debugging symbols
#-----------------------------------------------

$(shell sh ./build_detect_platform)

include build_config.mk

SNAPPY=0
ifeq ($(SNAPPY), 1)
SNAPPY_CFLAGS=
SNAPPY_LDFLAGS=
else
SNAPPY_CFLAGS=
SNAPPY_LDFLAGS=
endif

DEBUGFLAGS = 

CFLAGS = -c -I. -I./include $(DEBUGFLAGS) $(PORT_CFLAGS) $(OPT) $(SNAPPY_CFLAGS)
#$(PLATFORM_CFLAGS)
LDFLAGS=$(PLATFORM_LDFLAGS) $(SNAPPY_LDFLAGS) $(GOOGLE_PERFTOOLS_LDFLAGS)

FLAG=-D_FILE_OFFSET_BITS=64
FUSEFLAGS=`pkg-config fuse --cflags` $(FLAG)

FSLIBOJECTS=`pkg-config fuse --libs`

LIBOBJECTS = \
./include/leveldb/slice.o \
./fs/dcache.o \
./fs/tablefs.o \
./fs/tfs_state.o \
./fs/fswrapper.o \
./fs/icache.o \
./fs/inodemutex.o \
./adaptor/leveldb_adaptor.o \
./util/properties.o \
./util/logging.o \
./util/monitor.o \
./util/allocator.o \
./util/traceloader.o \
./util/command.o \
./util/testutil.o \
./util/myhash.o \
./util/socket.o \
./fs/vector.o \

METADBOBJECTS = \
./metadb/metadb_fs.o \
./metadb/sha.o

PROGRAMS = bench fsbench tablefs tablefs_lib tablefs-debug fstestcase ldb_test ldb_check

LIBRARY = $(LEVELDBLIB)

all: $(LIBOBJECTS)

clean:
	-rm -f $(PROGRAMS) ./*.o */*.o

dbbench: ./dbbench.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) dbbench.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

tablefs: ./tablefs_main.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) tablefs_main.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

tablefs_lib: ./tablefs_lib.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) tablefs_lib.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

fstest: ./fstest.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) fstest.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

fswrapper_test: ./fswrapper_test.o $(LIBOBJECTS) $(FSLIBOJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) fswrapper_test.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

fsbench: ./fsbench.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) fsbench.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) $(FUSELIBS) -o $@

fsbench_large: ./fsbench_large.o $(LIBOBJECTS) $(FSLIBOJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) fsbench_large.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

metadb_stress_test: ./metadb_stress_test.o $(LIBOBJECTS) $(FSLIBOJECTS) $(METADBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) metadb_stress_test.o $(LIBOBJECTS) $(FSLIBOJECTS) $(METADBOBJECTS) $(LIBRARY) -o $@

ldb_check: ./ldb_check.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) ldb_check.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

test: ./fs/dcache.o
	$(CC) $(LDFLAGS) $(FUSEFLAGS) ./fs/dcache.o $(LIBRARY) -o $@

metamgr_test: ./fs/metamgr_test.o $(LIBOBJECTS)
	$(CC) $(LDFLAGS) $(FUSEFLAGS) ./fs/metamgr_test.o $(LIBOBJECTS) $(FSLIBOJECTS) $(LIBRARY) -o $@

monitor_test: ./monitor_test.o ./util/monitor.o
	$(CC) $(LDFLAGS) $(FUSEFLAGS) ./monitor_test.o ./util/monitor.o -o $@

.cc.o:
	$(CC) $(FUSEFLAGS) $(CFLAGS) $< -o $@

.cpp.o:
	$(CC) $(FUSEFLAGS) $(CFLAGS) $< -o $@
