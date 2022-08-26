//
// Created by gnilk on 4/29/22.
//
#include "Memfile.h"
#include "Encoding.h"
#include "JSONDecoder.h"
#include <sys/stat.h>

#include <string>
#include "testinterface.h"

//#define __MEMFILE_STD__



using namespace gnilk;


extern "C" {
// New decoder
DLL_EXPORT int test_jsdecode(ITesting *t);
DLL_EXPORT int test_jsdecode_json_basic(ITesting *t);
DLL_EXPORT int test_jsdecode_intarray(ITesting *t);
DLL_EXPORT int test_jsdecode_objarray(ITesting *t);
DLL_EXPORT int test_jsdecode_callback(ITesting *t);
DLL_EXPORT int test_jsdecode_callback2(ITesting *t);
}

DLL_EXPORT int test_jsdecode(ITesting *t) {
    return kTR_Pass;
}
DLL_EXPORT int test_jsdecode_json_basic(ITesting *t) {
    std::string basic = "{ \"number\" : 10, \"obj\" : { \"str\" : \"this is a string\" } }";
    Memfile mf(basic);
    JSONDecoder jsNew(&mf,nullptr);
    jsNew.SetValueDelegate([](const char *object, const char *label, const char *value) {
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }

    });
    auto res = jsNew.ProcessData();
    TR_ASSERT(t, res == true);
    return kTR_Pass;
}

class MyIntArray : public IUnmarshal {
public:
    void Dump() {
        for(auto i : array) {
            printf("i: %d\n", i);
        }
    }
    // Called to set a field/variable in your class
    bool SetField(std::string &name, std::string &value) override;
    // Called to get an instance to a sub-object in your class
    IUnmarshal *GetUnmarshalForField(std::string &name) override;
    // Called to push things to an array you might have
    bool PushToArray(std::string &name, IUnmarshal *pData) override;

private:
    std::vector<int> array;
};
bool MyIntArray::SetField(std::string &name, std::string &value) {
    int v = atoi(value.c_str());
    array.push_back(v);
    printf("Push: %d to field: %s\n", v, name.c_str());
    return true;
}
IUnmarshal *MyIntArray::GetUnmarshalForField(std::string &name) {
    printf("MyIntArray::GetUnmarshalForField, %s\n", name.c_str());
    if (name == "Array") {
        return this;
    }
    return nullptr;
}
bool MyIntArray::PushToArray(std::string &name, IUnmarshal *pData) {
    return true;
}

DLL_EXPORT int test_jsdecode_intarray(ITesting *t) {
    std::string data = "{ \"Array\" : [1,2,3,4,5] }";

    MyIntArray myArray;
    Memfile mf(data);
    JSONDecoder jsNew(&mf,&myArray);

    jsNew.SetValueDelegate([](const char *object, const char *label, const char *value) {
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }

    });
    jsNew.ProcessData();

    myArray.Dump();

    return kTR_Pass;
}

class SubObject : public IUnmarshal {
public:
    void Dump() {
        printf("  Num: %d, Str: %s\n", numValue, stringValue.c_str());
    }
    bool SetField(std::string &name, std::string &value) override;
    // Called to get an instance to a sub-object in your class
    IUnmarshal *GetUnmarshalForField(std::string &name) override {  return nullptr; }
    // Called to push things to an array you might have
    bool PushToArray(std::string &name, IUnmarshal *pData) override {  return false; }
private:
    int numValue;
    std::string stringValue;
};

bool SubObject::SetField(std::string &name, std::string &value) {
    //printf("SubObject::SetField, %s:%s\n", name.c_str(), value.c_str());
    if (name == "Num") {
        numValue = atoi(value.c_str());
        return true;
    }
    if (name == "Str") {
        stringValue = std::string(value);
        return true;
    }
    return false;
}

class MyObjArray : public IUnmarshal {
public:
    void Dump() {
        printf("Array has: %d items\n", (int)array.size());
        for (auto p : array) {
            p->Dump();
        }
    }
    // Called to set a field/variable in your class
    bool SetField(std::string &name, std::string &value) override;
    // Called to get an instance to a sub-object in your class
    IUnmarshal *GetUnmarshalForField(std::string &name) override;
    // Called to push things to an array you might have
    bool PushToArray(std::string &name, IUnmarshal *pData) override;

private:
    std::vector<SubObject *> array;
};
bool MyObjArray::SetField(std::string &name, std::string &value) {
    return false;
}
IUnmarshal *MyObjArray::GetUnmarshalForField(std::string &name) {
    //printf("MyObjArray::GetUnmarshalForField, %s\n", name.c_str());
    if (name == "List") {
        return new SubObject();
    }
    return nullptr;
}
bool MyObjArray::PushToArray(std::string &name, IUnmarshal *pData) {
    if (name == "List") {
        //printf("Push to array!\n");
        array.push_back(static_cast<SubObject *>(pData));
        return true;
    }
    return true;
}

//
// Array of objects..
//
DLL_EXPORT int test_jsdecode_objarray(ITesting *t) {
    std::string data = "{ \"List\" : [{ \"Num\" : 2, \"Str\" : \"Hello\" }, { \"Str\" : \"World\", \"Num\" : 4 }] }";

    MyObjArray myArray;
    Memfile mf(data);
    JSONDecoder jsNew(&mf,&myArray);

    // Consider callback as an interface:
    // - Begin/End-Object as: void *BeginObject(name), EndObject(name, void *)
    // - Begin/End-Array  as: void *BeginArray(name), EndArray(name, void *)
    // - SetValue(label, value, void *);

    // Perhaps need a few more callback: BeginObject, EndObject, BeginArray, EndArray
    jsNew.SetValueDelegate([](const char *object, const char *label, const char *value) {
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }

    });
    jsNew.ProcessData();
    printf("Done, dump array\n");

    myArray.Dump();

    return kTR_Pass;
}

DLL_EXPORT int test_jsdecode_callback(ITesting *t) {
    std::string basic = "{ \"number\" : 10, \"obj\" : { \"str\" : \"this is a string\" } }";
    Memfile mf(basic);
    auto callback = [](const char *object, const char *label, const char *value) {
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }
    };

    JSONDecoder jsNew(&mf,callback);
    auto res = jsNew.ProcessData();
    TR_ASSERT(t, res == true);
    return kTR_Pass;
}

DLL_EXPORT int test_jsdecode_callback2(ITesting *t) {
    std::string data = "{ \"MyList\" : [{ \"Num\" : 2, \"Str\" : \"Hello\" }, { \"Str\" : \"World\", \"Num\" : 4 }] }";

    Memfile mf(data);
    auto callback = [](const char *object, const char *label, const char *value) {
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }
    };

    JSONDecoder jsNew(&mf,callback);
    auto res = jsNew.ProcessData();
    TR_ASSERT(t, res == true);
    return kTR_Pass;
}
