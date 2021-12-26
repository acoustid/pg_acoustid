MODULE_big = acoustid
OBJS = acoustid.o acoustid_compare.o acoustid_fingerprint_encoding.o

EXTENSION = acoustid
DATA = acoustid--1.0.sql acoustid--unpackaged--1.0.sql
PGFILEDESC = "acoustid - AcoustID utility functions"

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
