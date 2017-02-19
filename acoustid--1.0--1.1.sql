-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "ALTER EXTENSION acoustid UPDATE TO '1.1'" to load this file. \quit

CREATE OR REPLACE FUNCTION acoustid_extract_full_query(int4[]) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_extract_full_query(int4[], int) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_decode_fingerprint(bytea) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;
