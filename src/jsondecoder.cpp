/*-------------------------------------------------------------------------
File    : jsondecoder.cpp
Author  : gnilk
Version : $Revision: 1 $
Orginal : 2018-10-30
Descr   : Implements JSON decoding for classes implementing the IUnmarshal interface


NOTE: This code has been rewritten twice now, but it's still quite messy... Most logic is in the 'ProcessValue' directive

Set 'Releax' to true when calling ProcessData to allow the following:
    - Object and Arrays can be ended with either ']' or '}'
    - Don't care if array's are ended with a trailing ',' -> [4,5,] is ok..

---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
  - Delete temoprary objects in destructor!!!!!
  - Add 'GetLastError' and better error tracking!!!!!
  ! Fix statemachine (i.e. rewrite it) - removed it not needed
</pre>


\History
- 18.11.09, gnilk, Added access to objects parsed by the parser
- 18.11.05, gnilk, Rewrote parser
- 18.10.30, gnilk, Implementation

---------------------------------------------------------------------------*/

#include <string>
#include <vector>
#include <cctype>
#include <stdint.h>

#include "encoding.h"
#include "jsondecoder.h"
//#include "Common/strutil.h"

using namespace gnilk;

#define kParseState_ExpectStartToken 0x01
#define kParseState_Identifier 0x02
#define kParseState_ExpectFieldSeparator 0x03
#define kParseState_Field 0x04
#define kParseState_StringField 0x05




JSONDecoder::JSONDecoder(IReader *pStream, IUnmarshal *pRootObject) :
    ss(pStream),
    rootObject(pRootObject) {
    relaxed = false;
}

JSONDecoder::~JSONDecoder() {
    DisposeAllObjects();
}

void JSONDecoder::DisposeAllObjects() {
    for (size_t i=0;i<allObjects.size();i++) {
        delete(allObjects[i]);
    }
    allObjects.clear();
}

//
// ProcessData
//
bool JSONDecoder::ProcessData(bool relax /*= false*/) {
    int i=0;
    uint8_t c;
    bool done = false;
    relaxed = relax;

    pCurrentObject = NULL;

    identifier="";
    while(!done) {
        if ((i = ss->Read(&c, 1)) != 1) {
            // EOF
            if (i == 0) return true;
            // I/O error
            return false;
        }
        switch(c) {
            case '{' :
                if (!ProcessObject()) {                    
                    return false;
                }
                break;
            case '[' :
                if (!ProcessArray()) {
                    return false;
                }
                break;
            default:
                //printf("ERROR: JSONDecoder::ProcessData, illegal start char '%c', expected '{' or '['\n",c);
                return false;
        }
    }
    return true;
}
bool JSONDecoder::ProcessObject() {
    int i=0;
    uint8_t c;
    BeginObject(identifier, rootObject);
    while((i = ss->Read(&c, 1))==1) {
        //printf("JSONDecoder::ProcessObject, got '%c'\n", c);

        bool ok = true;

        switch(c) {
            case '{' :
                ok = ProcessObject();
                break;
            case '[' :
                ok = ProcessArray();
                break;
            case '\"' :
                ok = ProcessIdentifier();
                break;
            case ':' :
                ok = ProcessValue();
                break;
            case '}' :
                EndObject();
                return true;
            default:
                break;
        }
        if (!ok) {
            return false;
        }
    }
    return true;
}
bool JSONDecoder::ProcessArray() {
    BeginArray(identifier, rootObject);
    DecoderObject *pArray = pCurrentObject;
    while(true) {
        if (!ProcessValue()) {
            return false;
        }
        if (pArray != pCurrentObject) {
            break;
        }
    }
    return true;
}

bool JSONDecoder::ProcessIdentifier() {
    int i=0;
    uint8_t c;
    //std::string identifier = "";
    bool done = false;
    identifier= "";

    while(!done) {
        if ((i = ss->Read(&c, 1))!=1) {
            return false;
        }
        switch(c) {
            case '\"' :
                done = true;
                break;
            default:
                identifier += (char)c;
                break;
        }
    }
    //printf("JSONDecoder::ProcessIdentifier, identifier: '%s'\n", identifier.c_str());
    return true;
}

