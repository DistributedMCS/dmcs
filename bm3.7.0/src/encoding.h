#ifndef BMENCODING_H__INCLUDED__
#define BMENCODING_H__INCLUDED__
/*
Copyright(c) 2002-2009 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)


Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.


For more information please visit:   http://bmagic.sourceforge.net

*/

#include <memory.h>
#include "bmutil.h"

namespace bm
{


// ----------------------------------------------------------------
/*!
   \brief Memory encoding.
   
   Class for encoding data into memory. 
   Properly handles aligment issues with integer data types.
*/
class encoder
{
public:
    typedef unsigned char* position_type;
public:
    encoder(unsigned char* buf, unsigned size);
    void put_8(unsigned char c);
    void put_16(bm::short_t  s);
    void put_16(const bm::short_t* s, unsigned count);
    void put_32(bm::word_t  w);
    void put_32(const bm::word_t* w, unsigned count);
    void put_prefixed_array_32(unsigned char c, 
                               const bm::word_t* w, unsigned count);
    void put_prefixed_array_16(unsigned char c, 
                               const bm::short_t* s, unsigned count,
                               bool encode_count);
    unsigned size() const;
    unsigned char* get_pos() const;
    void set_pos(unsigned char* buf_pos);
private:
    unsigned char*  buf_;
    unsigned char*  start_;
    unsigned int    size_;
};

// ----------------------------------------------------------------
/**
    Base class for all decoding functionality
*/
class decoder_base
{
public:
    decoder_base(const unsigned char* buf) { buf_ = start_ = buf; }
    /// Reads character from the decoding buffer. 
    BMFORCEINLINE unsigned char get_8() { return *buf_++; }
    /// Returns size of the current decoding stream.
    BMFORCEINLINE 
    unsigned size() const { return (unsigned)(buf_ - start_); }
    /// change current position
    BMFORCEINLINE
    void seek(int delta) { buf_ += delta; }
protected:
   const unsigned char*   buf_;
   const unsigned char*   start_;
};


// ----------------------------------------------------------------
/**
   Class for decoding data from memory buffer.
   Properly handles aligment issues with integer data types.
*/
class decoder : public decoder_base
{
public:
    decoder(const unsigned char* buf);
    bm::short_t get_16();
    bm::word_t get_32();
    void get_32(bm::word_t* w, unsigned count);
    void get_16(bm::short_t* s, unsigned count);
};

// ----------------------------------------------------------------
/**
   Class for decoding data from memory buffer.
   Properly handles aligment issues with integer data types.
   Converts data to big endian architecture 
   (presumed it was encoded as little endian)
*/
typedef decoder decoder_big_endian;


// ----------------------------------------------------------------
/**
   Class for decoding data from memory buffer.
   Properly handles aligment issues with integer data types.
   Converts data to little endian architecture 
   (presumed it was encoded as big endian)
*/
class decoder_little_endian : public decoder_base
{
public:
    decoder_little_endian(const unsigned char* buf);
    bm::short_t get_16();
    bm::word_t get_32();
    void get_32(bm::word_t* w, unsigned count);
    void get_16(bm::short_t* s, unsigned count);
};


/** 
    Byte based writer for un-aligned bit streaming 

    @sa encoder
*/
template<class TEncoder>
class bit_out
{
public:
    bit_out(TEncoder& dest)
        : dest_(dest), used_bits_(0), accum_(0)
    {}

    ~bit_out()
    {
        if (used_bits_)
            dest_.put_32(accum_);
    }

    void put_bit(unsigned value)
    {
        BM_ASSERT(value <= 1);
        accum_ |= (value << used_bits_);
        if (++used_bits_ == (sizeof(accum_) * 8))
            flush_accum();
    }

    void put_bits(unsigned value, unsigned count)
    {
        unsigned used = used_bits_;
        unsigned acc = accum_;

        {
            unsigned mask = ~0;
            mask >>= (sizeof(accum_) * 8) - count;
            value &= mask;
        }
        for (;count;)
        {  
            acc |= value << used;

            unsigned free_bits = (sizeof(accum_) * 8) - used;
            if (count <= free_bits)
            {
                used += count;
                break;
            }
            else
            {
                value >>= free_bits;
                count -= free_bits;
                dest_.put_32(acc);
                acc = used = 0;
                continue;
            }
        }
        used_bits_ = used;
        accum_ = acc;
    }

