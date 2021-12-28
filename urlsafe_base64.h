/*
 * base64.h
 *	  Encoding and decoding routines for base64 without whitespace
 *	  support.
 *
 * Portions Copyright (c) 2001-2021, PostgreSQL Global Development Group
 *
 * src/include/common/base64.h
 */
#ifndef URLSAFE_BASE64_H
#define URLSAFE_BASE64_H

/* url-safe base 64 */
extern int	urlsafe_b64_encode(const char *src, int len, char *dst, int dstlen);
extern int	urlsafe_b64_decode(const char *src, int len, char *dst, int dstlen);
extern int	urlsafe_b64_enc_len(int srclen);
extern int	urlsafe_b64_dec_len(int srclen);

#endif							/* URLSAFE_BASE64_H */
