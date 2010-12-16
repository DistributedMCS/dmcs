#ifndef BM_BLOCKS__H__INCLUDED__
#define BM_BLOCKS__H__INCLUDED__
/*
Copyright(c) 2002-2010 Anatoliy Kuznetsov(anatoliy_kuznetsov at yahoo.com)

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

For more information please visit:  http://bmagic.sourceforge.net

*/


#include "bmfwd.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100)
#endif


namespace bm
{


/*!
   @brief bitvector blocks manager
        Embedded class managing bit-blocks on very low level.
        Includes number of functor classes used in different bitset algorithms. 
   @ingroup bvector
   @internal
*/

template<class Alloc/*, class MS*/> 
class blocks_manager
{
public:

    typedef Alloc allocator_type;

    /** Base functor class (block visitor)*/
    class bm_func_base
    {
    public:
        bm_func_base(blocks_manager& bman) : bm_(bman) {}

        void on_empty_top(unsigned /* top_block_idx*/ ) {}
        void on_empty_block(unsigned /* block_idx*/ ) {}
    private:
        bm_func_base(const bm_func_base&);
        bm_func_base& operator=(const bm_func_base&);
    protected:
        blocks_manager&  bm_;
    };


    /** Base functor class connected for "constant" functors*/
    class bm_func_base_const
    {
    public:
        bm_func_base_const(const blocks_manager& bman) : bm_(bman) {}

        void on_empty_top(unsigned /* top_block_idx*/ ) {}
        void on_empty_block(unsigned /* block_idx*/ ) {}
    private:
        bm_func_base_const(const bm_func_base_const&);
        bm_func_base_const& operator=(const bm_func_base_const&);
    protected:
        const blocks_manager&  bm_;
    };


    /** Base class for bitcounting functors */
    class block_count_base : public bm_func_base_const
    {
    protected:
        block_count_base(const blocks_manager& bm) 
            : bm_func_base_const(bm) {}

        bm::id_t block_count(const bm::word_t* block) const
        {
            return this->bm_.block_bitcount(block);
        }
    };


    /** Bitcounting functor */
    class block_count_func : public block_count_base
    {
    public:
        block_count_func(const blocks_manager& bm) 
            : block_count_base(bm), count_(0) {}

        bm::id_t count() const { return count_; }

        void operator()(const bm::word_t* block)
        {
            count_ += this->block_count(block);
        }

    private:
        bm::id_t count_;
    };


    /** Bitcounting functor filling the block counts array*/
    class block_count_arr_func : public block_count_base
    {
    public:
        block_count_arr_func(const blocks_manager& bm, unsigned* arr) 
            : block_count_base(bm), arr_(arr), last_idx_(0) 
        {
            arr_[0] = 0;
        }

        void operator()(const bm::word_t* block, unsigned idx)
        {
            while (++last_idx_ < idx)
            {
                arr_[last_idx_] = 0;
            }
            arr_[idx] = this->block_count(block);
            last_idx_ = idx;
        }

        unsigned last_block() const { return last_idx_; }

    private:
        unsigned*  arr_;
        unsigned   last_idx_;
    };

    /** bit value change counting functor */
    class block_count_change_func : public bm_func_base_const
    {
    public:
        block_count_change_func(const blocks_manager& bm) 
            : bm_func_base_const(bm),
                count_(0),
                prev_block_border_bit_(0)
        {}

        bm::id_t block_count(const bm::word_t* block, unsigned idx)
        {
            bm::id_t count = 0;
            bm::id_t first_bit;
            
            if (IS_FULL_BLOCK(block) || (block == 0))
            {
                count = 1;
                if (idx)
                {
                    first_bit = block ? 1 : 0;
                    count -= !(prev_block_border_bit_ ^ first_bit);
                }
                prev_block_border_bit_ = block ? 1 : 0;
            }
            else
            {
                if (BM_IS_GAP(block))
                {
                    gap_word_t* gap_block = BMGAP_PTR(block);
                    count = gap_length(gap_block) - 1;
                    if (idx)
                    {
                        first_bit = gap_test(gap_block, 0);
                        count -= !(prev_block_border_bit_ ^ first_bit);
                    }
                        
                    prev_block_border_bit_ = 
                        gap_test(gap_block, gap_max_bits-1);
                }
                else // bitset
                {
                    unsigned bit_count;
                    count = bit_block_calc_count_change(block,
                                                block + bm::set_block_size,
                                                &bit_count);
                    if (idx)
                    {
                        first_bit = block[0] & 1;
                        count -= !(prev_block_border_bit_ ^ first_bit);
                    }
                    prev_block_border_bit_ = 
                        block[set_block_size-1] >> ((sizeof(block[0]) * 8) - 1);
                    
                }
            }
            return count;
        }
        
        bm::id_t count() const { return count_; }

        void operator()(const bm::word_t* block, unsigned idx)
        {
            count_ += block_count(block, idx);
        }

    private:
        bm::id_t   count_;
        bm::id_t   prev_block_border_bit_;
    };


    /** Functor detects if any bit set*/
    class block_any_func : public bm_func_base_const
    {
    public:
        block_any_func(const blocks_manager& bm) 
            : bm_func_base_const(bm) 
        {}

