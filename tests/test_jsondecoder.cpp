#include "encoding.h"
#include "jsonencoder.h"
#include "jsondecoder.h"
#include <sys/stat.h>

#include <string>
#include "testinterface.h"
#include "Memfile.h"
//#define __MEMFILE_STD__



using namespace gnilk;





//
// NOTE: The test_decode_json_full also tests full marshalling with the JSON Encoder
//


//
// Test exports for test runner
//
extern "C" {
    // New decoder
    DLL_EXPORT int test_decode(ITesting *t);
    DLL_EXPORT int test_decode_json_basic(ITesting *t);
    DLL_EXPORT int test_decode_json_ident(ITesting *t);
    DLL_EXPORT int test_decode_json_emptystring(ITesting *t);

}

int test_decode_json_basic(ITesting *t) {
    std::string data("{{}}");

    Memfile ss(data);
    printf("Processing buffer: '%s'\n", (char *)ss.Buffer());
    JSONDecoder jsonDecoder(&ss, nullptr);
    // This should fail!!!
    if (!jsonDecoder.ProcessData()) {
        return kTR_Pass;
    }
    return kTR_Fail;
}

int test_decode_json_ident(ITesting *t) {
    std::string data("{\"field\":1}");

    Memfile ss(data);
    printf("Processing buffer: '%s'\n", (char *)ss.Buffer());
    JSONDecoder jsonDecoder(&ss, nullptr);
    if (jsonDecoder.ProcessData()) {
        return kTR_Pass;
    }
    return kTR_Fail;
}


int test_decode_json_cplxident(ITesting *t) {
    std::string data("{\"field_a\":1, \"field_b\":\"a string\", \"subobject\":{\"so_fa\":2,\"so_fb\":3}}  ");

    Memfile ss(data);
    printf("Processing buffer: '%s'\n", (char *)ss.Buffer());
    JSONDecoder jsonDecoder(&ss, nullptr);
    if (jsonDecoder.ProcessData()) {
        return kTR_Pass;
    }
    return kTR_Fail;
}


typedef std::vector<std::pair<std::string, std::string> > JSONFIELDS;

int test_decode_json_emptystring(ITesting *t) {
    std::string data("{\n\"Config\":{\n\"String\":\"\",\n\"number\":123,\n\"String2\":\"\"\n}\n}\n");

    Memfile ss(data);
    printf("Processing buffer: %s\n", (char *)ss.Buffer());
    JSONDecoder jsonDecoder(&ss, nullptr);
    if (!jsonDecoder.ProcessData()) {
        return kTR_Fail;
    }
    printf("Checking fields\n");

    const std::vector<DecoderObject *> &objects = jsonDecoder.Objects();
    for (int i=0;i<objects.size();i++) {
        DecoderObject *pObject = objects[i];
        const JSONFIELDS &fields = pObject->Fields();
        for(int f=0;f<fields.size();f++) {
            printf("  %d: %s = '%s'\n", f, fields[f].first.c_str(), fields[f].second.c_str());
        }
    }

    return kTR_Pass;
}


static void preCaseCB(ITesting *t) {
//    MemTrackerReset();
}
static void postCaseCB(ITesting *t) {
    printf("------ mem stats for test case\n");
//    MemTrackerDumpStats();
}


int test_decode(ITesting *t) {
    // Link up Pre/Post callbacks to track memory usage on a per-test-case level..
    t->SetPreCaseCallback(preCaseCB);
    t->SetPostCaseCallback(postCaseCB);

    auto ptr = new int[400];
//    MemTrackerEnable();
    return kTR_Pass;
}
