//
// Created by gnilk on 4/29/22.
//

#ifndef JSON_JSONDECODERNEW_H
#define JSON_JSONDECODERNEW_H

#include <functional>
#include <vector>
#include <string>
#include "encoding.h"



namespace gnilk {
    class JSONDecoderNew {
    public:
        using ValueDelegate = std::function<void(const char *object, const char *name, const char *value)>;
    public:
        JSONDecoderNew(IReader *pStream, IUnmarshal *pRootObject);
        virtual ~JSONDecoderNew();

        void SetValueDelegate(ValueDelegate valueDelegate) { cbValue = valueDelegate; }
        bool ProcessData(bool relax = false);
    private:
        void OnValue();
    private:
        typedef enum : int32_t {
            kConsume,       // 0
            kObjectStart,   // 1
            kLabel,         // 2
            kNameValueSep,  // 3
            kValueStart,    // 4
            kValueString,   // 5
            kValue,         // 6
            kEndOrNext,     // 7
            kEndArrayOrNext,     // 8
        } kState;
    private:
        bool NewObjectFromLabel(const char *label);
        bool PushObject(IUnmarshal *pObject);
        void PushArrayState(bool newArrayState);
        bool PopArrayState();
        IUnmarshal *PopObject();
        int Next();
        int Peek();
        bool Error(int chCurrent);
        void ChangeState(kState newState);
    private:
        IReader *inStream;
        kState state = kConsume;
        bool inArray = false;

        ValueDelegate cbValue = nullptr;
        IUnmarshal *pPreviousObject = nullptr;
        IUnmarshal *pCurrentObject;
        int idxParser;

        // Increase these two if you run into problems of strings too large or nested data too big...
        static const size_t szLabel = 64;
        static const size_t szObjectStack = 8;

        int idxObjectStack = 0;
        IUnmarshal *objectStack[szObjectStack];

        int idxInArrayState = 0;
        bool inArrayStateStack[szObjectStack];

        char objectCurrent[szLabel];

        int idxLabelCurrent;
        char labelCurrent[szLabel];

        int idxValueCurrent;
        char valueCurrent[szLabel];

    };

}


#endif //JSON_JSONDECODERNEW_H
