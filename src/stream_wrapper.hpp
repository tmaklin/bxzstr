/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * This file is a part of bxzstr (https://github.com/tmaklin/bxzstr)
 * Written by Tommi Mäklin (tommi@maklin.fi) */

#ifndef BXZSTR_STREAM_WRAPPER_HPP
#define BXZSTR_STREAM_WRAPPER_HPP

namespace bxz {
namespace detail {
    class stream_wrapper {
    private:
    public:
	stream_wrapper() {};
	stream_wrapper(bool _is_input, int _flag1, int _flag2) {};
	virtual ~stream_wrapper() = default;
	virtual int decompress(int _flags = 0) =0;
	virtual int compress(int _flags = 0) =0;
	virtual bool stream_end() const =0;
	virtual bool done() const =0;

	virtual const uint8_t* next_in() =0;
	virtual long avail_in() =0;
	virtual uint8_t* next_out() =0;
	virtual long avail_out() =0;
	virtual void set_next_in(const unsigned char* in) =0;
	virtual void set_avail_in(long in) =0;
	virtual void set_next_out(uint8_t* in) =0;
	virtual void set_avail_out(long in) =0;
};
} // namespace detail
} // namespace bxz

#endif
