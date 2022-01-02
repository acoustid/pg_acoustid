MODULE_big = acoustid

OBJS = \
	   utils/base64.o \
	   utils/packed_uint3_array.o \
	   utils/packed_uint5_array.o \
	   fingerprint.o \
	   main.o

PG_CPPFLAGS = -DHAVE_CONFIG_H

EXTENSION = acoustid
DATA = acoustid--2.0.sql
PGFILEDESC = "acoustid - AcoustID search utilities"

REGRESS = acoustid_fingerprint_in_out

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
