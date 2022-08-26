//
// See VectorWriter.cpp
//

#ifndef PUCKO_VECTORWRITER_H
#define PUCKO_VECTORWRITER_H

#include <stdint.h>
#include <vector>
#include "Encoding.h"

namespace gnilk {
    class StdVectorWriter : public IWriter {
    public:
        StdVectorWriter(std::vector <uint8_t> &_data);

        virtual int32_t Write(const uint8_t *buffer, size_t szdata);
        virtual int32_t Write(uint8_t byte);
    private:
        std::vector <uint8_t> &data;
    };
}

#endif //PUCKO_VECTORWRITER_H