        bool operator()(const bm::word_t* block, unsigned idx)
        {
            if (IS_FULL_BLOCK(block)) return true;

            if (BM_IS_GAP(block)) // gap block
            {
                if (!gap_is_all_zero(BMGAP_PTR(block), bm::gap_max_bits))
                {
                    return true;
                }
            }
            else  // bitset
            {
                bm::wordop_t* blk1 = (wordop_t*)block;
                bm::wordop_t* blk2 = 
                    (wordop_t*)(block + bm::set_block_size);
                if (!bit_is_all_zero(blk1, blk2))
                {
                    return true;
                }
            }
            return false;
        }
    };

    /*! Change GAP level lengths functor */
    class gap_level_func : public bm_func_base
    {
    public:
        gap_level_func(blocks_manager& bm, const gap_word_t* glevel_len)
            : bm_func_base(bm),
                glevel_len_(glevel_len)
        {
            BM_ASSERT(glevel_len);
        }

        void operator()(bm::word_t* block, unsigned idx)
        {
            blocks_manager& bman = this->bm_;
            
            if (!BM_IS_GAP(block))
                return;

            gap_word_t* gap_blk = BMGAP_PTR(block);

            // TODO: Use the same code as in the optimize functor
            if (gap_is_all_zero(gap_blk, bm::gap_max_bits))
            {
                bman.set_block_ptr(idx, 0);
                goto free_block;
            }
            else 
            if (gap_is_all_one(gap_blk, bm::gap_max_bits))
            {
                bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
            free_block:
                bman.get_allocator().free_gap_block(gap_blk, 
                                                    bman.glen());
                bman.set_block_bit(idx);
                return;
            }

            unsigned len = gap_length(gap_blk);
            int level = gap_calc_level(len, glevel_len_);
            if (level == -1)
            {
                bm::word_t* blk = 
                    bman.get_allocator().alloc_bit_block();
                bman.set_block_ptr(idx, blk);
                bman.set_block_bit(idx);
                gap_convert_to_bitset(blk, gap_blk);
            }
            else
            {
                gap_word_t* gap_blk_new = 
                    bman.allocate_gap_block(level, gap_blk, glevel_len_);

                bm::word_t* p = (bm::word_t*) gap_blk_new;
                BMSET_PTRGAP(p);
                bman.set_block_ptr(idx, p);
            }
            bman.get_allocator().free_gap_block(gap_blk, bman.glen());
        }

    private:
        const gap_word_t* glevel_len_;
    };


    /*! Bitblock optimization functor */
    class block_opt_func : public bm_func_base
    {
    public:
        block_opt_func(blocks_manager& bm, 
                        bm::word_t*     temp_block,
                        int             opt_mode,
                        bv_statistics*  bv_stat=0) 
            : bm_func_base(bm),
              temp_block_(temp_block),
              opt_mode_(opt_mode),
              stat_(bv_stat),
              empty_(0)
        {
            BM_ASSERT(temp_block);
        }

        void on_empty_top(unsigned i)
        {
            bm::word_t*** blk_root = this->bm_.get_rootblock();
            bm::word_t** blk_blk = blk_root[i];
            if (blk_blk) 
            {
                this->bm_.get_allocator().free_ptr(blk_blk);
                blk_root[i] = 0;
            }
            if (stat_)
            {
                stat_->max_serialize_mem += sizeof(unsigned) + 1;
            }
        }
        void on_empty_block(unsigned /* block_idx*/ ) { ++empty_; }

        void operator()(bm::word_t* block, unsigned idx)
        {
            blocks_manager& bman = this->bm_;
            if (IS_FULL_BLOCK(block)) 
            {
                ++empty_;
                return;
            }

            if (stat_) 
            {
                stat_->max_serialize_mem += empty_ << 2;
                empty_ = 0;
            }

            gap_word_t* gap_blk;
            if (BM_IS_GAP(block)) // gap block
            {
                gap_blk = BMGAP_PTR(block);
                // check if block is empty (or all 1)
                if (gap_is_all_zero(gap_blk, bm::gap_max_bits))
                {
                    bman.set_block_ptr(idx, 0);
                    this->free_block(gap_blk, idx);
                    ++empty_;
                }
                else 
                if (gap_is_all_one(gap_blk, bm::gap_max_bits))
                {
                    bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                    this->free_block(gap_blk, idx);
                    ++empty_;
                }
                else
                {
                    // regular gap block - compute statistics
                    if (stat_)
                    {
                        stat_->add_gap_block(
                                    bm::gap_capacity(gap_blk, bman.glen()),
                                    bm::gap_length(gap_blk));
                    }
                }
            }
            else // bit block
            {
                if (opt_mode_ < 3) // free_01 optimization
                {  
                    bm::wordop_t* blk1 = (wordop_t*)block;
                    bm::wordop_t* blk2 = 
                        (wordop_t*)(block + bm::set_block_size);
                
                    bool b = bit_is_all_zero(blk1, blk2);
                    if (b)
                    {
                        bman.get_allocator().free_bit_block(block);
                        bman.set_block_ptr(idx, 0);
                        ++empty_;
                    } 
                    else
                    if (opt_mode_ == 2) // check if it is all 1 block
                    {
                        b = is_bits_one(blk1, blk2);
                        if (b) 
                        {
                            bman.get_allocator().free_bit_block(block);
                            bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                            ++empty_;
                        }
                    }

                    if (!b && stat_)
                        stat_->add_bit_block();

                    return;
                }
            
                // try to compress
            
                gap_word_t* tmp_gap_blk = (gap_word_t*)temp_block_;
                *tmp_gap_blk = bm::gap_max_level << 1;

                unsigned threashold = bman.glen(bm::gap_max_level)-4;

                unsigned len = bit_convert_to_gap(tmp_gap_blk, 
                                                    block, 
                                                    bm::gap_max_bits, 
                                                    threashold);
                if (len)    // compression successful                
                {                
                    bman.get_allocator().free_bit_block(block);

                    // check if new gap block can be eliminated
                    if (gap_is_all_zero(tmp_gap_blk, bm::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, 0);
                        ++empty_;
                    }
                    else if (gap_is_all_one(tmp_gap_blk, bm::gap_max_bits))
                    {
                        bman.set_block_ptr(idx, FULL_BLOCK_ADDR);
                        ++empty_;
                    }
                    else
                    {
                        int level = bm::gap_calc_level(len, bman.glen());

                        gap_blk = 
                            bman.allocate_gap_block(level, tmp_gap_blk);
                        bman.set_block_ptr(idx, (bm::word_t*)gap_blk);
                        bman.set_block_gap(idx);
                        if (stat_)
                        {
                            stat_->add_gap_block(
                                    bm::gap_capacity(gap_blk, bman.glen()),
                                    bm::gap_length(gap_blk));
                        }
                    }
                }  
                else  // non-compressable bit-block
                {
                    if (stat_)
                        stat_->add_bit_block();
                }
            }
        }
    private:
        void free_block(gap_word_t* gap_blk, unsigned idx)
        {
            this->bm_.get_allocator().free_gap_block(gap_blk,
                                                     this->bm_.glen());
            this->bm_.set_block_bit(idx);
        }