    void put_zero_bit()
    {
        if (++used_bits_ == (sizeof(accum_) * 8))
            flush_accum();        
    }

    void put_zero_bits(register unsigned count)
    {
        register unsigned used = used_bits_;
        unsigned free_bits = (sizeof(accum_) * 8) - used;
        if (count >= free_bits)
        {
            flush_accum();
            count -= free_bits;
            used = 0;

            for ( ;count >= sizeof(accum_) * 8; count -= sizeof(accum_) * 8)
            {
                dest_.put_32(0);
            }
            used += count; 
        }
        else
        {
            used += count;
        }
        accum_ |= (1 << used);
        if (++used == (sizeof(accum_) * 8))
            flush_accum();
        else
            used_bits_ = used;
    }


    void gamma(unsigned value)
    {
        BM_ASSERT(value);

        unsigned logv = 
        #if defined(BM_x86) && (defined(__GNUG__) || defined(_MSC_VER))
            bm::bsr_asm32(value);
        #else
            bm::ilog2_LUT(value);
        #endif

        // Put zeroes + 1 bit

        unsigned used = used_bits_;
        unsigned acc = accum_;
        const unsigned acc_bits = (sizeof(acc) * 8);
        unsigned free_bits = acc_bits - used;

        {
        unsigned count = logv;
        if (count >= free_bits)
        {
            dest_.put_32(acc);
            acc = used ^= used;
            count -= free_bits;

            for ( ;count >= acc_bits; count -= acc_bits)
            {
                dest_.put_32(0);
            }
            used += count; 
        }
        else
        {
            used += count;
        }
        acc |= (1 << used);
        if (++used == acc_bits)
        {
            dest_.put_32(acc);
            acc = used ^= used;
        }
        }

        // Put the value bits
        //
        {
            unsigned mask = (~0u);
            mask >>= acc_bits - logv;
            value &= mask;
        }
        for (;logv;)
        {  
            acc |= value << used;
            free_bits = acc_bits - used;
            if (logv <= free_bits)
            {
                used += logv;
                break;
            }
            else
            {
                value >>= free_bits;
                logv -= free_bits;
                dest_.put_32(acc);
                acc = used ^= used;
                continue;
            }
        } // for

        used_bits_ = used;
        accum_ = acc;
    }


    void flush()
    {
        if (used_bits_)
            flush_accum();
    }

private:
    void flush_accum()
    {
        dest_.put_32(accum_);
        used_bits_ = accum_ = 0;
    }
private:
    bit_out(const bit_out&);
    bit_out& operator=(const bit_out&);

private:
    TEncoder&      dest_;      ///< Bit stream target
    unsigned       used_bits_; ///< Bits used in the accumulator
    unsigned       accum_;     ///< write bit accumulator 
};


/** 
    Byte based reader for un-aligned bit streaming 

    @sa encoder
*/
template<class TDecoder>
class bit_in
{
public:
    bit_in(TDecoder& decoder)
        : src_(decoder),
          used_bits_(sizeof(accum_) * 8),
          accum_(0)
    {
    }

