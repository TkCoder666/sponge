#include "wrapping_integers.hh"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <sys/types.h>
#include "iostream"
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    //!here is with some problems,do you know
    uint32_t isn_value =  isn.raw_value();
    return WrappingInt32{static_cast<uint32_t>((isn_value+n))};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.


uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // cout<<"pass here"<<endl;
    WrappingInt32 checkpoint_32 = wrap(checkpoint, isn);
    // cout<< "checkpoint is "<<checkpoint<<" " <<"checkpoint_32 is :" << checkpoint_32.raw_value()<<endl;
    // cout << "n is "<< n<<endl ;
    //!there are two direction distance,do you know,fuck
    int64_t dis1 = 0, dis2= 0;
    //dis1 is minus,dis2 is add
    if (checkpoint_32.raw_value() > n.raw_value())
    {
        dis1 = checkpoint_32.raw_value()  - n.raw_value(); 
        dis2 = (UINT32_MAX - checkpoint_32.raw_value()) + n.raw_value() + 1;//!+1 is must,draw a cycle,do you know
    }else {
        dis1 = (UINT32_MAX - n.raw_value()) + checkpoint_32.raw_value() + 1;
        dis2 = n.raw_value() - checkpoint_32.raw_value();
    }
    
    //!in uint64_t it's not possible to overflow,fuck
    uint64_t ans = checkpoint - dis1;
    if (dis1 < dis2 && ans < checkpoint) return ans; 
    else return checkpoint + dis2;
}