    private:
        bm::word_t*         temp_block_;
        int                 opt_mode_;
        bv_statistics*      stat_;
        unsigned            empty_;
    };

    /** Bitblock invert functor */
    class block_invert_func : public bm_func_base
    {
    public:
        block_invert_func(blocks_manager& bm) 
            : bm_func_base(bm) {}

        void operator()(bm::word_t* block, unsigned idx)
        {
            if (!block)
            {
                this->bm_.set_block(idx, FULL_BLOCK_ADDR);
            }
            else
            if (IS_FULL_BLOCK(block))
            {
                this->bm_.set_block_ptr(idx, 0);
            }
            else
            {
                if (BM_IS_GAP(block)) // gap block
                {
                    gap_invert(BMGAP_PTR(block));
                }
                else  // bit block
                {
                    bm::wordop_t* wrd_ptr = (wordop_t*) block;
                    bm::wordop_t* wrd_end = 
                            (wordop_t*) (block + bm::set_block_size);
                    bit_invert(wrd_ptr, wrd_end);
                }
            }

        }
    };

    /** Set block zero functor */
    class block_zero_func : public bm_func_base
    {
    public:
        block_zero_func(blocks_manager& bm) 
        : bm_func_base(bm)
        {}

        void operator()(bm::word_t* block, unsigned idx)
        {
            if (BM_IS_GAP(block))
                gap_set_all(BMGAP_PTR(block), bm::gap_max_bits, 0);
            else  // BIT block
            {
                if (IS_FULL_BLOCK(block))
                    this->bm_.set_block_ptr(idx, 0);
                else
                    bit_block_set(block, 0);
            }
        }
    };

    /** Fill block with all-one bits functor */
    class block_one_func : public bm_func_base
    {
    public:
        block_one_func(blocks_manager& bm) : bm_func_base(bm) {}

        void operator()(bm::word_t* block, unsigned idx)
        {
            if (!IS_FULL_BLOCK(block))
            {
                this->bm_.set_block_all_set(idx);
            }
        }
    };

    /** Block deallocation functor */
    class block_free_func : public bm_func_base
    {
    public:
        block_free_func(blocks_manager& bm) : bm_func_base(bm) {}

        void operator()(bm::word_t* block)//, unsigned idx)
        {
            blocks_manager& bman = this->bm_;
            if (BM_IS_GAP(block)) // gap block
            {
                bman.get_allocator().free_gap_block(BMGAP_PTR(block),
                                                    bman.glen());
            }
            else
            {
                bman.get_allocator().free_bit_block(block);
            }
        }
    };

    /** Block copy functor */
    class block_copy_func : public bm_func_base
    {
    public:
        block_copy_func(blocks_manager&        bm_target, 
                        const blocks_manager&  bm_src) 
            : bm_func_base(bm_target), 
              bm_src_(bm_src) 
        {}

        void operator()(bm::word_t* block, unsigned idx)
        {
            bm::word_t* new_blk;            
            blocks_manager& bman = this->bm_;

            bool is_gap = BM_IS_GAP(block);
            if (is_gap)
            {
                bm::gap_word_t* gap_block = BMGAP_PTR(block); 
                unsigned level = gap_level(gap_block);
                new_blk = (bm::word_t*)
                    bman.get_allocator().alloc_gap_block(level, 
                                                        bman.glen());
                int len = gap_length(BMGAP_PTR(block));
                ::memcpy(new_blk, gap_block, len * sizeof(gap_word_t));
            }
            else
            {
                new_blk = block;
                if (!IS_FULL_BLOCK(block))
                {
                    new_blk = bman.get_allocator().alloc_bit_block();
                    bit_block_copy(new_blk, block);
                }
            }
            bman.set_block(idx, new_blk, is_gap);
        }

    private:
        block_copy_func(const block_copy_func&);
        block_copy_func& operator=(const block_copy_func&);
    private:
        const blocks_manager&  bm_src_;
    };


public:

