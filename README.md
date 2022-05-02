## JSON Streaming Parser

This repository includes two JSON decoders and one JSON encoder.
The old (jsonencoder/jsondecoder) are taken from another project.

Should build on any modern CPP compiler - no fancy stuff being used.

### JSONDecoderNew.cpp
Implements a very rudimentary and minimalstic JSON parser in C/CPP.
* No allocations
* I/O through interface implementation
* Callback or simplistic unmarshall interface to value to object mappings
* Full iterative parsing in order to save stack space

Buffers are defined at compile time. Default values are 64bytes for any label
or string. 

### jsondecoder.cpp
A more robust parser which uses little memory but still does allocations.
Objects/Values are passed directly to the IUnmarshal interface. A local copy of
all traversed objects are kept in the parser until you dispose it.

### jsonencoder.cpp
Encoding of objects and similar. This is currently not functional (depends on commented out code in encoding.h)