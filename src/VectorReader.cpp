//
// IReader interface implementation for std::vector
// Connect VectorReader/VectorWriter to one vector to simulate reading/writing using the underlying std::vector
// as the bit-pipe (instead of a network interface, serial, file, or what ever) - good for debugging..
//
#include <vector>
#include <cstdint>
#include "VectorReader.h"

using namespace gnilk;

// Implementation
StdVectorReader::StdVectorReader(std::vector<uint8_t> &_data) :
        ofs(0),
        data(_data) {

}
int StdVectorReader::Read(uint8_t *buffer, size_t nread) {
    // EOF
    if ((ofs + nread) >= data.size()) {
        nread = data.size() - ofs;
    }
    for (size_t i=0;i<nread;i++) {
        buffer[i] = data[ofs];
        ofs++;
    }

    return nread;
}
int StdVectorReader::Peek() {
    if (ofs >= data.size()) {
        return -1;
    }

    return data[ofs];
}
bool StdVectorReader::Available() {
    if (ofs >= data.size()) {
        return false;
    }
    return true;
}
