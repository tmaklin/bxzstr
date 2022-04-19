# bxzstr documentation
This file details how to implement support for new compression types,
particularly those that do not have an API that is (almost) drop-in
replacement for zlib.

## Adding support for new compression types
### Creating a stream wrapper
A stream wrapper file forms the core of the compression type support
in bxzstr. Most types _should_ be implementable by simply defining a
`compression_type_stream_wrapper` class that implements the functions
from the abstract `stream_wrapper` defined in
[/include/stream_wrapper.hpp](/include/stream_wrapper.hpp). These
functions are used by the bxzstr ostreambuf and istreambuf classes to
manipulate the compressed stream without touching the implementation
of the compression algorithm itself.

Each stream wrapper should implement functionality for both
compression and decompression operations. The difference between them
is determined by an internal state variable.

#### Functions defined in stream\_wrapper
These are the functions that every stream\_wrapper class needs to implement:
```
    stream_wrapper(const bool _isInput, const int _level, const int _flags);
    virtual ~stream_wrapper() = default;
    virtual int decompress(const int _flags = 0) =0;
    virtual int compress(const int _flags = 0) =0;
    virtual bool stream_end() const =0;
    virtual bool done() const =0;

    virtual const uint8_t* next_in() const =0;
    virtual long avail_in() const =0;
    virtual uint8_t* next_out() const =0;
    virtual long avail_out() const =0;

    virtual void set_next_in(const unsigned char* in) =0;
    virtual void set_avail_in(const long in) =0;
    virtual void set_next_out(const uint8_t* in) =0;
    virtual void set_avail_out(const long in) =0;
```

The purpose of each function in the bxzstr write and read operations
will be covered below.

##### stream\_wrapper(const bool \_isInput, const int \_level, const int \_flags))
Each stream\_wrapper should have a constructor that accepts the following arguments:
- \_isInput: define whether we are compressing or decompressing a stream.
- \_level: compression level for the algorithm.
- \_flags: optional integer parameter.

The the \_level and/or \_flags arguments may default to 0 if they are
not required.

The constructor should initialize the stream as the correct
compressing or decompressing object with the supplied arguments. If
there are problems in initializing the stream, the constructor should
throw an error (more about error). If the compression API
provides functions for initializing the stream and checking for
errors, these should be used.

##### ~stream_wrapper();
The destructor should at minimum flush (end) the stream and deallocate
all memory that may have been allocated using the C-style
functions. If the compression API provides a function for ending the
stream, this should be used.

##### int decompress(const int _flags = 0)
TODO.
