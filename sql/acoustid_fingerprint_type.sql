CREATE EXTENSION acoustid;

SELECT '{198178303,198047167,164426175}'::acoustid_fingerprint;
SELECT '{4294967295}'::acoustid_fingerprint;
SELECT '{4294967295}v1'::acoustid_fingerprint;
SELECT '{}'::acoustid_fingerprint;
SELECT '1,2,3'::acoustid_fingerprint;
SELECT '{1,2,3'::acoustid_fingerprint;
SELECT '{a}'::acoustid_fingerprint;
SELECT '1,2,3}'::acoustid_fingerprint;
SELECT '{1, 2, 3}'::acoustid_fingerprint;

CREATE TABLE test_table (
    id SERIAL PRIMARY KEY,
    fingerprint acoustid_fingerprint NOT NULL
);

INSERT INTO test_table (fingerprint) VALUES
    ('{198178303,198047167,164426175}'),
    ('{4294967295}'),
    ('{4294967295}v1'),
    ('{}');

SELECT * FROM test_table;
