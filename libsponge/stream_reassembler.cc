#include "stream_reassembler.hh"
#include "byte_stream.hh"
#include <cstddef>
#include <cstdint>
#include <utility>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;


//!pair or set,which do you like?

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    //!note the eof here 
    uint64_t data_index = 0;


    for (;data_index < data.size(); data_index++) //push something out,do you know
        //!not inserted here may be?
    {
        uint64_t byte_index = data_index+index;
        if (byte_index < _index_needed || _buffer.count(byte_index) > 0) //!note the condition here 
        {
            continue;
        }else
        {
            _buffer.insert(pair<uint64_t,char>(byte_index, data[data_index]));
        }
    }

    
    if (data_index == data.size() && eof) _eof = true; 
    
    auto it = _buffer.begin();

    string bytes_for_write = "";

    uint64_t tmp_index = _index_needed;
    while (it != _buffer.end() && it->first == tmp_index)
    {
        bytes_for_write += it->second;
        it = _buffer.erase(it); 
        tmp_index++; 
    }

    size_t before_write = _output.bytes_written();
    _output.write(bytes_for_write);
    size_t after_write = _output.bytes_written();
    _index_needed += after_write - before_write;

    if (_output.remaining_capacity() == 0)
    {
        _buffer.clear();
    } 
    if (_eof)
    {
        if (tmp_index > _index_needed) _eof = false;
    }

    if (_eof && empty()){
        _output.end_input();
    }

}

size_t StreamReassembler::unassembled_bytes() const { 
    return _buffer.size();
 }

bool StreamReassembler::empty() const { 
    return unassembled_bytes() == 0;
 }
size_t StreamReassembler::remaining_size() const{
    return _capacity - _buffer.size() - _output.buffer_size();
}