    unsigned gamma()
    {
        unsigned acc = accum_;
        unsigned used = used_bits_;

        if (used == (sizeof(acc) * 8))
        {
            acc = src_.get_32();
            used ^= used;
        }
        unsigned zero_bits = 0;
        while (true)
        {
            if (acc == 0)
            {
                zero_bits += (sizeof(acc) * 8) - used;
                used = 0;
                acc = src_.get_32();
                continue;
            }
            unsigned first_bit_idx = 
                #if defined(BM_x86) && (defined(__GNUG__) || defined(_MSC_VER))
                    bm::bsf_asm32(acc);
                #else
                    bm::bit_scan_fwd(acc);
                #endif
            acc >>= first_bit_idx;
            zero_bits += first_bit_idx;
            used += first_bit_idx;
            break;
        } // while

        // eat the border bit
        //
        if (used == (sizeof(acc) * 8))
        {
            acc = src_.get_32();
            used = 1;
        }
        else
        {
            ++used;
        }
        acc >>= 1;

        // get the value
        unsigned current;
        
        unsigned free_bits = (sizeof(acc) * 8) - used;
        if (zero_bits <= free_bits)
        {
        take_accum:
            current = 
                (acc & block_set_table<true>::_left[zero_bits]) | (1 << zero_bits);
            acc >>= zero_bits;
            used += zero_bits;
            goto ret;
        }

        if (used == (sizeof(acc) * 8))
        {
            acc = src_.get_32();
            used ^= used;
            goto take_accum;
        }

        // take the part
        current = acc;
        // read the next word
        acc = src_.get_32();
        used = zero_bits - free_bits;
        current |= 
            ((acc & block_set_table<true>::_left[used]) << free_bits) | 
            (1 << zero_bits);

        acc >>= used;
    ret:
        accum_ = acc;
        used_bits_ = used;
        return current;
    }


private:
    bit_in(const bit_in&);
    bit_in& operator=(const bit_in&);
private:
    TDecoder&           src_;        ///< Source of bytes
    unsigned            used_bits_;  ///< Bits used in the accumulator
    unsigned            accum_;      ///< read bit accumulator
};


/**
    Functor for Elias Gamma encoding
*/
template<typename T, typename TBitIO>
class gamma_encoder
{
public:
    gamma_encoder(TBitIO& bout) : bout_(bout) 
    {}
        
    /**
        Encode word
    */
    BMFORCEINLINE
    void operator()(T value)
    {
        bout_.gamma(value);
    }
private:
    gamma_encoder(const gamma_encoder&);
    gamma_encoder& operator=(const gamma_encoder&);
private:
    TBitIO&  bout_;
};


/**
    Elias Gamma decoder
*/
template<typename T, typename TBitIO>
class gamma_decoder
{
public:
    gamma_decoder(TBitIO& bin) : bin_(bin) 
    {}
    
    /**
        Start encoding sequence
    */
    void start()
    {}
    
    /**
        Stop decoding sequence
    */
    void stop()
    {}
    
    /**
        Decode word
    */
    T operator()(void)
    {
        return (T)bin_.gamma();
    }
private:
    gamma_decoder(const gamma_decoder&);
    gamma_decoder& operator=(const gamma_decoder&);
private:
    TBitIO&  bin_;
};


// ----------------------------------------------------------------
// Implementation details. 
// ----------------------------------------------------------------

/*! 
    \fn encoder::encoder(unsigned char* buf, unsigned size) 
    \brief Construction.
    \param buf - memory buffer pointer.
    \param size - size of the buffer
*/
inline encoder::encoder(unsigned char* buf, unsigned size)
: buf_(buf), start_(buf), size_(size)
{
}
/*!
    \grief Encode 8-bit prefix + an array
*/
inline void encoder::put_prefixed_array_32(unsigned char c, 
                                           const bm::word_t* w, 
                                           unsigned count)
{
    put_8(c);
    put_32(w, count);
}

/*!
    \grief Encode 8-bit prefix + an array 
*/
inline void encoder::put_prefixed_array_16(unsigned char c, 
                                           const bm::short_t* s, 
                                           unsigned count,
                                           bool encode_count)
{
    put_8(c);
    if (encode_count)
        put_16((bm::short_t) count);
    put_16(s, count);
}


/*!
   \fn void encoder::put_8(unsigned char c) 
   \brief Puts one character into the encoding buffer.
   \param c - character to encode
*/
BMFORCEINLINE void encoder::put_8(unsigned char c)
{
    *buf_++ = c;
}

/*!
   \fn encoder::put_16(bm::short_t s)
   \brief Puts short word (16 bits) into the encoding buffer.
   \param s - short word to encode
*/
BMFORCEINLINE void encoder::put_16(bm::short_t s)
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
	*((bm::short_t*)buf_) = s;
	buf_ += sizeof(s);
#else
    *buf_++ = (unsigned char) s;
    s >>= 8;
    *buf_++ = (unsigned char) s;
#endif
}

