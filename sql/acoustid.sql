CREATE EXTENSION acoustid;

SELECT acoustid_decode_fingerprint(E''::bytea);
SELECT acoustid_decode_fingerprint(decode('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgc', 'base64'));
SELECT acoustid_decode_fingerprint(decode('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH/kH7amR5zgAnjgFAAMUMQA=', 'base64'));
SELECT acoustid_decode_fingerprint(decode('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH/kH7amR5zgAYCggGginjgFAKAMUMQA=', 'base64'));
SELECT acoustid_decode_fingerprint(decode('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH/kH7amR5zgAYCggGginjgFAKAMUMQA=', 'base64'), 10);
SELECT acoustid_decode_fingerprint(decode('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH/kH7amR5zgAYCggGginjgFAKAMUMQA=', 'base64'), 1);

SELECT acoustid_decode_fingerprint('');
SELECT acoustid_decode_fingerprint('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgc');
SELECT acoustid_decode_fingerprint('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH_kH7amR5zgAnjgFAAMUMQA');
SELECT acoustid_decode_fingerprint('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH_kH7amR5zgAYCggGginjgFAKAMUMQA');
SELECT acoustid_decode_fingerprint('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH_kH7amR5zgAYCggGginjgFAKAMUMQA', 10);
SELECT acoustid_decode_fingerprint('AQAAE0uinMoUPMKpI5q14Eez8mhi5uiSKDNin6jFBjgcpTmmZdPxH_kH7amR5zgAYCggGginjgFAKAMUMQA', 1);

SELECT acoustid_extract_full_query('{}'::int4[]);
SELECT acoustid_extract_full_query('{443056572,443062204,976852924,976928665,708492953,725131177}'::int4[]);
SELECT acoustid_extract_full_query('{443056572,443062204,976852924,976928665,708492953,725131177,443056572}'::int4[]);
SELECT acoustid_extract_full_query('{443056572,443062204,976852924,976928665,708492953,725131177}'::int4[], 8);