bool JSONDecoder::ProcessValue() {
    int i=0;
    uint8_t c;  
    std::string value = "";  
    bool done = false;
    bool skipAdd = false;
    //bool fieldIsString = false;
    bool skipWhiteSpace = true;
    while(!done) {
        if ((i = ss->Read(&c, 1))!=1) {
            if (i == 0) {
                // EOF
                if (objstack.empty()) {
                    // printf("EOF\n");
                    return true;
                }
                // printf("ERROR: Malformed JSON at tend, missing termination!\n");
                //exit(1);
                return false;
            }
            // Handle I/O
            // printf("JSONDecoder::ProcessValue, I/O Error: %d\n", i);
            return false;
        }

        // skip white space here!!!
        if (skipWhiteSpace) {
            // printf("JSONDecoder::ProcessValue, skip white space, %.2x\n", c);
            if (std::isspace(c)) continue;
        }

        // printf("JSONDecoder::ProcessValue, %c\n", (char)c);
        switch(c) {
            case '{' :
                if (!ProcessObject()) {
                    return false;
                }
                break;
            case '[' :
                if (!ProcessArray()) {
                    return false;
                }
                return true;
                break;
            case '}' :  // Need to handle this
                if (pCurrentObject->IsArray() && !relaxed) {
                    return false;
                } else if (!skipAdd) {
                    pCurrentObject->AddField(identifier, value);
                }
                // printf("JSONDecoder::ProcessValue, end-of-object: %s\n",pCurrentObject->Name().c_str());
                if (!EndObject()) {
                    return true;
                }
                skipAdd = true;
                break;
            case ']' :  // Need to handle this
                // no value, array was ended after ']' - this not allowed
                if (value.empty() && !pCurrentObject->Fields().empty() && !relaxed) {
                    //printf("Error: ending array with ,] is not allowed!\n");
                    return false;
                }

//                printf("JSONDecoder::ProcessValue, end-of-array: %s\n",pCurrentObject->Name().c_str());
                if (!pCurrentObject->IsArray()) {
                    return false;
                }
                if (!EndArray()) {
                    return true;
                }
                skipAdd = true;
                break;
            case '"' :
                if (skipWhiteSpace == true) {
                //    printf("JSONDecoder::ProcessValue, start string ('\"'), consuming data\n");
                    skipWhiteSpace = false;
                    //fieldIsString = true;
                } else { 
                //    printf("JSONDecoder::ProcessValue, end string ('\"')\n");
                    skipWhiteSpace = true;
                }
                break;
            case ',' :
                // Adding already done as part of 'end-object' / 'end-array'            
                if (!skipAdd) {
                    //printf("JSONDecoder::ProcessValue, adding field: %s = %s\n", identifier.c_str(), value.c_str());
                    pCurrentObject->AddField(identifier, value);
                } else {
                    // printf("JSONDecoder::ProcessValue, skip add value, already done!\n");
                }
                return true;
            default:
                value += (char)c;
        }
    }
    //printf("JSONDecoder::ProcessValue, value: %s\n", value.c_str());
    return true;
}



void JSONDecoder::BeginObject(std::string name, IUnmarshal *_rootObject /* = NULL*/) {

    IUnmarshal *unmarshal = _rootObject;
    if (name.empty() && (pCurrentObject == NULL)) {
        //printf("Root object\n");
        //name = rootObjectName;d
        unmarshal = rootObject;
    }
    if (pCurrentObject != NULL) {
        //printf("BeginObject, currentObject = %s (array: %s)\n", pCurrentObject->Name().c_str(), pCurrentObject->IsArray()?"Yes":"No");
        unmarshal = pCurrentObject->GetUnmarshalForField(name);
    }

    // if ((unmarshal == NULL) && (factory != NULL)) {
    //     unmarshal = factory->CreateObject(name);
    // } 

    // TODO: Enable this once new parser is done!!!    
//    if (unmarshal == NULL) {
//        printf("ERROR: No unmarshal support for '%s'\n", name.c_str());
//        exit(1);
//    }

    DecoderObject *obj = new DecoderObject(name, pCurrentObject, unmarshal);
    objstack.push_back(obj);
    allObjects.push_back(obj);
    pCurrentObject = obj;

//    printf("** JSONDecoder::BeginObject(), pCurrentObject=%p\n", pCurrentObject);

}
size_t JSONDecoder::EndObject() {
    //printf("*** JSONDecoder::EndObject(), current: %s (objstack: %d)\n", pCurrentObject->Name().c_str(), objstack.size());
    
    DecoderObject *pOld = pCurrentObject;
    identifier = "";    // kill the current identifier
    objstack.pop_back();
    if (objstack.size() > 0) {
        pCurrentObject = objstack.back();
    } else {
        pCurrentObject = NULL;
    }
//    printf("** JSONDecoder::EndObject(), pCurrentObject=%p\n", pCurrentObject);

    if (pCurrentObject == NULL) {
        // ROOT!
        return 0;
    }

    if (pOld != NULL) {
        // printf("EndObject: %s (array: %s) -> %s (array: %s)\n", 
        //              pOld->Name().c_str(), pOld->IsArray()?"Yes":"No",
        //              pCurrentObject->Name().c_str(), pCurrentObject->IsArray()?"Yes":"No");

        if (pCurrentObject->IsArray()) {
            pCurrentObject->PushToArray(pOld);
        }

    } else {
        
    }
    if (pCurrentObject != NULL) {
        identifier = pCurrentObject->Name();    // kill the current identifier
    }
    return objstack.size();

}
void JSONDecoder::BeginArray(std::string name, IUnmarshal *_rootObject /* = NULL */) {
    IUnmarshal *unmarshal = _rootObject;

    // No parent object to hold this array???
    // if (pCurrentObject == NULL) {
    //     // TODO: FIX THIS!!! We should create the root object here
    //     printf("ERROR: No parent object to hold array, bailing!\n");
    //     //exit(1);
    // }

    if (pCurrentObject != NULL) {
        unmarshal = pCurrentObject->GetUnmarshalForField(name);
    }

    // if ((unmarshal == NULL) && (factory != NULL)) {
    //     unmarshal = factory->CreateObject(name);
    // } 
   
    if (unmarshal == NULL) {
        printf("WARNING: No unmarshal support for '%s'\n", name.c_str());
    }

    DecoderArray *obj = new DecoderArray(name, pCurrentObject, unmarshal);

    objstack.push_back(obj);
    allObjects.push_back(obj);
    pCurrentObject = obj;

    //printf("** JSONDecoder::BeginArray(), objstack: %d\n", objstack.size());
}

