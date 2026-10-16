CREATE EXTENSION acoustid;

-- The example from Chromaprint's tests: two hashes, version 55.
SELECT acoustid_encode_fingerprint('{1,0}', 55);
SELECT acoustid_decode_fingerprint('\x370000024100');
SELECT acoustid_encode_fingerprint('{1,0}');

-- Negative int4 values are the hashes with the top bit set.
SELECT acoustid_encode_fingerprint('{-1,-2147483648,0}');
SELECT acoustid_decode_fingerprint(acoustid_encode_fingerprint('{-1,-2147483648,0}'));

-- A real fingerprint, as clients submit it (URL-safe base64).
CREATE TEMP TABLE fp AS
SELECT decode(translate('AQAAEwkjrUmSJQpUHflR9mjSJMdZpcO_Imdw9dCO9Clu4_wQPvhCB01w6xAtXNcAp5RASgDBhDSCGGIAcwA', '-_', '+/') || '=', 'base64') AS data;
SELECT acoustid_decode_fingerprint(data) FROM fp;
SELECT acoustid_encode_fingerprint(acoustid_decode_fingerprint(data)) = data FROM fp;

-- Empty fingerprints, also with data after the header.
SELECT acoustid_encode_fingerprint('{}');
SELECT acoustid_decode_fingerprint('\x01000000');
SELECT acoustid_decode_fingerprint('\x01000000ffb66d03');

-- Data after the end of a fingerprint is ignored.
SELECT acoustid_decode_fingerprint('\x370000024100ffffffff');
SELECT acoustid_decode_fingerprint('\x370000024100'::bytea || decode(repeat('ff', 1000000), 'hex'));

-- Arrays not starting at index 1 are encoded the same.
SELECT acoustid_encode_fingerprint('[5:6]={1,0}', 55);

-- NULL in, NULL out.
SELECT acoustid_encode_fingerprint(NULL) IS NULL, acoustid_decode_fingerprint(NULL) IS NULL;

-- Invalid fingerprints.
SELECT acoustid_decode_fingerprint('\x');
SELECT acoustid_decode_fingerprint('\x010000');
SELECT acoustid_decode_fingerprint('\x01000001');
SELECT acoustid_decode_fingerprint('\x01ffffff00');
-- Four hashes, but only three terminators.
SELECT acoustid_decode_fingerprint('\x370000044100');
-- A set bit past bit 32: six distances of 6, and 7 + 26.
SELECT acoustid_decode_fingerprint('\x01000001b66d03');
SELECT acoustid_decode_fingerprint('\x01000001071a');

-- Invalid arrays and versions.
SELECT acoustid_encode_fingerprint('{1,NULL}');
SELECT acoustid_encode_fingerprint('{{1,2},{3,4}}');
SELECT acoustid_encode_fingerprint('{1}', 256);
SELECT acoustid_encode_fingerprint('{1}', -1);

-- Upgrading an existing installation adds the functions.
DROP EXTENSION acoustid;
CREATE EXTENSION acoustid VERSION '1.0';
SELECT count(*) FROM pg_proc WHERE proname LIKE 'acoustid\_%code\_fingerprint';
ALTER EXTENSION acoustid UPDATE TO '1.1';
SELECT acoustid_decode_fingerprint(acoustid_encode_fingerprint('{5,-6,7}'));
