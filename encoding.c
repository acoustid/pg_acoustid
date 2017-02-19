#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "utils.h"

static uint8_t *
unpack_uint3_array(const uint8_t *first, const uint8_t *last, uint8_t *dest) {
	ssize_t size = last - first;
	const uint8_t *src = first;
	while (size >= 3) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		const uint8_t s2 = *src++;
		*dest++ = (s0 & 0x07);
		*dest++ = ((s0 & 0x38) >> 3);
		*dest++ = ((s0 & 0xc0) >> 6) | ((s1 & 0x01) << 2);
		*dest++ = ((s1 & 0x0e) >> 1);
		*dest++ = ((s1 & 0x70) >> 4);
		*dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x03) << 1);
		*dest++ = ((s2 & 0x1c) >> 2);
		*dest++ = ((s2 & 0xe0) >> 5);
		size -= 3;
	}
	if (size == 2) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		*dest++ = (s0 & 0x07);
		*dest++ = ((s0 & 0x38) >> 3);
		*dest++ = ((s0 & 0xc0) >> 6) | ((s1 & 0x01) << 2);
		*dest++ = ((s1 & 0x0e) >> 1);
		*dest++ = ((s1 & 0x70) >> 4);
	} else if (size == 1) {
		const uint8_t s0 = *src++;
		*dest++ = (s0 & 0x07);
		*dest++ = ((s0 & 0x38) >> 3);
	}
	return dest;
}

static uint8_t *
unpack_uint5_array(const uint8_t *first, const uint8_t *last, uint8_t *dest) {
	ssize_t size = last - first;
	const uint8_t *src = first;
	while (size >= 5) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		const uint8_t s2 = *src++;
		const uint8_t s3 = *src++;
		const uint8_t s4 = *src++;
		*dest++ = (s0 & 0x1f);
		*dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
		*dest++ = ((s1 & 0x7c) >> 2);
		*dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
		*dest++ = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
		*dest++ = ((s3 & 0x3e) >> 1);
		*dest++ = ((s3 & 0xc0) >> 6) | ((s4 & 0x07) << 2);
		*dest++ = ((s4 & 0xf8) >> 3);
		size -= 5;
	}
	if (size == 4) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		const uint8_t s2 = *src++;
		const uint8_t s3 = *src++;
		*dest++ = (s0 & 0x1f);
		*dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
		*dest++ = ((s1 & 0x7c) >> 2);
		*dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
		*dest++ = ((s2 & 0xf0) >> 4) | ((s3 & 0x01) << 4);
		*dest++ = ((s3 & 0x3e) >> 1);
	} else if (size == 3) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		const uint8_t s2 = *src++;
		*dest++ = (s0 & 0x1f);
		*dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
		*dest++ = ((s1 & 0x7c) >> 2);
		*dest++ = ((s1 & 0x80) >> 7) | ((s2 & 0x0f) << 1);
	} else if (size == 2) {
		const uint8_t s0 = *src++;
		const uint8_t s1 = *src++;
		*dest++ = (s0 & 0x1f);
		*dest++ = ((s0 & 0xe0) >> 5) | ((s1 & 0x03) << 3);
		*dest++ = ((s1 & 0x7c) >> 2);
	} else if (size == 1) {
		const uint8_t s0 = *src++;
		*dest++ = (s0 & 0x1f);
	}
	return dest;
}

PG_FUNCTION_INFO_V1(acoustid_decode_fingerprint);
Datum acoustid_decode_fingerprint(PG_FUNCTION_ARGS);

Datum
acoustid_decode_fingerprint(PG_FUNCTION_ARGS)
{
    bytea *in;
    ArrayType *out;
    uint8_t *bits, *exc_bits, *in_data;
    uint32_t value, *out_data;
    int i, j, off, num_values, num_bits, num_exc_bits, num_wanted_exc_bits, last_bit, in_size;

    in = PG_GETARG_BYTEA_P(0);
    in_data = (uint8_t *) VARDATA(in);
    in_size = VARSIZE(in) - VARHDRSZ;

    if (in_size <= 4) {
        ereport(ERROR, (errmsg("not enough data to decode fingerprint")));
    }

    num_values = (in_data[1] << 16) | (in_data[2] << 8) | in_data[3];

    off = 4;

    num_bits = (in_size - off) * 8 / 3;
    bits = (uint8_t *) palloc(num_bits);
    unpack_uint3_array(in_data + off, in_data + in_size, bits);

    num_wanted_exc_bits = 0;
    for (i = 0, j = 0; i < num_bits; i++) {
        if (bits[i] == 0) {
            if (++j == num_values) {
                num_bits = i + 1;
                break;
            }
        } else if (bits[i] == (1<<3)-1) {
            num_wanted_exc_bits++;
        }
    }

    if (j != num_values) {
        pfree(bits);
        ereport(ERROR, (errmsg("not enough data to decode fingerprint (normal bits)")));
    }

    off += (num_bits * 3 + 7) / 8;

    num_exc_bits = (in_size - off) * 8 / 5;
    if (num_exc_bits < num_wanted_exc_bits) {
        pfree(bits);
        ereport(ERROR, (errmsg("not enough data to decode fingerprint (exception bits)")));
    }

    if (num_exc_bits > 0) {
        exc_bits = (uint8_t *) palloc(num_exc_bits);
        unpack_uint5_array(in_data + off, in_data + in_size, exc_bits);

        for (i = 0, j = 0; i < num_bits; i++) {
            if (bits[i] == (1<<3)-1) {
                bits[i] += exc_bits[j++];
            }
        }

        pfree(exc_bits);
    }

	out = new_intArrayType(num_values);
	out_data = (uint32_t *) ARR_DATA_PTR(out);

    last_bit = 0;
    value = 0;
    for (i = 0, j = 0; i < num_bits; i++) {
        uint8_t bit = bits[i];
		if (bit == 0) {
		    out_data[j] = (j > 0) ? value ^ out_data[j-1] : value;
			value = 0;
			last_bit = 0;
            j++;
			continue;
		}
		bit += last_bit;
		last_bit = bit;
		value |= 1 << (bit - 1);
	}

    pfree(bits);
	PG_RETURN_ARRAYTYPE_P(out);
}
