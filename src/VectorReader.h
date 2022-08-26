//
// See VectorReader.cpp
//

#ifndef PUCKO_VECTORREADER_H
#define PUCKO_VECTORREADER_H

#include <stdint.h>
#include <vector>
#include "Encoding.h"

namespace gnilk {
    // Put these somewhere...
    class StdVectorReader : public IReader {
    public:
        StdVectorReader(std::vector <uint8_t> &_data);

        virtual int Read(uint8_t *buffer, size_t nread);
        virtual int Peek();
        virtual bool Available();

    private:
        size_t ofs;
        std::vector <uint8_t> &data;
    };

}

#endif //PUCKO_VECTORREADER_H