    blocks_manager(const gap_word_t* glevel_len, 
                    bm::id_t          max_bits,
                    const Alloc&      alloc = Alloc())
        : temp_block_(0),
          alloc_(alloc)
    {
        ::memcpy(glevel_len_, glevel_len, sizeof(glevel_len_));
        blocks_ = 0;
        top_block_size_ = effective_top_block_size_ = 0;
        if (max_bits) 
        {
            top_block_size_ = compute_top_block_size(max_bits);
            init_tree();
        } 
        volatile const char* vp = _copyright<true>::_p;
        char c = *vp;
        c = 0;
    }

    blocks_manager(const blocks_manager& blockman)
        : blocks_(0),
            top_block_size_(blockman.top_block_size_),
            effective_top_block_size_(blockman.effective_top_block_size_),
        #ifdef BM_DISBALE_BIT_IN_PTR
            gap_flags_(blockman.gap_flags_),
        #endif
            temp_block_(0),
            alloc_(blockman.alloc_)
    {
        ::memcpy(glevel_len_, blockman.glevel_len_, sizeof(glevel_len_));

        init_tree();

        blocks_manager* bm = 
            const_cast<blocks_manager*>(&blockman);

        word_t*** blk_root = bm->blocks_root();

        block_copy_func copy_func(*this, blockman);
        for_each_nzblock(blk_root, top_block_size_, copy_func);
    }

    ~blocks_manager()
    {
        alloc_.free_bit_block(temp_block_);
        deinit_tree();
    }

    void free_ptr(bm::word_t** ptr)
    {
        if (ptr) alloc_.free_ptr(ptr);
    }

    /**
        \brief Compute size of the block array needed to store bits
        \param bits_to_store - supposed capacity (number of bits)
        \return size of the top level block
    */
    unsigned compute_top_block_size(bm::id_t bits_to_store)
    {
        if (bits_to_store == bm::id_max)  // working in full-range mode
        {
            return bm::set_array_size;
        }

        unsigned top_block_size = 
            bits_to_store / (bm::set_block_size * sizeof(bm::word_t) * 
                                                bm::set_array_size * 8);
        return top_block_size + (top_block_size < bm::set_array_size);
    }

    /**
        Returns current capacity (bits)
    */
    bm::id_t capacity() const
    {
        // arithmetic overflow protection...
        return top_block_size_ == bm::set_array_size ? bm::id_max :
            top_block_size_ * bm::set_array_size * bm::bits_in_block;
    }

    /**
        \brief Finds block in 2-level blocks array  
        \param nb - Index of block (logical linear number)
        \return block adress or NULL if not yet allocated
    */
    bm::word_t* get_block(unsigned nb) const
    {
        unsigned block_idx = nb >> bm::set_array_shift;
        if (block_idx >= top_block_size_)
        {
            return 0;
        }
        bm::word_t** blk_blk = blocks_[block_idx];
        return blk_blk ? blk_blk[nb & bm::set_array_mask] : 0;
    }

    /**
        \brief Finds block in 2-level blocks array  
        Specilized version of get_block(unsigned), returns an additional
        condition when there are no more blocks

        \param nb - Index of block (logical linear number)
        \param no_more_blocks - 1 if there are no more blocks at all
        \return block adress or NULL if not yet allocated
    */
    bm::word_t* get_block(unsigned nb, int* no_more_blocks) const
    {
        unsigned block_idx = nb >> bm::set_array_shift;
        if (block_idx >= top_block_size_)
        {
            *no_more_blocks = 1;
            return 0;
        }
        *no_more_blocks = 0;
        bm::word_t** blk_blk = blocks_[block_idx];
        return blk_blk ? blk_blk[nb & bm::set_array_mask] : 0;
    }

    /** 
    Recalculate absolute block address into coordinates
    */
    void get_block_coord(unsigned nb, unsigned* i, unsigned* j) const
    {
        BM_ASSERT(i);
        BM_ASSERT(j);

        *i = nb >> bm::set_array_shift; // top block address
        *j = nb &  bm::set_array_mask;  // address in sub-block
    }

    /**
    Find the next non-zero block starting from nb
    \param nb - block index
    \param deep_scan - flag to perform detailed bit-block analysis
    @return bm::set_total_blocks - no more blocks
    */
    unsigned find_next_nz_block(unsigned nb, bool deep_scan = true) const
    {
        unsigned i,j;
        get_block_coord(nb, &i, &j);
        for (;i < effective_top_block_size_; ++i) 
        { 
            bm::word_t** blk_blk = blocks_[i];
            if (blk_blk)
            {
               unsigned r = i * bm::set_array_size;
               do
               {
                   if (blk_blk[j] && !is_block_zero(r + j, blk_blk[j], deep_scan)) 
                       return r + j;
                   ++j;
               } while (j < bm::set_array_size);
            }
            j = 0;
        } // for i

        return bm::set_total_blocks;
    }

    /**
        \brief Finds block in 2-level blocks array
        \param i - top level block index
        \param j - second level block index
        \return block adress or NULL if not yet allocated
    */
    const bm::word_t* get_block(unsigned i, unsigned j) const
    {
        if (i >= top_block_size_) return 0;
        const bm::word_t* const* blk_blk = blocks_[i];
        return (blk_blk == 0) ? 0 : blk_blk[j];
    }

    /**
        \brief Function returns top-level block in 2-level blocks array
        \param i - top level block index
        \return block adress or NULL if not yet allocated
    */
    const bm::word_t* const * get_topblock(unsigned i) const
    {
        return (i >= top_block_size_) ? 0 : blocks_[i];
    }

    /** 
        \brief Returns root block in the tree.
    */
    bm::word_t*** get_rootblock() const
    {
        blocks_manager* bm = 
            const_cast<blocks_manager*>(this);
        return bm->blocks_root();
    }

    void free_block(bm::word_t* block)
    {
        if (BM_IS_GAP(block))
        {
            alloc_.free_gap_block(BMGAP_PTR(block), glevel_len_);
        }
        else
        {
            alloc_.free_bit_block(block);
        }
    }

    void set_block_all_set(unsigned nb)
    {
        bm::word_t* block = 
            set_block(nb, const_cast<bm::word_t*>(FULL_BLOCK_ADDR));
        free_block(block);
    }

    /**
        Create(allocate) bit block. Old block (if exists) gets deleted.
    */
    bm::word_t* alloc_bit_block(unsigned nb)
    {
        bm::word_t* block = this->get_allocator().alloc_bit_block();
        bm::word_t* old_block = set_block(nb, block);
        if (IS_VALID_ADDR(old_block))
        {
            free_block(old_block);
        }
        return block;
    }

    /**
        Create all-zeros bit block. Old block (if exists) gets deleted.
    */
    bm::word_t* make_bit_block(unsigned nb)
    {
        bm::word_t* block = this->alloc_bit_block(nb);
        bit_block_set(block, 0);
        return block;
    }

    /**
        Create bit block as a copy of source block (bit or gap).
        Old block (if exists) gets deleted.
    */
    bm::word_t* copy_bit_block(unsigned          nb, 
                               const bm::word_t* block_src, int is_src_gap)
    {
        if (block_src == 0)
        {
            zero_block(nb);
            return 0;
        }
        bm::word_t* block = alloc_bit_block(nb);
        if (is_src_gap)
        {
            gap_word_t* gap_block = BMGAP_PTR(block_src);
            gap_convert_to_bitset(block, gap_block);
        }
        else
        {            
            bit_block_copy(block, block_src);
        }
        return block;
    }

    /**
        Copy block from another vector.
        Note:Target block is always replaced through re-allocation.
    */
    bm::word_t* copy_block(unsigned idx, const blocks_manager& bm_src)
    {
        const bm::word_t* block = bm_src.get_block(idx);
		if (block == 0)
		{
			zero_block(idx);
			return 0;
		}
        bm::word_t* new_blk = 0;
        bool is_gap = BM_IS_GAP(block);

        if (is_gap)
        {
            bm::gap_word_t* gap_block = BMGAP_PTR(block); 
            unsigned level = gap_level(gap_block);
            new_blk = (bm::word_t*)
                get_allocator().alloc_gap_block(level, glen());
            int len = gap_length(BMGAP_PTR(block));
            ::memcpy(new_blk, gap_block, len * sizeof(gap_word_t));
            set_gap_level(new_blk, level);
        }
        else
        {
            if (IS_FULL_BLOCK(block))
            {
                new_blk = FULL_BLOCK_ADDR;
            }
            else
            {
                new_blk = get_allocator().alloc_bit_block();
                bit_block_copy(new_blk, block);
            }
        }
        set_block(idx, new_blk, is_gap);
        return new_blk;
    }


    /** 
        Function checks if block is not yet allocated, allocates it and sets to
        all-zero or all-one bits. 

        If content_flag == 1 (ALLSET block) requested and taken block is already ALLSET,
        function will return NULL

        initial_block_type and actual_block_type : 0 - bitset, 1 - gap
    */
    bm::word_t* check_allocate_block(unsigned nb, 
                                     unsigned content_flag,
                                     int      initial_block_type,
                                     int*     actual_block_type,
                                     bool     allow_null_ret=true)
    {
        bm::word_t* block = this->get_block(nb);

        if (!IS_VALID_ADDR(block)) // NULL block or ALLSET
        {
            // if we wanted ALLSET and requested block is ALLSET return NULL
            unsigned block_flag = IS_FULL_BLOCK(block);
            *actual_block_type = initial_block_type;
            if (block_flag == content_flag && allow_null_ret)
            {
                return 0; // it means nothing to do for the caller
            }

            if (initial_block_type == 0) // bitset requested
            {
                block = alloc_.alloc_bit_block();
                // initialize block depending on its previous status
                bit_block_set(block, block_flag ? 0xFF : 0);
                set_block(nb, block, false /*bit*/);
            }
            else // gap block requested
            {
                bm::gap_word_t* gap_block = allocate_gap_block(0);
                gap_set_all(gap_block, bm::gap_max_bits, block_flag);
                set_block(nb, (bm::word_t*)gap_block, true/*gap*/);
                return (bm::word_t*)gap_block;
            }
        }
        else // block already exists
        {
            *actual_block_type = BM_IS_GAP(block);
        }

        return block;
    }

    /*! @brief Fills all blocks with 0.
        @param free_mem - if true function frees the resources
    */
    void set_all_zero(bool free_mem)
    {        
        if (free_mem)
        {
            // TODO: optimization of top-level realloc
            deinit_tree();
            init_tree();
        }
        else
        {
            block_zero_func zero_func(*this);
            unsigned top_size = this->effective_top_block_size();
            for_each_nzblock(blocks_, top_size,  zero_func);
        }
    }

    /*! Replaces all blocks with ALL_ONE block.
    */
    void set_all_one()
    {
        block_one_func func(*this);
        for_each_block(blocks_, top_block_size_, 
                                bm::set_array_size, func);
    }

