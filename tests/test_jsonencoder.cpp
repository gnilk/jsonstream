#include "encoding.h"
#include "jsonencoder.h"
#include "jsondecoder.h"

#include <string>
#include "testinterface.h"

using namespace gnilk;


class Memfile : public IWriter {
public:
    int32_t Write(const uint8_t *buffer, size_t szBuffer) override {
        for(size_t i=0;i<szBuffer;i++) {
            data.push_back(buffer[i]);
        }
        return (int32_t)szBuffer;
    }
    std::string String() {
        return std::string((char *)data.data());
    }
private:
    std::vector<uint8_t> data;
};


extern "C" {
    DLL_EXPORT int test_encode(ITesting *t);
    DLL_EXPORT int test_encode_intarray(ITesting *t);
}

int test_encode_intarray(ITesting *t) {
    Memfile mf;
    JSONEncoder encoder(&mf);
    encoder.BeginObject("");
    encoder.BeginArray("ArrayTest", kArraySameType);
    for(int i=0;i<10;i++) {
        encoder.WriteIntField("", i);
    }
    encoder.EndArray();
    encoder.EndObject();

    printf("Data: %s\n", mf.String().c_str());
    return kTR_Pass;
}


static void preCaseCB(ITesting *t) {
//    MemTrackerReset();
}
static void postCaseCB(ITesting *t) {
    printf("------ mem stats for test case\n");
//    MemTrackerDumpStats();
}


int test_encode(ITesting *t) {
    // Link up Pre/Post callbacks to track memory usage on a per-test-case level..
    t->SetPreCaseCallback(preCaseCB);
    t->SetPostCaseCallback(postCaseCB);

//    MemTrackerEnable();
    return kTR_Pass;
}

