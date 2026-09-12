/* Soporte básico para Nanopb */
#ifndef PB_H_INCLUDED
#define PB_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define PB_PROTO_HEADER_VERSION 30
#define PB_HTYPE_REQUIRED 0x00
#define PB_HTYPE_OPTIONAL 0x10
#define PB_HTYPE_REPEATED 0x20
#define PB_LTYPE_VARINT  0x00
#define PB_LTYPE_FIXED32 0x02
#define PB_LTYPE_FIXED64 0x03
#define PB_LTYPE_BYTES   0x04
#define PB_LTYPE_STRING  0x05
#define PB_LTYPE_SUBMESSAGE 0x06

typedef struct pb_field_s pb_field_t;
struct pb_field_s {
    uint32_t tag;
    uint8_t  type;
    uint8_t  data_offset;
    int8_t   size_offset;
    uint8_t  data_size;
    uint8_t  array_size;
    const void *ptr;
} __attribute__((packed));

typedef struct pb_ostream_s pb_ostream_t;
struct pb_ostream_s {
    bool (*callback)(pb_ostream_t *stream, const uint8_t *buf, size_t count);
    void *state;
    size_t max_size;
    size_t bytes_written;
};

#endif