#include "tcp_connection.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"

#include <cstdint>
#include <iostream>
#include <limits>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { 
    return _outbound_stream.remaining_capacity();
}

size_t TCPConnection::bytes_in_flight() const {
    return _sender.bytes_in_flight();
}

size_t TCPConnection::unassembled_bytes() const { 
    return _receiver.unassembled_bytes();
}

size_t TCPConnection::time_since_last_segment_received() const { 
    return _time_since_last_segment_received;
}

void TCPConnection::move_seg_from_sender(bool with_rst)
{
    while (!_sender.segments_out().empty()) {

        TCPSegment & seg = _sender.segments_out().front();//!here may be wrong
        _sender.segments_out().pop();

        seg.header().win =min(static_cast<size_t>(UINT16_MAX),_receiver.window_size());

        if (_receiver.ackno().has_value())
        {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }

        if (with_rst) 
        {
            seg.header().rst = true;
            _inbound_stream.set_error();
            _outbound_stream.set_error();
            _active_or_not = false;
        }


        _segments_out.push(seg);
    }
}

void TCPConnection::send_segments(bool empty_seg,bool with_rst)
{   
    if (with_rst || empty_seg)
    {
        if (with_rst) {
            _sender.clear_segments_out();
            clear_segments_out();
        }
        _sender.send_empty_segment();
    }else {
        _sender.fill_window();
    }

    if (_sender.segments_out().empty()){
        _sender.send_empty_segment();
    }

    move_seg_from_sender(with_rst);
}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    _time_since_last_segment_received = 0;

    if (seg.header().rst){ //!has rst
        _inbound_stream.set_error();
        _outbound_stream.set_error();
        _active_or_not = false;
        return;//TODO:how to kill connection-->not active?
    }

    _receiver.segment_received(seg);

    if (_inbound_stream.eof() && !_outbound_stream.eof()){ //TODO:!here may be some problems
        _linger_after_streams_finish = false;
    } 

    //!if the ack flag is set, tells the TCPSender
    if (seg.header().ack)
    {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    //!if the incoming segment occupied any sequence numbers,

    if (seg.length_in_sequence_space() > 0)
    {
        send_segments(); //!send and we must send,fuck
    }


    //!special case 
    if (_receiver.ackno().has_value() and (seg.length_in_sequence_space() == 0)
        and seg.header().seqno == _receiver.ackno().value() - 1) 
    {
        send_segments(true); //TODO:here use this method may be wrong
    }  

}

bool TCPConnection::active() const { 

    // if (!_inbound_stream.eof() || !_outbound_stream.eof() || _linger_after_streams_finish){
    //     return true;
    // }
    // return false;
    return _active_or_not;
}

size_t TCPConnection::write(const string &data) {
    size_t write_size =  _outbound_stream.write(data);
    return write_size;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 

    _time_since_last_segment_received += ms_since_last_tick;

    _sender.tick(ms_since_last_tick);

    if (_outbound_stream.buffer_empty()) move_seg_from_sender(false);
    else send_segments();
    
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) //!here ?
    {   
        //TODO:how to abort the connection?
        _inbound_stream.set_error();
        _outbound_stream.set_error();
        _active_or_not = false;
        send_segments(false,true);
    }
    
    

    if (_sender.fin_acked() && _receiver.receive_finish()){
        if (!_linger_after_streams_finish)
        {
            _active_or_not = false;
        }
        else{
            if (time_since_last_segment_received() >= 10*_cfg.rt_timeout)
            {
                _active_or_not = false;
            }
        }
    }

}

void TCPConnection::end_input_stream() {
    _outbound_stream.end_input();
    send_segments();
}

void TCPConnection::connect() {
    // _sender.fill_window();
    // TCPSegment seg = _sender.segments_out().front();
    // _sender.segments_out().pop();
    // //!seg.header().ackno = _receiver.ackno(); no ackno at very first
    // seg.header().win = _receiver.window_size();
    // _segments_out.push(seg);
    send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            send_segments(false,true);
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
