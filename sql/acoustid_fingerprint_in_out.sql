CREATE EXTENSION acoustid;

SELECT '13123@$#@!'::acoustid_fingerprint;

SELECT 'AQAAO4qkJVGqKIH_An_hwx1-ofiLHj9-_NA1AvUT8K5x4sel4JGQrEda4YenG940o8h_JM4aw5_BH_dy_GiUFbW0oM2I5sgevHgLzUfjJEdJw6_Rj2De5PCHWjj64OLR5zjxo60M18I5wSt-AAWcMVA4JR4SxmigBEOOKAEFIIwIJoRgSgiLAAUEKAEGEkIRCwgCAA'::acoustid_fingerprint;

SELECT CAST('AQAAE4qkJVGqKIH_An_hwx1-ofiLHj9-_NA1AvUT8K5x4gcFnDFQOCUeEsZooARDDg'::acoustid_fingerprint AS bytea);
SELECT CAST('AQAAE4qkJVGqKIH_An_hwx1-ofiLHj9-_NA1AvUT8K5x4gcFnDFQOCUeEsZooARDDg'::acoustid_fingerprint AS text);