#ifndef TYPEDBUFFER_H
#define TYPEDBUFFER_H

#include <ByteBuffer.h>

#include <vector>

#include "glad/gl.h"

namespace OM3D {

template<typename T>
class TypedBuffer : public ByteBuffer {
    public:
        TypedBuffer() = default;

        TypedBuffer(Span<const T> data) : TypedBuffer(data.data(), data.size()) {
        }

        TypedBuffer(const T* data, size_t count) : ByteBuffer(data, count * sizeof(T)) {
        }

        size_t element_count() const {
            DEBUG_ASSERT(byte_size() % sizeof(T) == 0);
            return byte_size() / sizeof(T);
        }

        BufferMapping<T> map(AccessType access = AccessType::ReadWrite) {
            return BufferMapping<T>(ByteBuffer::map_internal(access), byte_size(), handle());
        }

        std::vector<T> get_data() const {
            std::vector<T> data(element_count());
            glGetNamedBufferSubData(handle().get(), 0, byte_size(), data.data());
            return data;
        }
};
}

#endif // TYPEDBUFFER_H
