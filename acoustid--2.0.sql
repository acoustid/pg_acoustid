\echo Use "CREATE EXTENSION acoustid" to load this file. \quit

CREATE OR REPLACE FUNCTION acoustid_fingerprint_in(cstring)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_out(acoustid_fingerprint)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_cast_to_bytea(acoustid_fingerprint)
RETURNS bytea
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Compressed fingerprint
CREATE TYPE acoustid_fingerprint (
    INPUT = acoustid_fingerprint_in,
    OUTPUT = acoustid_fingerprint_out,
    INTERNALLENGTH = VARIABLE,
    STORAGE = EXTERNAL
);

CREATE CAST (acoustid_fingerprint AS bytea)
    WITH FUNCTION acoustid_fingerprint_cast_to_bytea(acoustid_fingerprint)
    AS IMPLICIT;