size_t JSONDecoder::EndArray() {

    //printf("JSONDecoder::EndArray, objstack: %d\n", objstack.size());

    DecoderObject *pOld = pCurrentObject;
    objstack.pop_back();
    //pCurrentObject = objstack.back();

    if (objstack.size() > 0) {
        pCurrentObject = objstack.back();
    } else {
        pCurrentObject = NULL;
    }
    // printf("** JSONDecoder::EndArray(), pCurrentObject=%p\n", pCurrentObject);

    if (pCurrentObject == NULL) {
        // ROOT!
        return 0;
    }


    if (pOld != NULL) {
        // printf("EndArray: %s (array: %s) -> %s (array: %s)\n", 
        //             pOld->Name().c_str(), pOld->IsArray()?"Yes":"No",
        //             pCurrentObject->Name().c_str(), pCurrentObject->IsArray()?"Yes":"No");
    } else {
        
    }
    if (pCurrentObject != NULL) {
        identifier = pCurrentObject->Name();    // kill the current identifier
    }
    return objstack.size();
}




// Helper
DecoderArray::DecoderArray(std::string _name, DecoderObject *_parent, IUnmarshal *_unmarshal) :
    DecoderObject(_name, _parent, _unmarshal) {

    isArray = true;
}
IUnmarshal *DecoderArray::GetUnmarshalForField(std::string fieldname) {
//    printf("Decoder Array, fetching unmarshal for id: %s\n", name.c_str());
    if ((parent != NULL) && (parent->unmarshal != NULL)) {
        return parent->unmarshal->GetUnmarshalForField(name);
    } else {
        if (unmarshal != NULL) {
            return unmarshal->GetUnmarshalForField(name);
        }
    }
    return NULL;
}


DecoderObject::DecoderObject(std::string _name, DecoderObject *_parent, IUnmarshal *_unmarshal) : 
    name(_name),
    parent(_parent),
    unmarshal(_unmarshal) {
    isArray = false;
//    printf("  New Object: %s\n", _name.c_str());
}

// TODO: Need a hint here if the field was string or not!
void DecoderObject::AddField(std::string fieldname, std::string value) {

    //printf("AddField, raw:  %s = %s\n", fieldname.c_str(), value.c_str());

    value = strutil::trim(value,"\"\n");

    if (unmarshal != NULL) {
        if (!unmarshal->SetField(fieldname, value)) {
            //printf("DecoderObject::AddField, Error: No such field: '%s' in object '%s'\n", fieldname.c_str(), name.c_str());
        }
    } else {
        //printf("DecoderObject::AddField, Error: object '%s' has no IUnmarshal support\n", name.c_str());
    }

    fields.push_back(std::pair<std::string, std::string>(fieldname, value));
}

bool DecoderObject::PushToArray(DecoderObject *otherObject) {
    // printf("DecoderObject::PushToArray\n");
    // printf(" this: %s\n", this->name.c_str());
    // printf(" parent: %s\n", this->parent->name.c_str());
    // printf(" other: %s\n", otherObject->name.c_str());
    //calling: %s::PushToArray, other: %s\n", parent->name.c_str(),otherObject->name.c_str());
    if (parent != NULL) {
        return parent->unmarshal->PushToArray(name, otherObject->unmarshal);
    } else {
        if (unmarshal != NULL) {
            unmarshal->PushToArray(otherObject->Name(), otherObject->unmarshal);
        }
    }
    return false;
}


IUnmarshal *DecoderObject::GetUnmarshalForField(std::string fieldname) {
    if (unmarshal == NULL) {
        return NULL;
    }
    return unmarshal->GetUnmarshalForField(fieldname);
}
