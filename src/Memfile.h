//
// Created by gnilk on 4/29/22.
//

#ifndef JSON_MEMFILE_H
#define JSON_MEMFILE_H

#include <vector>
#include <string>
#include <cstdint>
#include "Encoding.h"
namespace gnilk {
    // NOTE: DOES NOT SUPPORT SEEK!!!!
    class Memfile : public IReader, public IWriter {
    public:
        Memfile(const std::string &input) {
            // Lousy...
            for (auto ch : input) {
                data.push_back(ch);
            }
        }

        int32_t Write(const uint8_t *buffer, size_t szBuffer) override {
            for (size_t i = 0; i < szBuffer; i++) {
                data.push_back(buffer[i]);
            }
            return (int32_t) szBuffer;
        }

        // Should return the number of bytes read or a negative number on error
        int Read(uint8_t *buffer, size_t szData) override {
            if (rpos >= data.size()) {
                return 0;
            }

            size_t nRead = 0;
            while (nRead < szData) {
                buffer[nRead] = data[rpos];
                nRead++;
                rpos++;
                if (rpos >= data.size()) {
                    break;
                }
            }
            return nRead;
        }

        // Returns the next byte available or negative on error
        int Peek() {
            return data[rpos];
        }

        // Returns true if data is available otherwise false (also if not supported/implemented)
        bool Available() override {
            return rpos < data.size();
        }


        std::string String() {
            return std::string((char *) data.data());
        }

        uint8_t *Buffer() {
            return data.data();
        }

    private:
        size_t rpos = 0;
        std::vector<uint8_t> data;
    };

}


#endif //JSON_MEMFILE_H
