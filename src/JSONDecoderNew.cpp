//
// Very small stream based and callback based JSON parser/decoder
// allocation free...
//

#include "JSONDecoderNew.h"

using namespace gnilk;

JSONDecoderNew::JSONDecoderNew(IReader *pStream, IUnmarshal *pRootObject) : inStream(pStream), pCurrentObject(pRootObject) {

}

JSONDecoderNew::~JSONDecoderNew() {

}

// NOTE: Nested object/unmarshalling not tested...
bool JSONDecoderNew::ProcessData(bool relax /*= false*/) {
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
                    ChangeState(kEndOrNext);
                } else if (ch == '}') {
                    OnValue();
                    PopObject();
                    PopArrayState();
                    ChangeState(kEndArrayOrNext);
                } else if (!std::isspace(ch)) {
                    valueCurrent[idxValueCurrent++] = ch;
                    valueCurrent[idxValueCurrent] = '\0';
                } else {
                    OnValue();
                    ChangeState(kEndOrNext);
                }
                break;
            case kValueString :
                if (ch == '\"') {
                    OnValue();
                    ChangeState(kEndOrNext);
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
                    ChangeState(kConsume);
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
                    if (inArray) {
                        //printf("kEndOrNext - In array\n");
                        ChangeState(kEndArrayOrNext);
                    } else {
                        ChangeState(kConsume);
                    }
                } else if (!std::isspace(ch)) {
                    return Error(ch);
                }
        }
    }
    return true;
}

// Private
bool JSONDecoderNew::NewObjectFromLabel(const char *label) {
    if (pCurrentObject == nullptr) {
        return true;
    }
    auto strObjectName = std::string(label);
    auto newObject = pCurrentObject->GetUnmarshalForField(strObjectName);
    if (newObject == nullptr) {
        //printf("[Warning] No unmarshalling for '%s'\n", label);
    }
    return PushObject(newObject);
}

void JSONDecoderNew::PushArrayState(bool newArrayState) {
    if (idxInArrayState >= (szObjectStack-1)) {
        printf("[ERROR] Array State stack exhausted, increase size\n");
        exit(1);
    }
    //printf("Push ArrayState: %d, %s\n", idxInArrayState, newArrayState?"true":"false");
//    if (newArrayState) {
//        printf("Array Label: %s\n", objectCurrent);
//    }
    inArray = newArrayState;
    inArrayStateStack[idxInArrayState] = newArrayState;
    idxInArrayState++;
}

bool JSONDecoderNew::PopArrayState() {
    if (idxInArrayState == 0) {
        printf("[ERROR] Array state stack underflow\n");
        exit(1);
    }
    idxInArrayState--;
    if (idxInArrayState > 0) {
        inArray = inArrayStateStack[idxInArrayState-1];
    }
    //printf("Pop ArrayState: %d, %s\n", idxInArrayState, inArray?"true":"false");

    return inArray;
}

bool JSONDecoderNew::PushObject(IUnmarshal *pObject) {
    if (idxObjectStack >= (szObjectStack-1)) {
        printf("[ERROR] Object stack exhausted, increase size!\n");
        return false;
    }
    objectStack[idxObjectStack] = pObject;
    pCurrentObject= pObject;
    idxObjectStack++;
    return true;
}
IUnmarshal *JSONDecoderNew::PopObject() {
    idxObjectStack--;
    if (idxObjectStack > 0) {
        pPreviousObject = pCurrentObject;
        pCurrentObject = objectStack[idxObjectStack-1];
    }
    return pCurrentObject;
}

void JSONDecoderNew::OnValue() {
    if ((cbValue == nullptr) && (pCurrentObject == nullptr)) {
        printf("Value: '%s' = '%s'\n", labelCurrent, valueCurrent);
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
bool JSONDecoderNew::Error(int chCurrent) {
    printf("[ERROR] in state '%d', index: %d, token: %c\n", state, idxParser, chCurrent);
    return false;
}

void JSONDecoderNew::ChangeState(kState newState) {
    //printf("state change: %d -> %d\n", state, newState);
    state = newState;
}

int JSONDecoderNew::Next() {
    if (!inStream->Available()) {
        return -1;
    }
    char ch;
    inStream->Read((uint8_t *)&ch, 1);
    idxParser++;
    return ch;
}
int JSONDecoderNew::Peek() {
    if (!inStream->Available()) {
        return -1;
    }
    return inStream->Peek();
}