    /**
        Places new block into descriptors table, returns old block's address.
        NOTE:Old block is not deleted.
    */
    bm::word_t* set_block(unsigned nb, bm::word_t* block)
    {
        register unsigned nblk_blk = nb >> bm::set_array_shift;

        // auto-resize the top block array
        if (nblk_blk >= top_block_size_)
            reserve_top_blocks(nblk_blk + 1);
        if (nblk_blk >= effective_top_block_size_)
            effective_top_block_size_ = nblk_blk + 1;

        // If first level array not yet allocated, allocate it and
        // assign block to it
        if (blocks_[nblk_blk] == 0) 
        {
            blocks_[nblk_blk] = (bm::word_t**)alloc_.alloc_ptr();
            ::memset(blocks_[nblk_blk], 0, 
                bm::set_array_size * sizeof(bm::word_t*));
        }

        // NOTE: block will be replaced without freeing
        unsigned j = nb & bm::set_array_mask;
        bm::word_t* old_block = blocks_[nblk_blk][j];
        blocks_[nblk_blk][j] = block;

        return old_block;
    }


    /**
    Allocate an place new GAP block (copy of provided block)
    */
    bm::word_t* set_gap_block(unsigned      nb,
                          const gap_word_t* gap_block_src,
                          int               level)
    {
        if (level == -1)
        {
            bm::word_t* blk = get_allocator().alloc_bit_block();
            set_block(nb, blk);
            gap_convert_to_bitset(blk, gap_block_src);
            return blk;
        }
        else
        {
            gap_word_t* gap_blk = get_allocator().alloc_gap_block(
                                                         level, this->glen());
            gap_word_t* gap_blk_ptr = BMGAP_PTR(gap_blk);
            ::memcpy(gap_blk_ptr, gap_block_src, 
                                  gap_length(gap_block_src) * sizeof(gap_word_t));
            set_gap_level(gap_blk_ptr, level);
            set_block(nb, (bm::word_t*)gap_blk, true /*GAP*/);
            return (bm::word_t*)gap_blk;
        }
    }

    /**
        Places new block into descriptors table, returns old block's address.
        Old block is not deleted.
    */
    bm::word_t* set_block(unsigned nb, bm::word_t* block, bool gap)
    {
        if (block)
        {
            block = 
                (bm::word_t*) (gap ? BMPTR_SETBIT0(block) : BMPTR_CLEARBIT0(block));
        }

        // top block index
        register unsigned nblk_blk = nb >> bm::set_array_shift;

        // auto-resize the top block array
        if (nblk_blk >= top_block_size_)
            reserve_top_blocks(nblk_blk + 1);
        if (nblk_blk >= effective_top_block_size_)
            effective_top_block_size_ = nblk_blk + 1;

        // If first level array not yet allocated, allocate it and
        // assign block to it
        if (blocks_[nblk_blk] == 0) 
        {
            blocks_[nblk_blk] = (bm::word_t**)alloc_.alloc_ptr();
            ::memset(blocks_[nblk_blk], 0, 
                bm::set_array_size * sizeof(bm::word_t*));
        }

        // NOTE: block will be replaced without freeing
        unsigned j = nb & bm::set_array_mask;
        bm::word_t* old_block = blocks_[nblk_blk][j];
        blocks_[nblk_blk][j] = block; 

        return old_block;
    }

    /**
        Places new block into blocks table.
    */
    void set_block_ptr(unsigned nb, bm::word_t* block)
    {
        BM_ASSERT((nb >> bm::set_array_shift) < effective_top_block_size_);
        blocks_[nb >> bm::set_array_shift][nb & bm::set_array_mask] = block;
    }
        
    /** 
        \brief Converts block from type gap to conventional bitset block.
        \param nb - Block's index. 
        \param gap_block - Pointer to the gap block, 
                            if NULL block nb will be taken
        \param gap_len - GAP block length (optional)
        \return new bit block's memory
    */
    bm::word_t* convert_gap2bitset(unsigned nb, 
                                   const gap_word_t* gap_block=0, 
                                   unsigned gap_len=0)
    {
        bm::word_t* block = this->get_block(nb);
        if (gap_block == 0)
        {
            gap_block = BMGAP_PTR(block);
        }

        BM_ASSERT(IS_VALID_ADDR((bm::word_t*)gap_block));

        bm::word_t* new_block = alloc_.alloc_bit_block();
        gap_convert_to_bitset_l(new_block, gap_block, gap_len);

        // new block will replace the old one(no deletion)
        //set_block_ptr(nb, new_block); 
        if (block)
        {
            set_block_ptr(nb, new_block); 
            alloc_.free_gap_block(BMGAP_PTR(block), glen());
        }
        else
        {
            set_block(nb, new_block); 
        }

        return new_block;
    }

    /**
        Make sure block turns into true bit-block if it is GAP or a full block
    */
    bm::word_t* deoptimize_block(unsigned nb)
    {
        bm::word_t* block = this->get_block(nb);
        if (BM_IS_GAP(block))
        {
            return convert_gap2bitset(nb);
        }
        if (IS_FULL_BLOCK(block)) 
        {
            bm::word_t* new_block = get_allocator().alloc_bit_block();
            bit_block_copy(new_block, block);
            set_block(nb, new_block);
            return new_block;
        }
        return block;
    }

    /**
    Free block, make it zero pointer in the tree
    */
    bm::word_t* zero_block(unsigned nb)
    {
        bm::word_t* block = this->get_block(nb);
        if (!block) return block;
        free_block(block);
        set_block(nb, 0);
        return 0;
    }

