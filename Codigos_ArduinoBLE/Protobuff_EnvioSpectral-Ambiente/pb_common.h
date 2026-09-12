/* Funciones comunes para Nanopb */
#ifndef PB_COMMON_H_INCLUDED
#define PB_COMMON_H_INCLUDED
#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

bool pb_encode_varint(pb_ostream_t *stream, uint64_t value);
bool pb_write(pb_ostream_t *stream, const uint8_t *buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif