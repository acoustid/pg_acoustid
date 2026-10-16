-- complain if script is sourced in psql, rather than via ALTER EXTENSION
\echo Use "ALTER EXTENSION acoustid UPDATE TO '1.1'" to load this file. \quit

CREATE FUNCTION acoustid_encode_fingerprint(int4[], int4 DEFAULT 1) RETURNS bytea
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION acoustid_decode_fingerprint(bytea) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
