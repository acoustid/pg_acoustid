CREATE EXTENSION IF NOT EXISTS acoustid;
SET client_min_messages TO DEBUG1;

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    max_results := 0
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    max_distance := 1
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    '{440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    '{994277718,440498550,440498550,180443510,180476246,180485494,193052022,193052006,1233231206,1501666678}'::acoustid_raw_fingerprint,
    max_distance := 2
);

SELECT aoffset, to_char(ascore, '999D99') AS ascore FROM align_fingerprints(
    'AQAAE4qkJVGqKIH_An_hwx1-ofiLHj9-_NA1AvUT8K5x4gcFnDFQOCUeEsZooARDDg'::acoustid_fingerprint,
    'AQAAE4qkJVGqKIH_An_hwx1-ofiLHj9-_NA1AvUT8K5x4gcFnDFQOCUeEsZooARDDg'::acoustid_fingerprint
);
