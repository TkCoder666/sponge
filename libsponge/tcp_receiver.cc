#include "tcp_receiver.hh"
#include "buffer.hh"
#include "tcp_header.hh"
#include "wrapping_integers.hh"
#include <cstdint>
#include <iterator>
#include <ostream>
#include "iostream"
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    if (header.syn) {
        _syn = true;
        _isn = header.seqno.raw_value();
    }

    if (!_syn) return;

    //!debug infor
    // cout <<" seqno: "<<header.seqno <<" syn: "<<header.syn<<" fin: "<<header.fin <<endl;
    // cout << "length_in_sequence_space: " <<seg.length_in_sequence_space() <<endl;


    // uint32_t window_start = ackno()->raw_value();
    // uint32_t window_end = ackno()->raw_value() + window_size(); //1 or something
    // uint32_t seqno_start = header.seqno.raw_value();
    // uint32_t seqno_end = seqno_start + seg.length_in_sequence_space(); 

    // if (seqno_end < window_start || seqno_start > window_end ) return false; //!seqno =0 and window = 0

    Buffer payload =  seg.payload();
    string data = payload.copy();

    uint64_t abs_seqno = unwrap(header.seqno, WrappingInt32(_isn), _checkpoint);
    if (header.syn) abs_seqno++;

    if (abs_seqno == 0) return;
    _reassembler.push_substring(data, abs_seqno - 1,header.fin);
    //!here deal with seg that exceed the window,do you know
    
    
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    //!calculate something,fuck
    if (!_syn) return optional<WrappingInt32>();
    else {
        uint64_t index_needed = _reassembler.head_index(); 
        if (_reassembler.input_ended()) index_needed +=1;
        return wrap(index_needed + 1, WrappingInt32(_isn));
    }

 }

size_t TCPReceiver::window_size() const { 
    return _capacity - _reassembler.stream_out().buffer_size();
 }
