// Adapted from zstr (https://github.com/mateidavid/zstr)
// zstr was written by Matei David (matei@cs.toronto.edu)

#ifndef __BZSTR_HPP
#define __BZSTR_HPP

#include <cassert>
#include <fstream>
#include <sstream>
#include <bzlib.h>
#include "strict_fstream.hpp"

namespace bzstr
{

/// Exception class thrown by failed bzlib operations.
class bzException
    : public std::exception
{
public:
    bzException(bz_stream * bzstrm_p, int ret)
        : _msg("bzlib: ")
    {
        switch (ret)
        {
        case BZ_CONFIG_ERROR:
            _msg += "BZ_CONFIG_ERROR: ";
            break;
        case BZ_SEQUENCE_ERROR:
            _msg += "BZ_SEQUENCE_ERROR: ";
            break;
        case BZ_PARAM_ERROR:
            _msg += "BZ_PARAM_ERROR: ";
            break;
        case BZ_MEM_ERROR:
            _msg += "BZ_MEM_ERROR: ";
            break;
        case BZ_DATA_ERROR:
            _msg += "BZ_DATA_ERROR: ";
            break;
        case BZ_DATA_ERROR_MAGIC:
            _msg += "BZ_DATA_ERROR_MAGIC: ";
            break;
        case BZ_IO_ERROR:
            _msg += "BZ_IO_ERROR: ";
            break;
        case BZ_UNEXPECTED_EOF:
            _msg += "BZ_UNEXPECTED_EOF: ";
            break;
        case BZ_OUTBUFF_FULL:
            _msg += "BZ_OUTBUFF_FULL: ";
            break;
        default:
            std::ostringstream oss;
            oss << ret;
            _msg += "[" + oss.str() + "]: ";
            break;
        }
        _msg += ret;
    }
    bzException(const std::string msg) : _msg(msg) {}
    const char * what() const noexcept { return _msg.c_str(); }
private:
    std::string _msg;
}; // class bzException

namespace detail
{
    class bz_stream_wrapper : public bz_stream
    {
    public:
	bz_stream_wrapper(bool _is_input = true, int _level = 9, int _wf = 30)
	    : is_input(_is_input)
	{
	    this->bzalloc = NULL;
	    this->bzfree = NULL;
	    this->opaque = NULL;
	    int ret;
	    if (is_input)
		{
		    this->avail_in = 0;
		    this->next_in = NULL;
		    ret = BZ2_bzDecompressInit(this, 0, 0);
		}
	    else
		{
		    ret = BZ2_bzCompressInit(this, _level, 0, _wf);
		}
	    if (ret != BZ_OK) {
		throw bzException(this, ret);
	    }
	}
	~bz_stream_wrapper()
	{
	    if (is_input)
		{
		    BZ2_bzDecompressEnd(this);
		}
	    else
		{
		    BZ2_bzCompressEnd(this);
		}
	}
    private:
	bool is_input;
    }; // class bz2_stream_wrapper
} // namespace detail

class istreambuf
    : public std::streambuf
{
public:
    istreambuf(std::streambuf * _sbuf_p,
               std::size_t _buff_size = default_buff_size, bool _auto_detect = true)
        : sbuf_p(_sbuf_p),
          bzstrm_p(nullptr),
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
        if (bzstrm_p) delete bzstrm_p;
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
                    // Ref:
		    // https://en.wikipedia.org/wiki/Bzip2
e
                    is_text = ! (in_buff_start + 2 <= in_buff_end
                                 && ((b0 == 0x42 && b1 == 0x5a && b2 == 0x68))); // bz2 header
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
                    if (! bzstrm_p) bzstrm_p = new detail::bz_stream_wrapper(true);
                    bzstrm_p->next_in = reinterpret_cast< decltype(bzstrm_p->next_in) >(in_buff_start);
                    bzstrm_p->avail_in = in_buff_end - in_buff_start;
                    bzstrm_p->next_out = reinterpret_cast< decltype(bzstrm_p->next_out) >(out_buff_free_start);
                    bzstrm_p->avail_out = (out_buff + buff_size) - out_buff_free_start;
                    int ret = BZ2_bzDecompress(bzstrm_p);
                    // process return code
                    if (ret != BZ_OK && ret != BZ_STREAM_END) {
			throw bzException(bzstrm_p, ret);
		    }
                    // update in&out pointers following inflate()
                    in_buff_start = reinterpret_cast< decltype(in_buff_start) >(bzstrm_p->next_in);
                    in_buff_end = in_buff_start + bzstrm_p->avail_in;
                    out_buff_free_start = reinterpret_cast< decltype(out_buff_free_start) >(bzstrm_p->next_out);
                    assert(out_buff_free_start + bzstrm_p->avail_out == out_buff + buff_size);
                    // if stream ended, deallocate inflator
                    if (ret == BZ_STREAM_END)
                    {
                        delete bzstrm_p;
                        bzstrm_p = nullptr;
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
    detail::bz_stream_wrapper * bzstrm_p;
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
               std::size_t _buff_size = default_buff_size, int _level = 9, int _wf = 30)
        : sbuf_p(_sbuf_p),
          bzstrm_p(new detail::bz_stream_wrapper(false, _level, _wf)),
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

    int deflate_loop(int flush)
    {
        while (true)
        {
            bzstrm_p->next_out = reinterpret_cast< decltype(bzstrm_p->next_out) >(out_buff);
            bzstrm_p->avail_out = buff_size;
            int ret = BZ2_bzCompress(bzstrm_p, flush);
	    if (!ret) {
		throw bzException(bzstrm_p, ret);
	    }
            std::streamsize sz = sbuf_p->sputn(out_buff, reinterpret_cast< decltype(out_buff) >(bzstrm_p->next_out) - out_buff);
            if (sz != reinterpret_cast< decltype(out_buff) >(bzstrm_p->next_out) - out_buff)
            {
                // there was an error in the sink stream
                return -1;
            }
            if (ret == BZ_STREAM_END || sz == 0)
            {
                break;
            }
        }
        return 0;
    }

    virtual ~ostreambuf()
    {
        // flush the bzlib stream
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
        delete bzstrm_p;
    }
    virtual std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof())
    {
        bzstrm_p->next_in = reinterpret_cast< decltype(bzstrm_p->next_in) >(pbase());
        bzstrm_p->avail_in = pptr() - pbase();
        while (bzstrm_p->avail_in > 0)
        {
            int r = deflate_loop(BZ_RUN);
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
        // then, call deflate asking to finish the bzlib stream
        bzstrm_p->next_in = nullptr;
        bzstrm_p->avail_in = 0;
        if (deflate_loop(BZ_FINISH) != 0) return -1;
	delete bzstrm_p;
	bzstrm_p = new detail::bz_stream_wrapper(false);
        return 0;
    }
private:
    std::streambuf * sbuf_p;
    char * in_buff;
    char * out_buff;
    detail::bz_stream_wrapper * bzstrm_p;
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

} // namespace bzstr

#endif
