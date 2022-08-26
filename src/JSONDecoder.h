//
// Created by gnilk on 4/29/22.
//

#ifndef JSON_JSONDECODERNEW_H
#define JSON_JSONDECODERNEW_H

#include <functional>
#include <vector>
#include <string>
#include <utility>
#include "Encoding.h"



namespace gnilk {

    // Controls the max length for labels and string values..
#ifndef GNILK_JSON_MAX_LABEL_LEN
#define GNILK_JSON_MAX_LABEL_LEN 64
#endif

// Controls the max depth of nesting structures/objects (incl. arrays)
// This has a nesting level of three: {[{"a":1},{...}]}
#ifndef GNILK_JSON_MAX_STACK_DEPTH
#define GNILK_JSON_MAX_STACK_DEPTH 8
#endif

    /**
     * @brief Zero allocation JSON Decoder
     *
     * Parses JSON data and provides result via callback or the IUnmarshal interface.
     *
     * Can operate on a callback basis or via the IUnmarshal interface
     * This is mainly intended for embedded use.
     *
     * Override the default stack/label length with:
     * ```
     * -D GNILK_JSON_MAX_LABEL_LEN=<value>
     * -D GNILK_JSON_MAX_STACK_DEPTH=<value>
     * ```
     *
     * Note: Maximum value length must be defined at compile time (default is 64) same as Max name length
     */
    class JSONDecoder {
    public:
        /**
         * @brief Value callback definition
         *
         * You need to implement a function with this signature in order to use the callback mechanism
         * For the 'ROOT' object, the length of 'object' is 0.
         *
         * @param object name of currently parsed object or list
         * @param name name of value
         * @param value the value it self (always as a string, you need to know if this is a number or not)
         *
         */
        using ValueDelegate = std::function<void(const char *object, const char *name, const char *value)>;
    public:

        /**
         * @brief Construct the decoder with an input stream and callback
         *
         * This will call the ValueDelegate for any parsed data encountered. The callback is not well suited for
         * advanced data models like lists with objects and so forth. But it is fairly quick to parse simple JSON.
         *
         * Example:
         * ```cpp
         *     Memfile mf(data);
         *    auto callback = [](const char *object, const char *label, const char *value) {
         *        if (strlen(object) > 0) {
         *            printf("%s : %s = '%s'\n", object, label, value);
         *        } else {
         *            printf("ROOT: '%s' = '%s'\n", label, value);
         *        }
         *    };
         *
         *    JSONDecoder jsNew(&mf,callback);
         *    auto res = jsNew.ProcessData();
         * ```
         *
         * @param pStream Data input stream
         * @param valueDelegate Callback
         */
        JSONDecoder(IReader *pStream, ValueDelegate valueDelegate);

        /**
         * @Brief Construct the decoder with an input stream and a model object
         *
         * The data model must implement the IUnmarshal interface in order for it to work
         *
         * @param pStream Stream to read data from
         * @param pRootObject Data model
         */
        JSONDecoder(IReader *pStream, IUnmarshal *pRootObject);
        virtual ~JSONDecoder() = default;

        /**
         * @brief Explicitly set the callback
         *
         * If you want to use both IUnmarshal CTOR and the callback interface
         *
         * @param valueDelegate callback for any value parsed
         */
        void SetValueDelegate(ValueDelegate valueDelegate) { cbValue = valueDelegate; }

        /**
         * @brief Process and parse the data
         *
         * After constructing the object, call this function to parse the data.
         * Relaxed parsing allows trailing comma (,) in lists and on last element of objects
         *
         * @param relax Relax parsing
         * @return true/false on success or failure
         */
        bool ProcessData(bool relax = false);


        template<class T>
        static std::pair<T, bool> Deserialize(IReader *pStream, bool relax = false) {
            T model;
            JSONDecoder decoder(pStream, &model);
            auto res = decoder.ProcessData(relax);
            return std::make_pair(model, res);
        }

    private:
        void OnValue();
    private:
        typedef enum : int {
            kConsume,       // 0
            kObjectStart,   // 1
            kLabel,         // 2
            kNameValueSep,  // 3
            kValueStart,    // 4
            kValueString,   // 5
            kValue,         // 6
            kEndOrNext,     // 7
            kEndArrayOrNext, // 8
        } kState;
    private:
        bool NewObjectFromLabel(const char *label);
        bool PushObject(IUnmarshal *pObject);
        void PushArrayState(bool newArrayState);
        bool PopArrayState();
        IUnmarshal *PopObject();
        int Next();
        bool Error(int chCurrent);
        void ChangeState(kState newState);
    private:
        IReader *inStream = nullptr;
        kState state = kConsume;
        bool inArray = false;

        ValueDelegate cbValue = nullptr;
        IUnmarshal *pPreviousObject = nullptr;
        IUnmarshal *pCurrentObject = nullptr;
        int idxParser = 0;

        // Increase these two if you run into problems of strings too large or nested data too big...
        static const int szLabel = GNILK_JSON_MAX_LABEL_LEN;
        static const int szObjectStack = GNILK_JSON_MAX_STACK_DEPTH;

        int idxObjectStack = 0;
        IUnmarshal *objectStack[szObjectStack] = {};

        int idxInArrayState = 0;
        bool inArrayStateStack[szObjectStack] = {};

        char objectCurrent[szLabel] = {};

        int idxLabelCurrent = 0;
        char labelCurrent[szLabel] = {};

        int idxValueCurrent = 0;
        char valueCurrent[szLabel] = {};

    };
}


#endif //JSON_JSONDECODERNEW_H
