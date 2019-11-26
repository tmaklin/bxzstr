# bxzstr — A C++11 ZLib / libBZ2 / libLZMA wrapper

Header-only library for using standard c++ iostreams to access streams
compressed with ZLib, libBZ2, or liblzma (.gz, .bz2, and .xz files).

For decompression, the format is automatically detected. For
compression, the only parameter exposed is the compression algorithm.

bxzstr is a fork of the [zstr](https://github.com/mateidavid/zstr)
library by [Matei David](https://github.com/mateidavid), and the core
functionality of this library remains largely the same.

## Input detection

The library automatically detects whether the input stream is
compressed or not, and with which algorithm. The detection is based on
identifying the headers based on the magic numbers:
* GZip header, starting with **1F 8B**
* ZLib header, starting with **78 01**, **78 9c**, and **78 DA**
* BZ2 header, starting with **42 5a 68**
* LZMA header, starting with **FD 37 7A 58 5A 00**

when no header is identified, the strema is treated as plain text (uncompressed).

## Usage
The streams can be accessed through 6 classes that function similarly
to their standard library counterparts

* `bxz::istreambuf` is the core decompression class.
* `bxz::ostreambuf` is the core compression class.
* `bxz::istream` is a wrapper for the `bxz::istreambuf` class.
* `bxz::ostream` is a wrapper for the `bxz::ostreambuf` class.
* `bxz::ifstream` is a wrapper for the `bxz::istreambuf` class that
  can be used to open a file and read decompressed data from it.
* `bxz::ofstream` is a wrapper for the `bxz::ostreambuf` class that
  can be used to write compressed data to a file.

For the classes derived from `bxz::ostreambuf`, the compression
algorithm must be specified as the second argument:
```
bxz::ofstream("filename", bxz::z);
bxz::ostream(std::cin, bxz::bz2);
bxz::ostreambuf(std::cin.rdbuf(), bxz::lzma);
```

It's also possible to specify the compression level (1-9) as the third
parameter (default level is 6):
```
bxz::ofstream("filename", bxz::z, 1);
bxz::ostream(std::cin, bxz::bz2, 5);
bxz::ostreambuf(std::cin.rdbuf(), bxz::lzma, 9);
```

If the stream objects fail at any point, `failbit` exception mask will
be turned on.

## Requirements and dependencies
* Compiler with c++11 support
* libz, libbz2, and liblzma

You can use the library without one of libz, libbz2, or liblzma by
commenting out the the releant include in `compression_types.hpp`. For
example, to disable lzma support, comment out lzma_stream_wrapper.hpp
as follows:
```
#ifndef BXZSTR_COMPRESSION_TYPES_HPP
#define BXZSTR_COMPRESSION_TYPES_HPP

#include <exception>

#include "stream_wrapper.hpp"
#include "bz_stream_wrapper.hpp"
// #include "lzma_stream_wrapper.hpp"
#include "z_stream_wrapper.hpp"

```
The rest of the header will configure itself accordingly.

## License
The source code from this project is subject to the terms of the
Mozilla Public License, v. 2.0. A copy of the MPL is supplied with the
project, or can be obtained at
[https://mozilla.org/MPL/2.0/](https://mozilla.org/MPL/2.0/).
