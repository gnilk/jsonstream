# JSON Streaming Parser

This is a zero allocation JSON Streaming Parser.
Mainly intended for embedded use.

Should build on any modern CPP compiler - no fancy stuff being used.

In order to run the unit tests you need https://github.com/gnilk/testrunner

## JSONDecoder.cpp
Implements a very rudimentary and minimalstic JSON parser in C/CPP.
* No allocations
* I/O through interface implementation
* Callback or simplistic unmarshall interface to value to object mappings
* Full iterative parsing in order to save stack space

Buffers are defined at compile time. Default values are 64bytes for any label
or string. 

## JSONEncoder.cpp
Encoding of objects and similar. This is currently not functional (depends on commented out code in encoding.h)

## Support interfaces
### IReader
Defines two functions
- Read, reads a bunch of bytes from the underlying data stream
- Available, checks if more data is available

See the VectorReader or Memfile for an example..

### IUnmarshal
Defines a small interface in order to map the parsed data to the CPP data model

See the unit tests for an example


