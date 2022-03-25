#include "byte_stream.hh"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):_capacity(capacity) {  }

size_t ByteStream::write(const string &data) {
    size_t size = min(data.size(),remaining_capacity());
    for (size_t i=0;i<size;++i){
        _buffer.push_back(data[i]);
    }
    _write_count += size;
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string ans = "";
    size_t size = min(buffer_size(),len);
    auto it = _buffer.begin();
    for (size_t i=0;i<size;++i){
        ans += *it;
        it ++;
    }
    return ans;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t size = min(buffer_size(),len);
    for (size_t i=0;i<size;++i){
        _buffer.pop_front();
    }
    _read_count += size;
 }

void ByteStream::end_input() {
    _input_ended_flag = true;
}  

bool ByteStream::input_ended() const { return _input_ended_flag; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.size() == 0; }

bool ByteStream::eof() const { 
    return input_ended() && buffer_empty();   
}

size_t ByteStream::bytes_written() const { 
    return _write_count;
 }

size_t ByteStream::bytes_read() const {
    return _read_count;
 }

size_t ByteStream::remaining_capacity() const { 
    return _capacity - _buffer.size();
 }
