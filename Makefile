MODULE_big = acoustid

OBJS = \
	   utils/base64.o \
	   utils/packed_int3_array.o \
	   utils/packed_int5_array.o \
	   fingerprint.o

PG_CPPFLAGS = \
	-DCU_ALLOC=palloc \
	-DCU_FREE=pfree \
	-DCU_REALLOC=repalloc \
	-DCU_ASSERT=Assert

EXTENSION = acoustid
DATA = acoustid--2.0.sql
PGFILEDESC = "acoustid - AcoustID search utilities"

REGRESS = \
	acoustid_fingerprint_type \
	acoustid_fingerprint_encode \
	acoustid_fingerprint_encode_binary \
	acoustid_fingerprint_decode \
	acoustid_fingerprint_decode_binary

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
