#include "uint8_vector.h"
#include "acoustid.h"

void uint8_vector_alloc(UInt8Vector *v, size_t capacity) {
    v->data = palloc(capacity * sizeof(uint8_t));
    v->size = 0;
    v->capacity = capacity;
}

void uint8_vector_free(UInt8Vector *v) {
    pfree(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

void uint8_vector_realloc(UInt8Vector *v, size_t capacity) {
    v->data = repalloc(v->data, capacity * sizeof(uint8_t));
    v->capacity = capacity;
}

void uint8_vector_push_back(UInt8Vector *v, uint8_t x) {
    if (v->size == v->capacity) {
        uint8_vector_realloc(v, v->capacity * 2);
    }
    v->data[v->size++] = x;
}
