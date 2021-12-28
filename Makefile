MODULE_big = acoustid
OBJS = \
	   acoustid.o \
	   acoustid_fingerprint_type.o \
	   acoustid_fingerprint_encoding.o \
	   acoustid_compare.o \
	   pack_int3_array.o \
	   pack_int5_array.o \
	   unpack_int3_array.o \
	   unpack_int5_array.o \
	   base64.o

EXTENSION = acoustid
DATA = acoustid--1.0.sql acoustid--1.0--1.1.sql acoustid--unpackaged--1.0.sql
PGFILEDESC = "acoustid - AcoustID utility functions"

REGRESS = acoustid

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
