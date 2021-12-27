MODULE_big = acoustid
OBJS = acoustid.o acoustid_fingerprint_type.o acoustid_compare.o

EXTENSION = acoustid
DATA = acoustid--1.0.sql acoustid--1.0--1.1.sql acoustid--unpackaged--1.0.sql
PGFILEDESC = "acoustid - AcoustID utility functions"

REGRESS = acoustid

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
