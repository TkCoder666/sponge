#include "tcp_sender.hh"

#include "buffer.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

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
    TCPSegment seg;
    string pay_load;
    uint16_t bytes_for_sending = min(_window_size,static_cast<uint16_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    // bytes_for_sending = max(bytes_for_sending,static_cast<uint16_t>(1));//TODO:deal window as 1 if it's 0
    
    if (next_seqno_absolute() == 0) {
        seg.header().syn = true;
        pay_load = _stream.read(bytes_for_sending-1); //!consider syn here
    }else {
        pay_load = _stream.read(bytes_for_sending);    
    }

    seg.header().seqno = wrap(next_seqno_absolute(), _isn);

    // Buffer buffer(std::move(pay_load)); //!error how to init?

    seg.payload() = Buffer(std::move(pay_load));

    if (_stream.eof() && seg.length_in_sequence_space() < bytes_for_sending){
        
        seg.header().fin = true;
    } 

    cout << "the length is "<<seg.length_in_sequence_space()<<endl;
    if (seg.length_in_sequence_space() == 0) 
        return; 

    _next_seqno += seg.length_in_sequence_space();
    _bytes_in_flight += seg.length_in_sequence_space();
    _segments_out.push(seg);
    _outstanding_segs.push(seg);
    _window_size -= seg.length_in_sequence_space();
    //TODO:set time with some flag here,do you know
    _send_ms = 0;
    _timer_running = true;

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    _ackno = ackno.raw_value();
    _window_size = window_size;
    //!here is to pop out outstanding data,do you know
    //!ackno next 
    //?acknowledges the successful receipt of new data
    bool ack_new_data = false;


    while (_outstanding_segs.front().header().seqno.raw_value()+_outstanding_segs.front().length_in_sequence_space() <= ackno.raw_value())
    {   
        ack_new_data = true;

        _bytes_in_flight -= _outstanding_segs.front().length_in_sequence_space();//!nor right

        _outstanding_segs.pop();
        _rto = _initial_retransmission_timeout;
        if (_outstanding_segs.empty()) break;
    }
    
    if (!_outstanding_segs.empty() && ackno.raw_value() > _outstanding_segs.front().header().seqno.raw_value())
    {
        _bytes_in_flight -= ackno.raw_value()-_outstanding_segs.front().header().seqno.raw_value();
    }

    if (_outstanding_segs.empty() && ack_new_data) {
        _timer_running = false;
        _send_ms = 0;
        return;
    }
    if (ack_new_data){
        _timer_running = true;
        _consecutive_retransmissions_nums = 0;
    }
 }

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer_running)  return;
    _send_ms += ms_since_last_tick;
    //TODO: resend here
    if (_send_ms > _rto)
    {
        _segments_out.push(_outstanding_segs.front());
        if (_window_size > 0)
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
    seg.header().seqno = wrap(_next_seqno,_isn); //TODO:how to set seqno here
    _segments_out.push(seg);
}
