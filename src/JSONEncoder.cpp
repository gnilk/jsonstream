/*-------------------------------------------------------------------------
File    : jsonencoder.cpp
Author  : gnilk
Version : $Revision: 1 $
Orginal : 2018-10-29
Descr   : Implements JSON encoding for classes implementing the IMarshal interface

---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 22.02.02, gnilk, Refactored so can be reentrant
- 19.06.27, gnilk, Added state handling, tracking the writes of ',' - simplifies interface
- 19.06.27, gnilk, Enhanced WriteField to support native types and not just objects
- 18.10.30, gnilk, Implementation

---------------------------------------------------------------------------*/
#include <string>
#include <stdint.h>
#include <stdio.h>

#include "Encoding.h"
#include "JSONEncoder.h"


using namespace gnilk;


//
// Marshalling
//
static const std::string beginobj("{");
static const std::string endobj("}");
static const std::string beginarray("[");
static const std::string endarray("]");
static const std::string quote("\"");
static const std::string separator(":");
static const std::string nextfield(",");
static const std::string jsontrue("true");
static const std::string jsonfalse("false");
static const std::string jsonnull("null");


//
// JSON Marshalling, simplified and doesn't support everything
//
// Details: https://tools.ietf.org/html/rfc4627
//
JSONEncoder::JSONEncoder(IWriter *p_stream) :
    ss(p_stream),
    fieldCount(0) {
}

void JSONEncoder::PrettyPrint(bool use) {
    pretty = use;
    eol = pretty ? StringWriter::eol : "";
}

std::string JSONEncoder::spacing() {
    return pretty ? std::string(fieldStack.size(), '\t') : "";
}

void JSONEncoder::Begin(IWriter *writer) {
    ss.Begin(writer);
    // Clear out any left-overs from previous run...
    fieldStack.clear();
    fieldCount = 0;
}

IWriter *JSONEncoder::Writer() {
    return ss.Writer();
}

// Begin object
void JSONEncoder::BeginObject(std::string name/*=""*/) {
    WriteFieldSeparator();    
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << beginobj << eol;
    } else {
        ss << spacing() << beginobj << eol;
    }
    fieldStack.push_back(fieldCount);
    fieldCount = 0;
}

// end object
void JSONEncoder::EndObject(bool hasNext /*= false*/) {
    fieldCount = fieldStack.back();
    fieldStack.pop_back();

    ss << eol << spacing() << endobj;

    fieldCount += 1;
}

// Start an array=> "data":[
void JSONEncoder::BeginArray(std::string name, kArrayTypeSpec typeSpec) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << beginarray << eol;
    } else {
        ss << spacing() << beginarray << eol;
    }
    fieldStack.push_back(fieldCount);
    fieldCount = 0;
}


// End array, insert array end-marker => ]
void JSONEncoder::EndArray(bool hasNext /* = false */) {
    fieldCount = fieldStack.back();
    fieldStack.pop_back();

    ss << eol << spacing() << endarray << eol;

    fieldCount += 1;
}

// Private - this will check the field count and write a separator if needed..
void JSONEncoder::WriteFieldSeparator() {
    if (fieldStack.empty()) return;

    if (fieldCount > 0) {
        ss << nextfield << eol;
    }

    fieldCount+=1;
}


//
// Write int
//
void JSONEncoder::WriteIntField(std::string name, int value, bool hasNext /* = true */) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << strutil::to_string(value);
    } else {
        ss << spacing() << strutil::to_string(value);
    }
}

//
// Write float 
//
void JSONEncoder::WriteFloatField(std::string name, float value, bool hasNext /* = true */) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << strutil::to_string(value);
    } else {
        ss << spacing() << strutil::to_string(value);
    }
}

//
// Write string
//
void JSONEncoder::WriteTextField(std::string name, std::string value, bool hasNext /* = true */) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << quote << value << quote;
    } else {
        ss << spacing() << quote << value << quote;
    }
}

//
// boolean
//
void JSONEncoder::WriteBoolField(std::string name, bool value, bool hasNext /* = true */) {
    WriteFieldSeparator();
    if (!name.empty()) {
        ss << spacing() << quote << name << quote << separator << (value?jsontrue:jsonfalse);
    } else {
        ss << spacing() << (value?jsontrue:jsonfalse);
    }
}
