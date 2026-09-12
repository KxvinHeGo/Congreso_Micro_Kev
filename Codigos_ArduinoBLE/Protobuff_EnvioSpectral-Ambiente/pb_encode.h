/* Funciones de codificación para Nanopb */
#ifndef PB_ENCODE_H_INCLUDED
#define PB_ENCODE_H_INCLUDED
#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

pb_ostream_t pb_ostream_from_buffer(uint8_t *buf, size_t bufsize);
bool pb_encode(pb_ostream_t *stream, const pb_field_t fields[], const void *src_struct);
bool pb_encode_tag_for_field(pb_ostream_t *stream, const pb_field_t *field);
bool pb_encode_string(pb_ostream_t *stream, const uint8_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif