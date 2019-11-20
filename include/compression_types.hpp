/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_COMPRESSION_TYPES_HPP
#define BXZSTR_COMPRESSION_TYPES_HPP

#include <exception>

#include "stream_wrapper.hpp"
#include "bz_stream_wrapper.hpp"
#include "lzma_stream_wrapper.hpp"
#include "z_stream_wrapper.hpp"

namespace bxz {
    enum Compression { z, bz2, lzma, plaintext };
    Compression detect_type(char* in_buff_start, char* in_buff_end) {
	unsigned char b0 = *reinterpret_cast< unsigned char * >(in_buff_start);
	unsigned char b1 = *reinterpret_cast< unsigned char * >(in_buff_start + 1);
        if (in_buff_start + 2 <= in_buff_end
	    && ((b0 == 0x1F && b1 == 0x8B)         // gzip header
		|| (b0 == 0x78 && (b1 == 0x01      // zlib header
				   || b1 == 0x9C
				   || b1 == 0xDA))))
	    return z;
	unsigned char b2 = *reinterpret_cast< unsigned char * >(in_buff_start + 2);
        if (in_buff_start + 2 <= in_buff_end
	    && ((b0 == 0x42 && b1 == 0x5a && b2 == 0x68))) // bz2 header
	    return bz2;
	unsigned char b3 = *reinterpret_cast< unsigned char * >(in_buff_start + 3);
	unsigned char b4 = *reinterpret_cast< unsigned char * >(in_buff_start + 4);
	unsigned char b5 = *reinterpret_cast< unsigned char * >(in_buff_start + 5);
	if (in_buff_start + 6 <= in_buff_end
	    && ((b0 == 0xFD && b1 == 0x37 && b2 == 0x7A
		 && b3 == 0x58 && b4 == 0x5A && b5 == 0x00))) // liblzma header
	    return lzma;
	return plaintext;
    }
    void init_stream(const Compression &type, detail::stream_wrapper **strm_p, bool is_input = true) {
	switch (type) {
	case lzma : *strm_p = new detail::lzma_stream_wrapper(is_input); break;
	case bz2 : *strm_p = new detail::bz_stream_wrapper(is_input); break;
	case z : *strm_p = new detail::z_stream_wrapper(is_input); break;
	default : throw std::runtime_error("Unrecognized compression type.");
	}
    }
    int bxz_run(const Compression &type){
	switch(type){
        case lzma: return 0; break; // LZMA_RUN
        case bz2: return 0; break; // BZ_RUN
        case z: return 0; break; // Z_NO_FLUSH
	default: throw std::runtime_error("Unrecognized compression type.");
	}
    }
    int bxz_finish(const Compression &type){
	switch(type){
        case lzma: return 3; break; // LZMA_FINISH
        case bz2: return 2; break; // BZ_FINISH
        case z: return 4; break; // Z_FINISH
	default: throw std::runtime_error("Unrecognized compression type.");
	}
    }
}

#endif