    /**
    Free block, make it zero pointer in the tree
    */
    bm::word_t* zero_block(unsigned i, unsigned j)
    {
        bm::word_t** blk_blk = blocks_[i];
        bm::word_t* block = blk_blk[j];
        blk_blk[j] = 0;

        free_block(block);

        return 0;
    }


    /**
        Count number of bits ON in the block
    */
    bm::id_t block_bitcount(const bm::word_t* block) const
    { 
        if (!block) return 0;
        id_t count;

        if (BM_IS_GAP(block))
        {
            count = gap_bit_count(BMGAP_PTR(block));
        }
        else // bitset
        {
            count = (IS_FULL_BLOCK(block)) ? bm::bits_in_block
                : bit_block_calc_count(block, block + bm::set_block_size);
        }
        return count;
    }

    /**
        \brief Extends GAP block to the next level or converts it to bit block.
        \param nb - Block's linear index.
        \param blk - Blocks's pointer 

        \return new GAP block pointer or NULL if block type mutated
    */
    bm::gap_word_t* extend_gap_block(unsigned nb, gap_word_t* blk)
    {
        unsigned level = bm::gap_level(blk);
        unsigned len = bm::gap_length(blk);
        if (level == bm::gap_max_level || len >= gap_max_buff_len)
        {
            convert_gap2bitset(nb);
        }
        else
        {
            bm::gap_word_t* new_gap_blk = allocate_gap_block(++level, blk);
            bm::word_t* new_blk = (bm::word_t*)new_gap_blk;

            BMSET_PTRGAP(new_blk);

            set_block_ptr(nb, new_blk);
            alloc_.free_gap_block(blk, glen());

            return new_gap_blk;
        }
        return 0;
    }

    bool is_block_gap(unsigned nb) const 
    {
        bm::word_t* block = get_block(nb);
        return BMPTR_TESTBIT0(block) != 0;
    }

    void set_block_bit(unsigned nb) 
    { 
        bm::word_t* block = get_block(nb);
        block = (bm::word_t*) BMPTR_CLEARBIT0(block);
        set_block_ptr(nb, block);
    }

    void set_block_gap(unsigned nb) 
    {
        bm::word_t* block = get_block(nb);
        block = (bm::word_t*)BMPTR_SETBIT0(block);
        set_block_ptr(nb, block);
    }

    /**
        \fn bool bm::bvector::blocks_manager::is_block_zero(unsigned nb, bm::word_t* blk)
        \brief Checks all conditions and returns true if block consists of only 0 bits
        \param nb - Block's linear index.
        \param blk - Blocks's pointer
        \param deep_scan - flag to do full bit block verification (scan)
                           when deep scan is not requested result can be approximate
        \returns true if all bits are in the block are 0.
    */
    bool is_block_zero(unsigned          nb, 
                       const bm::word_t* blk, 
                       bool              deep_scan = true) const
    {
        if (blk == 0) return true;

        if (BM_IS_GAP(blk))
        {
            gap_word_t* b = BMGAP_PTR(blk);
            return gap_is_all_zero(b, bm::gap_max_bits);
        }

        if (!deep_scan) 
            return false; // block exists - presume it has bits

        // BIT
        for (unsigned i = 0; i <  bm::set_block_size; ++i)
        {
            if (blk[i] != 0)
                return false;
        }
        return true;
    }

    /**
        \brief Checks if block has only 1 bits
        \param nb - Index of the block.
        \param blk - Block's pointer
        \param deep_scan - flag to do full bit block verification (scan)
                           when deep scan is not requested result can be approximate
        \return true if block consists of 1 bits.
    */
    bool is_block_one(unsigned          nb, 
                      const bm::word_t* blk,
                      bool              deep_scan = true) const
    {
        if (blk == 0) return false;

        if (BM_IS_GAP(blk))
        {
            gap_word_t* b = BMGAP_PTR(blk);
            return gap_is_all_one(b, bm::gap_max_bits);
        }

        // BIT block
        if (IS_FULL_BLOCK(blk))
        {
            return true;
        }
        if (!deep_scan) 
            return false; // block exists - presume it has 0 bits

        return is_bits_one((wordop_t*)blk, 
                           (wordop_t*)(blk + bm::set_block_size));
    }

    /*! Returns temporary block, allocates if needed. */
    bm::word_t* check_allocate_tempblock()
    {
        return temp_block_ ? temp_block_ 
                            : (temp_block_ = alloc_.alloc_bit_block());
    }

    /*! Assigns new GAP lengths vector */
    void set_glen(const gap_word_t* glevel_len)
    {
        ::memcpy(glevel_len_, glevel_len, sizeof(glevel_len_));
    }


    bm::gap_word_t* allocate_gap_block(unsigned level, 
                                       const gap_word_t* src = 0,
                                       const gap_word_t* glevel_len = 0)
    {
        if (!glevel_len)
            glevel_len = glevel_len_;
        gap_word_t* ptr = alloc_.alloc_gap_block(level, glevel_len);
        if (src)
        {
            unsigned len = gap_length(src);
            ::memcpy(ptr, src, len * sizeof(gap_word_t));
            // Reconstruct the mask word using the new level code.
            *ptr = (gap_word_t)(((len-1) << 3) | (level << 1) | (*src & 1));
        }
        else
        {
            *ptr = (gap_word_t)(level << 1);
        }
        return ptr;
    }

    unsigned mem_used() const
    {
        unsigned mem_used = sizeof(*this);
        mem_used += temp_block_ ? sizeof(word_t) * bm::set_block_size : 0;
        mem_used += sizeof(bm::word_t**) * top_block_size_;

        for (unsigned i = 0; i < top_block_size_; ++i)
        {
            mem_used += blocks_[i] ? sizeof(void*) * bm::set_array_size : 0;
        }

        return mem_used;
    }

