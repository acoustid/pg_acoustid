-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION acoustid" to load this file. \quit

CREATE OR REPLACE FUNCTION acoustid_compare2(int4[], int4[], int DEFAULT 0) RETURNS float4
    AS 'MODULE_PATHNAME'
    LANGUAGE C VOLATILE STRICT -- marked as VOLATILE to avoid multiple calls, even though the results are immutable
    COST 1000;

CREATE OR REPLACE FUNCTION acoustid_extract_query(int4[]) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_extract_full_query(int4[]) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_extract_full_query(int4[], int) RETURNS int4[]
    AS 'MODULE_PATHNAME'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_decode_fingerprint(bytea) RETURNS int4[]
    AS 'MODULE_PATHNAME', 'acoustid_decode_fingerprint_bytea'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION acoustid_decode_fingerprint(text) RETURNS int4[]
    AS 'MODULE_PATHNAME', 'acoustid_decode_fingerprint_text'
    LANGUAGE C IMMUTABLE STRICT;
