MODULE_big = acoustid
OBJS = acoustid_compare.o

EXTENSION = acoustid
DATA = acoustid--1.0.sql acoustid--unpackaged--1.0.sql
PGFILEDESC = "acoustid - AcoustID utility functions"

REGRESS = acoustid

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
