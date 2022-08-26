//
// Very small stream based and callback based JSON parser/decoder
// allocation free...
//

#include "JSONDecoder.h"

using namespace gnilk;

#ifdef DEBUG
static void errx(const char *file, size_t line, const char *msg) {
    printf("ERR: %s @ %d - %s\n",file, line, msg);
    exit(1);
}
#else
// Just here in case we/you want something different in release mode..
static void errx(const char *file, size_t line, const char *msg) {
    printf("ERR: %s @ %d - %s\n",file, (int)line, msg);
    exit(1);
}
#endif

JSONDecoder::JSONDecoder(IReader *pStream, IUnmarshal *pRootObject) : inStream(pStream), pCurrentObject(pRootObject) {

}

JSONDecoder::JSONDecoder(IReader *pStream, ValueDelegate valueDelegate) : inStream(pStream), cbValue(valueDelegate) {

}


// NOTE: Nested object/unmarshalling not tested...
bool JSONDecoder::ProcessData(bool relax /*= false*/) {
    int ch;
    memset(objectCurrent, 0, szLabel);
    PushObject(pCurrentObject);

    idxParser = 0;
    while((ch = Next()) > 0) {
        switch(state) {
            case kConsume :
                if (ch == '{') {
                    PushArrayState(false);
                    ChangeState(kObjectStart);
                } else if (ch == '[') {
                    PushArrayState(true);
                    ChangeState(kValueStart);
                }
                break;
            case kObjectStart :
                if (ch == '\"') {
                    idxLabelCurrent = 0;
                    memset(labelCurrent,0, szLabel);
                    ChangeState(kLabel);
                } else if (!std::isspace(ch)) {
                    return Error(ch);
                }
                break;
            case kLabel :
                if (ch == '\"') {
                    ChangeState(kNameValueSep);
                } else {
                    labelCurrent[idxLabelCurrent++] = ch;
                    labelCurrent[idxLabelCurrent] = '\0';
                }
                break;
            case kNameValueSep :
                if (ch == ':') {
                    idxValueCurrent = 0;
                    memset(valueCurrent,0,szLabel);
                    ChangeState(kValueStart);
                } else if (!std::isspace(ch)) {
                    return Error(ch);
                }
                break;
            case kValueStart :
                if (ch == '\"') {
                    ChangeState(kValueString);
                } else if (ch == '{') {
                    //printf("kValueStart, next is object, inArray: %s\n", inArray?"yes":"no");
                    // This should make it backwards compatible with our old decoder...
                    // Otherwise the List object would have to return itself...
                    // Note: We don't allow mixed object types in the list...
                    if (!inArray) {
                        memcpy(objectCurrent, labelCurrent, szLabel);
                        if (!NewObjectFromLabel(objectCurrent)) {
                            return Error(ch);
                        }
                        memset(labelCurrent, 0, szLabel);
                    }
                    PushArrayState(false);
                    ChangeState(kObjectStart);
                } else if (ch == '[') {
                    memcpy(objectCurrent, labelCurrent, szLabel);
                    // This ought to match
                    if (!NewObjectFromLabel(objectCurrent)) {
                        return Error(ch);
                    }
                    PushArrayState(true);
                    // Stays in state...

                } else if (((ch == ',')  || (ch == '}') || (ch ==']')) && (!relax)) {
                    // bla
                    return Error(ch);
                } else if (!std::isspace(ch)) {
                    valueCurrent[idxValueCurrent++] = ch;
                    valueCurrent[idxValueCurrent] = '\0';
                    ChangeState(kValue);
                }

                break;
            case kValue :
                if (ch == ',') {
                    OnValue();
                    if (inArray) {
                        ChangeState(kValueStart);
                    } else {
                        ChangeState(kObjectStart);
                    }
                } else if (ch == ']') {
                    // Commit last value...
                    //printf("kValue, end array token\n");
                    OnValue();
                    PopArrayState();
                    PopObject();
                    inArray?ChangeState(kEndArrayOrNext):ChangeState(kEndOrNext);
                } else if (ch == '}') {
                    OnValue();
                    PopObject();
                    PopArrayState();
                    inArray?ChangeState(kEndArrayOrNext):ChangeState(kEndOrNext);
                } else if (!std::isspace(ch)) {
                    valueCurrent[idxValueCurrent++] = ch;
                    valueCurrent[idxValueCurrent] = '\0';
                } else {
                    OnValue();
                    inArray?ChangeState(kEndArrayOrNext):ChangeState(kEndOrNext);
                }
                break;
            case kValueString :
                if (ch == '\"') {
                    OnValue();
                    inArray?ChangeState(kEndArrayOrNext):ChangeState(kEndOrNext);
                } else {
                    valueCurrent[idxValueCurrent++] = ch;
                    valueCurrent[idxValueCurrent] = '\0';
                }
                break;
            case kEndArrayOrNext :
                if (ch == ',') {
                    // Push this to the array
                    if (pCurrentObject!=nullptr) {
                        std::string dummy = objectCurrent;
                        pCurrentObject->PushToArray(dummy, pPreviousObject);
                    }
                    // Create new instance...
                    if (!NewObjectFromLabel(objectCurrent)) {
                        return Error(ch);
                    }
                    ChangeState(kValueStart);
                } else if (ch == ']') {
                    PopObject();
                    PopArrayState();
                    // Push last item to the array
                    if (pCurrentObject != nullptr) {
                        std::string dummy = objectCurrent;
                        pCurrentObject->PushToArray(dummy, pPreviousObject);
                    }
                    ChangeState(kEndOrNext);
                } else if (!std::isspace(ch) ){
                    return Error(ch);
                }
                break;
            case kEndOrNext :
                if (ch == ',') {
                    ChangeState(kObjectStart);
                } else if (ch == '}') {
                    PopObject();
                    PopArrayState();
                    inArray?ChangeState(kEndArrayOrNext):ChangeState(kEndOrNext);
                } else if (!std::isspace(ch)) {
                    return Error(ch);
                }
        }
    }
    return true;
}

