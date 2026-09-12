#include "pb_encode.h"
#include "pb_common.h"

static bool buf_write(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
    uint8_t *dest = (uint8_t*)stream->state;
    memcpy(&dest[stream->bytes_written], buf, count);
    return true;
}

pb_ostream_t pb_ostream_from_buffer(uint8_t *buf, size_t bufsize) {
    pb_ostream_t stream = {&buf_write, (void*)buf, bufsize, 0};
    return stream;
}

bool pb_encode(pb_ostream_t *stream, const pb_field_t fields[], const void *src_struct) {
    const pb_field_t *field = fields;
    while (field->tag != 0) {
        const void *pData = (const char*)src_struct + field->data_offset;
        if (!pb_encode_tag_for_field(stream, field)) return false;

        switch (field->type) {
            case PB_LTYPE_VARINT: 
                if (!pb_encode_varint(stream, *(const uint32_t*)pData)) return false; 
                break;
            case PB_LTYPE_FIXED32: 
                if (!pb_write(stream, (const uint8_t*)pData, 4)) return false; 
                break;
            case PB_LTYPE_STRING: {
                size_t size = strlen((const char*)pData);
                if (!pb_encode_varint(stream, (uint64_t)size)) return false;
                if (!pb_write(stream, (const uint8_t*)pData, size)) return false;
                break;
            }
        }
        field++;
    }
    return true;
}

bool pb_encode_tag_for_field(pb_ostream_t *stream, const pb_field_t *field) {
    uint32_t wire_type = (field->type == PB_LTYPE_FIXED32) ? 5 : 
                         (field->type == PB_LTYPE_STRING) ? 2 : 0;
    return pb_encode_varint(stream, (uint32_t)((field->tag << 3) | wire_type));
}