//
// Created by gnilk on 3/1/2021.
//

#ifndef GNILK_STRINGWRITER_H
#define GNILK_STRINGWRITER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <string>


#include "encoding.h"


namespace gnilk {
    class StringWriter {
    public:
        StringWriter(IWriter *_writer = nullptr);
        void Begin(IWriter *_writer);
        IWriter *Writer();

        int printf(const char *format, ...);
        int println(const char *format, ...);
        size_t WriteFormat(const char *format, ...);
        size_t WriteLine(const char *format, ...);

        __inline int Write(const float value) { return(printf("%f", value)); }
        __inline int Write(const double value) { return(printf("%f",value)); }
        // stdint
        __inline int Write(const char value) { return(printf("%c", value)); }
        __inline int Write(const int8_t value) { return(printf("%d", value)); }
        __inline int Write(const uint8_t value) { return(printf("%u", value)); }
        __inline int Write(const int16_t value) { return(printf("%d", value)); }
        __inline int Write(const uint16_t value) { return(printf("%u", value)); }
        __inline int Write(const int32_t value) { return(printf("%d", value)); }
        __inline int Write(const uint32_t value) { return(printf("%u", value)); }
        __inline int Write(const char *str, size_t len) { return writer->Write((uint8_t *)str, len); }

        __inline int Write(const std::string &value) {
            return(Write(value.c_str(), value.length()));
        }
        inline gnilk::StringWriter& operator<<(const std::string &str) {
            Write(str);
            return *this;
        }

#define SW_OP_WRITE(__T__) inline gnilk::StringWriter& operator<<(const __T__ value) { Write(value); return *this; }
        SW_OP_WRITE(float);
        SW_OP_WRITE(double);
        SW_OP_WRITE(int8_t);
        SW_OP_WRITE(uint8_t);
        SW_OP_WRITE(int16_t);
        SW_OP_WRITE(uint16_t);
        SW_OP_WRITE(int32_t);
        SW_OP_WRITE(uint32_t);
#undef SW_OP_WRITE

        inline gnilk::StringWriter& operator<<(const char *cstr) {
            //assert(writer != nullptr);

            writer->Write((uint8_t *)cstr, strlen(cstr));
            return *this;
        }

        static const std::string endl;
        static const std::string eol;
        static const std::string eos;

    protected:
        int DoPrint(int flags, const char *format, va_list &values);

    private:
        IWriter *writer;
    };
}


#endif //GNILK_STRINGWRITER_H
