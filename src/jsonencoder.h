#ifndef PUCKO_JSON_ENCODER_H
#define PUCKO_JSON_ENCODER_H

#include <string>
#include <vector>
#include <stdint.h>

#include "StringWriter.h"

namespace gnilk {
        class JSONEncoder {
        public:
            JSONEncoder() = default;
            JSONEncoder(IWriter *p_stream);
            virtual ~JSONEncoder() {
            }

            IWriter *Writer();
            void PrettyPrint(bool use);

            // This is an optional call to set the writer
            void Begin(IWriter *writer);

            // This is used by the messages
            void BeginObject(std::string name = "");
            void EndObject(bool hasNext = false);

            // Note: 'hasNext' is deprecated, parameter ignored
            void WriteBoolField(std::string name, bool value, bool hasNext = true);
            void WriteIntField(std::string name, int value, bool hasNext = true);
            void WriteFloatField(std::string name, float value, bool hasNext = true);
            void WriteTextField(std::string name, std::string value, bool hasNext = true);

            void BeginArray(std::string name, kArrayTypeSpec typeSpec);
            void EndArray(bool hasNext = false);

        private:
            std::string spacing();
            bool pretty = false;
            std::string eol = "";
            StringWriter ss;
            void WriteFieldSeparator();
            std::vector<int> fieldStack;
            int fieldCount;
        };
}
#endif