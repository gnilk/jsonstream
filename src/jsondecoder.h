/*-------------------------------------------------------------------------
File    : jsondecoder.h
Author  : Gnilk
Version : $Revision: 1 $
Orginal : 2018-11-05
Descr   : Definition of the JSON Decoder interface


Modified: $Date: $ by $Author: FKling $
---------------------------------------------------------------------------
TODO: [ -:Not done, +:In progress, !:Completed]
<pre>
</pre>


\History
- 18.11.05, gnilk, Implementation

---------------------------------------------------------------------------*/

#ifndef GNILK_JSON_DECODER_H
#define GNILK_JSON_DECODER_H

#include <string>
#include <vector>
#include <stdint.h>

#include "encoding.h"

namespace gnilk {
        class DecoderObject;

        class JSONDecoder {
        public:
            JSONDecoder(IReader *pStream, IUnmarshal *pRootObject);
            virtual ~JSONDecoder();
            bool ProcessData(bool relax = false);


            // You should not need to call this one!!!
            const std::vector<DecoderObject *> &Objects() const { return allObjects; }

        private:
            void DisposeAllObjects();
            bool ProcessObject();
            bool ProcessArray();
            bool ProcessIdentifier();
            bool ProcessValue();
        private:
            void BeginArray(std::string name, IUnmarshal *_rootObject = NULL);
            size_t EndArray();
            void BeginObject(std::string name, IUnmarshal *_rootObject = NULL);
            size_t EndObject();

            IReader *ss;
            IUnmarshal *rootObject;


            std::string identifier;
            std::string fieldvalue;

            bool relaxed;

            DecoderObject *pCurrentObject;
            //std::string rootObjectName;
            std::vector<DecoderObject *> objstack;
            std::vector<DecoderObject *> allObjects;
        };

// Helper classes - TODO:  Move to jsondecoder_internal.h
        class DecoderArray;

        class DecoderObject {
            friend class DecoderArray;

        public:
            DecoderObject(std::string _name, DecoderObject *_parent, IUnmarshal *_unmarshal);
            virtual ~DecoderObject() = default;
            void AddField(std::string fieldname, std::string value);
            bool PushToArray(DecoderObject *otherObject);
            virtual IUnmarshal *GetUnmarshalForField(std::string fieldname);

            std::string &Name() { return name; }

            bool IsArray() { return isArray; }

            const std::vector<std::pair<std::string, std::string> > &Fields() { return fields; }

        protected:
            bool isArray;
            std::string name;
            DecoderObject *parent;
            IUnmarshal *unmarshal;
            std::vector<std::pair<std::string, std::string> > fields;
        };

        class DecoderArray : public DecoderObject {
//friend DecoderObject;

        public:
            DecoderArray(std::string _name, DecoderObject *_parent, IUnmarshal *_unmarshal);
            virtual IUnmarshal *GetUnmarshalForField(std::string fieldname);

        };

}
#endif