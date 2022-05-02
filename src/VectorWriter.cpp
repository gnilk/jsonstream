//
// IWriter interface implementation for std::vector array's
// Connect VectorReader/VectorWriter to one vector to simulate reading/writing using the underlying std::vector
// as the bit-pipe (instead of a network interface, serial, file, or what ever) - good for debugging..
//

#include <cstdint>
#include <vector>
#include "VectorWriter.h"

using namespace gnilk;

StdVectorWriter::StdVectorWriter(std::vector<uint8_t> &_data) :
        data(_data) {

}
int32_t StdVectorWriter::Write(const uint8_t *buffer, size_t szdata) {
    // Use move schemantics or something...
    //printf("StdVectorWriter::Write, this: 0x%p, capacity: %d, size: %d, new data size: %d\n", this, data.capacity(), data.size(), szdata);
    for(size_t i=0;i<szdata;i++) {
        data.push_back(buffer[i]);
    }
    //printf("StdVectorWriter::Write, done\n");
    return szdata;
}
int32_t StdVectorWriter::Write(uint8_t byte) {
    data.push_back(byte);
    return 0;
}

