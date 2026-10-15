MODULE_big = acoustid
OBJS = acoustid_compare.o acoustid_fingerprint.o fingerprint_codec.o

EXTENSION = acoustid
DATA = acoustid--1.0.sql acoustid--1.0--1.1.sql acoustid--unpackaged--1.0.sql
PGFILEDESC = "acoustid - AcoustID utility functions"

REGRESS = acoustid_fingerprint

EXTRA_CLEAN = test/test_fingerprint_codec test/test_fingerprint_codec_portable

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# The fingerprint codec on its own, outside of PostgreSQL. The second build
# takes the byte-by-byte path big-endian machines use.
CODEC_TEST_CFLAGS ?= -std=c99 -Wall -Wextra -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all
CODEC_TEST_SOURCES = test/test_fingerprint_codec.c test/real_fingerprints.h fingerprint_codec.c fingerprint_codec.h

test/test_fingerprint_codec: $(CODEC_TEST_SOURCES)
	$(CC) $(CODEC_TEST_CFLAGS) -o $@ test/test_fingerprint_codec.c fingerprint_codec.c

test/test_fingerprint_codec_portable: $(CODEC_TEST_SOURCES)
	$(CC) $(CODEC_TEST_CFLAGS) -U__BYTE_ORDER__ -o $@ test/test_fingerprint_codec.c fingerprint_codec.c

check-codec: test/test_fingerprint_codec test/test_fingerprint_codec_portable
	./test/test_fingerprint_codec
	./test/test_fingerprint_codec_portable

.PHONY: check-codec
