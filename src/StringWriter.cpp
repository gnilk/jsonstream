//
// Created by gnilk on 3/1/2021.
//

#include <assert.h>
#include "encoding.h"
#include "StringWriter.h"
//#include <assert.h>

using namespace gnilk;

#define SW_FLAG_NONE 0x00
#define SW_FLAG_APPEND_NL 0x01

// Static in class
// TODO: Define for special platforms if needed...
#ifdef WIN32
const std::string gnilk::StringWriter::endl = std::string("\r\n");
const std::string gnilk::StringWriter::eol = std::string("\r\n");
const std::string gnilk::StringWriter::eos = std::string("\0");
#else
const std::string gnilk::StringWriter::endl = std::string("\n");
const std::string gnilk::StringWriter::eol = std::string("\n");
const std::string gnilk::StringWriter::eos = std::string("\0");
#endif



StringWriter::StringWriter(IWriter *_writer) : writer(_writer) {

}

void StringWriter::Begin(IWriter *_writer) {
    this->writer = _writer;
}

IWriter *StringWriter::Writer() {
    return this->writer;
}

int StringWriter::DoPrint(int flags, const char *format, va_list &values) {
    static char newstr[256];        // You should replace this if your platform has constrained memory

    assert(writer != nullptr);

    vsnprintf(newstr, 256, format, values);
    va_end(values);
    int res = writer->Write((uint8_t *)newstr, strlen(newstr));
    if (flags & SW_FLAG_APPEND_NL) {
        res += Write(endl);
    }
    return res;
}

int StringWriter::printf(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    return DoPrint(SW_FLAG_NONE, format, values);
}

int StringWriter::println(const char *format,...) {
    va_list	values;
    va_start( values, format );
    return DoPrint(SW_FLAG_APPEND_NL, format, values);
}

size_t StringWriter::WriteFormat(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    return DoPrint(SW_FLAG_NONE, format, values);
}

size_t StringWriter::WriteLine(const char *format, ...) {
    va_list	values;
    va_start( values, format );
    return DoPrint(SW_FLAG_APPEND_NL, format, values);
}

