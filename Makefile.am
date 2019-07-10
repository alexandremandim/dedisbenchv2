#SUBDIRS = libGenerator
#ACLOCAL_AMFLAGS = -I m4./
AUTOMAKE_OPTIONS = subdir-objects
AM_CXXFLAGS = -g -Wall -Iutils/random/randomgen
bin_PROGRAMS=DEDISbench DEDISgen DEDISgenutils
DEDISbench_SOURCES= benchcore/duplicates/duplicatedist.c libGenerator/generator.cpp libGenerator/generator.h libGenerator/generatorW.cpp libGenerator/generatorW.h benchcore/integrity/integrity.c benchcore/integrity/integrity.h  utils/random/random.c utils/db/berk.c structs/structs.h benchcore/accesses/iodist.c benchcore/io.c populate/populate.c benchcore/sharedmem/sharedmem.c DEDISbench.c parserconf/inih/ini.c io/plotio.c utils/utils.c
#DEDISbench_LDADD = libGenerator/libGenerator.la
DEDISbench_CFLAGS= -g -O2 -Wall -Iutils/random/randomgen -DINI_INLINE_COMMENT_PREFIXES=\"\#\"
DEDISbench_CXXFLAGS = -g -O2 -lstdc++ -lm -std=gnu++11
DEDISgen_SOURCES=DEDISgen.c utils/db/berk.c
DEDISgen_CFLAGS = -Wall
DEDISgenutils_SOURCES=DEDISgen-utils.c utils/db/berk.c
DEDISgenutils_CFLAGS = -Wall
