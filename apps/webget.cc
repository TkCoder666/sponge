#include "address.hh"
#include "socket.hh"
#include "util.hh"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <sys/socket.h>

using namespace std;

void get_URL(const string &host, const string &path) {
    // Your code here.

    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).
    //!request path string do you know
    //! connect host and request path
    TCPSocket sock;
    // cout << "pass" <<1 <<" with host and path "<< host << " " << path << endl;
    Address addr(host,"http");
    // cout << "pass" <<2 << endl;
    sock.connect(Address(host,"http")); //!how to construct a addr for host    

    string end_str = "\r\n";
    string command1 = "GET "+path+" HTTP/1.1"+end_str;
    string command2 = "Host: "+host+end_str+end_str;//!two end_str here, fuck
    sock.write(command1);
    sock.write(command2);
    sock.shutdown(SHUT_WR);
    // cout <<"command 1 :"<<command1 << endl;
    // cout <<"command 2 :"<<command2 << endl;
    // cout << "pass" <<3 << endl;

    while (!sock.eof()) {  
        auto recvd = sock.read();

        // cout << "pass" <<4 << endl;


        // if (recvd == "eof" || recvd == "EOF") break;

        // cout << "pass" <<5 << endl;


        cout << recvd;


        // int ending = -1;
        // int begin = 0;
        // while ((ending = recvd.find(end_str)) != static_cast<int>(string::npos))
        // {
        //     cout << recvd.substr(begin,ending - begin)<<ending;
        //     begin = ending + end_str.size();
        // }
    }
    
    
    sock.close();
    // cerr << "Function called: get_URL(" << host << ", " << path << ").\n";


    
    // cerr << "Warning: get_URL() has not been implemented yet.\n";
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH\n";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
