\echo Use "CREATE EXTENSION acoustid" to load this file. \quit

CREATE OR REPLACE FUNCTION acoustid_fingerprint_in(cstring)
RETURNS acoustid_fingerprint
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_fingerprint_out(acoustid_fingerprint)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_raw_fingerprint_in(cstring)
RETURNS acoustid_raw_fingerprint
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION acoustid_raw_fingerprint_out(acoustid_raw_fingerprint)
RETURNS cstring
AS 'MODULE_PATHNAME'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION extract_version(acoustid_fingerprint)
RETURNS int4
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_extract_version'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION extract_terms(acoustid_fingerprint)
RETURNS int4[]
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_extract_terms'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION uncompress(acoustid_fingerprint)
RETURNS acoustid_raw_fingerprint
AS 'MODULE_PATHNAME', 'acoustid_fingerprint_extract_terms'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION extract_terms(acoustid_raw_fingerprint)
RETURNS int4[]
AS 'MODULE_PATHNAME', 'acoustid_raw_fingerprint_extract_terms'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION align_fingerprints(
    in fp1 acoustid_raw_fingerprint,
    in fp2 acoustid_raw_fingerprint,
    in max_results int4 default 3,
    in max_distance int4 default -1, 
    out aoffset int4,
    out ascore float4
)
RETURNS setof record
AS 'MODULE_PATHNAME', 'acoustid_align_fingerprints'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION match_fingerprints(
    in fp1 acoustid_raw_fingerprint,
    in fp2 acoustid_raw_fingerprint,
    in aoffset int4 default 0,
    in max_results int4 default 10,
    out pos1 int4,
    out pos2 int4,
    out len int4,
    out score float4
)
RETURNS setof record
AS 'MODULE_PATHNAME', 'acoustid_match_fingerprints'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Compressed fingerprint
CREATE TYPE acoustid_fingerprint (
    INPUT = acoustid_fingerprint_in,
    OUTPUT = acoustid_fingerprint_out,
    STORAGE = EXTERNAL,
    LIKE = bytea
);

CREATE CAST (acoustid_fingerprint AS bytea)
    WITHOUT FUNCTION
    AS IMPLICIT;

-- Raw uncompressed fingerprint
CREATE TYPE acoustid_raw_fingerprint (
    INPUT = acoustid_raw_fingerprint_in,
    OUTPUT = acoustid_raw_fingerprint_out,
    LIKE = int4[]
);

CREATE CAST (acoustid_raw_fingerprint AS int4[])
    WITH FUNCTION extract_terms(acoustid_raw_fingerprint)
    AS IMPLICIT;
