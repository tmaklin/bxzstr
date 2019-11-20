// Adapted from zstr (https://github.com/mateidavid/zstr)
// zstr was written by Matei David (matei@cs.toronto.edu)

#ifndef __LZMASTR_HPP
#define __LZMASTR_HPP

#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <lzma.h>
#include "strict_fstream.hpp"

namespace lzmastr
{

/// Exception class thrown by failed liblzma operations.
class lzmaException
    : public std::exception
{
public:
    lzmaException(lzma_stream * lzmastrm_p, lzma_ret ret)
        : _msg("liblzma: ")
    {
        switch (ret)
        {
        case LZMA_MEM_ERROR:
            _msg += "LZMA_MEM_ERROR: ";
        case LZMA_OPTIONS_ERROR:
            _msg += "LZMA_OPTIONS_ERROR: ";
            break;
        case LZMA_UNSUPPORTED_CHECK:
            _msg += "LZMA_UNSUPPORTED_CHECK: ";
            break;
        case LZMA_PROG_ERROR:
            _msg += "LZMA_PROG_ERROR: ";
            break;
        case LZMA_BUF_ERROR:
            _msg += "LZMA_BUF_ERROR: ";
            break;
        case LZMA_DATA_ERROR:
            _msg += "LZMA_DATA_ERROR: ";
            break;
        case LZMA_FORMAT_ERROR:
            _msg += "LZMA_FORMAT_ERROR: ";
            break;
        case LZMA_NO_CHECK:
            _msg += "LZMA_NO_CHECK: ";
            break;
        case LZMA_MEMLIMIT_ERROR:
            _msg += "LZMA_MEMLIMIT_ERROR: ";
            break;
        default:
            std::ostringstream oss;
            oss << ret;
            _msg += "[" + oss.str() + "]: ";
            break;
        }
        _msg += ret;
    }
    lzmaException(const std::string msg) : _msg(msg) {}
    const char * what() const noexcept { return _msg.c_str(); }
private:
    std::string _msg;
}; // class lzmaException

namespace detail
{
class lzma_stream_wrapper
    : public lzma_stream
{
public:
    lzma_stream_wrapper(bool _is_input = true, int _level = 2, int _flags = 0)
        : is_input(_is_input), lzma_stream(LZMA_STREAM_INIT)
    {
        lzma_ret ret;
        if (is_input)
        {
            this->avail_in = 0;
            this->next_in = NULL;
	    ret = lzma_auto_decoder(this, UINT64_MAX, _flags);
        }
        else
        {
	    ret = lzma_easy_encoder(this, _level, LZMA_CHECK_CRC64);
        }
	if (ret != LZMA_OK) throw lzmaException(this, ret);
    }
    ~lzma_stream_wrapper()
    {
	lzma_end(this);
    }
private:
    bool is_input;
}; // class lzma_stream_wrapper
} // namespace detail

class istreambuf
    : public std::streambuf
{
public:
    istreambuf(std::streambuf * _sbuf_p,
               std::size_t _buff_size = default_buff_size, bool _auto_detect = true)
        : sbuf_p(_sbuf_p),
          lzmastrm_p(nullptr),
          buff_size(_buff_size),
          auto_detect(_auto_detect),
          auto_detect_run(false),
          is_text(false)
    {
        assert(sbuf_p);
        in_buff = new char [buff_size];
        in_buff_start = in_buff;
        in_buff_end = in_buff;
        out_buff = new char [buff_size];
        setg(out_buff, out_buff, out_buff);
    }

    istreambuf(const istreambuf &) = delete;
    istreambuf(istreambuf &&) = default;
    istreambuf & operator = (const istreambuf &) = delete;
    istreambuf & operator = (istreambuf &&) = default;

    virtual ~istreambuf()
    {
        delete [] in_buff;
        delete [] out_buff;
        if (lzmastrm_p) delete lzmastrm_p;
    }

    virtual std::streambuf::int_type underflow()
    {
        if (this->gptr() == this->egptr())
        {
            // pointers for free region in output buffer
            char * out_buff_free_start = out_buff;
            do
            {
                // read more input if none available
                if (in_buff_start == in_buff_end)
                {
                    // empty input buffer: refill from the start
                    in_buff_start = in_buff;
                    std::streamsize sz = sbuf_p->sgetn(in_buff, buff_size);
                    in_buff_end = in_buff + sz;
                    if (in_buff_end == in_buff_start) break; // end of input
                }
                // auto detect if the stream contains text or deflate data
                if (auto_detect && ! auto_detect_run)
                {
                    auto_detect_run = true;
                    unsigned char b0 = *reinterpret_cast< unsigned char * >(in_buff_start);
                    unsigned char b1 = *reinterpret_cast< unsigned char * >(in_buff_start + 1);
                    unsigned char b2 = *reinterpret_cast< unsigned char * >(in_buff_start + 2);
                    unsigned char b3 = *reinterpret_cast< unsigned char * >(in_buff_start + 3);
                    unsigned char b4 = *reinterpret_cast< unsigned char * >(in_buff_start + 4);
                    unsigned char b5 = *reinterpret_cast< unsigned char * >(in_buff_start + 5);
                    // Ref:
		    // https://tukaani.org/xz/xz-file-format.txt
		    // subsection 2.1.1.1.
                    is_text = ! (in_buff_start + 6 <= in_buff_end
                                 && ((b0 == 0xFD && b1 == 0x37 && b2 == 0x7A
				      && b3 == 0x58 && b4 == 0x5A && b5 == 0x00))); // liblzma header
                }
                if (is_text)
                {
                    // simply swap in_buff and out_buff, and adjust pointers
                    assert(in_buff_start == in_buff);
                    std::swap(in_buff, out_buff);
                    out_buff_free_start = in_buff_end;
                    in_buff_start = in_buff;
                    in_buff_end = in_buff;
                }
                else
                {
                    // run inflate() on input
                    if (! lzmastrm_p) lzmastrm_p = new detail::lzma_stream_wrapper(true);
                    lzmastrm_p->next_in = reinterpret_cast< decltype(lzmastrm_p->next_in) >(in_buff_start);
                    lzmastrm_p->avail_in = in_buff_end - in_buff_start;
                    lzmastrm_p->next_out = reinterpret_cast< decltype(lzmastrm_p->next_out) >(out_buff_free_start);
                    lzmastrm_p->avail_out = (out_buff + buff_size) - out_buff_free_start;
                    lzma_ret ret = lzma_code(lzmastrm_p, LZMA_RUN);
                    // process return code
                    if (ret != LZMA_OK && ret != LZMA_STREAM_END) throw lzmaException(lzmastrm_p, ret);
                    // update in&out pointers following inflate()
		    auto tmp = const_cast< unsigned char* >(lzmastrm_p->next_in); // cast away const qualifiers
                    in_buff_start = reinterpret_cast< decltype(in_buff_start) >(tmp);
                    in_buff_end = in_buff_start + lzmastrm_p->avail_in;
                    out_buff_free_start = reinterpret_cast< decltype(out_buff_free_start) >(lzmastrm_p->next_out);
                    assert(out_buff_free_start + lzmastrm_p->avail_out == out_buff + buff_size);
                    // if stream ended, deallocate inflator
                    if (ret == LZMA_STREAM_END)
                    {
                        delete lzmastrm_p;
                        lzmastrm_p = nullptr;
                    }
                }
            } while (out_buff_free_start == out_buff);
            // 2 exit conditions:
            // - end of input: there might or might not be output available
            // - out_buff_free_start != out_buff: output available
            this->setg(out_buff, out_buff, out_buff_free_start);
        }
        return this->gptr() == this->egptr()
            ? traits_type::eof()
            : traits_type::to_int_type(*this->gptr());
    }
private:
    std::streambuf * sbuf_p;
    char * in_buff;
    char * in_buff_start;
    char * in_buff_end;
    char * out_buff;
    detail::lzma_stream_wrapper * lzmastrm_p;
    std::size_t buff_size;
    bool auto_detect;
    bool auto_detect_run;
    bool is_text;

    static const std::size_t default_buff_size = (std::size_t)1 << 20;
}; // class istreambuf

class ostreambuf
    : public std::streambuf
{
public:
    ostreambuf(std::streambuf * _sbuf_p,
               std::size_t _buff_size = default_buff_size, int _level = 2)
        : sbuf_p(_sbuf_p),
          lzmastrm_p(new detail::lzma_stream_wrapper(false, _level)),
          buff_size(_buff_size)
    {
        assert(sbuf_p);
        in_buff = new char [buff_size];
        out_buff = new char [buff_size];
        setp(in_buff, in_buff + buff_size);
    }

    ostreambuf(const ostreambuf &) = delete;
    ostreambuf(ostreambuf &&) = default;
    ostreambuf & operator = (const ostreambuf &) = delete;
    ostreambuf & operator = (ostreambuf &&) = default;

    int deflate_loop(lzma_action action)
    {
        while (true)
        {
            lzmastrm_p->next_out = reinterpret_cast< decltype(lzmastrm_p->next_out) >(out_buff);
            lzmastrm_p->avail_out = buff_size;
            lzma_ret ret = lzma_code(lzmastrm_p, action);
            if (ret != LZMA_OK && ret != LZMA_STREAM_END && ret != LZMA_BUF_ERROR) throw lzmaException(lzmastrm_p, ret);
            std::streamsize sz = sbuf_p->sputn(out_buff, reinterpret_cast< decltype(out_buff) >(lzmastrm_p->next_out) - out_buff);
            if (sz != reinterpret_cast< decltype(out_buff) >(lzmastrm_p->next_out) - out_buff)
            {
                // there was an error in the sink stream
                return -1;
            }
            if (ret == LZMA_STREAM_END || ret == LZMA_BUF_ERROR || sz == 0)
            {
                break;
            }
        }
        return 0;
    }

    virtual ~ostreambuf()
    {
        // flush the lzma stream
        //
        // NOTE: Errors here (sync() return value not 0) are ignored, because we
        // cannot throw in a destructor. This mirrors the behaviour of
        // std::basic_filebuf::~basic_filebuf(). To see an exception on error,
        // close the ofstream with an explicit call to close(), and do not rely
        // on the implicit call in the destructor.
        //
        sync();
        delete [] in_buff;
        delete [] out_buff;
        delete lzmastrm_p;
    }
    virtual std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof())
    {
        lzmastrm_p->next_in = reinterpret_cast< decltype(lzmastrm_p->next_in) >(pbase());
        lzmastrm_p->avail_in = pptr() - pbase();
        while (lzmastrm_p->avail_in > 0)
        {
            int r = deflate_loop(LZMA_RUN);
            if (r != 0)
            {
                setp(nullptr, nullptr);
                return traits_type::eof();
            }
        }
        setp(in_buff, in_buff + buff_size);
        return traits_type::eq_int_type(c, traits_type::eof()) ? traits_type::eof() : sputc(c);
    }
    virtual int sync()
    {
        // first, call overflow to clear in_buff
        overflow();
        if (! pptr()) return -1;
        // then, call deflate asking to finish the zlib stream
        lzmastrm_p->next_in = nullptr;
        lzmastrm_p->avail_in = 0;
        if (deflate_loop(LZMA_FINISH) != 0) return -1;
	delete lzmastrm_p;
	lzmastrm_p = new detail::lzma_stream_wrapper(false);
        return 0;
    }
private:
    std::streambuf * sbuf_p;
    char * in_buff;
    char * out_buff;
    detail::lzma_stream_wrapper * lzmastrm_p;
    std::size_t buff_size;

