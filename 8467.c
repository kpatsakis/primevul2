    BufferPartial(const Buffer::Ptr &buffer, size_t offset, size_t size){
        _buffer = buffer;
        _data = buffer->data() + offset;
        _size = size;
    }