/*!
   \brief Method puts array of short words (16 bits) into the encoding buffer.
*/
inline void encoder::put_16(const bm::short_t* s, unsigned count)
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
    unsigned short* buf = (unsigned short*)buf_;
    const bm::short_t* s_end = s + count;
    do 
    {
		*buf++ = *s++;
    } while (s < s_end);
		
	buf_ = (unsigned char*)buf;
#else
    unsigned char* buf = buf_;
    const bm::short_t* s_end = s + count;
    do 
    {
        bm::short_t w16 = *s++;
        unsigned char a = (unsigned char)  w16;
        unsigned char b = (unsigned char) (w16 >> 8);
        
        *buf++ = a;
        *buf++ = b;
                
    } while (s < s_end);
    
    buf_ = (unsigned char*)buf;
#endif
}


/*!
   \fn unsigned encoder::size() const
   \brief Returns size of the current encoding stream.
*/
inline unsigned encoder::size() const
{
    return (unsigned)(buf_ - start_);
}

/**
    \brief Get current memory stream position
*/
inline encoder::position_type encoder::get_pos() const
{
    return buf_;
}

/**
    \brief Set current memory stream position
*/
inline void encoder::set_pos(encoder::position_type buf_pos)
{
    buf_ = buf_pos;
}


/*!
   \fn void encoder::put_32(bm::word_t w)
   \brief Puts 32 bits word into encoding buffer.
   \param w - word to encode.
*/
BMFORCEINLINE void encoder::put_32(bm::word_t w)
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
	*((bm::word_t*) buf_) = w;
	buf_ += sizeof(w);
#else
    *buf_++ = (unsigned char) w;
    *buf_++ = (unsigned char) (w >> 8);
    *buf_++ = (unsigned char) (w >> 16);
    *buf_++ = (unsigned char) (w >> 24);
#endif
}

/*!
    \brief Encodes array of 32-bit words
*/
inline 
void encoder::put_32(const bm::word_t* w, unsigned count)
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
	bm::word_t* buf = (bm::word_t*)buf_;
    const bm::word_t* w_end = w + count;
    do 
    {
		*buf++ = *w++;
    } while (w < w_end);
    
    buf_ = (unsigned char*)buf;
#else
    unsigned char* buf = buf_;
    const bm::word_t* w_end = w + count;
    do 
    {
        bm::word_t w32 = *w++;
        unsigned char a = (unsigned char) w32;
        unsigned char b = (unsigned char) (w32 >> 8);
        unsigned char c = (unsigned char) (w32 >> 16);
        unsigned char d = (unsigned char) (w32 >> 24);

        *buf++ = a;
        *buf++ = b;
        *buf++ = c;
        *buf++ = d;
    } while (w < w_end);
    
    buf_ = (unsigned char*)buf;
#endif
}


// ---------------------------------------------------------------------

/*!
   \fn decoder::decoder(const unsigned char* buf) 
   \brief Construction
   \param buf - pointer to the decoding memory. 
*/
inline decoder::decoder(const unsigned char* buf) 
: decoder_base(buf)
{
}

/*!
   \fn bm::short_t decoder::get_16()
   \brief Reads 16bit word from the decoding buffer.
*/
BMFORCEINLINE bm::short_t decoder::get_16() 
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
	bm::short_t a = *((bm::short_t*)buf_);
#else
    bm::short_t a = (bm::short_t)(buf_[0] + ((bm::short_t)buf_[1] << 8));
#endif
	buf_ += sizeof(a);
    return a;
}

/*!
   \fn bm::word_t decoder::get_32()
   \brief Reads 32 bit word from the decoding buffer.
*/
BMFORCEINLINE bm::word_t decoder::get_32() 
{
#if (BM_UNALIGNED_ACCESS_OK == 1)
	bm::word_t a = *((bm::word_t*)buf_);
#else
	bm::word_t a = buf_[0]+ ((unsigned)buf_[1] << 8) +
                   ((unsigned)buf_[2] << 16) + ((unsigned)buf_[3] << 24);
#endif
    buf_+=sizeof(a);
    return a;
}


