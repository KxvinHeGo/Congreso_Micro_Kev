#include "pb_common.h"

bool pb_encode_varint(pb_ostream_t *stream, uint64_t value) {
    uint8_t buffer[10];
    size_t i = 0;
    if (value == 0) buffer[i++] = 0;
    while (value > 0) {
        buffer[i++] = (uint8_t)((value & 0x7F) | (value > 0x7F ? 0x80 : 0));
        value >>= 7;
    }
    return pb_write(stream, buffer, i);
}

bool pb_write(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
    if (stream->callback) {
        if (!stream->callback(stream, buf, count)) return false;
    } else {
        if (stream->bytes_written + count > stream->max_size) return false;
        memcpy((uint8_t*)stream->state + stream->bytes_written, buf, count);
    }
    stream->bytes_written += count;
    return true;
}