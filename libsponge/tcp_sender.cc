#include "tcp_sender.hh"

#include "buffer.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <algorithm>
#include <cstdint>
#include <random>
#include <utility>
#include <iostream>
// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity), _rto(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { 
    return _bytes_in_flight;
}

void TCPSender::fill_window() {

    uint16_t seg_window = 0;

    if (_new_ack) 
    {
        _window_size = max(_window_size, static_cast<uint16_t>(1));//!fuck here
        _window_size -= wrap(next_seqno_absolute(), _isn).raw_value() - _ackno;
        _new_ack = false;
    }

    seg_window = _window_size;
    while (true)
    {
        if (_finish) return;

        TCPSegment seg;
        string pay_load;
        // uint16_t bytes_for_sending = min(_window_size,static_cast<uint16_t>(TCPConfig::MAX_PAYLOAD_SIZE));
        // uint16_t seg_window_size = max(bytes_for_sending,_window_size);
        // bytes_for_sending = max(bytes_for_sending,static_cast<uint16_t>(1));
        //!we should discuss here
        //!two pay_load_size and window_size-->two 
 
        if (next_seqno_absolute() == 0) {
            seg.header().syn = true;
            // pay_load = _stream.read(bytes_for_sending-1); //!consider syn here
            seg_window--; 
            _ackno = _isn.raw_value();//!init _ackno
        }
        pay_load = _stream.read(min(seg_window,static_cast<uint16_t>(TCPConfig::MAX_PAYLOAD_SIZE)));    
        
        
        seg.header().seqno = wrap(next_seqno_absolute(), _isn);

        // Buffer buffer(std::move(pay_load)); //!error how to init?

        seg.payload() = Buffer(std::move(pay_load));

        if (_stream.eof() && seg.length_in_sequence_space() < seg_window){
            
            seg.header().fin = true;
            _finish = true;
        } 

        // cout << "the length is "<<seg.length_in_sequence_space()<<endl;
        if (seg.length_in_sequence_space() == 0) 
            return; 

        _next_seqno += seg.length_in_sequence_space();
        _bytes_in_flight += seg.length_in_sequence_space();
        _segments_out.push(seg);
        _outstanding_segs.push(seg);

        if (seg.header().syn) seg_window+=1;
        seg_window -= seg.length_in_sequence_space();
        _window_size = seg_window; 

        _timer_running = true;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    
    //!here is to pop out outstanding data,do you know
    //!ackno next 
    //!using a flag to do so,do you know to judge eof
    if (ackno.raw_value() > wrap(next_seqno_absolute(), _isn).raw_value()) return;

    if (ackno.raw_value() < _ackno) return; //!may be repeated numbers but to tell bigger window,do you know
    
    if (_finish) {_fin_acked = true;}

    _bytes_in_flight -= ackno.raw_value() - _ackno;

    bool ack_new_data = false;
    if (ackno.raw_value() - _ackno > 0) ack_new_data = true;

    while (!_outstanding_segs.empty() && _outstanding_segs.front().header().seqno.raw_value()+_outstanding_segs.front().length_in_sequence_space() <= ackno.raw_value())
    {   
        _send_ms = 0;
        _outstanding_segs.pop(); 
        if (_outstanding_segs.empty()) break;
    }
    
    // if (!_outstanding_segs.empty() && ackno.raw_value() > _outstanding_segs.front().header().seqno.raw_value())
    // {
    //     _bytes_in_flight -= ackno.raw_value()-_outstanding_segs.front().header().seqno.raw_value();
    // }
     
    if (ack_new_data){
        _rto = _initial_retransmission_timeout;
        _consecutive_retransmissions_nums = 0;
    }

    _ackno = ackno.raw_value();

    _window_size = window_size;
    
    _ack_window_size = window_size;

    _new_ack = true;

    if (_outstanding_segs.empty()) {
        _timer_running = false;
    }else if (ack_new_data)
    {
        _timer_running = true;
    }    
 }

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer_running)  return;
    _send_ms += ms_since_last_tick;
    if (_send_ms >= _rto)
    {   
        _segments_out.push(_outstanding_segs.front());
        if (_ack_window_size > 0 || _new_ack) //true or false 
        {
            _consecutive_retransmissions_nums++;
            _rto *= 2;
        }
        _send_ms = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { 
    return _consecutive_retransmissions_nums;    
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno,_isn); 
    _segments_out.push(seg);
}