    static const std::size_t default_buff_size = (std::size_t)1 << 20;
}; // class ostreambuf

class istream
    : public std::istream
{
public:
    istream(std::istream & is)
        : std::istream(new istreambuf(is.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    explicit istream(std::streambuf * sbuf_p)
        : std::istream(new istreambuf(sbuf_p))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~istream()
    {
        delete rdbuf();
    }
}; // class istream

class ostream
    : public std::ostream
{
public:
    ostream(std::ostream & os)
        : std::ostream(new ostreambuf(os.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    explicit ostream(std::streambuf * sbuf_p)
        : std::ostream(new ostreambuf(sbuf_p))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~ostream()
    {
        delete rdbuf();
    }
}; // class ostream

namespace detail
{

template < typename FStream_Type >
struct strict_fstream_holder
{
    strict_fstream_holder() {};
    strict_fstream_holder(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in)
        : _fs(filename, mode)
    {}
    FStream_Type _fs;
}; // class strict_fstream_holder

} // namespace detail

class ifstream
    : private detail::strict_fstream_holder< strict_fstream::ifstream >,
      public std::istream
{
public:
    ifstream() : std::istream(new istreambuf(_fs.rdbuf())) {}
    explicit ifstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in)
        : detail::strict_fstream_holder< strict_fstream::ifstream >(filename, mode),
          std::istream(new istreambuf(_fs.rdbuf()))
    {
        this->setstate(_fs.rdstate());
        exceptions(std::ios_base::badbit);
    }
    virtual ~ifstream()
    {
        if (rdbuf()) delete rdbuf();
    }
}; // class ifstream

class ofstream
    : private detail::strict_fstream_holder< strict_fstream::ofstream >,
      public std::ostream
{
public:
    explicit ofstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out)
        : detail::strict_fstream_holder< strict_fstream::ofstream >(filename, mode | std::ios_base::binary),
          std::ostream(new ostreambuf(_fs.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~ofstream()
    {
        if (rdbuf()) delete rdbuf();
    }
}; // class ofstream

} // namespace lzmastr

#endif