    /** Returns true if second level block pointer is 0.
    */
    bool is_subblock_null(unsigned nsub) const
    {
        return blocks_[nsub] == NULL;
    }


    bm::word_t***  blocks_root()
    {
        return blocks_;
    }

    /*! \brief Returns current GAP level vector
    */
    const gap_word_t* glen() const
    {
        return glevel_len_;
    }

    /*! \brief Returns GAP level length for specified level
        \param level - level number
    */
    unsigned glen(unsigned level) const
    {
        return glevel_len_[level];
    }

    /*! \brief Swaps content 
        \param bm  another blocks manager
    */
    void swap(blocks_manager& bm)
    {
        BM_ASSERT(this != &bm);

        word_t*** btmp = blocks_;
        blocks_ = bm.blocks_;
        bm.blocks_ = btmp;

		xor_swap(this->top_block_size_, bm.top_block_size_);
        xor_swap(this->effective_top_block_size_, bm.effective_top_block_size_);

        BM_ASSERT(sizeof(glevel_len_) / sizeof(glevel_len_[0]) 
                                    == bm::gap_levels); // paranoiya check
        for (unsigned i = 0; i < bm::gap_levels; ++i)
        {
		    xor_swap(glevel_len_[i], bm.glevel_len_[i]);
        }
    }

    /*! \brief Returns size of the top block array in the tree 
    */
    unsigned top_block_size() const
    {
        return top_block_size_;
    }

    /*! \brief Returns effective size of the top block array in the tree 
    Effective size excludes NULL pointers at the top descriptor end
    */
    unsigned effective_top_block_size() const
    {
        return effective_top_block_size_;
    }

    /**
        \brief reserve capacity for specified number of bits
    */
    void reserve(unsigned max_bits)
    {
        if (max_bits) 
        {
            unsigned b = compute_top_block_size(max_bits);
            reserve_top_blocks(b);
        }
    }

    /*!
        \brief Make sure blocks manager has enough blocks capacity
    */
    void reserve_top_blocks(unsigned top_blocks) 
    {
        BM_ASSERT(top_blocks <= bm::set_array_size);
        if (top_blocks <= top_block_size_) return; // nothing to do
        bm::word_t*** new_blocks = 
            (bm::word_t***)alloc_.alloc_ptr(top_blocks);

        for (unsigned i = 0; i < top_block_size_; ++i)
        {
            new_blocks[i] = blocks_[i];
        }
        for (unsigned j = top_block_size_; j < top_blocks; ++j)
        {
            new_blocks[j] = 0;
        }
        alloc_.free_ptr(blocks_, top_block_size_);
        blocks_ = new_blocks;
        top_block_size_ = top_blocks;
    }
    
    /** \brief Returns reference on the allocator
    */
    allocator_type& get_allocator() { return alloc_; }

    /** \brief Returns allocator
    */
    allocator_type get_allocator() const { return alloc_; }

private:

    void operator =(const blocks_manager&);

    void deinit_tree()
    {
        if (blocks_ == 0) return;
        unsigned top_size = this->effective_top_block_size();
        block_free_func  free_func(*this);
        for_each_nzblock2(blocks_, top_size, free_func);
        free_top_block();
        alloc_.free_ptr(blocks_, top_block_size_);
        blocks_ = 0;
    }

    void free_top_block()
    {
        for(unsigned i = 0; i < top_block_size_; ++i)
        {
            bm::word_t** blk_blk = blocks_[i];
            if (blk_blk) 
            {
                alloc_.free_ptr(blk_blk); blocks_[i] = 0;
            }
        }
    }

    /// allocate first level of descr. of blocks 
    void init_tree()
    {
        if (top_block_size_)
        {
            blocks_ = (bm::word_t***) alloc_.alloc_ptr(top_block_size_); 
            ::memset(blocks_, 0, top_block_size_ * sizeof(bm::word_t**));
        }
        else
        {
            blocks_ = 0;
        }
        effective_top_block_size_ = 1;
    }

private:
    /// Tree of blocks.
    bm::word_t***                          blocks_;
    /// Size of the top level block array in blocks_ tree
    unsigned                               top_block_size_;
    /// Effective size of the top level block array in blocks_ tree
    unsigned                               effective_top_block_size_;
    /// Temp block.
    bm::word_t*                            temp_block_; 
    /// vector defines gap block lengths for different levels 
    gap_word_t                             glevel_len_[bm::gap_levels];
    /// allocator
    allocator_type                         alloc_;
};

/**
    Bit block buffer guard
    \internal
*/
template<class BlocksManager>
class bit_block_guard
{
public:
    bit_block_guard(BlocksManager& bman, bm::word_t* blk=0) 
        : bman_(bman), 
          block_(blk)
    {}
    ~bit_block_guard()
    {
        bman_.get_allocator().free_bit_block(block_, 3);
    }
    void attach(bm::word_t* blk)
    {
        bman_.get_allocator().free_bit_block(block_);
        block_ = blk;
    }
    bm::word_t* allocate()
    {
        attach(bman_.get_allocator().alloc_bit_block(3));
        return block_;
    }
    bm::word_t* get() { return block_; }

private:
    bit_block_guard(const bit_block_guard&);
    bit_block_guard& operator=(const bit_block_guard&);
private:
    BlocksManager& bman_;
    bm::word_t*    block_;
};


}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif

