-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION intarray FROM unpackaged" to load this file. \quit

ALTER EXTENSION intarray ADD function acoustid_compare2(int4[], int4[], int);
ALTER EXTENSION intarray ADD function acoustid_extract_query(int4[]);
