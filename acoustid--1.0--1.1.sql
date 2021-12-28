\echo Use "CREATE EXTENSION acoustid" to load this file. \quit

CREATE OR REPLACE FUNCTION acoustid_fingerprint_in(cstring)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_out(acoustid_fingerprint)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_recv(internal)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_send(acoustid_fingerprint)
RETURNS bytea
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_decode(text)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_decode_from_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_decode_binary(bytea)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_decode_from_bytea'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_encode(acoustid_fingerprint)
RETURNS text
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_encode_to_text'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_encode_binary(acoustid_fingerprint)
RETURNS bytea
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_encode_to_bytea'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE acoustid_fingerprint (
    INPUT = acoustid_fingerprint_in,
    OUTPUT = acoustid_fingerprint_out,
    RECEIVE = acoustid_fingerprint_recv,
    SEND = acoustid_fingerprint_send,
    INTERNALLENGTH = VARIABLE,
    STORAGE = EXTERNAL
);
