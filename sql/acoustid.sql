CREATE EXTENSION acoustid;

SET client_min_messages TO INFO;

SELECT '{1,2,3}'::acoustid_fingerprint;

CREATE TABLE test_acoustid_fingerprint (
    id serial PRIMARY KEY,
    fingerprint acoustid_fingerprint NOT NULL
);

INSERT INTO test_acoustid_fingerprint (fingerprint) VALUES ('{1,2,3}');

SELECT * FROM test_acoustid_fingerprint;

SELECT acoustid_fingerprint_decode('AQAAC0mSJJGSJMmSCD_uQzi-4xd-w4HwC4AQcQ4AIohBAA');

SELECT acoustid_fingerprint_decode(E'\\x0100000b499224919224c992083fee4338bee3177ec381f00b8010710e0022884100'::bytea)