// Private
bool JSONDecoder::NewObjectFromLabel(const char *label) {
    if (pCurrentObject == nullptr) {
        return true;
    }
    auto strObjectName = std::string(label);
    auto newObject = pCurrentObject->GetUnmarshalForField(strObjectName);

    // Special case for first object, we allow it to be same as current..
    // This allows for the following to be treat in the same way..
    // "{ \"Object\" : { \"Field\" : 1 } }"   ==    "{ \"Field\" : 1 }"
    //
    //  Side effect, this would override field with '2':
    // { "Field" : 4, "Object" : { "Field" : 2 } }
    //
    if ((newObject == nullptr) && (idxObjectStack == 1)) {
        //printf("[Warning] reusing root object for first level umarshalling of '%s'\n", label);
        newObject = pCurrentObject;
    } else {
        //printf("[Warning] No unmarshalling for '%s'\n", label);
    }
    return PushObject(newObject);
}

void JSONDecoder::PushArrayState(bool newArrayState) {
    if (idxInArrayState >= (szObjectStack-1)) {
        errx(__FILE__,__LINE__, "Array State stack exhausted, increase size");
    }
    //printf("Push ArrayState: %d, %s\n", idxInArrayState, newArrayState?"true":"false");
//    if (newArrayState) {
//        printf("Array Label: %s\n", objectCurrent);
//    }
    inArray = newArrayState;
    inArrayStateStack[idxInArrayState] = newArrayState;
    idxInArrayState++;
}

bool JSONDecoder::PopArrayState() {
    if (idxInArrayState == 0) {
        errx(__FILE__, __LINE__, "[ERROR] Array state stack underflow\n");
    }
    idxInArrayState--;
    if (idxInArrayState > 0) {
        inArray = inArrayStateStack[idxInArrayState-1];
    }

    return inArray;
}

bool JSONDecoder::PushObject(IUnmarshal *pObject) {
    if (idxObjectStack >= (szObjectStack-1)) {
        errx(__FILE__, __LINE__, "[ERROR] Object stack exhausted, increase size!\n");
    }
    objectStack[idxObjectStack] = pObject;
    pCurrentObject= pObject;
    idxObjectStack++;
    return true;
}
IUnmarshal *JSONDecoder::PopObject() {
    idxObjectStack--;
    if (idxObjectStack > 0) {
        pPreviousObject = pCurrentObject;
        pCurrentObject = objectStack[idxObjectStack-1];
    }
    return pCurrentObject;
}

void JSONDecoder::OnValue() {
    if ((cbValue == nullptr) && (pCurrentObject == nullptr)) {
        //printf("Value: '%s' = '%s'\n", labelCurrent, valueCurrent);
    } else {
        if (pCurrentObject != nullptr) {
            auto strLabel = std::string(labelCurrent);
            auto strValue = std::string(valueCurrent);
            pCurrentObject->SetField(strLabel, strValue);
        }
        if (cbValue != nullptr) {
            cbValue(objectCurrent, labelCurrent, valueCurrent);
        }
    }

    idxValueCurrent = 0;
    memset(valueCurrent,0,szLabel);

}

// private
bool JSONDecoder::Error(int chCurrent) {
    printf("[ERROR] in state '%d', index: %d, token: %c\n", state, idxParser, chCurrent);
    return false;
}


void JSONDecoder::ChangeState(kState newState) {
/*
    static std::unordered_map<kState, std::string> stateToName = {
            {kConsume, "Consume"},       // 0
            {kObjectStart, "ObjectStart"},   // 1
            {kLabel, "Label"},         // 2
            {kNameValueSep, "NameValueSep"},  // 3
            {kValueStart,"ValueStart"},    // 4
            {kValueString,"ValueString"},   // 5
            {kValue,"Value"},         // 6
            {kEndOrNext,"EndOrNext"},     // 7
            {kEndArrayOrNext,"EndArrayOrNext"}     // 8
    };
    printf("state change: %s (%d) -> %s (%d), label currently: %s\n", stateToName[state].c_str(), state, stateToName[newState].c_str(), newState, labelCurrent);
*/
    state = newState;
}

int JSONDecoder::Next() {
    if (!inStream->Available()) {
        return -1;
    }
    char ch;
    inStream->Read((uint8_t *)&ch, 1);
    idxParser++;
    return ch;
}
