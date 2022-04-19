/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#if defined(BXZSTR_ZSTD_SUPPORT) && (BXZSTR_ZSTD_SUPPORT) == 1

#ifndef BXZSTR_ZSTD_STREAM_WRAPPER_HPP
#define BXZSTR_ZSTD_STREAM_WRAPPER_HPP

#include <zstd.h>

#include <string>
#include <sstream>
#include <exception>

#include "stream_wrapper.hpp"

namespace bxz {
/// Exception class thrown by failed zlib operations.
class zstdException : public std::exception {
public:
    zstdException(const z_stream* zstrm_p, const int ret) : _msg("zlib: ") {
        switch (ret) {
            case Z_STREAM_ERROR:
		_msg += "Z_STREAM_ERROR: ";
		break;
            case Z_DATA_ERROR:
		_msg += "Z_DATA_ERROR: ";
		break;
            case Z_MEM_ERROR:
		_msg += "Z_MEM_ERROR: ";
		break;
            case Z_VERSION_ERROR:
		_msg += "Z_VERSION_ERROR: ";
		break;
            case Z_BUF_ERROR:
		_msg += "Z_BUF_ERROR: ";
		break;
            default:
		std::ostringstream oss;
		oss << ret;
		_msg += "[" + oss.str() + "]: ";
		break;
        }
        _msg += zstrm_p->msg;
    }
    zstdException(const std::string msg) : _msg(msg) {}

    const char * what() const noexcept { return _msg.c_str(); }

  private:
    std::string _msg;
}; // class zException

namespace detail {
class zstd_stream_wrapper : public stream_wrapper {
  public:
    zstd_stream_wrapper(const bool _is_input = true,
		     const int _level = Z_DEFAULT_COMPRESSION, const int = 0)
	    : is_input(_is_input) {
	if (is_input) {
	    this->dctx = ZSTD_createDCtx(); // TODO: Check that dctx != NULL, and use params, and size buffers
	} else {
	    this->cctx = ZSTD_createCCtx(); // TODO: as above.
	    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, _level);
	    // ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, nbThreads); // TODO: implement multithreading.
	}
	//if (ret != Z_OK) throw zException(this, ret);
    }
    ~zstd_stream_wrapper() {
	if (is_input) {
	    ZSTD_freeDCtx(this->dctx);
	} else {
	    ZSTD_freeCCtx(this->cctx);
	    //deflateEnd(this);
	}
    }

    int decompress(const int _flags = Z_NO_FLUSH) override {
	ZSTD_inBuffer input = { this->buffIn, this->buffInSize, 0 };
	ZSTD_outBuffer output = { this->buffOut, this->buffOutSize, 0 };
	this->ret = ZSTD_decompressStream(this->dctx, &output, &input); // TODO: check_zstd(ret)

	// Update internal state
	this->set_next_out(this->next_out() + output.pos);
	this->set_avail_out(this->avail_out() - output.pos);
	this->set_next_in(this->next_in() + input.pos);
	this->set_avail_in(this->avail_in() - input.pos);

	return ret; // TODO: checks
    }
    int compress(const int _flags = Z_NO_FLUSH) override {
	ZSTD_inBuffer input = { this->buffIn, this->buffInSize, 0 };
	ZSTD_outBuffer output = { this->buffOut, this->buffOutSize, 0 };
	if (_flags == 0) {
	    size_t remaining = ZSTD_compressStream2(this->cctx, &output, &input, ZSTD_e_continue);

	    // Update internal state
	    this->set_next_in(this->next_in() + input.pos);
	    this->set_avail_in(this->avail_in() - input.pos);

	    this->ret = (input.pos == input.size);
	} else {
	    ret = ZSTD_endStream(this->cctx, &output);
	}
	this->set_next_out(this->next_out() + output.pos);
	this->set_avail_out(this->avail_out() - output.pos);

	return ret; // TODO: checks
    }
    bool stream_end() const override { return this->ret == 0; }
    bool done() const override { return this->stream_end(); }

    const uint8_t* next_in() const override { return static_cast<unsigned char*>(this->buffIn); }
    long avail_in() const override { return this->buffInSize; }
    uint8_t* next_out() const override { return static_cast<unsigned char*>(this->buffOut); }
    long avail_out() const override { return this->buffOutSize; }

    void set_next_in(const unsigned char* in) override { this->buffIn = (void*)in; }
    void set_avail_in(long in) override { this->buffInSize = in; }
    void set_next_out(const uint8_t* in) override { this->buffOut = (void*)in; }
    void set_avail_out(long in) override { this->buffOutSize = in; }

  private:
    bool is_input;
    int ret;

    ZSTD_DCtx* dctx;
    ZSTD_CCtx* cctx;

    size_t buffInSize;
    void* buffIn;
    size_t buffOutSize;;
    void* buffOut;
}; // class zstd_stream_wrapper
} // namespace detail
} // namespace bxz

#endif
#endif
