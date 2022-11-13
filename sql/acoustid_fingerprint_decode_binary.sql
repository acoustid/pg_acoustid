SELECT acoustid_fingerprint_decode_binary(decode('', 'base64'));
SELECT acoustid_fingerprint_decode_binary(decode('AQAAAA==', 'base64'));
SELECT acoustid_fingerprint_decode_binary(decode('AQAAA0mSJJGSJMm=', 'base64'));
SELECT acoustid_fingerprint_decode_binary(decode('AQAAA0mSJJGSJMmSCD/uA4AQ', 'base64'));
SELECT acoustid_fingerprint_decode_binary(decode('AQAAA0mSJJGSJMmSCD/uA4AQAQ==', 'base64'));