/*!
   \fn void decoder::get_32(bm::word_t* w, unsigned count)
   \brief Reads block of 32-bit words from the decoding buffer.
   \param w - pointer on memory block to read into.
   \param count - size of memory block in words.
*/
inline void decoder::get_32(bm::word_t* w, unsigned count)
{
    if (!w) 
    {
        seek(count * 4);
        return;
    }
#if (BM_UNALIGNED_ACCESS_OK == 1)
	memcpy(w, buf_, count * sizeof(bm::word_t));
	seek(count * 4);
	return;
#else
    const unsigned char* buf = buf_;
    const bm::word_t* w_end = w + count;
    do 
    {
        bm::word_t a = buf[0]+ ((unsigned)buf[1] << 8) +
                   ((unsigned)buf[2] << 16) + ((unsigned)buf[3] << 24);
        *w++ = a;
        buf += sizeof(a);
    } while (w < w_end);
    buf_ = (unsigned char*)buf;
#endif
}

/*!
   \fn void decoder::get_16(bm::short_t* s, unsigned count)
   \brief Reads block of 32-bit words from the decoding buffer.
   \param s - pointer on memory block to read into.
   \param count - size of memory block in words.
*/
inline void decoder::get_16(bm::short_t* s, unsigned count)
{
    if (!s) 
    {
        seek(count * 2);
        return;
    }
#if (BM_UNALIGNED_ACCESS_OK == 1)
	const bm::short_t* buf = (bm::short_t*)buf_;
    const bm::short_t* s_end = s + count;
    do 
    {
        *s++ = *buf++;
    } while (s < s_end);
#else
    const unsigned char* buf = buf_;
    const bm::short_t* s_end = s + count;
    do 
    {
        bm::short_t a = (bm::short_t)(buf[0] + ((bm::short_t)buf[1] << 8));
        *s++ = a;
        buf += sizeof(a);
    } while (s < s_end);
#endif
    buf_ = (unsigned char*)buf;
}



// ---------------------------------------------------------------------

inline decoder_little_endian::decoder_little_endian(const unsigned char* buf)
: decoder_base(buf)
{
}

BMFORCEINLINE bm::short_t decoder_little_endian::get_16()
{
    bm::short_t a = ((bm::short_t)buf_[0] << 8) + ((bm::short_t)buf_[1]);
    buf_ += sizeof(a);
    return a;
}

BMFORCEINLINE bm::word_t decoder_little_endian::get_32() 
{
    bm::word_t a = ((unsigned)buf_[0] << 24)+ ((unsigned)buf_[1] << 16) +
                   ((unsigned)buf_[2] << 8) + ((unsigned)buf_[3]);
    buf_+=sizeof(a);
    return a;
}

inline void decoder_little_endian::get_32(bm::word_t* w, unsigned count)
{
    if (!w) 
    {
        seek(count * 4);
        return;
    }

    const unsigned char* buf = buf_;
    const bm::word_t* w_end = w + count;
    do 
    {
        bm::word_t a = ((unsigned)buf[0] << 24)+ ((unsigned)buf[1] << 16) +
                       ((unsigned)buf[2] << 8) + ((unsigned)buf[3]);
        *w++ = a;
        buf += sizeof(a);
    } while (w < w_end);
    buf_ = (unsigned char*)buf;
}

inline void decoder_little_endian::get_16(bm::short_t* s, unsigned count)
{
    if (!s) 
    {
        seek(count * 2);
        return;
    }

    const unsigned char* buf = buf_;
    const bm::short_t* s_end = s + count;
    do 
    {
        bm::short_t a = ((bm::short_t)buf[0] << 8) + ((bm::short_t)buf[1]);
        *s++ = a;
        buf += sizeof(a);
    } while (s < s_end);
    buf_ = (unsigned char*)buf;
}


} // namespace bm

#endif
