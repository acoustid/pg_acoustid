#ifndef PG_ACOUSTID_UINT8_VECTOR_H
#define PG_ACOUSTID_UINT8_VECTOR_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t size;
    size_t capacity;
    uint8_t *data;
} UInt8Vector;

void uint8_vector_alloc(UInt8Vector *v, size_t capacity);
void uint8_vector_free(UInt8Vector *v);
void uint8_vector_realloc(UInt8Vector *v, size_t capacity);
void uint8_vector_push_back(UInt8Vector *v, uint8_t value);

#endif
