#ifndef GNILK_JSON_ENCODING_H
#define GNILK_JSON_ENCODING_H

#include <cstring>
#include <algorithm>
#include <cstdint>
#include <string>


namespace gnilk {

	class strutil {
	public:

		static inline std::string to_string(int value) {
		    // Not available on all platforms
		    return std::to_string(value);

		    // This is the lousy alternative
		    // char buffer[32];
		    // snprintf(buffer,32,"%d",value);
		    // return std::string(buffer);
		}

		static inline std::string to_string(uint32_t value) {
		    // Not available on all platforms
		    return std::to_string(value);

		    // This is the lousy alternative
			// char buffer[32];
			// snprintf(buffer,32,"%u",value);
			// return std::string(buffer);
	    }


		static inline std::string to_string(float value) {
			// Not available on all platforms
		    return std::to_string(value);

		    // This is the lousy alternative
		    // char buffer[32];
		    // snprintf(buffer,32,"%f",value);
		    // return std::string(buffer);
		}

	    static inline std::string &ltrim(std::string &s, const std::string& chars /* = "\t\n\v\f\r " */) {
	        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [chars](unsigned char ch) {
	            return !std::strchr(chars.c_str(), ch);
	        }));
	        return s;
	    }

	    static inline std::string& rtrim(std::string &s, const std::string& chars /* = "\t\n\v\f\r " */) {
	        s.erase(std::find_if(s.rbegin(), s.rend(), [chars](unsigned char ch) {
	            return !std::strchr(chars.c_str(), ch);
	        }).base(), s.end());
	        return s;
	    }

	    static inline std::string& trim(std::string& str,const std::string& chars /* = "\t\n\v\f\r " */) {
	        return ltrim(rtrim(str, chars), chars);
	    }
	};

    class IWriter {
    public:
        // Should return the number of bytes written or a negative number on error
        virtual int32_t Write(const uint8_t *buffer, size_t szBuffer) = 0;
    };

    class IReader {
    public:
        // Should return the number of bytes read or a negative number on error
        virtual int Read(uint8_t *data, size_t szData) = 0;
        // Returns true if data is available otherwise false (also if not supported/implemented)
        virtual bool Available() = 0;
    };

    typedef enum {
        kArrayMixedType,
        kArraySameType,
    } kArrayTypeSpec;

    // class IEncoder {
    // public:
    //     // Optional (for app) call to set the output stream/writer
    //     virtual void Begin(IWriter *writer) = 0;

    //     // Begin encoding of an identifier
    //     virtual void BeginObject(std::string name = "") = 0;
    //     // End encoding of an identifier
    //     virtual void EndObject(bool hasNext = false) = 0;
    //     // Write various fields (between Begin()  / End() call's)
    //     virtual void WriteBoolField(std::string name, bool value, bool hasNext = true) = 0;
    //     virtual void WriteIntField(std::string name, int value, bool hasNext = true) = 0;
    //     virtual void WriteFloatField(std::string name, float value, bool hasNext = true) = 0;
    //     virtual void WriteTextField(std::string name, std::string value, bool hasNext = true) = 0;

    //     // example of some overloading
    //     void Write(std::string name, bool value, bool hasNext = true){
    //         WriteBoolField(name, value, hasNext);
    //     };

    //     // Begin an array
    //     virtual void BeginArray(std::string name, kArrayTypeSpec typeSpec) = 0;
    //     // End an array
    //     virtual void EndArray(bool hasNext = false) = 0;

    //     virtual IWriter *Writer() = 0;
    // };



    //
    // This must be implemented by any structure that can be marshalled
    //
    // class IMarshal {
    // public:
    //     // encoder - encoding object
    //     // name - supply this to encoder.Begin()
    //     // hasNext - supply this to encoder.End()
    //     virtual void Marshal(IEncoder &encoder, std::string name = "", bool hasNext = false) const = 0;
    // };

    //
    // This is for unmarshalling support - bit more complicated
    // You need to inherit this if you want unmarshalling
    //
    class IUnmarshal {
    public:
        // Called to set a field/variable in your class
        virtual bool SetField(std::string &name, std::string &value) = 0;
        // Called to get an instance to a sub-object in your class
        virtual IUnmarshal *GetUnmarshalForField(std::string &name) = 0;
        // Called to push things to an array you might have
        virtual bool PushToArray(std::string &name, IUnmarshal *pData) = 0;
    };




}


#endif
