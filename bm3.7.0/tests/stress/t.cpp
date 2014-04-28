/*
Copyright (c) 2002-2009 Anatoliy Kuznetsov.

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
*/


//#define BM_SET_MMX_GUARD
//#define BMSSE2OPT
//#define BMSSE42OPT
//#define BM64OPT
//#define BMCOUNTOPT

#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <memory.h>
#include <time.h>
#include <math.h>

#include <iostream>
#include <iomanip>

#include <bm.h>
#include <bmalgo.h>
#include <bmutil.h>
#include <bmserial.h>
#include <bmrandom.h>
#include <bmvmin.h>

using namespace bm;
using namespace std;

#include "rlebtv.h"
#include <encoding.h>
#include <limits.h>

#include <bmdbg.h>

#include <vector>


#define POOL_SIZE 5000

//#define MEM_POOL


template<class T> T* pool_allocate(T** pool, int& i, size_t n)
{
    return i ? pool[i--] : (T*) ::malloc(n * sizeof(T));
}

inline void* pool_allocate2(void** pool, int& i, size_t n)
{
    return i ? pool[i--] : malloc(n * sizeof(void*));
}



template<class T> void pool_free(T** pool, int& i, T* p)
{
    i < POOL_SIZE ? (free(p),(void*)0) : pool[++i]=p;
}


class pool_block_allocator
{
public:

    static bm::word_t* allocate(size_t n, const void *)
    {
        int *idx = 0;
        bm::word_t** pool = 0;

        switch (n)
        {
        case bm::set_block_size:
            idx = &bit_blocks_idx_;
            pool = free_bit_blocks_;
            break;

        case 64:
            idx = &gap_blocks_idx0_;
            pool = gap_bit_blocks0_;
            break;

        case 128:
            idx = &gap_blocks_idx1_;
            pool = gap_bit_blocks1_;
            break;
        
        case 256:
            idx = &gap_blocks_idx2_;
            pool = gap_bit_blocks2_;
            break;

        case 512:
            idx = &gap_blocks_idx3_;
            pool = gap_bit_blocks3_;
            break;

        default:
            assert(0);
        }

        return pool_allocate(pool, *idx, n);
    }

    static void deallocate(bm::word_t* p, size_t n)
    {
        int *idx = 0;
        bm::word_t** pool = 0;

        switch (n)
        {
        case bm::set_block_size:
            idx = &bit_blocks_idx_;
            pool = free_bit_blocks_;
            break;

        case 64:
            idx = &gap_blocks_idx0_;
            pool = gap_bit_blocks0_;
            break;

        case 128:
            idx = &gap_blocks_idx1_;
            pool = gap_bit_blocks1_;
            break;
        
        case 256:
            idx = &gap_blocks_idx2_;
            pool = gap_bit_blocks2_;
            break;

        case 512:
            idx = &gap_blocks_idx3_;
            pool = gap_bit_blocks3_;
            break;

        default:
            assert(0);
        }

        pool_free(pool, *idx, p);
    }

private:
    static bm::word_t* free_bit_blocks_[];
    static int         bit_blocks_idx_;

    static bm::word_t* gap_bit_blocks0_[];
    static int         gap_blocks_idx0_;

    static bm::word_t* gap_bit_blocks1_[];
    static int         gap_blocks_idx1_;

    static bm::word_t* gap_bit_blocks2_[];
    static int         gap_blocks_idx2_;

    static bm::word_t* gap_bit_blocks3_[];
    static int         gap_blocks_idx3_;
};

bm::word_t* pool_block_allocator::free_bit_blocks_[POOL_SIZE];
int pool_block_allocator::bit_blocks_idx_ = 0;

bm::word_t* pool_block_allocator::gap_bit_blocks0_[POOL_SIZE];
int pool_block_allocator::gap_blocks_idx0_ = 0;

bm::word_t* pool_block_allocator::gap_bit_blocks1_[POOL_SIZE];
int pool_block_allocator::gap_blocks_idx1_ = 0;

bm::word_t* pool_block_allocator::gap_bit_blocks2_[POOL_SIZE];
int pool_block_allocator::gap_blocks_idx2_ = 0;

bm::word_t* pool_block_allocator::gap_bit_blocks3_[POOL_SIZE];
int pool_block_allocator::gap_blocks_idx3_ = 0;




class pool_ptr_allocator
{
public:

    static void* allocate(size_t n, const void *)
    {
        return pool_allocate2(free_ptr_blocks_, ptr_blocks_idx_, n);
    }

    static void deallocate(void* p, size_t)
    {
        pool_free(free_ptr_blocks_, ptr_blocks_idx_, p);
    }

private:
    static void*  free_ptr_blocks_[];
    static int    ptr_blocks_idx_;
};

void* pool_ptr_allocator::free_ptr_blocks_[POOL_SIZE];
int pool_ptr_allocator::ptr_blocks_idx_ = 0;

#if defined(BMSSE2OPT) || defined(BMSSE42OPT)
#else
# define MEM_DEBUG
#endif
 
#ifdef MEM_DEBUG


class dbg_block_allocator
{
public:
static unsigned na_;
static unsigned nf_;

    static bm::word_t* allocate(size_t n, const void *)
    {
        ++na_;
        assert(n);
        bm::word_t* p =
            (bm::word_t*) ::malloc((n+1) * sizeof(bm::word_t));
        if (!p)
        {
            std::cerr << "Failed allocation!" << endl;
            exit(1);
        }
        *p = n;
        return ++p;
    }

    static void deallocate(bm::word_t* p, size_t n)
    {
        ++nf_;
        --p;
        if(*p != n)
        {
            printf("Block memory deallocation error!\n");
            exit(1);
        }
        ::free(p);
    }

    static int balance()
    {
        return nf_ - na_;
    }
};

unsigned dbg_block_allocator::na_ = 0;
unsigned dbg_block_allocator::nf_ = 0;

class dbg_ptr_allocator
{
public:
static unsigned na_;
static unsigned nf_;

    static void* allocate(size_t n, const void *)
    {
        ++na_;
        assert(sizeof(size_t) == sizeof(void*));
        void* p = ::malloc((n+1) * sizeof(void*));
        if (!p)
        {
            std::cerr << "Failed allocation!" << endl;
            exit(1);
        }
        size_t* s = (size_t*) p;
        *s = n;
        return (void*)++s;
    }

    static void deallocate(void* p, size_t n)
    {
        ++nf_;
        size_t* s = (size_t*) p;
        --s;
        if(*s != n)
        {
            printf("Ptr memory deallocation error!\n");
            exit(1);
        }
        ::free(s);
    }

    static int balance()
    {
        return nf_ - na_;
    }

};

unsigned dbg_ptr_allocator::na_ = 0;
unsigned dbg_ptr_allocator::nf_ = 0;


typedef mem_alloc<dbg_block_allocator, dbg_ptr_allocator> dbg_alloc;

typedef bm::bvector<dbg_alloc> bvect;
typedef bm::bvector_mini<dbg_block_allocator> bvect_mini;

#else

#ifdef MEM_POOL

typedef mem_alloc<pool_block_allocator, pool_ptr_allocator> pool_alloc;
typedef bm::bvector<pool_alloc> bvect;
typedef bm::bvector_mini<bm::block_allocator> bvect_mini;


#else

typedef bm::bvector<> bvect;
typedef bm::bvector_mini<bm::block_allocator> bvect_mini;

#endif

#endif

//const unsigned BITVECT_SIZE = 100000000 * 8;

// This this setting program will consume around 150M of RAM
const unsigned BITVECT_SIZE = 100000000 * 2;

const unsigned ITERATIONS = 100000;
const unsigned PROGRESS_PRINT = 2000000;



void CheckVectors(bvect_mini &bvect_min, 
                  bvect      &bvect_full,
                  unsigned size,
                  bool     detailed = false);


unsigned random_minmax(unsigned min, unsigned max)
{
    unsigned r = (rand() << 16) | rand();
    return r % (max-min) + min;
}


void FillSets(bvect_mini* bvect_min, 
              bvect* bvect_full,
              unsigned min, 
              unsigned max,
              unsigned fill_factor)
{
    unsigned i;
    unsigned id;

    //Random filling
    if(fill_factor == 0)
    {
        unsigned n_id = (max - min) / 100;
        printf("random filling : %i\n", n_id);
        for (i = 0; i < n_id; i++)
        {
            id = random_minmax(min, max);
            bvect_min->set_bit(id);
            bvect_full->set_bit(id);
            if (PROGRESS_PRINT)
            {
                if ( (i % PROGRESS_PRINT) == 0)
                {
                    cout << "+" << flush;
                }
            }
        }
        cout << endl;
    }
    else
    {
        printf("fill_factor random filling : factor = %i\n", fill_factor);

        for(i = 0; i < fill_factor; i++)
        {
            int k = rand() % 10;
            if (k == 0)
                k+=2;

            //Calculate start
            unsigned start = min + (max - min) / (fill_factor * k);

            //Randomize start
            start += random_minmax(1, (max - min) / (fill_factor * 10));

            if (start > max)
            {
                start = min;
            }
            
            //Calculate end 
            unsigned end = start + (max - start) / (fill_factor *2);

            //Randomize end
            end -= random_minmax(1, (max - start) / (fill_factor * 10));

            if (end > max )
            {
                end = max;
            }

            
            if (fill_factor > 1)
            {
                for(; start < end;)
                {
                    int r = rand() % 8;

                    if (r > 7)
                    {
                        int inc = rand() % 3;
                        ++inc;
                        unsigned end2 = start + rand() % 1000;
                        if (end2 > end)
                            end2 = end;
                        while (start < end2)
                        {
                            bvect_min->set_bit(start);
                            bvect_full->set_bit(start);  
                            start += inc;
                        }

                        if (PROGRESS_PRINT)
                        {
                            if ( ( ((i+1)*(end-start))  % PROGRESS_PRINT) == 0)
                            {
                                cout << "+" << flush;
                            }
                        }

                        continue;
                    }

                    if (r)
                    {
                        bvect_min->set_bit(start);
                        bvect_full->set_bit(start);
                        ++start;
                    }
                    else
                    {
                        start+=r;
                        bvect_min->set_bit(start);
                        bvect_full->set_bit(start);
                    }

                    if (PROGRESS_PRINT)
                    {
                        if ( ( ((i+1)*(end-start))  % PROGRESS_PRINT) == 0)
                        {
                            cout << "+" << flush;
                        }
                    }
                }

            }
            else
            {
                int c = rand() % 15;
                if (c == 0)
                    ++c;
                for(; start < end; ++start)
                {
                    bvect_min->set_bit(start);
                    bvect_full->set_bit(start);

                    if (start % c)
                    {
                        start += c;
                    }

                    if (PROGRESS_PRINT)
                    {
                        if ( ( ((i+1)*(end-start))  % PROGRESS_PRINT) == 0)
                        {
                            cout << "+" << flush;
                        }
                    }

                }
            }
            cout << endl;

        }
    }
}

//
// Interval filling.
// 111........111111........111111..........11111111.......1111111...
//


void FillSetsIntervals(bvect_mini* bvect_min, 
              bvect* bvect_full,
              unsigned min, 
              unsigned max,
              unsigned fill_factor,
              bool set_flag=true)
{

    while(fill_factor==0)
    {
        fill_factor=rand()%10;
    }

    cout << "Intervals filling. Factor=" 
         <<  fill_factor << endl << endl;

    unsigned i, j;
    unsigned factor = 70 * fill_factor;
    for (i = min; i < max; ++i)
    {
        unsigned len, end; 

        do
        {
            len = rand() % factor;
            end = i+len;
            
        } while (end >= max);
/*
        if (set_flag == false)
        {
            cout << "Cleaning: " << i << "-" << end << endl;
        }
        else
        {
            cout << "Add: " << i << "-" << end << endl;
        }
*/
        if (i < end)
        {
            bvect_full->set_range(i, end-1, set_flag);
        }
       
        for (j = i; j < end; ++j)
        {
            if (set_flag)
            {
                bvect_min->set_bit(j);
                //bvect_full->set_bit(j);
            }
            else
            {
                bvect_min->clear_bit(j);
                //bvect_full->clear_bit(j);
/*
        if (g_cnt_check)
        {
            bool b = bvect_full->count_check();
            if(!b)
            {
                cout << "Count check failed (clear)!" << endl;
                cout << "bit=" << j << endl;
                exit(1);
            }
        }
*/
            }

                           
        } // j

//cout << "Checking range filling " << from << "-" << to << endl;
//CheckVectors(*bvect_min, *bvect_full, 100000000);


        i = end;


        len = rand() % (factor* 10 * bm::gap_max_bits);
        if (len % 2)
        {
            len *= rand() % (factor * 10);
        }

        i+=len;

        if ( (len % 6) == 0)  
        {
/*
if (set_flag == false)
{
    cout << "Additional Cleaning: " << i << "-" << end << endl;
}
*/
            for(unsigned k=0; k < 1000 && i < max; k+=3,i+=3)
            {
                if (set_flag)
                {
                    bvect_min->set_bit(i);
                    bvect_full->set_bit(i);            
                }
                else
                {
                    bvect_min->clear_bit(j);
                    bvect_full->clear_bit(j);
                }

            }
        }

    } // for i

}

void FillSetClearIntervals(bvect_mini* bvect_min, 
              bvect* bvect_full,
              unsigned min, 
              unsigned max,
              unsigned fill_factor)
{
    FillSetsIntervals(bvect_min, bvect_full, min, max, fill_factor, true);
    FillSetsIntervals(bvect_min, bvect_full, min, max, fill_factor, false);
}

void FillSetsRandomOne(bvect_mini* bvect_min, 
					   bvect* bvect_full,
					   unsigned min, 
                       unsigned max)
{
	unsigned range = max - min;
	unsigned bit_idx = rand() % range;
	bvect_min->set_bit(bit_idx);
	bvect_full->set_bit(bit_idx);
    cout << "Bit_idx=" << bit_idx << endl;
}

void FillSetsRandom(bvect_mini* bvect_min, 
              bvect* bvect_full,
              unsigned min, 
              unsigned max,
              unsigned fill_factor)
{
    unsigned diap = max - min;

    unsigned count;


    switch (fill_factor)
    {
    case 0:
        count = diap / 1000;
        break;
    case 1:
        count = diap / 100;
        break;
    default:
        count = diap / 10;
        break;

    }

    for (unsigned i = 0; i < count; ++i)
    {
        unsigned bn = rand() % count;
        bn += min;

        if (bn > max)
        {
            bn = max;
        }
        bvect_min->set_bit(bn);
        bvect_full->set_bit(bn);   
        
        if ( (i  % PROGRESS_PRINT) == 0)
        {
            cout << "+" << flush;
        }
    }
    cout << "Ok" << endl;

}


//
//  Quasi random filling with choosing randomizing method.
//
//
void FillSetsRandomMethod(bvect_mini* bvect_min, 
                          bvect* bvect_full,
                          unsigned min, 
                          unsigned max,
                          int optimize = 0,
                          int method = -1)
{
    if (method == -1)
    {
        method = rand() % 6;
    }
    unsigned factor;
//method = 0;
    switch (method)
    {

    case 0:
        cout << "Random filling: method - FillSets - factor(0)" << endl;
        FillSets(bvect_min, bvect_full, min, max, 0);
        break;

    case 1:
        cout << "Random filling: method - FillSets - factor(random)" << endl;
        factor = rand()%3;
        FillSets(bvect_min, bvect_full, min, max, factor?factor:1);
        break;

    case 2:
        cout << "Random filling: method - Set-Clear Intervals - factor(random)" << endl;
        factor = rand()%10;
        FillSetClearIntervals(bvect_min, bvect_full, min, max, factor);
        break;
    case 3:
        cout << "Random filling: method - FillRandom - factor(random)" << endl;
        factor = rand()%3;
        FillSetsRandom(bvect_min, bvect_full, min, max, factor?factor:1);
        break;
	case 4:
		cout << "Random set one bit" << endl;
		FillSetsRandomOne(bvect_min, bvect_full, min, max);
		break;
    default:
        cout << "Random filling: method - Set Intervals - factor(random)" << endl;
        factor = rand()%10;
        FillSetsIntervals(bvect_min, bvect_full, min, max, factor);
        break;

    } // switch

    if (optimize && (method <= 1))
    {
        cout << "Vector optimization..." << flush;
        bvect_full->optimize();
        cout << "OK" << endl;
    }
}

// do logical operation through serialization
unsigned SerializationOperation(bvect*             bv_target,
                                /*const*/ bvect&   bv1,
                                /*const*/ bvect&   bv2,
                                set_operation      op,
                                bool               check_reverse=false)
{
    bvect bv_tmp;
    if (!bv_target)
    {
        bv_target = &bv_tmp;
    }

    if (op == set_COUNT_SUB_AB ||
        op == set_COUNT_SUB_BA)
    {
        check_reverse = false;
    }

    // serialize input vectors
    bvect::statistics *st1_op, *st2_op;
    st1_op = new bvect::statistics;
    st2_op = new bvect::statistics;

    bv1.optimize(0, bvect::opt_compress, st1_op);
    bv2.optimize(0, bvect::opt_compress, st2_op);


   struct bvect::statistics st1, st2;
   bv1.calc_stat(&st1);
   bv2.calc_stat(&st2);


   if (st1.max_serialize_mem > st1_op->max_serialize_mem)
   {
       cout << "Optimize failed to compute max_serialize_mem" << endl;
       cout << "calc_stat=" << st1.max_serialize_mem << endl;
       cout << "optimize=" << st1_op->max_serialize_mem << endl;
       exit(1);
   }
   if (st2.max_serialize_mem > st2_op->max_serialize_mem)
   {
       cout << "Optimize failed to compute max_serialize_mem" << endl;
       cout << "calc_stat=" << st2.max_serialize_mem << endl;
       cout << "optimize=" << st2_op->max_serialize_mem << endl;
       exit(1);
   }

   delete st1_op;
   delete st2_op;

   unsigned char* smem1 = new unsigned char[st1.max_serialize_mem];
   unsigned char* smem2 = new unsigned char[st2.max_serialize_mem];

   unsigned slen1 = bm::serialize(bv1, smem1);
   unsigned slen2 = bm::serialize(bv2, smem2);

   if (slen1 > st1.max_serialize_mem || slen2 > st2.max_serialize_mem)
   {
       cout << "Serialization override detected!" << endl;
       exit(1);
   }


   unsigned count =
       operation_deserializer<bvect>::deserialize(*bv_target,
                                                  smem1,
                                                  0,
                                                  set_ASSIGN);
   cout << slen1 << " " << slen2 << endl;
   int res = bv1.compare(*bv_target);
   if (res != 0)
   {
       cout << "---------------------------------- " << endl;
       cout << "bv1.count()=" << bv1.count() << endl;
       print_stat(bv1);
       cout << "---------------------------------- " << endl;
       cout << "bv_target.count()=" << bv_target->count() << endl;
       print_stat(*bv_target);
       
       bv_target->bit_xor(bv1);
       cout << "First diff=" << bv_target->get_first() << endl;
       cout << "set_ASSIGN 1 failed!" << endl;
       exit (1);
   }
   cout << "Deserialization ASSIGN into bv1 OK" << endl;

   {
       bvect* bv_tmp2 = new bvect();
       bm::deserialize(*bv_tmp2, smem1);
       if (*bv_tmp2 != bv1)
       {
           cout << "Deserialize NOT equal to Operation deserialize!" << endl;
           exit(1);
       }
       delete bv_tmp2;
   }


   cout << "Operation deserialization... " << op << endl;
    count=
       operation_deserializer<bvect>::deserialize(*bv_target,
                                                  smem2,
                                                  0,
                                                  op);
    cout << "OK" << endl;

    // check if operation was ok
    {
        bvect bvt(bv1);
        switch(op)
        {
        case bm::set_OR:
            bvt |= bv2;
            break;
        case bm::set_XOR:
            bvt ^= bv2;
            break;
        case bm::set_AND:
            bvt &= bv2;
            break;
        case bm::set_SUB:
            bvt -= bv2;
            break;
        default:
            goto no_compare;
        }
        if (bvt.compare(*bv_target) != 0)
        {
            cout << "Direct Serial operation comparison failed!" << endl;
            exit(1);
        }
        no_compare:
        ;

    }
/*    
    if (op == bm::set_AND || op == bm::set_OR || op == bm::set_XOR || op == bm::set_SUB)
    {
        cout << "3 way operation check... " << op << endl;
        operation_deserializer<bvect>::deserialize(*bv_target,
                                                   bv1,
                                                   smem2,
                                                   0,
                                                   op);
        cout << "OK" << endl;

        bvect bvt(bv1);
        switch(op)
        {
        case bm::set_OR:
            bvt |= bv2;
            break;
        case bm::set_XOR:
            bvt ^= bv2;
            break;
        case bm::set_AND:
            bvt &= bv2;
            break;
        case bm::set_SUB:
            bvt -= bv2;
            break;
        default:
            goto no_compare2;
        }
        if (bvt.compare(*bv_target) != 0)
        {
            cout << "3-way Serial operation comparison failed!" << endl;
            exit(1);
        }
        no_compare2:
        ;
    }
*/

   if (check_reverse)
   {
        cout << "Reverse check... " << endl;
        bvect bv_tmp2(BM_GAP);
        operation_deserializer<bvect>::deserialize(bv_tmp2,
                                                   smem2,
                                                   0,
                                                   set_ASSIGN);
        int res = bv_tmp2.compare(bv2);
        if (res != 0)
        {
            cout << "set_ASSIGN failed 2! " << endl;
            exit(1);
        }
        cout << "Deserialization assign to bv_tmp2 OK" << endl;
        unsigned count_rev =
        operation_deserializer<bvect>::deserialize(bv_tmp2,
                                                   smem1,
                                                   0,
                                                   op);
        if (count != count_rev)
        {
//            print_stat(bv1);
/*
            unsigned c = count_or(bv1, bv2);
            cout << "Correct count=" << c << endl;

            c = count_or(bv2, bv1);
            cout << "Correct count=" << c << endl;

            bv1 |= bv2;
            cout << "Count3 = " << bv1.count() << endl;;
*/
            //SaveBVector("err1.bv", bv1);
            //SaveBVector("err2.bv", bv2);

            

            cout << "Operation=" << op << endl;

            cout << "Serialization operation reverse check failed"
                 << " count = " << count 
                 << " count rev= " << count_rev
                 << endl;
            cout << "See bvector dumps: err1.bv, err2.bv" << endl;

            exit(1);
        }

   }

   delete [] smem1;
   delete [] smem2;

   return count;
}

void SerializationOperation2Test(bvect*        bv_target,
                                 bvect&        bv1,
                                 bvect&        bv2,
                                 unsigned      predicted_count,
                                 set_operation op_count,
                                 set_operation op_combine)
{
    bv_target->clear(true);
    cout << "Serialization operation count..." << endl;

    unsigned scount1 = SerializationOperation(0, 
                                              bv1,
                                              bv2,
                                              op_count,
                                              true //reverse check
                                              );
    cout << "Serialization operation count OK." << endl;

    cout << "Serialization operation. " << endl;
    unsigned scount2 = SerializationOperation(bv_target, 
                                              bv1,
                                              bv2,
                                              op_combine);
    scount2 = bv_target->count();
    if (predicted_count != scount2 || scount1 != scount2)
    {
        cout << "Serialization count != predicted" << endl 
             << " predicted=" << predicted_count 
             << " scount1="   << scount1
             << " scount2="   << scount2
             << endl;

        cout << endl << "target:" << endl;
        print_stat(*bv_target);
        cout << endl << endl << "Reference" << endl;
        if (op_combine == set_OR)
        {
            bv1 |= bv2;
            if (bv1 != *bv_target)
            {
                cout << "Comparison OR error!" << endl;
            }
            cout << "OR operation count=" << bv1.count() << endl;
            print_stat(bv1);
        } else
        if (op_combine == set_AND)
        {
            bv1 &= bv2;
            print_stat(bv1);
        }

        exit(1);
    }
    cout << "OK" << endl;
}


void print_mv(const bvect_mini &bvect_min, unsigned size)
{
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        bool bflag = bvect_min.is_bit_true(i) != 0; 

        if (bflag)
            printf("1");
        else
            printf("0");
        if ((i % 31) == 0 && (i != 0))
            printf(".");
    }

    printf("\n");
}

void print_gap(const gap_vector& gap_vect, unsigned size)
{
    const gap_word_t *buf = gap_vect.get_buf();
    unsigned len = gap_length(buf);
    printf("[%i:]", *buf++ & 1);

    for (unsigned i = 1; i < len; ++i)
    {
        printf("%i,", *buf++);
    }

    printf("\n");
}

void CheckGAPMin(const gap_vector& gapv, const bvect_mini& bvect_min, unsigned len)
{
    for (unsigned i = 0; i < len; ++i)
    {
        int bit1 = (gapv.is_bit_true(i) == 1);
        int bit2 = (bvect_min.is_bit_true(i) != 0);
        if(bit1 != bit2)
        {
           cout << "Bit comparison failed. " << "Bit N=" << i << endl;
           assert(0);
           exit(1);
        }
    }
}

void CheckIntervals(const bvect& bv, unsigned max_bit)
{
    unsigned cnt0 = count_intervals(bv);
    unsigned cnt1 = 1;
    bool bit_prev = bv.test(0);
    for (unsigned i = 1; i < max_bit; ++i)
    {
        bool bit = bv.test(i);
        cnt1 += bit_prev ^ bit;
        bit_prev = bit;
    }
    if (cnt0 != cnt1)
    {
        cout << "CheckIntervals error. " << "bm count=" << cnt0
             << " Control = " << cnt1 << endl;
        exit(1);
    }
}

template<class T> void CheckCountRange(const T& vect, 
                                       unsigned left, 
                                       unsigned right,
                                       unsigned* block_count_arr=0)
{
    unsigned cnt1 = vect.count_range(left, right, block_count_arr);
    unsigned cnt2 = 0;
//cout << endl;
    for (unsigned i = left; i <= right; ++i)
    {
        if (vect.test(i))
        {
//            cout << i << " " << flush;
            ++cnt2;
        }
    }
//cout << endl;
    if (cnt1 != cnt2)
    {
        cout << "Bitcount range failed!" << "left=" << left 
             << " right=" << right << endl
             << "count_range()=" << cnt1 
             << " check=" << cnt2;
        exit(1);
    }
}


unsigned BitCountChange(unsigned word)
{
    unsigned count = 1;
    unsigned bit_prev = word & 1;
    word >>= 1;
    for (unsigned i = 1; i < 32; ++i)
    {
        unsigned bit = word & 1;
        count += bit ^ bit_prev;
        bit_prev = bit;
        word >>= 1;
    }
    return count;
}


void DetailedCheckVectors(const bvect_mini &bvect_min, 
                          const bvect      &bvect_full,
                          unsigned size)
{
    cout << "Detailed check" << endl;

    //bvect_full.stat();

    // detailed bit by bit comparison. Paranoia check.

    unsigned i;
    for (i = 0; i < size; ++i)
    {
        bool bv_m_flag = bvect_min.is_bit_true(i) != 0; 
        bool bv_f_flag = bvect_full.get_bit(i) != 0;

        if (bv_m_flag != bv_f_flag)
        {
            printf("Bit %u is non conformant. vect_min=%i vect_full=%i\n",
                i, (int)bv_m_flag, (int)bv_f_flag);

            cout << "Non-conformant block number is: " << unsigned(i >>  bm::set_block_shift) << endl;
//            throw 110;
            exit(1);
        }

        if (PROGRESS_PRINT)
        {
            if ( (i % PROGRESS_PRINT) == 0)
            {
                printf(".");
            }
        }
             
    }
    
    printf("\n detailed check ok.\n");

}


// vectors comparison check

void CheckVectors(bvect_mini &bvect_min, 
                  bvect      &bvect_full,
                  unsigned size,
                  bool     detailed)
{
    cout << "\nVectors checking...bits to compare = " << size << endl;

    cout << "Bitcount summary : " << endl;
    unsigned min_count = bvect_min.bit_count();
    cout << "minvector count = " << min_count << endl;
    unsigned count = bvect_full.count();
    unsigned full_count = bvect_full.recalc_count();
    cout << "fullvector re-count = " << full_count << endl;
    
    if (min_count != full_count)
    {
        cout << "fullvector count = " << count << endl;
        cout << "Count comparison failed !!!!" << endl;
        print_stat(bvect_full);
        DetailedCheckVectors(bvect_min, bvect_full, size);

        exit(1);  
    } 

    if (full_count)
    {
        bool any = bvect_full.any();
        if (!any)
        {
            cout << "Anycheck failed!" << endl;
            exit(1);
        }
    }

    // get_next comparison

    cout << "Positive bits comparison..." << flush;
    unsigned nb_min = bvect_min.get_first();
    unsigned nb_ful = bvect_full.get_first();

    bvect::counted_enumerator en = bvect_full.first();
    unsigned nb_en = *en;
    if (nb_min != nb_ful)
    {
         cout << "!!!! First bit comparison failed. Full id = " 
              << nb_ful << " Min id = " << nb_min 
              << endl;

         bool bit_f = bvect_full.get_bit(nb_ful);
         cout << "Full vector'd bit #" << nb_ful << "is:" 
              << bit_f << endl;

         bool bit_m = (bvect_min.is_bit_true(nb_min) == 1);
         cout << "Min vector'd bit #" << nb_min << "is:" 
              << bit_m << endl;


         print_stat(bvect_full);

         DetailedCheckVectors(bvect_min, bvect_full, size);

         exit(1);
    }

    if (full_count)
    {
       unsigned bit_count = 1;
       unsigned en_prev = nb_en;

       do
       {
           nb_min = bvect_min.get_next(nb_min);
           if (nb_min == 0)
           {
               break;
           }

           en_prev = nb_en;
           ++en;

           nb_en = *en;
//           nb_en = bvect_full.get_next(nb_en);

           ++bit_count;

           if (nb_en != nb_min)
           {
               nb_ful = bvect_full.get_next(en_prev);
               cout << "!!!!! next bit comparison failed. Full id = " 
                    << nb_ful << " Min id = " << nb_min 
                    << " Enumerator = " << nb_en
                    << endl;

     //          bvect_full.stat();

     //          DetailedCheckVectors(bvect_min, bvect_full, size);

               exit(1);
           }
            if ( (bit_count % PROGRESS_PRINT) == 0)
           {
                cout << "." << flush;
            }

       } while (en.valid());
       if (bit_count != min_count)
       {
           cout << " Bit count failed."  
                << " min = " << min_count 
                << " bit = " << bit_count 
                << endl;
           exit(1);
       }
    }

    cout << "OK" << endl;

    return;
}


void ClearAllTest()
{
    bvect     bvect_full;

    for (int i = 0; i < 100000; ++i)
    {
        bvect_full.set_bit(i);
    }
    bvect_full.optimize();
    bvect_full.clear();

    print_stat(bvect_full);

    int count = bvect_full.count();
    assert(count == 0);
    print_stat(bvect_full);
}


void WordCmpTest()
{
    cout << "---------------------------- WordCmp test" << endl;

    for (int i = 0; i < 10000000; ++i)
    {
        unsigned w1 = rand();
        unsigned w2 = rand();
        int res = wordcmp0(w1, w2);
        int res2 = wordcmp(w1, w2);
        if (res != res2)
        {
            printf("WordCmp failed !\n");
            exit(1);
        }

        res = wordcmp0((unsigned)0U, (unsigned)w2);
        res2 = wordcmp((unsigned)0U, (unsigned)w2);

        if (res != res2)
        {
            printf("WordCmp 0 test failed !\n");
            exit(1);
        }

        res = wordcmp0((unsigned)~0U, (unsigned)w2);
        res2 = wordcmp((unsigned)~0U, (unsigned)w2);

        if (res != res2)
        {
            printf("WordCmp ~0 test failed !\n");
            exit(1);
        }

        res = wordcmp0((unsigned)w2, (unsigned)0);
        res2 = wordcmp((unsigned)w2, (unsigned)0);

        if (res != res2)
        {
            printf("WordCmp 0-2 test failed !\n");
            exit(1);
        }

    }

    cout << "Ok." << endl;
}

void BasicFunctionalityTest()
{
    cout << "---------------------------- Basic functinality test" << endl;

    assert(ITERATIONS < BITVECT_SIZE);


    bvect_mini     bvect_min(BITVECT_SIZE);
    bvect          bvect_full;
    bvect          bvect_full1;

    printf("\nBasic functionality test.\n");
    
    // filling vectors with regular values

    unsigned i;
    for (i = 0; i < ITERATIONS; ++i)
    {
        bvect_min.set_bit(i);
        bvect_full.set_bit(i);
    }
    
    bvect_full1.set_range(0, ITERATIONS-1);
    
    CheckCountRange(bvect_full, 0, ITERATIONS);
    CheckCountRange(bvect_full, 10, ITERATIONS+10);

    if (bvect_full1 != bvect_full)
    {
        cout << "set_range failed!" << endl;
        print_stat(bvect_full1);
        exit(1);
    }

    print_stat(bvect_full);
    print_stat(bvect_full1);

    // checking the results
    unsigned count_min = 0;
    for (i = 0; i < ITERATIONS; ++i)
    {
        if (bvect_min.is_bit_true(i))
            ++count_min;
    }


    
    unsigned count_full = bvect_full.count();

    if (count_min == count_full)
    {
        printf("simple count test ok.\n");
    }
    else
    {
        printf("simple count test failed count_min = %i  count_full = %i\n", 
               count_min, count_full);
        exit(1);
    }


    // detailed vectors verification

    CheckVectors(bvect_min, bvect_full, ITERATIONS);

    // now clearning

    for (i = 0; i < ITERATIONS; i+=2)
    {
        bvect_min.clear_bit(i);
        bvect_full.clear_bit(i);
        bvect_full1.set_range(i, i, false);
    }

    CheckVectors(bvect_min, bvect_full, ITERATIONS);
    CheckVectors(bvect_min, bvect_full1, ITERATIONS);

    for (i = 0; i < ITERATIONS; ++i)
    {
        bvect_min.clear_bit(i);
    }
    bvect_full.clear();

    CheckVectors(bvect_min, bvect_full, ITERATIONS);

    cout << "Random step filling" << endl;

    for (i = rand()%10; i < ITERATIONS; i+=rand()%10)
    {
        bvect_min.clear_bit(i);
        bvect_full.clear_bit(i);
    }
    
    CheckVectors(bvect_min, bvect_full, ITERATIONS);

    bvect bv1;
    bvect bv2;

    bv1[10] = true;
    bv1[1000] = true;

    bv2[200] = bv2[700] = bv2[500] = true;

    bv1.swap(bv2);

    if (bv1.count() != 3)
    {
        cout << "Swap test failed!" << endl;
        exit(1);
    }

    if (bv2.count() != 2)
    {
        cout << "Swap test failed!" << endl;
        exit(1);
    }
}



void TestRandomSubset(const bvect& bv, bm::random_subset<bvect>& rsub)
{
    bvect bv_subset;
    unsigned bcnt = bv.count();

    unsigned samples[] = 
      { 0, 1, 2, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, bcnt / 5, bcnt / 4, bcnt / 3, bcnt / 2, (bcnt * 2)/3, bcnt };
    unsigned samples_size = sizeof(samples)/sizeof(*samples);

    printf("Taking random sub-sets: ");
    
    for (unsigned i = 0; i < samples_size; ++i)
    {
        unsigned sample_count = samples[i];
        printf(" %u, ", sample_count);
        rsub.sample(bv_subset, bv, sample_count);
        if (sample_count > bcnt)
            sample_count = bcnt;

        if (sample_count != bv_subset.count())
        {
            printf("\nRandom subset failed! sample_count = %u result_count=%u\n", 
                   sample_count,
                   bv_subset.count());
            exit(1);
        }
        bv_subset -= bv;
        if (bv_subset.count() != 0)
        {
            printf("\nRandom subset failed! Extra bits set! \n");
            exit(1);
        }    
    }
    printf("\n");
}

void SimpleRandomFillTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    bm::random_subset<bvect> rsub;

    cout << "-------------------------- SimpleRandomFillTest" << endl;

    printf("Test for Random inverted subset.");

    {
        bvect bv;
        bv.invert();
        TestRandomSubset(bv, rsub);
    }


    {
    printf("Simple random fill test 1.");
    bvect_mini   bvect_min(BITVECT_SIZE);
    bvect      bvect_full;
    bvect_full.set_new_blocks_strat(bm::BM_BIT);


    unsigned iter = ITERATIONS / 5;

    printf("\nSimple Random fill test ITERATIONS = %i\n", iter);

    bvect_min.set_bit(0);
    bvect_full.set_bit(0);

    unsigned i;
    for (i = 0; i < iter; ++i)
    {
        unsigned num = ::rand() % iter;
        bvect_min.set_bit(num);
        bvect_full.set_bit(num);
        if ((i % 1000) == 0) cout << "." << flush;
        CheckCountRange(bvect_full, 0, num);
        CheckCountRange(bvect_full, num, num+iter);
    }

    CheckVectors(bvect_min, bvect_full, iter);
    CheckCountRange(bvect_full, 0, iter);

    TestRandomSubset(bvect_full, rsub);

    printf("Simple random fill test 2.");

    for(i = 0; i < iter; ++i)
    {
        unsigned num = ::rand() % iter;
        bvect_min.clear_bit(num);
        bvect_full.clear_bit(num);
    }

    CheckVectors(bvect_min, bvect_full, iter);
    }


    {
    printf("\nSimple random fill test 3.\n");
    bvect_mini   bvect_min(BITVECT_SIZE);
    bvect      bvect_full(bm::BM_GAP);


    unsigned iter = ITERATIONS;

    printf("\nSimple Random fill test ITERATIONS = %i\n", iter);

    unsigned i;
    for(i = 0; i < iter; ++i)
    {
        unsigned num = ::rand() % iter;
        bvect_min.set_bit(num);
        bvect_full.set_bit(num);
        CheckCountRange(bvect_full, 0, 65535);
        CheckCountRange(bvect_full, 0, num);
        CheckCountRange(bvect_full, num, num+iter);
        if ((i % 1000) == 0) cout << "." << flush;
    }

    CheckVectors(bvect_min, bvect_full, iter);

    TestRandomSubset(bvect_full, rsub);

    printf("Simple random fill test 4.");

    for(i = 0; i < iter; ++i)
    {
        unsigned num = ::rand() % iter;
        bvect_min.clear_bit(num);
        bvect_full.clear_bit(num);
        CheckCountRange(bvect_full, 0, num);
        CheckCountRange(bvect_full, num, num+iter);
        if ((i % 1000) == 0) cout << "." << flush;
    }

    CheckVectors(bvect_min, bvect_full, iter);
    CheckCountRange(bvect_full, 0, iter);

    TestRandomSubset(bvect_full, rsub);
    }

}




void RangeRandomFillTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    cout << "----------------------------------- RangeRandomFillTest" << endl;

    {
    bvect_mini   bvect_min(BITVECT_SIZE);
    bvect     bvect_full;

    printf("Range Random fill test\n");

    unsigned min = BITVECT_SIZE / 2;
    unsigned max = BITVECT_SIZE / 2 + ITERATIONS;
    if (max > BITVECT_SIZE) 
        max = BITVECT_SIZE - 1;

    FillSets(&bvect_min, &bvect_full, min, max, 0);

    CheckVectors(bvect_min, bvect_full, BITVECT_SIZE);
    CheckCountRange(bvect_full, min, max);

    }

    
    {
    bvect_mini   bvect_min(BITVECT_SIZE);
    bvect     bvect_full;

    printf("Range Random fill test\n");

    unsigned min = BITVECT_SIZE / 2;
    unsigned max = BITVECT_SIZE / 2 + ITERATIONS;
    if (max > BITVECT_SIZE) 
        max = BITVECT_SIZE - 1;

    FillSetsIntervals(&bvect_min, &bvect_full, min, max, 4);

    CheckVectors(bvect_min, bvect_full, BITVECT_SIZE);
    CheckCountRange(bvect_full, min, max);
    }
    

}



void AndOperationsTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    cout << "----------------------------------- AndOperationTest" << endl;

    {

    bvect_mini   bvect_min1(256);
    bvect_mini   bvect_min2(256);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);



    printf("AND test\n");

    bvect_min1.set_bit(1);
    bvect_min1.set_bit(12);
    bvect_min1.set_bit(13);

    bvect_min2.set_bit(12);
    bvect_min2.set_bit(13);

    bvect_min1.combine_and(bvect_min2);

    bvect_full1.set_bit(1);
    bvect_full1.set_bit(12);
    bvect_full1.set_bit(13);

    bvect_full2.set_bit(12);
    bvect_full2.set_bit(13);

    bm::id_t predicted_count = bm::count_and(bvect_full1, bvect_full2);

    bm::id_t predicted_any = bm::any_and(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_AND,
                                set_AND);


    bvect_full1.bit_and(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, 256);
    CheckVectors(bvect_min1, bv_target_s, 256);
    CheckCountRange(bvect_full1, 0, 256);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;


    printf("AND test stage 1.\n");

    for (int i = 0; i < 112; ++i)
    {
        bvect_min1.set_bit(i);
        bvect_full1.set_bit(i);

        bvect_min2.set_bit(i);
        bvect_full2.set_bit(i);

    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

//    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/7, 0);
//    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/7, 0);

    bvect_min1.combine_and(bvect_min2);

    bm::id_t predicted_count = bm::count_and(bvect_full1,bvect_full2);
    bm::id_t predicted_any = bm::any_and(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_AND,
                                set_AND);

    bvect_full1.bit_and(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

    }


    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    printf("AND test stage 2.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/7, 0);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/7, 0);

    bm::id_t predicted_count = bm::count_and(bvect_full1,bvect_full2);
    bm::id_t predicted_any = bm::any_and(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_AND,
                                set_AND);

    bvect_min1.combine_and(bvect_min2);

    bvect_full1.bit_and(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        print_stat(bvect_full1);
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_BIT);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);

    cout << "------------------------------" << endl;
    printf("AND test stage 3.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/5, 2);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/5, 2);

    bvect_min1.combine_and(bvect_min2);

    bm::id_t predicted_count = bm::count_and(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_and(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }
    
    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_AND,
                                set_AND);

    bvect_full1.bit_and(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE);

    bvect_full1.optimize();
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE);
    CheckCountRange(bvect_full1, BITVECT_SIZE/2, BITVECT_SIZE);

    }

    cout << "------------------------------" << endl;
    printf("AND test stage 4. combine_and_sorted\n");
    {
    unsigned ids[] = {0, 1, 2, 3, 10, 65535, 65536, 65535*2, 65535*3};
    unsigned to_add = sizeof(ids)/sizeof(unsigned);
    bvect        bvect_full1;
    bvect        bvect_full2;    
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);
    
    for (unsigned i = 2; i < to_add; ++i)
    {
        bvect_full1.set(ids[i]);
        bvect_min1.set_bit(ids[i]);
        bvect_full2.set(ids[i]);
        bvect_min2.set_bit(ids[i]);
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bvect_min1.combine_and(bvect_min2);

    bm::combine_and_sorted(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    }

}


void OrOperationsTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    cout << "----------------------------------- OrOperationTest" << endl;

    {

    bvect_mini   bvect_min1(256);
    bvect_mini   bvect_min2(256);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);



    printf("OR test\n");

    bvect_min1.set_bit(1);
    bvect_min1.set_bit(12);
    bvect_min1.set_bit(13);

    bvect_min2.set_bit(12);
    bvect_min2.set_bit(13);

    bvect_min1.combine_or(bvect_min2);

    bvect_full1.set_bit(1);
    bvect_full1.set_bit(12);
    bvect_full1.set_bit(13);

    bvect_full2.set_bit(12);
    bvect_full2.set_bit(13);
    
    bm::id_t predicted_count = bm::count_or(bvect_full1, bvect_full2);    
    bm::id_t predicted_any = bm::any_or(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_OR,
                                set_OR);


    bvect_full1.bit_or(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        cout << predicted_count << " " << count << endl;
        print_stat(bvect_full1);
        exit(1);
    }


    CheckVectors(bvect_min1, bvect_full1, 256);
    CheckVectors(bvect_min1, bv_target_s, 256);
    CheckCountRange(bvect_full1, 0, 256);
    CheckCountRange(bvect_full1, 128, 256);
    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    printf("OR test stage 2.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/7, 0);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/7, 0);

    bvect_min1.combine_or(bvect_min2);

    bm::id_t predicted_count = bm::count_or(bvect_full1, bvect_full2);    
    bm::id_t predicted_any = bm::any_or(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_OR,
                                set_OR);


    bvect_full1.bit_or(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_BIT);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);

    cout << "------------------------------" << endl;
    printf("OR test stage 3.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/5, 2);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/5, 2);

    bvect_min1.combine_or(bvect_min2);
    unsigned mcnt = bvect_min1.bit_count();

    cout << mcnt << endl;
    
    bm::id_t predicted_count = bm::count_or(bvect_full1, bvect_full2);    
    cout << predicted_count << endl;
    bm::id_t predicted_any = bm::any_or(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_OR,
                                set_OR);

    bvect_full1.bit_or(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bvect_full1.optimize();

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE);


    }
    
    cout << "Testing combine_or" << endl;
    
    {
    
    bvect        bvect_full1;
    bvect        bvect_full2;
    bvect_mini   bvect_min1(BITVECT_SIZE);
    
    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    unsigned ids[10000];
    unsigned to_add = 10000;
    
    unsigned bn = 0;
    for (unsigned i = 0; i < to_add; ++i)
    {
        ids[i] = bn;
        bvect_full2.set(bn);
        bvect_min1.set_bit(bn);
        bn += 15;
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_or(bvect_full1, first, last);

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    
    bm::combine_or(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    
    }
    
    
    {
    unsigned ids[] = {0, 65536, 65535, 65535*3, 65535*2, 10};
    unsigned to_add = sizeof(ids)/sizeof(unsigned);
    bvect        bvect_full1;
    bvect        bvect_full2;    
    bvect_mini   bvect_min1(BITVECT_SIZE);

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);
    
    unsigned bn = 0;
    for (unsigned i = 0; i < to_add; ++i)
    {
        ids[i] = bn;
        bvect_full2.set(bn);
        bvect_min1.set_bit(bn);
        bn += 15;
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_or(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bm::combine_or(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);    
    }
    

}



void SubOperationsTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    cout << "----------------------------------- SubOperationTest" << endl;

    {

    bvect_mini   bvect_min1(256);
    bvect_mini   bvect_min2(256);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);



    printf("SUB test\n");

    bvect_min1.set_bit(1);
    bvect_min1.set_bit(12);
    bvect_min1.set_bit(13);

    bvect_min2.set_bit(12);
    bvect_min2.set_bit(13);

    bvect_min1.combine_sub(bvect_min2);

    bvect_full1.set_bit(1);
    bvect_full1.set_bit(12);
    bvect_full1.set_bit(13);

    bvect_full2.set_bit(12);
    bvect_full2.set_bit(13);

    bm::id_t predicted_count = bm::count_sub(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_sub(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_SUB_AB,
                                set_SUB);


    bvect_full1.bit_sub(bvect_full2);
    
    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, 256);
    CheckVectors(bvect_min1, bv_target_s, 256);
    CheckCountRange(bvect_full1, 0, 256);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    printf("SUB test stage 2.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/7, 0);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/7, 0);

    bvect_min1.combine_sub(bvect_min2);

    bm::id_t predicted_count = bm::count_sub(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_sub(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_SUB_AB,
                                set_SUB);

    bvect_full1.bit_sub(bvect_full2);
    
    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        cout << predicted_count << " " << count << endl;
        print_stat(bvect_full1);    
        
        exit(1);
    }
    

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_BIT);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);

    cout << "------------------------------" << endl;
    printf("SUB test stage 3.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/5, 2);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/5, 2);

    bvect_min1.combine_sub(bvect_min2);
    
    bm::id_t predicted_count = bm::count_sub(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_sub(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_SUB_AB,
                                set_SUB);

    bvect_full1.bit_sub(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "Predicted count error!" << endl;
        exit(1);
    }


    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bvect_full1.optimize();
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE);

    }

}



void XorOperationsTest()
{
    assert(ITERATIONS < BITVECT_SIZE);

    cout << "----------------------------------- XorOperationTest" << endl;
    {

    bvect_mini   bvect_min1(256);
    bvect_mini   bvect_min2(256);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);



    printf("XOR test\n");

    bvect_min1.set_bit(1);
    bvect_min1.set_bit(12);
    bvect_min1.set_bit(13);

    bvect_min2.set_bit(12);
    bvect_min2.set_bit(13);

    bvect_min1.combine_xor(bvect_min2);

    bvect_full1.set_bit(1);
    bvect_full1.set_bit(12);
    bvect_full1.set_bit(13);

    bvect_full2.set_bit(12);
    bvect_full2.set_bit(13);

    bm::id_t predicted_count = bm::count_xor(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_xor(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_XOR,
                                set_XOR);


    bvect_full1.bit_xor(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "1.Predicted count error!" << endl;
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, 256);
    CheckVectors(bvect_min1, bv_target_s, 256);
    CheckCountRange(bvect_full1, 0, 256);
    CheckCountRange(bvect_full1, 128, 256);

    }
    {
        bvect  bvect1;
        bvect_mini  bvect_min1(BITVECT_SIZE);

        bvect  bvect2;
        bvect_mini  bvect_min2(BITVECT_SIZE);


        for (int i = 0; i < 150000; ++i)
        {
            bvect2.set_bit(i);
            bvect_min2.set_bit(i);
        }

        bvect2.optimize();

        bm::id_t predicted_count = bm::count_xor(bvect1, bvect2);
        bm::id_t predicted_any = bm::any_xor(bvect1, bvect2);
        if (predicted_any == 0 && predicted_count != 0)
        {
            cout << "Predicted any error!" << endl;
            exit(1);
        }

        bvect    bv_target_s;
        SerializationOperation2Test(&bv_target_s,
                                    bvect1,
                                    bvect2,
                                    predicted_count,
                                    set_COUNT_XOR,
                                    set_XOR);

        bvect1.bit_xor(bvect2);
        
        bm::id_t count = bvect1.count();
        if (count != predicted_count)
        {
            cout << "2.Predicted count error!" << endl;
            exit(1);
        }
        
        bvect_min1.combine_xor(bvect_min2);
        CheckVectors(bvect_min1, bvect1, BITVECT_SIZE, true);
        CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE, true);
        CheckCountRange(bvect1, 0, BITVECT_SIZE);
    }


    {
        bvect  bvect1;
        bvect_mini  bvect_min1(BITVECT_SIZE);

        bvect  bvect2;
        bvect_mini  bvect_min2(BITVECT_SIZE);


        for (int i = 0; i < 150000; ++i)
        {
            bvect1.set_bit(i);
            bvect_min1.set_bit(i);
        }

        bvect1.optimize();
        
        bm::id_t predicted_count = bm::count_xor(bvect1, bvect2);
        bm::id_t predicted_any = bm::any_xor(bvect1, bvect2);
        if (predicted_any == 0 && predicted_count != 0)
        {
            cout << "Predicted any error!" << endl;
            exit(1);
        }

        bvect    bv_target_s;
        SerializationOperation2Test(&bv_target_s,
                                    bvect1,
                                    bvect2,
                                    predicted_count,
                                    set_COUNT_XOR,
                                    set_XOR);

        bvect1.bit_xor(bvect2);

        bm::id_t count = bvect1.count();
        if (count != predicted_count)
        {
            cout << "3.Predicted count error!" << endl;
            exit(1);
        }
        
        bvect_min1.combine_xor(bvect_min2);
        CheckVectors(bvect_min1, bvect1, BITVECT_SIZE, true);
        CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE, true);
    }


    {
        bvect  bvect1;
        bvect_mini  bvect_min1(BITVECT_SIZE);

        bvect  bvect2;
        bvect_mini  bvect_min2(BITVECT_SIZE);


        for (int i = 0; i < 150000; ++i)
        {
            bvect1.set_bit(i);
            bvect_min1.set_bit(i);
            bvect2.set_bit(i);
            bvect_min2.set_bit(i);
        }

        bvect1.optimize();
        
        bm::id_t predicted_count = bm::count_xor(bvect1, bvect2);
        bm::id_t predicted_any = bm::any_xor(bvect1, bvect2);
        if (predicted_any == 0 && predicted_count != 0)
        {
            cout << "Predicted any error!" << endl;
            exit(1);
        }

        bvect    bv_target_s;
        SerializationOperation2Test(&bv_target_s,
                                    bvect1,
                                    bvect2,
                                    predicted_count,
                                    set_COUNT_XOR,
                                    set_XOR);

        bvect1.bit_xor(bvect2);

        bm::id_t count = bvect1.count();
        if (count != predicted_count)
        {
            cout << "4.Predicted count error!" << endl;
            cout << count << " " << predicted_count << endl;
            
            exit(1);
        }
        
        bvect_min1.combine_xor(bvect_min2);
        CheckVectors(bvect_min1, bvect1, BITVECT_SIZE, true);
    }



    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    printf("XOR test stage 2.\n");

    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/7, 0);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/7, 0);

    bvect_min1.combine_xor(bvect_min2);
    
    bm::id_t predicted_count = bm::count_xor(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_xor(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_XOR,
                                set_XOR);


    bvect_full1.bit_xor(bvect_full2);
    
    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "5.Predicted count error!" << endl;
        cout << count << " " << predicted_count << endl;
        print_stat(bvect_full1);
        exit(1);
    }

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE/10+10);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE/10+10);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE/10+10);

    }

    {

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_BIT);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);

    cout << "------------------------------" << endl;
    printf("XOR test stage 3.\n");


    FillSets(&bvect_min1, &bvect_full1, 1, BITVECT_SIZE/5, 2);
    FillSets(&bvect_min2, &bvect_full2, 1, BITVECT_SIZE/5, 2);

    bm::id_t predicted_count = bm::count_xor(bvect_full1, bvect_full2);
    bm::id_t predicted_any = bm::any_xor(bvect_full1, bvect_full2);
    if (predicted_any == 0 && predicted_count != 0)
    {
        cout << "Predicted any error!" << endl;
        exit(1);
    }

    bvect    bv_target_s;
    SerializationOperation2Test(&bv_target_s,
                                bvect_full1,
                                bvect_full2,
                                predicted_count,
                                set_COUNT_XOR,
                                set_XOR);

    bvect_min1.combine_xor(bvect_min2);

    bvect_full1.bit_xor(bvect_full2);

    bm::id_t count = bvect_full1.count();
    if (count != predicted_count)
    {
        cout << "6.Predicted count error!" << endl;
        exit(1);
    }


    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bvect_full1.optimize();
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    CheckVectors(bvect_min1, bv_target_s, BITVECT_SIZE);
    CheckCountRange(bvect_full1, 0, BITVECT_SIZE);


    }


    cout << "Testing combine_xor" << endl;
    
    {
    
    bvect        bvect_full1;
    bvect        bvect_full2;
    bvect_mini   bvect_min1(BITVECT_SIZE);
    
    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    unsigned ids[10000];
    unsigned to_add = 10000;
    
    unsigned bn = 0;
    for (unsigned i = 0; i < to_add; ++i)
    {
        ids[i] = bn;
        bvect_full2.set(bn);
        bvect_min1.set_bit(bn);
        bn += 15;
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_xor(bvect_full1, first, last);

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    
    bm::combine_xor(bvect_full1, first, last);
    if (bvect_full1.count())
    {
        cout << "combine_xor count failed!" << endl;
        exit(1);
    }
    
    }

    {
    
    bvect        bvect_full1;
    bvect        bvect_full2;
    bvect_mini   bvect_min1(BITVECT_SIZE);
    
    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    unsigned ids[10000]={0,};
    unsigned to_add = 10000;
    
    for (unsigned i = 0; i < to_add; i+=100)
    {
        ids[i] = i;
        bvect_full2.set(i);
        bvect_min1.set_bit(i);
    }
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_xor(bvect_full1, first, last);

    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);
    
    bm::combine_xor(bvect_full1, first, last);
    if (bvect_full1.count())
    {
        cout << "combine_xor count failed!" << endl;
        exit(1);
    }
    
    }

    
    {
    unsigned ids[] = {0, 65536, 65535, 65535*3, 65535*2, 10};
    unsigned to_add = sizeof(ids)/sizeof(unsigned);
    bvect        bvect_full1;
    bvect        bvect_full2;    
    bvect_mini   bvect_min1(BITVECT_SIZE);

    bvect_full1.set_new_blocks_strat(bm::BM_BIT);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);
    
    unsigned bn = 0;
    for (unsigned i = 0; i < to_add; ++i)
    {
        ids[i] = bn;
        bvect_full2.set(bn);
        bvect_min1.set_bit(bn);
        bn += 15;
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_xor(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bm::combine_xor(bvect_full1, first, last);
    if (bvect_full1.count())
    {
        cout << "combine_xor count failed!" << endl;
        exit(1);
    }
    }
    
    
    {
    unsigned ids[] = {0, 65536, 65535, 65535*3, 65535*2, 10};
    unsigned to_add = sizeof(ids)/sizeof(unsigned);
    bvect        bvect_full1;
    bvect        bvect_full2;    
    bvect_mini   bvect_min1(BITVECT_SIZE);

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);
    
    unsigned bn = 0;
    for (unsigned i = 0; i < to_add; ++i)
    {
        ids[i] = bn;
        bvect_full2.set(bn);
        bvect_min1.set_bit(bn);
        bn += 15;
    }
    
    unsigned* first = ids;
    unsigned* last = ids + to_add;
    
    bm::combine_xor(bvect_full1, first, last);
    CheckVectors(bvect_min1, bvect_full1, BITVECT_SIZE);

    bm::combine_xor(bvect_full1, first, last);
    if (bvect_full1.count())
    {
        cout << "combine_xor count failed!" << endl;
        exit(1);
    }
    }

}


void ComparisonTest()
{
    cout << "-------------------------------------- ComparisonTest" << endl;

    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;
    int res1, res2;

    bvect_full1.set_bit(31); 
    bvect_full2.set_bit(63); 

    res1 = bvect_full1.compare(bvect_full2);
    if (res1 != 1)
    {
        printf("Comparison test failed 1\n");
        exit(1);
    }

    bvect_full1.clear();
    bvect_full2.clear();

    bvect_min1.set_bit(10);
    bvect_min2.set_bit(10);

    bvect_full1.set_bit(10);
    bvect_full2.set_bit(10);

    res1 = bvect_min1.compare(bvect_min2);
    res2 = bvect_full1.compare(bvect_full2);

    if (res1 != res2)
    {
        printf("Comparison test failed 1\n");
        exit(1);
    }

    printf("Comparison 2.\n");

    bvect_min1.set_bit(11);
    bvect_full1.set_bit(11);

    res1 = bvect_min1.compare(bvect_min2);
    res2 = bvect_full1.compare(bvect_full2);

    if (res1 != res2 && res1 != 1)
    {
        printf("Comparison test failed 2\n");
        exit(1);
    }

    res1 = bvect_min2.compare(bvect_min1);
    res2 = bvect_full2.compare(bvect_full1);

    if (res1 != res2 && res1 != -1)
    {
        printf("Comparison test failed 2.1\n");
        exit(1);
    }

    printf("Comparison 3.\n");

    bvect_full1.optimize();

    res1 = bvect_min1.compare(bvect_min2);
    res2 = bvect_full1.compare(bvect_full2);

    if (res1 != res2 && res1 != 1)
    {
        printf("Comparison test failed 3\n");
        exit(1);
    }

    res1 = bvect_min2.compare(bvect_min1);
    res2 = bvect_full2.compare(bvect_full1);

    if (res1 != res2 && res1 != -1)
    {
        printf("Comparison test failed 3.1\n");
        exit(1);
    }

    printf("Comparison 4.\n");

    bvect_full2.optimize();

    res1 = bvect_min1.compare(bvect_min2);
    res2 = bvect_full1.compare(bvect_full2);

    if (res1 != res2 && res1 != 1)
    {
        printf("Comparison test failed 4\n");
        exit(1);
    }

    res1 = bvect_min2.compare(bvect_min1);
    res2 = bvect_full2.compare(bvect_full1);

    if (res1 != res2 && res1 != -1)
    {
        printf("Comparison test failed 4.1\n");
        exit(1);
    }

    printf("Comparison 5.\n");

    unsigned i;
    for (i = 0; i < 65536; ++i)
    {
        bvect_full1.set_bit(i);
    }

    res1 = bvect_min1.compare(bvect_min2);
    res2 = bvect_full1.compare(bvect_full2);

    if (res1 != res2 && res1 != 1)
    {
        printf("Comparison test failed 5\n");
        exit(1);
    }

    bvect_full1.optimize();

    res1 = bvect_min2.compare(bvect_min1);
    res2 = bvect_full2.compare(bvect_full1);

    if (res1 != res2 && res1 != -1)
    {
        printf("Comparison test failed 5.1\n");
        exit(1);
    }

}

void DesrializationTest2()
{
   bvect  bvtotal;
   unsigned size = BITVECT_SIZE - 10;


   bvect  bv1;
   bvect  bv2;
   int i;
   for (i = 10; i < 165536; i+=2)
   {
      bv1.set_bit(i);
   }

   bv1.optimize();
   print_stat(bv1);

   struct bvect::statistics st1;
   bv1.calc_stat(&st1);


   unsigned char* sermem = new unsigned char[st1.max_serialize_mem];

   unsigned slen2 = bm::serialize(bv1, sermem);
   assert(slen2);
   slen2 = 0;

   bm::deserialize(bvtotal, sermem);
    bvect  bv_target_s;
    operation_deserializer<bvect>::deserialize(bv_target_s,
                                                sermem,
                                                0,
                                                set_OR);

   bvtotal.optimize();
   int res = bvtotal.compare(bv_target_s);
   if (res != 0)
   {
       cout << "Operation deserialization error 1" << endl;
       exit(1);
   }

   for (i = 55000; i < 165536; ++i)
   {
      bv2.set_bit(i);
   }
   bv2.optimize();
   print_stat(bv2);

   struct bvect::statistics st2;
   bv2.calc_stat(&st2);

   unsigned char* sermem2 = new unsigned char[st2.max_serialize_mem];

   unsigned slen = bm::serialize(bv2, sermem2);
   assert(slen);
   slen = 0;

   bm::deserialize(bvtotal, sermem2);
   print_stat(bvtotal);
    operation_deserializer<bvect>::deserialize(bv_target_s,
                                               sermem2,
                                               0,
                                               set_OR);
    res = bvtotal.compare(bv_target_s);
    if (res != 0)
    {
        cout << "Operation deserialization error 2" << endl;
        exit(1);
    }

//   bvtotal.optimize();
 //  bvtotal.stat();

   bm::deserialize(bvtotal, sermem2);

   bm::deserialize(bvtotal, sermem);

    operation_deserializer<bvect>::deserialize(bv_target_s,
                                               sermem2,
                                               0,
                                               set_OR);
    operation_deserializer<bvect>::deserialize(bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);

    res = bvtotal.compare(bv_target_s);
    if (res != 0)
    {
        cout << "Deserialization test failed! 3" << endl;
        exit(1);
    }

   delete [] sermem;
   delete [] sermem2;


   bvtotal.clear();
   bv_target_s.clear(false);

   int clcnt = 0;

   int repetitions = 25;
   for (i = 0; i < repetitions; ++i)
   {
        cout << endl << endl << "Deserialization STEP " << i << endl;

        bvect_mini*   bvect_min1= new bvect_mini(size);
        bvect*        bvect_full1= new bvect();

        FillSetsRandomMethod(bvect_min1, bvect_full1, 1, size, 1);

       struct bvect::statistics st;
       bvect_full1->calc_stat(&st);

       unsigned char* sermem = new unsigned char[st.max_serialize_mem];

       unsigned slen = bm::serialize(*bvect_full1, sermem);

       unsigned char* smem = new unsigned char[slen];
       ::memcpy(smem, sermem, slen);

//       cout << "Serialized vector" << endl;
//       bvect_full1->stat();

//       cout << "Before deserialization" << endl;
//       bvtotal.stat();

        bm::deserialize(bvtotal, smem);
        operation_deserializer<bvect>::deserialize(bv_target_s,
                                                   smem,
                                                   0,
                                                   set_OR);
        res = bvtotal.compare(bv_target_s);
        if (res != 0)
        {
			unsigned bit_idx = bv_target_s.get_first();
			cout << bit_idx << " " << bv_target_s.get_next(bit_idx) << endl;;
			print_stat(bv_target_s);
            cout << "Operation deserialization error 2" << endl;
            exit(1);
        }

//       cout << "After deserialization" << endl;
//       bvtotal.stat();

       bvtotal.optimize();
       bv_target_s.optimize();

//       cout << "After optimization" << endl;
//       bvtotal.stat();


       if (++clcnt == 5)
       {
          clcnt = 0;
          bvtotal.clear();
          bv_target_s.clear();

//          cout << "Post clear." << endl;
//          bvtotal.stat();

       }

       delete [] sermem;
       delete [] smem;
       delete bvect_min1;
       delete bvect_full1;

   } // for i

}


void StressTest(int repetitions, int set_operation = -1)
{

   unsigned RatioSum = 0;
   unsigned SRatioSum = 0;
   unsigned DeltaSum = 0;
   unsigned SDeltaSum = 0;

   unsigned clear_count = 0;

   bvect  bvtotal;
   bvtotal.set_new_blocks_strat(bm::BM_GAP);

   bm::random_subset<bvect> rsub;


   cout << "----------------------------StressTest" << endl;

   unsigned size = BITVECT_SIZE - 10;
//size = BITVECT_SIZE / 10;
   int i;
   for (i = 0; i < repetitions; ++i)
   {
        cout << endl << " - - - - - - - - - - - - STRESS STEP " << i; 
        switch (set_operation)
        {
        case 0: cout << " [OR]"; break;
        case 1: cout << " [SUB]";break;
        case 2: cout << " [XOR]";break;
        case 3: cout << " [AND]";break;
        default:
            cout << " [RANDOM]";
        }
        cout << endl;

        switch (rand() % 3)
        {
        case 0:
            size = BITVECT_SIZE / 10;
            break;
        case 1:
            size = BITVECT_SIZE / 2;
            break;
        default:
            size = BITVECT_SIZE - 10;
            break;
        } // switch


        bvect_mini*   bvect_min1= new bvect_mini(size);
        bvect_mini*   bvect_min2= new bvect_mini(size);
        bvect*        bvect_full1= new bvect();
        bvect*        bvect_full2= new bvect();

        bvect_full1->set_new_blocks_strat(i&1 ? bm::BM_GAP : bm::BM_BIT);
        bvect_full2->set_new_blocks_strat(i&1 ? bm::BM_GAP : bm::BM_BIT);

        int opt = rand() % 2;

        unsigned start1 = 0;

        switch (rand() % 3)
        {
        case 1:
            start1 += size / 5;
            break;
        default:
            break;
        }

        unsigned start2 = 0;

        switch (rand() % 3)
        {
        case 1:
            start2 += size / 5;
            break;
        default:
            break;
        }
/*
        if (i == 3)
        {
            g_cnt_check = 1;
        }
*/
        FillSetsRandomMethod(bvect_min1, bvect_full1, start1, size, opt);
        FillSetsRandomMethod(bvect_min2, bvect_full2, start2, size, opt);



        unsigned arr[bm::set_total_blocks]={0,};
        bm::id_t cnt = bvect_full1->count();
        unsigned last_block = bvect_full1->count_blocks(arr);
        unsigned sum = bm::sum_arr(&arr[0], &arr[last_block+1]);

        if (sum != cnt)
        {
            cout << "Error in function count_blocks." << endl;
            cout << "Array sum = " << sum << endl;
            cout << "BitCount = " << cnt << endl;
            cnt = bvect_full1->count();
            for (unsigned i = 0; i <= last_block; ++i)
            {
                if (arr[i])
                {
                    cout << "[" << i << ":" << arr[i] << "]";
                }
            }
            cout << endl;
            cout << "================" << endl;
            print_stat(*bvect_full1);


            exit(1);
        }

        CheckCountRange(*bvect_full1, start1, BITVECT_SIZE, arr);
        CheckIntervals(*bvect_full1, BITVECT_SIZE);


        CheckCountRange(*bvect_full2, start2, BITVECT_SIZE);

        CheckCountRange(*bvect_full1, 0, start1, arr);
        CheckCountRange(*bvect_full2, 0, start2);


        TestRandomSubset(*bvect_full1, rsub);
        TestRandomSubset(*bvect_full2, rsub);

/*        
        cout << "!!!!!!!!!!!!!!!" << endl;
        CheckVectors(*bvect_min1, *bvect_full1, size);
        cout << "!!!!!!!!!!!!!!!" << endl;
        CheckVectors(*bvect_min2, *bvect_full2, size);
        cout << "!!!!!!!!!!!!!!!" << endl;
 
        
         bvect_full1->stat();
         cout << " --" << endl;
         bvect_full2->stat();
*/

        int operation = rand()%5;
        if (set_operation != -1)
            operation = set_operation;

        switch(operation)
        {
        case 0:
            cout << "Operation OR" << endl;
            bvect_min1->combine_or(*bvect_min2);
            break;

        case 1:
            cout << "Operation SUB" << endl;
            bvect_min1->combine_sub(*bvect_min2);
            break;

        case 2:
            cout << "Operation XOR" << endl;
            bvect_min1->combine_xor(*bvect_min2);
            break;

        default:
            cout << "Operation AND" << endl;
            bvect_min1->combine_and(*bvect_min2);
            break;
        }

        int cres1 = bvect_min1->compare(*bvect_min2);

        delete bvect_min2;

        switch(operation)
        {
        case 0:
            {
            cout << "Operation OR" << endl;

            bm::id_t predicted_count = bm::count_or(*bvect_full1, *bvect_full2);
            bm::id_t predicted_any = bm::any_or(*bvect_full1, *bvect_full2);
            if (predicted_any == 0 && predicted_count != 0)
            {
                cout << "Predicted any error!" << endl;
                exit(1);
            }
            
            bvect    bv_target_s;
            SerializationOperation2Test(&bv_target_s,
                                        *bvect_full1,
                                        *bvect_full2,
                                        predicted_count,
                                        set_COUNT_OR,
                                        set_OR);

            bvect_full1->bit_or(*bvect_full2);
            
            bm::id_t count = bvect_full1->count();

            if (count != predicted_count)
            {
                cout << "Predicted count error!" << endl;
                cout << "Count = " << count << "Predicted count = " << predicted_count << endl;
                exit(1);
            }
            int res = bvect_full1->compare(bv_target_s);
            if (res != 0)
            {
                cout << "Serialization operation failed!" << endl;
                exit(1);
            }
            
            }
            break;

        case 1:
            {
            cout << "Operation SUB" << endl;
            
            bm::id_t predicted_count = bm::count_sub(*bvect_full1, *bvect_full2);
            bm::id_t predicted_any = bm::any_sub(*bvect_full1, *bvect_full2);
            if (predicted_any == 0 && predicted_count != 0)
            {
                cout << "Predicted any error!" << endl;
                exit(1);
            }
            
            bvect    bv_target_s;
            SerializationOperation2Test(&bv_target_s,
                                        *bvect_full1,
                                        *bvect_full2,
                                        predicted_count,
                                        set_COUNT_SUB_AB,
                                        set_SUB);

            bvect_full1->bit_sub(*bvect_full2);
            
            bm::id_t count = bvect_full1->count();

            if (count != predicted_count)
            {
                cout << "Predicted count error!" << endl;
                cout << "Count = " << count << "Predicted count = " << predicted_count << endl;
                exit(1);
            }
            int res = bvect_full1->compare(bv_target_s);
            if (res != 0)
            {
                cout << "Serialization operation failed!" << endl;
                exit(1);
            }
            
            
            }
            break;

        case 2:
            {
            cout << "Operation XOR <<<" << endl;
           
            bm::id_t predicted_count = bm::count_xor(*bvect_full1, *bvect_full2);
            bm::id_t predicted_any = bm::any_xor(*bvect_full1, *bvect_full2);
            if (predicted_any == 0 && predicted_count != 0)
            {
                cout << "Predicted any error!" << endl;
                exit(1);
            }

            bvect    bv_target_s;
            SerializationOperation2Test(&bv_target_s,
                                        *bvect_full1,
                                        *bvect_full2,
                                        predicted_count,
                                        set_COUNT_XOR,
                                        set_XOR);
            
            bvect_full1->bit_xor(*bvect_full2);
            
            bm::id_t count = bvect_full1->count();

            if (count != predicted_count)
            {
                cout << "Predicted count error!" << endl;
                cout << "Count = " << count << "Predicted count = " << predicted_count << endl;
                exit(1);
            }
            int res = bvect_full1->compare(bv_target_s);
            if (res != 0)
            {
                cout << "Serialization operation failed!" << endl;
                exit(1);
            }
            
            }
            
            break;

        default:
            {
            cout << "Operation AND" << endl;

            bm::id_t predicted_count = bm::count_and(*bvect_full1, *bvect_full2);
            bm::id_t predicted_any = bm::any_and(*bvect_full1, *bvect_full2);
            if (predicted_any == 0 && predicted_count != 0)
            {
                cout << "Predicted any error!" << endl;
                exit(1);
            }

            bvect    bv_target_s;
            SerializationOperation2Test(&bv_target_s,
                                        *bvect_full1,
                                        *bvect_full2,
                                        predicted_count,
                                        set_COUNT_AND,
                                        set_AND);

            TestRandomSubset(bv_target_s, rsub);


            bvect bv1(*bvect_full1);


            bvect_full1->bit_and(*bvect_full2);
            bm::id_t count = bvect_full1->count();

            int res = bvect_full1->compare(bv_target_s);
            if (res != 0)
            {
                //SaveBVector("bv1.bv", bv1);
                //SaveBVector("bv2.bv", *bvect_full2);
                cout << "Serialization operation failed!" << endl;
                exit(1);
            }

            if (count != predicted_count)
            {
                cout << "Predicted count error!" << endl;
                cout << "Count = " << count << "Predicted count = " << predicted_count << endl;
                exit(1);
            }

            }
            break;
        }



        cout << "Operation comparison" << endl;
        CheckVectors(*bvect_min1, *bvect_full1, size);

        int cres2 = bvect_full1->compare(*bvect_full2);

        CheckIntervals(*bvect_full1, BITVECT_SIZE);

        if (cres1 != cres2)
        {
            cout << cres1 << " " << cres2 << endl;
            cout << bvect_full1->get_first() << " " << bvect_full1->count() << endl;
            cout << bvect_full2->get_first() << " " << bvect_full2->count() << endl;

           // bvect_full1->stat(1000);
            cout << endl;
           // bvect_full2->stat(1000);
            printf("Bitset comparison operation failed.\n");
            exit(1);
        }

        {
            bvect bv1(*bvect_full1);
            unsigned idx = rand() % size;
            bool b = bv1[idx];
            bool changed;
            if (b) 
            {
                changed = bv1.set_bit_conditional(idx, true, false);
                if (changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                b = bv1[idx];
                if (!b)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }

                changed = bv1.set_bit_conditional(idx, false, false);
                if (changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                changed = bv1.set_bit_conditional(idx, true, true);
                if (changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                changed = bv1.set_bit_conditional(idx, false, true);
                if (!changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                b = bv1[idx];
                if (b)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
            } 
            else 
            {
                changed = bv1.set_bit_conditional(idx, false, true);
                if (changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                changed = bv1.set_bit_conditional(idx, true, false);
                if (!changed)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
                b = bv1[idx];
                if (!b)
                {
                    cout << "Set bit conditional failed!" << endl;
                    exit(1);
                }
            }


        }

        delete bvect_full2;


        struct bvect::statistics st1;
        bvect_full1->calc_stat(&st1);
        bvect_full1->optimize();
        bvect_full1->optimize_gap_size();
        struct bvect::statistics st2;
        bvect_full1->calc_stat(&st2);

        unsigned Ratio = (st2.memory_used * 100)/st1.memory_used;
        RatioSum+=Ratio;
        DeltaSum+=st1.memory_used - st2.memory_used;

        cout << "Optimization statistics: " << endl  
             << "   MemUsedBefore=" << st1.memory_used
             << "   MemUsed=" << st2.memory_used 
             << "   Ratio=" << Ratio << "%"
             << "   Delta=" << st1.memory_used - st2.memory_used
             << endl;
                
        cout << "Optimization comparison" << endl;

        CheckVectors(*bvect_min1, *bvect_full1, size);

        bvect_full1->set_gap_levels(gap_len_table_min<true>::_len);
        CheckVectors(*bvect_min1, *bvect_full1, size);
        CheckIntervals(*bvect_full1, BITVECT_SIZE);

        //CheckCountRange(*bvect_full1, 0, size);


        // Serialization
        bvect_full1->calc_stat(&st2);

        cout << "Memory allocation: " << st2.max_serialize_mem << endl;
        unsigned char* sermem = new unsigned char[st2.max_serialize_mem];

//    bvect_full1->stat();

        cout << "Serialization...";
        unsigned slen = 
            bm::serialize(*bvect_full1, sermem, 
                          BM_NO_GAP_LENGTH|BM_NO_BYTE_ORDER);
        cout << "Ok" << endl;

        delete bvect_full1;

        unsigned SRatio = (slen*100)/st2.memory_used;
        SRatioSum+=SRatio;
        SDeltaSum+=st2.memory_used - slen;


        cout << "Serialized mem_max = " << st2.max_serialize_mem 
             << " size= " << slen 
             << " Ratio=" << SRatio << "%"
             << " Delta=" << st2.memory_used - slen
             << endl;

        bvect*        bvect_full3= new bvect();
        unsigned char* new_sermem = new unsigned char[slen];
        memcpy(new_sermem, sermem, slen);
        delete [] sermem;

        cout << "Deserialization...";

        bm::deserialize(*bvect_full3, new_sermem);

        bm::deserialize(bvtotal, new_sermem);

        bvect* bv_target_s=new bvect();
        operation_deserializer<bvect>::deserialize(*bv_target_s,
                                            new_sermem,
                                            0,
                                            set_OR);

        cout << "Ok." << endl;
        delete [] new_sermem;

        cout << "Optimization...";
        bvtotal.optimize();
        cout << "Ok." << endl;

        ++clear_count;

        if (clear_count == 4)
        {
           bvtotal.clear();
           clear_count = 0;
        }

        cout << "Serialization comparison" << endl;

        CheckVectors(*bvect_min1, *bvect_full3, size, true);
        int res = bv_target_s->compare(*bvect_full3);
        if (res != 0)
        {
            CheckVectors(*bvect_min1, *bv_target_s, size, true);
        }

        delete bv_target_s;
        delete bvect_min1;
        delete bvect_full3;

    }

    --i;
    cout << "Repetitions:" << i <<
            " AVG optimization ratio:" << RatioSum/i 
         << " AVG Delta:" << DeltaSum/i
         << endl
         << " AVG serialization Ratio:"<< SRatioSum/i
         << " Delta:" << SDeltaSum/i
         << endl;
}


void CheckGap2DGap(gap_vector& gapv)
{
   bm::gap_word_t   dgap_buf[bm::gap_max_buff_len+3]; 
   bm::gap_word_t    gap_buf[bm::gap_max_buff_len+3] = {0, }; 
   
   bm::gap_2_dgap(gapv.get_buf(), dgap_buf);
   bm::dgap_2_gap(dgap_buf, gap_buf);
   
   int c = bm::gapcmp(gap_buf, gapv.get_buf());
   if (c != 0)
   {
        cout << "Gap1: ";
        PrintGap(gapv.get_buf());
        cout << "D-Gap: ";
        PrintGap(dgap_buf);
        cout << "Gap2:";
        PrintGap(gap_buf);
        
        cout << "DGap conversion failed!" << endl;
        exit(1);
   }
} 


void GAPCheck()
{
   cout << "-------------------------------------------GAPCheck" << endl;

    {

    gap_vector   gapv(0);
    bvect_mini  bvect_min(bm::gap_max_bits);

    unsigned i;
    for( i = 0; i < 454; ++i)
    {
        bvect_min.set_bit(i);
        gapv.set_bit(i);
    }

    for(i = 0; i < 254; ++i)
    {
        bvect_min.clear_bit(i);
        gapv.clear_bit(i);
    }

    for(i = 5; i < 10; ++i)
    {
        bvect_min.set_bit(i);
        gapv.set_bit(i);
    }

    for( i = 0; i < bm::gap_max_bits; ++i)
    {
        int bit1 = (gapv.is_bit_true(i) == 1);
        int bit2 = (bvect_min.is_bit_true(i) != 0);
        int bit3 = (gapv.test(i) == 1);
        if (bit1 != bit2)
        {
            cout << "problem with bit comparison " << i << endl;
            exit(1);
        }
        if (bit1 != bit3)
        {
            cout << "problem with bit test comparison " << i << endl;
            exit(1);
        }

    }

    }


   {
   gap_vector gapv(1);
   int bit = gapv.is_bit_true(65535);

   if (bit != 1)
   {
      cout << "Bit != 1" << endl;
      exit(1);
   }
   
   int i;
   for (i = 0; i < 65536; ++i)
   {
        bit = gapv.is_bit_true(i);
        if (bit != 1)
        {
            cout << "2.Bit != 1" << endl;
            exit(1);
        }
   }
   unsigned cnt = gapv.count_range(0, 65535);
   if (cnt != 65536)
   {
       cout << "count_range failed:" << cnt << endl;
       exit(1);
   }
   
   CheckCountRange(gapv, 10, 20);
   CheckCountRange(gapv, 0, 20);

   CheckGap2DGap(gapv);

   printf("gapv 1 check ok\n");
   }

   {
   gap_vector gapv(0);


   int bit = gapv.is_bit_true(65535);
   int bit1 = gapv.test(65535);
   if(bit != 0)
   {
      cout << "Bit != 0" << endl;
      exit(1);
   }
      
   int i;
   for (i = 0; i < 65536; ++i)
   {
        bit = gapv.is_bit_true(i);
        bit1 = gapv.test(i);
        if (bit != 0)
        {
            cout << "2.Bit != 0 bit =" << i << endl;
            exit(1);
        }
        if (bit1 != 0)
        {
            cout << "2.Bit test != 0 bit =" << i << endl;
            exit(1);
        }
   }
   unsigned cnt = gapv.count_range(0, 65535);
   if (cnt != 0)
   {
       cout << "count_range failed:" << cnt << endl;
       exit(1);
   }
   CheckCountRange(gapv, 10, 20);
   CheckCountRange(gapv, 0, 20);

   CheckGap2DGap(gapv);



   printf("gapv 2 check ok\n");
   }

   {
   gap_vector gapv(0);

   gapv.set_bit(1);
   gapv.set_bit(0);

   gapv.control();
   CheckCountRange(gapv, 0, 20);

   int bit = gapv.is_bit_true(0);

   if (bit != 1)
   {
      cout << "Trouble" << endl;
      exit(1);
   }
   
   bit = gapv.is_bit_true(1);
   if (bit != 1)
   {
      cout << "Trouble 2." << endl;
      exit(1);
   }


   bit = gapv.is_bit_true(2);
   if(bit != 0)
   {
      cout << "Trouble 3." << endl;
      exit(1);
   }

   CheckGap2DGap(gapv);

   }

   {
   gap_vector gapv(0);

   gapv.set_bit(0);
   gapv.control();
   gapv.set_bit(1);
   gapv.control();

   gapv.set_bit(4);
   gapv.control();
   gapv.set_bit(5);
   gapv.control();
   CheckCountRange(gapv, 4, 5);
   CheckCountRange(gapv, 3, 5);

   gapv.set_bit(3);
   CheckCountRange(gapv, 3, 3);
   CheckCountRange(gapv, 3, 5);

   gapv.control();
   
   int bit = gapv.is_bit_true(0);
   if(bit!=1)
   {
      cout << "Bug" << endl;
   }
   bit = gapv.is_bit_true(1);
   if(bit!=1)
   {
      cout << "Bug2" << endl;
   }

   gapv.control();
   gapv.set_bit(4);
   gapv.control();

   CheckGap2DGap(gapv);


   printf("gapv 3 check ok\n");
   }

   {
        gap_vector gapv(0);
        bvect_mini   bvect_min(bm::gap_max_bits);
        
        cout << "++++++1" << endl;
        print_gap(gapv, 10);
        
        gapv.set_bit(bm::gap_max_bits-1);
        gapv.control();
        print_gap(gapv, 10);


        bvect_min.set_bit(bm::gap_max_bits-1);
        
        cout << "++++++3" << endl;
        
        gapv.set_bit(5);
        print_gap(gapv,15);
        gapv.control();
        bvect_min.set_bit(5);
        
        cout << "++++++4" << endl;

        CheckCountRange(gapv, 13, 150);
        gapv.control();
        
        CheckGap2DGap(gapv);


        unsigned i;
        for (i = 0; i < bm::gap_max_bits; ++i)
        {
            if (i == 65535)
                printf("%i\n", i);
            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            int bit3 = (gapv.test(i) == 1);
            if (bit1 != bit2)
            {
                cout << "problem with bit comparison " << i << endl;
            }
            if (bit1 != bit3)
            {
                cout << "problem with bit test comparison " << i << endl;
            }

        }

        gapv.clear_bit(5);
        bvect_min.clear_bit(5);
        gapv.control();
        
        CheckGap2DGap(gapv);


        for ( i = 0; i < bm::gap_max_bits; ++i)
        {
            if (i == 65535)
                printf("%i\n", i);
            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            int bit3 = (gapv.test(i) == 1);
            if (bit1 != bit2)
            {
                cout << "2.problem with bit comparison " << i << endl;
            }
            if (bit1 != bit3)
            {
                cout << "2.problem with bit test comparison " << i << endl;
            }
        }
   printf("gapv check 4 ok.\n");
   }

   {
        gap_vector gapv(0);
        bvect_mini   bvect_min(65536);
        
        int i;
        for (i = 10; i > 0; i-=2)
        {
            bvect_min.set_bit(i);
            gapv.set_bit(i);
            gapv.control();
            CheckCountRange(gapv, 0, i);
            

            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            int bit3 = (gapv.test(i) != 0);
            if (bit1 != bit2)
            {
                cout << "3.problem with bit comparison " << i << endl;
            }
            if (bit1 != bit3)
            {
                cout << "3.problem with bit test comparison " << i << endl;
            }
            
            CheckGap2DGap(gapv);
            

        }
        for (i = 0; i < (int)bm::gap_max_bits; ++i)
        {
            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            int bit3 = (gapv.test(i) == 1);

            if (bit1 != bit2)
            {
                cout << "3.problem with bit comparison " << i << endl;
            }
            if (bit1 != bit3)
            {
                cout << "3.problem with bit test comparison " << i << endl;
            }
        }
   printf("gapv check 5 ok.\n");
   }

   {
        gap_vector gapv(0);
        bvect_mini   bvect_min(bm::gap_max_bits);
        
        int i;
        for (i = 0; i < 25; ++i)
        {
            unsigned id = random_minmax(0, bm::gap_max_bits);
            bvect_min.set_bit(id);
            gapv.set_bit(id);
            gapv.control();
            CheckCountRange(gapv, 0, id);
            CheckCountRange(gapv, id, 65535);
            
            CheckGap2DGap(gapv);

        }

        for (i = 0; i < (int)bm::gap_max_bits; ++i)
        {
            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            if (bit1 != bit2)
            {
                cout << "4.problem with bit comparison " << i << endl;
            }
        }

        for (i = bm::gap_max_bits; i < 0; --i)
        {
            int bit1 = (gapv.is_bit_true(i) == 1);
            int bit2 = (bvect_min.is_bit_true(i) != 0);
            if (bit1 != bit2)
            {
                cout << "5.problem with bit comparison " << i << endl;
            }
        }
   printf("gapv check 6 ok.\n");

   }

   printf("gapv random bit set check ok.\n");


   // conversion functions test
   
   {
   // aligned position test
   bvect        bvect;

   bvect.set_bit(1);
   bvect.clear();


   unsigned* buf = (unsigned*) bvect.get_block(0);

   bm::or_bit_block(buf, 0, 4);
   unsigned cnt = bm::bit_block_calc_count_range(buf, 0, 3);
   assert(cnt == 4);
   
   bool bit = bvect.get_bit(0);
   assert(bit);
   bit = bvect.get_bit(1);
   assert(bit);
   bit = bvect.get_bit(2);
   assert(bit);
   bit = bvect.get_bit(3);
   assert(bit);
   bit = bvect.get_bit(4);
   assert(bit==0);

   bm::or_bit_block(buf, 0, 36); 
   cnt = bm::bit_block_calc_count_range(buf, 0, 35);
   assert(cnt == 36);

   for (int i = 0; i < 36; ++i)
   {
        bit = (bvect.get_bit(i) != 0);
        assert(bit);
   }
   bit = (bvect.get_bit(36) != 0);
   assert(bit==0);

   unsigned count = bvect.recalc_count();
   assert(count == 36);   
   
   cout << "Aligned position test ok." << endl; 

   }


   {
   // unaligned position test
   bvect   bvect;

   bvect.set_bit(0);
   bvect.clear();

   unsigned* buf = (unsigned*) bvect.get_block(0);

   bm::or_bit_block(buf, 5, 32);
   bool bit = (bvect.get_bit(4) != 0);
   assert(bit==0);
   unsigned cnt = bm::bit_block_calc_count_range(buf, 5, 5+32-1);
   assert(cnt == 32);
   cnt = bm::bit_block_calc_count_range(buf, 5, 5+32);
   assert(cnt == 32);

   int i;
   for (i = 5; i < 4 + 32; ++i)
   {
        bit = bvect.get_bit(i);
        assert(bit);
   }
   int count = bvect.recalc_count();
   assert(count==32);

   cout << "Unaligned position ok." << endl;

   } 

   // random test
   {
   cout << "random test" << endl;

   bvect   bvect;

   bvect.set_bit(0);
   bvect.clear();

   unsigned* buf = (unsigned*) bvect.get_block(0);
   for (int i = 0; i < 5000; ++i)
   {
        unsigned start = rand() % 65535;
        unsigned end = rand() % 65535;
        if (start > end)
        {
            unsigned tmp = end;
            end = start;
            start = tmp;
        }
        unsigned len = end - start;
        if (len)
        {
           bm::or_bit_block(buf, start, len);
           unsigned cnt = bm::bit_block_calc_count_range(buf, start, end);
           if (cnt != len)
           {
            cout << "random test: count_range comparison failed. " 
                 << " LEN = " << len << " cnt = " << cnt
                 << endl;
                 exit(1);
           }

           unsigned count = bvect.recalc_count();

           if (count != len)
           {
            cout << "random test: count comparison failed. " 
                 << " LEN = " << len << " count = " << count
                 << endl;
                 exit(1);
           }            

           for (unsigned j = start; j < end; ++j)
           {
                bool bit = bvect.get_bit(j);
                if (!bit)
                {
                    cout << "random test: bit comparison failed. bit#" 
                         << i << endl;
                    exit(1);
                } 
           } // for j

        } 
        bvect.clear();

        if ((i % 100)==0)
        {
            cout << "*" << flush;
        }
   } // for i

   cout << endl << "Random test Ok." << endl;

   }


   // conversion test
 
   cout << "Conversion test" << endl;
    
   {
   
   gap_vector gapv(0);
   bvect   bvect;

   gapv.set_bit(0);
   gapv.set_bit(2);
   gapv.set_bit(10);
   gapv.set_bit(11);
   gapv.set_bit(12);
   
   CheckCountRange(gapv, 3, 15);

   print_gap(gapv, 100);
   bvect.set_bit(0);
   bvect.clear();

   unsigned* buf = (unsigned*) bvect.get_block(0);


   gapv.convert_to_bitset(buf);


   unsigned bitcount = bvect.recalc_count();


   if (bitcount != 5)
   {
      cout << "test failed: bitcout = " << bitcount << endl;
      exit(1);
   }


   gap_vector gapv1(0);
   gap_word_t* gap_buf = gapv1.get_buf();
   *gap_buf = 0;
   bit_convert_to_gap(gap_buf, buf, bm::gap_max_bits, bm::gap_max_buff_len);
   print_gap(gapv1, 100);

   bitcount = gapv1.bit_count();
   if(bitcount != 5)
   {
      cout << "2.test_failed: bitcout = " << bitcount << endl;
      exit(1);
   }

   printf("conversion test ok.\n");
    
   }

   // gap AND test

   {
   // special case 1: operand is all 1
   gap_vector gapv1(0);
   gapv1.set_bit(2);
   gap_vector gapv2(1); 

   gapv1.combine_and(gapv2.get_buf());
   gapv1.control();
   print_gap(gapv1, 0);

   int count = gapv1.bit_count();
   assert(count == 1);
   int bit = gapv1.is_bit_true(2);
   if(bit == 0)
   {
      cout << "Wrong bit" << endl;
      exit(1);
   }
   CheckCountRange(gapv1, 0, 17);

   }

   {
   // special case 2: src is all 1
   gap_vector gapv1(1);
   gap_vector gapv2(0); 
   gapv2.set_bit(2);

   gapv1.combine_and(gapv2.get_buf());
   gapv1.control();
   print_gap(gapv1, 0);

   int count = gapv1.bit_count();
   assert(count == 1);
   int bit = gapv1.is_bit_true(2);
   assert(bit);

   }

   {
   gap_vector gapv;
   gap_vector gapv1(0);

   gapv1.set_bit(3);
   gapv1.set_bit(4);
   print_gap(gapv1, 0);

   gap_vector gapv2(0); 
   gapv2.set_bit(2);
   gapv2.set_bit(3);
   print_gap(gapv2, 0);

   unsigned dsize=0;
   bm::gap_buff_op((gap_word_t*)gapv.get_buf(), 
                         gapv1.get_buf(), 0,
                         gapv2.get_buf(), 0, bm::and_op, 
                         dsize); 
   print_gap(gapv, 0);
   gapv.control();


    int bit1 = (gapv.is_bit_true(3) == 1);
    if(bit1 == 0)
    {
       cout << "Checking failed." << endl;
       exit(0);
    }

   gapv1.combine_or(gapv2);
   print_gap(gapv1, 0);
   gapv1.control();

   }

   {
        printf("gap AND test 1.\n");
        gap_vector gapv1(0);
        gap_vector gapv2(0);
        bvect_mini   bvect_min1(65536);
        bvect_mini   bvect_min2(65536);

        gapv1.set_bit(65535);
        bvect_min1.set_bit(65535);
        gapv1.set_bit(4);
        bvect_min1.set_bit(4);

        gapv2.set_bit(65535);
        bvect_min2.set_bit(65535);
        gapv2.set_bit(3);
        bvect_min2.set_bit(3);
        CheckCountRange(gapv2, 3, 65535);

        gapv2.control();

        printf("vect1:"); print_gap(gapv1, 0);
        printf("vect2:");print_gap(gapv2, 0);

        gapv1.combine_and(gapv2.get_buf());
        printf("vect1:");print_gap(gapv1, 0);

        gapv1.control();
        unsigned bit1 = gapv1.is_bit_true(65535);
        assert(bit1);

        bvect_min1.combine_and(bvect_min2);
        CheckGAPMin(gapv1, bvect_min1, bm::gap_max_bits);
   }

   {
        printf("gap random AND test.\n");
        gap_vector gapv1(0);
        gap_vector gapv2(0);
        bvect_mini   bvect_min1(65536);
        bvect_mini   bvect_min2(65536);
        
        int i;
        for (i = 0; i < 25; ++i)
        {
            unsigned id = random_minmax(0, 65535);
            bvect_min1.set_bit(id);
            gapv1.set_bit(id);
            gapv1.control();
            CheckCountRange(gapv1, 0, id);
            CheckCountRange(gapv1, id, 65535);
        }
        for (i = 0; i < 25; ++i)
        {
            unsigned id = random_minmax(0, 65535);
            bvect_min2.set_bit(id);
            gapv2.set_bit(id);
            gapv2.control();
        }

        gapv1.combine_and(gapv2.get_buf());
        gapv1.control();
        gapv2.control();
        bvect_min1.combine_and(bvect_min2);

        CheckGAPMin(gapv1, bvect_min1, bm::gap_max_bits);

        printf("gap random AND test ok.\n");

   }

   {
        printf("gap OR test.\n");

        gap_vector gapv1(0);
        gap_vector gapv2(0);

        gapv1.set_bit(2);
        gapv2.set_bit(3);

        gapv1.combine_or(gapv2);
        gapv1.control();
        print_gap(gapv1, 0);   
        int bit1 = (gapv1.is_bit_true(0) == 1);
        assert(bit1==0);
        bit1=(gapv1.is_bit_true(2) == 1);
        assert(bit1);
        bit1=(gapv1.is_bit_true(3) == 1);
        assert(bit1);
   }

   {
        printf("gap XOR test.\n");

        gap_vector gapv1(0);
        gap_vector gapv2(0);

        gapv1.set_bit(2);
        gapv2.set_bit(3);
        gapv1.set_bit(4);
        gapv2.set_bit(4);
        print_gap(gapv1, 0);   
        print_gap(gapv2, 0);   

        gapv1.combine_xor(gapv2);
        gapv1.control();
        print_gap(gapv1, 0);   
        int bit1 = (gapv1.is_bit_true(0) == 0);
        assert(bit1);
        bit1=(gapv1.is_bit_true(2) == 1);
        assert(bit1);
        bit1=(gapv1.is_bit_true(3) == 1);
        assert(bit1);
        bit1=(gapv1.is_bit_true(4) == 0);
        assert(bit1);

   }


   {
        int i;
        printf("gap random OR test.\n");
        gap_vector gapv1(0);
        gap_vector gapv2(0);
        bvect_mini   bvect_min1(bm::gap_max_bits);
        bvect_mini   bvect_min2(bm::gap_max_bits);
        
        for (i = 0; i < 10; ++i)
        {
            unsigned id = random_minmax(0, 100);
            bvect_min1.set_bit(id);
            gapv1.set_bit(id);
            gapv1.control();
        }
        for (i = 0; i < 10; ++i)
        {
            unsigned id = random_minmax(0, 100);
            bvect_min2.set_bit(id);
            gapv2.set_bit(id);
            gapv2.control();
        }

        print_mv(bvect_min1, 64);
        print_mv(bvect_min2, 64);

        gapv1.combine_or(gapv2);
        gapv1.control();
        gapv2.control();
        bvect_min1.combine_or(bvect_min2);

        print_mv(bvect_min1, 64);

        CheckGAPMin(gapv1, bvect_min1, bm::gap_max_bits);

        printf("gap random OR test ok.\n");

   }


   {
        int i;
        printf("gap random SUB test.\n");
        gap_vector gapv1(0);
        gap_vector gapv2(0);
        bvect_mini   bvect_min1(bm::gap_max_bits);
        bvect_mini   bvect_min2(bm::gap_max_bits);
        
        for (i = 0; i < 25; ++i)
        {
            unsigned id = random_minmax(0, 100);
            bvect_min1.set_bit(id);
            gapv1.set_bit(id);
            gapv1.control();
        }
        for (i = 0; i < 25; ++i)
        {
            unsigned id = random_minmax(0, 100);
            bvect_min2.set_bit(id);
            gapv2.set_bit(id);
            gapv2.control();
        }

        print_mv(bvect_min1, 64);
        print_mv(bvect_min2, 64);

        gapv1.combine_sub(gapv2);
        gapv1.control();
        gapv2.control();
        bvect_min1.combine_sub(bvect_min2);

        print_mv(bvect_min1, 64);

        CheckGAPMin(gapv1, bvect_min1, bm::gap_max_bits);

        printf("gap random SUB test ok.\n");
   }

   {
       printf("GAP comparison test.\n");

       gap_vector gapv1(0);
       gap_vector gapv2(0);

       gapv1.set_bit(3);
       gapv2.set_bit(3);

       int res = gapv1.compare(gapv2);
       if (res != 0)
       {
           printf("GAP comparison failed!");
           exit(1);
       }

       gapv1.set_bit(4);
       gapv2.set_bit(4);

       res = gapv1.compare(gapv2);
       if (res != 0)
       {
           printf("GAP comparison failed!");
           exit(1);
       }

       gapv1.set_bit(0);
       gapv1.set_bit(1);

       res = gapv1.compare(gapv2);
       if (res != 1)
       {
           printf("GAP comparison failed!");
           exit(1);
       }

       gapv2.set_bit(0);
       gapv2.set_bit(1);
       res = gapv1.compare(gapv2);
       if (res != 0)
       {
           printf("GAP comparison failed!");
           exit(1);
       }

       gapv1.clear_bit(1);

       res = gapv1.compare(gapv2);
       if (res != -1)
       {
           printf("GAP comparison failed!");
           exit(1);
       }


   }


}

// -----------------------------------------------------------------------------

void MutationTest()
{

    cout << "--------------------------------- MutationTest" << endl;

    bvect_mini     bvect_min(BITVECT_SIZE);
    bvect          bvect_full;

    printf("\nMutation test.\n");

    bvect_full.set_new_blocks_strat(bm::BM_GAP);

    bvect_full.set_bit(5);
    bvect_full.set_bit(5);

    bvect_min.set_bit(5);

    bvect_full.set_bit(65535);
    bvect_full.set_bit(65537);
    bvect_min.set_bit(65535);
    bvect_min.set_bit(65537);

    bvect_min.set_bit(100000);
    bvect_full.set_bit(100000);

    // detailed vectors verification
    ::CheckVectors(bvect_min, bvect_full, ITERATIONS, false);

    int i;
    for (i = 5; i < 20000; i+=3)
    {
        bvect_min.set_bit(i);
        bvect_full.set_bit(i);
    }
    ::CheckVectors(bvect_min, bvect_full, ITERATIONS, false);

    for (i = 100000; i < 200000; i+=3)
    {
        bvect_min.set_bit(i);
        bvect_full.set_bit(i);
    }

    ::CheckVectors(bvect_min, bvect_full, 300000);

    // set-clear functionality

    {
        printf("Set-clear functionality test.");

        bvect_mini     bvect_min(BITVECT_SIZE);
        bvect          bvect_full;
        bvect_full.set_new_blocks_strat(bm::BM_GAP);

        int i;
        for (i = 100000; i < 100010; ++i)
        {
            bvect_min.set_bit(i);
            bvect_full.set_bit(i);            
        }
        ::CheckVectors(bvect_min, bvect_full, 300000);

        for (i = 100000; i < 100010; ++i)
        {
            bvect_min.clear_bit(i);
            bvect_full.clear_bit(i);            
        }
        ::CheckVectors(bvect_min, bvect_full, 300000);
        
        bvect_full.optimize();
        CheckVectors(bvect_min, bvect_full, 65536);//max+10);
    }

}

void MutationOperationsTest()
{

   cout << "------------------------------------ MutationOperationsTest" << endl;

   printf("Mutation operations test 1.\n");
   {
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_BIT);

    bvect_full1.set_bit(100);
    bvect_min1.set_bit(100);

    int i;
    for(i = 0; i < 10000; i+=2)
    {
       bvect_full2.set_bit(i);
       bvect_min2.set_bit(i);
    }
    print_stat(bvect_full2);
    CheckVectors(bvect_min2, bvect_full2, 65536, true);
    
    bvect_min1.combine_and(bvect_min2);
    bvect_full1.bit_and(bvect_full2);

    CheckVectors(bvect_min1, bvect_full1, 65536);//max+10);

   }

   printf("Mutation operations test 2.\n");
   {
    unsigned delta = 65536;
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

    int i;
    for(i = 0; i < 1000; i+=1)
    {
       bvect_full1.set_bit(delta+i);
       bvect_min1.set_bit(delta+i);
    }

    for(i = 0; i < 100; i+=2)
    {
       bvect_full2.set_bit(delta+i);
       bvect_min2.set_bit(delta+i);
    }
//    CheckVectors(bvect_min2, bvect_full2, 65536);
    
    bvect_min1.combine_and(bvect_min2);
    bvect_full1.bit_and(bvect_full2);

    CheckVectors(bvect_min1, bvect_full1, 65536);//max+10);
    bvect_full1.optimize();
    CheckVectors(bvect_min1, bvect_full1, 65536);//max+10);

   }

   {
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect        bvect_full1;

    bvect_full1.set_bit(3);
    bvect_min1.set_bit(3);

    struct bvect::statistics st;
    bvect_full1.calc_stat(&st);

    // serialization

    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(bvect_full1, sermem);
    cout << "BVECTOR SERMEM=" << slen << endl;


    bvect        bvect_full3;
    bm::deserialize(bvect_full3, sermem);
    print_stat(bvect_full3);
    CheckVectors(bvect_min1, bvect_full3, 100, true);
   }


   printf("Mutation operations test 3.\n");
   {
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect_mini   bvect_min2(BITVECT_SIZE);
    bvect        bvect_full1;
    bvect        bvect_full2;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
    bvect_full2.set_new_blocks_strat(bm::BM_GAP);

   
    unsigned min = BITVECT_SIZE / 2 - ITERATIONS;
    unsigned max = BITVECT_SIZE / 2 + ITERATIONS;
    if (max > BITVECT_SIZE) 
        max = BITVECT_SIZE - 1;

    unsigned len = max - min;

    FillSets(&bvect_min1, &bvect_full1, min, max, 0);
    FillSets(&bvect_min1, &bvect_full1, 0, len, 5);
    printf("Bvect_FULL 1 STAT\n");
    print_stat(bvect_full1);
//    CheckVectors(bvect_min1, bvect_full1, max+10, false);
    FillSets(&bvect_min2, &bvect_full2, min, max, 0);
    FillSets(&bvect_min2, &bvect_full2, 0, len, 0);
    printf("Bvect_FULL 2 STAT\n");
    print_stat(bvect_full2);
//    CheckVectors(bvect_min2, bvect_full2, max+10);
    

    bvect_min1.combine_and(bvect_min2);
    bvect_full1.bit_and(bvect_full2);
    printf("Bvect_FULL 1 STAT after AND\n");
    print_stat(bvect_full1);

    CheckVectors(bvect_min1, bvect_full1, max+10, false);

    struct bvect::statistics st;
    bvect_full1.calc_stat(&st);
    cout << "BVECTOR: GAP=" << st.gap_blocks << " BIT=" << st.bit_blocks 
         << " MEM=" << st.memory_used << " SERMAX=" << st.max_serialize_mem
         << endl;
    cout << "MINIVECT: " << bvect_min1.mem_used() << endl;

    bvect_full1.optimize();
    print_stat(bvect_full1);

    CheckVectors(bvect_min1, bvect_full1, max+10, false);

    bvect_full1.calc_stat(&st);
    cout << "BVECTOR: GAP=" << st.gap_blocks << " BIT=" << st.bit_blocks 
         << " MEM=" << st.memory_used << " SERMAX=" << st.max_serialize_mem
         << endl;
    cout << "MINIVECT: " << bvect_min1.mem_used() << endl;



    // serialization

    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(bvect_full1, sermem);
    cout << "BVECTOR SERMEM=" << slen << endl;


    
    bvect        bvect_full3;
    bm::deserialize(bvect_full3, sermem);
    print_stat(bvect_full3);
    CheckVectors(bvect_min1, bvect_full3, max+10, true);
    
    delete [] sermem;
    

    cout << "Copy constructor check." << endl;


    {
    bvect       bvect_full4(bvect_full3);
    print_stat(bvect_full3);
    CheckVectors(bvect_min1, bvect_full4, max+10, true);
    }
    

   }

}


void SerializationTest()
{

   cout << " ----------------------------------- SerializationTest" << endl;

   cout << "Serialization STEP 0" << endl;

   {
    unsigned size = BITVECT_SIZE/6000;


    bvect_mini*   bvect_min1= new bvect_mini(BITVECT_SIZE);
    bvect*        bvect_full1= new bvect();
    bvect*        bvect_full2= new bvect();
    bvect*        bv_target_s= new bvect();

    bvect_full1->set_new_blocks_strat(bm::BM_BIT);
    bvect_full2->set_new_blocks_strat(bm::BM_BIT);

    for(unsigned i = 0; i < size; ++i)
    {
        bvect_full1->set_bit(i);
        bvect_min1->set_bit(i);
    }

    bvect_full1->optimize();
    CheckVectors(*bvect_min1, *bvect_full1, size, true);



    bvect::statistics st;
    bvect_full1->calc_stat(&st);
    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(*bvect_full1, sermem);
    cout << "Serialized mem_max = " << st.max_serialize_mem 
         << " size= " << slen 
         << " Ratio=" << (slen*100)/st.max_serialize_mem << "%"
         << endl;

    bm::deserialize(*bvect_full2, sermem);
    operation_deserializer<bvect>::deserialize(*bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);
    delete [] sermem;


    CheckVectors(*bvect_min1, *bvect_full2, size, true);
    CheckVectors(*bvect_min1, *bv_target_s, size, true);


    delete bvect_full2;
    delete bvect_min1;
    delete bvect_full1;
    delete bv_target_s;

    }


   {
    unsigned size = BITVECT_SIZE/6000;


    bvect_mini*   bvect_min1= new bvect_mini(BITVECT_SIZE);
    bvect*        bvect_full1= new bvect();
    bvect*        bvect_full2= new bvect();
    bvect*        bv_target_s= new bvect();

    bvect_full1->set_new_blocks_strat(bm::BM_BIT);
    bvect_full2->set_new_blocks_strat(bm::BM_BIT);

        bvect_full1->set_bit(131072);
        bvect_min1->set_bit(131072);
    

    bvect_full1->optimize();

    bvect::statistics st;
    bvect_full1->calc_stat(&st);
    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(*bvect_full1, sermem);
    cout << "Serialized mem_max = " << st.max_serialize_mem 
         << " size= " << slen 
         << " Ratio=" << (slen*100)/st.max_serialize_mem << "%"
         << endl;

    bm::deserialize(*bvect_full2, sermem);
    operation_deserializer<bvect>::deserialize(*bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);

    delete [] sermem;

    CheckVectors(*bvect_min1, *bvect_full2, size, true);
    CheckVectors(*bvect_min1, *bv_target_s, size, true);

    delete bvect_full2;
    delete bvect_min1;
    delete bvect_full1;
    delete bv_target_s;

    }


    cout << "Serialization STEP 1." << endl;

    {
    bvect_mini   bvect_min1(BITVECT_SIZE);
    bvect        bvect_full1;

    bvect_full1.set_new_blocks_strat(bm::BM_GAP);
   
    unsigned min = BITVECT_SIZE / 2 - ITERATIONS;
    unsigned max = BITVECT_SIZE / 2 + ITERATIONS;
    if (max > BITVECT_SIZE) 
        max = BITVECT_SIZE - 1;

    unsigned len = max - min;

    FillSets(&bvect_min1, &bvect_full1, min, max, 0);
    FillSets(&bvect_min1, &bvect_full1, 0, len, 5);

    // shot some random bits

    int i;
    for (i = 0; i < 10000; ++i)
    {
        unsigned bit = rand() % BITVECT_SIZE;
        bvect_full1.set_bit(bit);
        bvect_min1.set_bit(bit);
    }

    bvect::statistics st;
    bvect_full1.calc_stat(&st);

    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    print_stat(bvect_full1);
    
    unsigned slen = bm::serialize(bvect_full1, sermem);

    cout << "Serialized len = " << slen << endl;

    bvect        bvect_full3;
    bm::deserialize(bvect_full3, sermem);
    bvect*  bv_target_s = new bvect();
    operation_deserializer<bvect>::deserialize(*bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);

    CheckVectors(bvect_min1, bvect_full3, max+10, true);
    CheckVectors(bvect_min1, *bv_target_s, max+10, true);

    delete [] sermem;
    delete bv_target_s;

    }


   cout << "Stage 2" << endl;

   {

    bvect_mini*   bvect_min1= new bvect_mini(BITVECT_SIZE);
//    bm::bvect_mini*   bvect_min2= new bm::bvect_mini(BITVECT_SIZE);
    bvect*        bvect_full1= new bvect();
    bvect*        bvect_full2= new bvect();

    bvect_full1->set_new_blocks_strat(bm::BM_GAP);
    bvect_full2->set_new_blocks_strat(bm::BM_GAP);

    FillSetsRandomMethod(bvect_min1, bvect_full1, 1, BITVECT_SIZE-10, 1);
//    FillSetsRandomMethod(bvect_min2, bvect_full2, 1, BITVECT_SIZE-10, 1);

//bvect_full1->stat();
cout << "Filling. OK." << endl;
    bvect::statistics st;
    bvect_full1->calc_stat(&st);
cout << st.max_serialize_mem << endl;
    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
cout << "Serialization" << endl;
    unsigned slen = bm::serialize(*bvect_full1, sermem);

    cout << "Serialized mem_max = " << st.max_serialize_mem 
         << " size= " << slen 
         << " Ratio=" << (slen*100)/st.max_serialize_mem << "%"
         << endl;
cout << "Deserialization" << endl;
    bm::deserialize(*bvect_full2, sermem);
cout << "Deserialization ok" << endl;
    bvect*  bv_target_s=new bvect();
    operation_deserializer<bvect>::deserialize(*bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);

    CheckVectors(*bvect_min1, *bvect_full2, BITVECT_SIZE, true);
    CheckVectors(*bvect_min1, *bv_target_s, BITVECT_SIZE, true);

    delete [] sermem;

    delete bv_target_s;
    delete bvect_full2;
    delete bvect_min1;
    delete bvect_full1;

    }



   cout << "Stage 3" << endl;

   {

    bvect_mini*   bvect_min1= new bvect_mini(BITVECT_SIZE);
    bvect_mini*   bvect_min2= new bvect_mini(BITVECT_SIZE);
    bvect*        bvect_full1= new bvect();
    bvect*        bvect_full2= new bvect();

    bvect_full1->set_new_blocks_strat(bm::BM_GAP);
    bvect_full2->set_new_blocks_strat(bm::BM_GAP);


    FillSetsRandomMethod(bvect_min1, bvect_full1, 1, BITVECT_SIZE, 1);
    FillSetsRandomMethod(bvect_min2, bvect_full2, 1, BITVECT_SIZE, 1);
    CheckVectors(*bvect_min1, *bvect_full1, BITVECT_SIZE, true);
    CheckVectors(*bvect_min2, *bvect_full2, BITVECT_SIZE, true);


    bvect::statistics st;
    bvect_full1->calc_stat(&st);
    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(*bvect_full1, sermem);

    bvect bvt;
    bm::deserialize(bvt, sermem);
    if (bvt != *bvect_full1)
    {
        print_stat(bvt);
        print_stat(*bvect_full1);
        cout << "Error!" << endl;
        exit(1);
    }

    CheckVectors(*bvect_min1, *bvect_full1, BITVECT_SIZE, true);
    CheckVectors(*bvect_min2, *bvect_full2, BITVECT_SIZE, true);

    cout << "Serialized mem_max = " << st.max_serialize_mem 
         << " size= " << slen 
         << " Ratio=" << (slen*100)/st.max_serialize_mem << "%"
         << endl;

    bvect*  bv_target_s=new bvect(*bvect_full2);
    print_stat(*bv_target_s);

    print_stat(*bvect_full2);

    bvect*  bvect_full3= new bvect();
    *bvect_full3 = *bvect_full1;
    *bvect_full3 |= *bvect_full2;
//    CheckVectors(*bvect_min2, *bvect_full3, BITVECT_SIZE, true);


    bm::deserialize(*bvect_full2, sermem);

    operation_deserializer<bvect>::deserialize(*bv_target_s,
                                               sermem,
                                               0,
                                               set_OR);
    delete [] sermem;
    
    CheckVectors(*bvect_min1, *bvect_full1, BITVECT_SIZE, true);
//    CheckVectors(*bvect_min1, *bvect_full3, BITVECT_SIZE, true);

    bvect_min2->combine_or(*bvect_min1);
    delete bvect_min1;
    
    if (*bvect_full2 != *bvect_full3)
    {
        print_stat(*bvect_full2);
        print_stat(*bvect_full3);

        cout << "Error!" << endl;
        exit(1);
    }


    CheckVectors(*bvect_min2, *bvect_full2, BITVECT_SIZE, true);
    CheckVectors(*bvect_min2, *bv_target_s, BITVECT_SIZE, true);

    delete bv_target_s;
    delete bvect_full1;
    delete bvect_full2;
    delete bvect_full3;
    delete bvect_min2;    


    }

   cout << "Stage 4. " << endl;

   {
    unsigned size = BITVECT_SIZE/3;


    bvect_mini*   bvect_min1= new bvect_mini(BITVECT_SIZE);
    bvect*        bvect_full1= new bvect();
    bvect*        bvect_full2= new bvect();

    bvect_full1->set_new_blocks_strat(bm::BM_BIT);
    bvect_full2->set_new_blocks_strat(bm::BM_BIT);

    unsigned i;
    for(i = 0; i < 65000; ++i)
    {
        bvect_full1->set_bit(i);
        bvect_min1->set_bit(i);
    }

    for(i = 65536; i < 65536+65000; ++i)
    {
        bvect_full1->set_bit(i);
        bvect_min1->set_bit(i);
    }

    for (i = 65536*2; i < size/6; ++i)
    {
        bvect_full1->set_bit(i);
        bvect_min1->set_bit(i);
    }


    bvect_full1->optimize();

    print_stat(*bvect_full1);


    bvect::statistics st;
    bvect_full1->calc_stat(&st);
    unsigned char* sermem = new unsigned char[st.max_serialize_mem];
    unsigned slen = bm::serialize(*bvect_full1, sermem);
    cout << "Serialized mem_max = " << st.max_serialize_mem 
         << " size= " << slen 
         << " Ratio=" << (slen*100)/st.max_serialize_mem << "%"
         << endl;
    
    unsigned char* new_sermem = new unsigned char[st.max_serialize_mem];
    ::memcpy(new_sermem, sermem, slen);

    bvect  bv_target_s(*bvect_full2);

    bm::deserialize(*bvect_full2, new_sermem);
    operation_deserializer<bvect>::deserialize(bv_target_s,
                                               new_sermem,
                                               0,
                                               set_OR);

    delete [] sermem;
    delete [] new_sermem;

    CheckVectors(*bvect_min1, *bvect_full2, size, true);
    CheckVectors(*bvect_min1, bv_target_s, size, true);


    delete bvect_full2;
    delete bvect_min1;
    delete bvect_full1;

    }


}

void GetNextTest()
{
   cout << "-------------------------------------------- GetNextTest" << endl;

   int i;
   for(i = 0; i < 2; ++i)
   {
      cout << "Strategy " << i << endl;

   {
      bvect       bvect_full1;
      bvect_mini  bvect_min1(BITVECT_SIZE);

      bvect_full1.set_new_blocks_strat(i ? bm::BM_GAP : bm::BM_BIT);

      bvect_full1.set_bit(0);
      bvect_min1.set_bit(0);


      bvect_full1.set_bit(65536);
      bvect_min1.set_bit(65536);

      unsigned nbit1 = bvect_full1.get_first();
      unsigned nbit2 = bvect_min1.get_first();

      if (nbit1 != nbit2)
      {
         cout << "1. get_first failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }
      nbit1 = bvect_full1.get_next(nbit1);
      nbit2 = bvect_min1.get_next(nbit2);
      if ((nbit1 != nbit2) || (nbit1 != 65536))
      {
         cout << "1. get_next failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }
   }



   {
      bvect       bvect_full1;
      bvect_mini  bvect_min1(BITVECT_SIZE);
      bvect_full1.set_new_blocks_strat(i ? bm::BM_GAP : bm::BM_BIT);

      bvect_full1.set_bit(65535);
      bvect_min1.set_bit(65535);

      unsigned nbit1 = bvect_full1.get_first();
      unsigned nbit2 = bvect_min1.get_first();

      if ((nbit1 != nbit2) || (nbit1 != 65535))
      {
         cout << "1. get_first failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }
      nbit1 = bvect_full1.get_next(nbit1);
      nbit2 = bvect_min1.get_next(nbit2);
      if (nbit1 != nbit2 )
      {
         cout << "1. get_next failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }
   }

   {
      cout << "--------------" << endl;
      bvect       bvect_full1;
      bvect_mini  bvect_min1(BITVECT_SIZE);
      bvect_full1.set_new_blocks_strat(i ? bm::BM_GAP : bm::BM_BIT);

      bvect_full1.set_bit(655350);
      bvect_min1.set_bit(655350);

      unsigned nbit1 = bvect_full1.get_first();
      unsigned nbit2 = bvect_min1.get_first();

      if (nbit1 != nbit2 || nbit1 != 655350)
      {
         cout << "1. get_first failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }

      nbit1 = bvect_full1.get_next(nbit1);
      nbit2 = bvect_min1.get_next(nbit2);
      if (nbit1 != nbit2)
      {
         cout << "1. get_next failed() " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }
   }


   {
   bvect       bvect_full1;
   bvect_mini  bvect_min1(BITVECT_SIZE);

   bvect_full1.set_new_blocks_strat(i ? bm::BM_GAP : bm::BM_BIT);

   bvect_full1.set_bit(256);
   bvect_min1.set_bit(256);

//   bvect_full1.clear_bit(256);
   bvect_full1.set_bit(65536);
   bvect_min1.set_bit(65536);

   unsigned nbit1 = bvect_full1.get_first();
   unsigned nbit2 = bvect_min1.get_first();

   if (nbit1 != nbit2)
   {
      cout << "get_first failed " <<  nbit1 << " " << nbit2 << endl;
      exit(1);
   }

   while (nbit1)
   {
      cout << nbit1 << endl;
      nbit1 = bvect_full1.get_next(nbit1);
      nbit2 = bvect_min1.get_next(nbit2);
      if (nbit1 != nbit2)
      {
         cout << "get_next failed " <<  nbit1 << " " << nbit2 << endl;
         exit(1);
      }

   } // while

   }

   
   }// for

}

// Test contributed by Maxim Shemanarev.

void MaxSTest()
{
   bvect vec;

   int i, j;
   unsigned id;
   for(i = 0; i < 100; i++)
   {
      int n = rand() % 2000 + 1;
      id = 1;
      for(j = 0; j < n; j++)
      {
         id += rand() % 10 + 1;
         vec.set_bit(id);

      }
      vec.optimize();
      vec.clear();
      fprintf(stderr, ".");
   }
}


void CalcBeginMask()
{
    printf("BeginMask:\n");

    int i;
    for (i = 0; i < 32; ++i)
    {
    unsigned mask = 0;

        for(int j = i; j < 32; ++j)
        {
            unsigned nbit  = j; 
            nbit &= bm::set_word_mask;
            bm::word_t  mask1 = (((bm::word_t)1) << j);

            mask |= mask1;
        }

        printf("0x%x, ", mask);
        
    } 
    printf("\n");
}

void CalcEndMask()
{
    printf("EndMask:\n");

    int i;
    for (i = 0; i < 32; ++i)
    {
    unsigned mask = 1;

        for(int j = i; j > 0; --j)
        {
            unsigned nbit  = j; 
            nbit &= bm::set_word_mask;
            bm::word_t  mask1 = (((bm::word_t)1) << j);

            mask |= mask1;
        }

        printf("0x%x,", mask);
        
    } 
    printf("\n");
}


void EnumeratorTest()
{
    cout << "-------------------------------------------- EnumeratorTest" << endl;

    {
    bvect bvect1;

    bvect1.set_bit(100);

    bvect::enumerator en = bvect1.first();

    if (*en != 100)
    {
        cout << "Enumerator error !" << endl;
        exit(1);
    }

    bvect1.clear_bit(100);

    bvect1.set_bit(2000000000);
    en.go_first();

    if (*en != 2000000000)
    {
        cout << "Enumerator error !" << endl;
        exit(1);
    }
    }

    {
        bvect bvect1;
        bvect1.set_bit(0);
        bvect1.set_bit(10);
        bvect1.set_bit(35);
        bvect1.set_bit(1000);
        bvect1.set_bit(2016519);
        bvect1.set_bit(2034779);
        bvect1.set_bit(bm::id_max-1);

        bvect::enumerator en = bvect1.first();

        unsigned num = bvect1.get_first();

        bvect::enumerator end = bvect1.end();
        while (en < end)
        {
            if (*en != num)
            {
                cout << "Enumeration comparison failed !" << 
                        " enumerator = " << *en <<
                        " get_next() = " << num << endl; 
                exit(1);
            }
            ++en;
            num = bvect1.get_next(num);
        }
        if (num != 0)
        {
            cout << "Enumeration error!" << endl;
            exit(1);
        }
    }
/*
    {
        bvect bvect1;
        bvect1.set();

        bvect::enumerator en = bvect1.first();

        unsigned num = bvect1.get_first();

        while (en < bvect1.end())
        {
            if (*en != num)
            {
                cout << "Enumeration comparison failed !" << 
                        " enumerator = " << *en <<
                        " get_next() = " << num << endl; 
                exit(1);
            }

            ++en;
            num = bvect1.get_next(num);
        }
        if (num != 0)
        {
            cout << "Enumeration error!" << endl;
            exit(1);
        }
    }
*/

    {
        bvect bvect1;

        int i;
        for(i = 0; i < 65536; ++i)
        {
            bvect1.set_bit(i);
        }
/*
        for(i = 65536*10; i < 65536*20; i+=3)
        {
            bvect1.set_bit(i);
        }
*/

        bvect::enumerator en = bvect1.first();

        unsigned num = bvect1.get_first();

        while (en < bvect1.end())
        {
            if (*en != num)
            {
                cout << "Enumeration comparison failed !" << 
                        " enumerator = " << *en <<
                        " get_next() = " << num << endl; 
                exit(1);
            }
            ++en;
            num = bvect1.get_next(num);
            if (num == 31)
            {
                num = num + 0;
            }
        }
        if (num != 0)
        {
            cout << "Enumeration error!" << endl;
            exit(1);
        }
    }


    {
    bvect bvect1;
    bvect1.set_new_blocks_strat(bm::BM_GAP);
    bvect1.set_bit(100);

    bvect::enumerator en = bvect1.first();

    if (*en != 100)
    {
        cout << "Enumerator error !" << endl;
        exit(1);
    }

    bvect1.clear_bit(100);

    bvect1.set_bit(2000000);
    en.go_first();

    if (*en != 2000000)
    {
        cout << "Enumerator error !" << endl;
        exit(1);
    }
    print_stat(bvect1);
    }

    {
        bvect bvect1;
        bvect1.set_new_blocks_strat(bm::BM_GAP);
        bvect1.set_bit(0);
        bvect1.set_bit(1);
        bvect1.set_bit(10);
        bvect1.set_bit(100);
        bvect1.set_bit(1000);

        bvect::enumerator en = bvect1.first();

        unsigned num = bvect1.get_first();

        while (en < bvect1.end())
        {
            if (*en != num)
            {
                cout << "Enumeration comparison failed !" << 
                        " enumerator = " << *en <<
                        " get_next() = " << num << endl; 
                exit(1);
            }
            ++en;
            num = bvect1.get_next(num);
        }
        if (num != 0)
        {
            cout << "Enumeration error!" << endl;
            exit(1);
        }
    }


}



void BlockLevelTest()
{
    bvect  bv;
    bvect  bv2;

    bv.set_new_blocks_strat(bm::BM_GAP);
    bv2.set_new_blocks_strat(bm::BM_GAP);

    int i;
    for (i = 0; i < 500; i+=1)
    {
        bv.set_bit(i);
    }
    print_stat(bv);

    for (i = 0; i < 1000; i+=2)
    {
        bv2.set_bit(i);
    }
    print_stat(bv2);

    struct bvect::statistics st;
    bv2.calc_stat(&st);

    unsigned char* sermem = new unsigned char[st.max_serialize_mem];

    unsigned slen = bm::serialize(bv2, sermem);
    assert(slen);
    slen = 0;
    
    bm::deserialize(bv, sermem);
//    bv.optimize();

    print_stat(bv);

}

/*
__int64 CalcBitCount64(__int64 b)
{
    b = (b & 0x5555555555555555) + (b >> 1 & 0x5555555555555555);
    b = (b & 0x3333333333333333) + (b >> 2 & 0x3333333333333333);
    b = b + (b >> 4) & 0x0F0F0F0F0F0F0F0F;
    b = b + (b >> 8);
    b = b + (b >> 16);
    b = b + (b >> 32) & 0x0000007F;
    return b;
}


*/


void SyntaxTest()
{
    cout << "----------------------------- Syntax test." << endl;
    bvect bv1;
    
    bvect::allocator_type a = bv1.get_allocator();

    bvect bv2(bv1);
    bvect bv3;
    bv3.swap(bv1);
     
    bv1[100] = true;
    bool v = bv1[100];
    assert(v);
    v = false;

    bv1[100] = false;

    bv2 |= bv1;
    bv2 &= bv1;
    bv2 ^= bv1;
    bv2 -= bv1;

    bv3 = bv1 | bv2;

    if (bv1 < bv2)
    {
    }

    bvect::reference ref = bv1[10];
    bool bn = !ref;
    bool bn2 = ~ref;

    bn = bn2 = false;

    ref.flip();

    bvect bvn = ~bv1;

    cout << "----------------------------- Syntax test ok." << endl;
}


void SetTest()
{
    unsigned cnt;
    bvect bv;

    bv.set();

    cnt = bv.count();
    if (cnt != bm::id_max)
    {
        cout << "Set test failed!." << endl;
        exit(1);
    }

    bv.invert();
    cnt = bv.count();
    if (cnt != 0)
    {
        cout << "Set invert test failed!." << endl;
        exit(1);
    }

    bv.set(0);
    bv.set(bm::id_max-1);
    cnt = bv.count();

    assert(cnt == 2);

    bv.invert();
    print_stat(bv);
    cnt = bv.count();

    if (cnt != bm::id_max-2)
    {
        cout << "Set invert test failed!." << endl;
        exit(1);
    }

    bv.clear();
    bv[1] &= true;
    bool v = bv[1];
    if (v)
    {
        cout << "Set &= test failed!" << endl;
        exit(1);
    }


    bv[1] = true;
    bv[1] &= true;
    v = bv[1];
    if (!v)
    {
        cout << "Set &= test failed (2)!" << endl;
        exit(1);
    }
    bv.clear(true);
    bv.invert();
    bv[1] &= true;
    v = bv[1];
    if (!v)
    {
        cout << "Set &= test failed (2)!" << endl;
        exit(1);
    }


    bvect bv2;
    bv2[1] = true;
    bv2[1] = false;
    bvect::statistics stat1;
    bv2.calc_stat(&stat1);
    
    bv2.optimize();

    bvect::statistics stat2;
    bv2.calc_stat(&stat2);

    if (stat2.bit_blocks != 0 || 
        stat2.gap_blocks != 0 ||
        stat1.memory_used <= stat2.memory_used)
    {
        cout << "Optimization memory free test failed (2)!" << endl;
        exit(1);
    }


    {
        bvect bv3;
        bool changed;
        changed = bv3.set_bit_conditional(10, true, true);
        v = bv3[10];
        if (v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, true, false);
        v = bv3[10];
        if (!v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, false);
        v = bv3[10];
        if (!v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, true);
        v = bv3[10];
        if (v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
    }

    {
        bvect bv3(bm::BM_GAP);
        bool changed;
        changed = bv3.set_bit_conditional(10, true, true);
        v = bv3[10];
        if (v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, true, false);
        v = bv3[10];
        if (!v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, false);
        v = bv3[10];
        if (!v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, true);
        v = bv3[10];
        if (v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
    }

    {
        bvect bv3(bm::BM_GAP);
        bv3.invert();
        bv3.optimize();
        bool changed;
        changed = bv3.set_bit_conditional(10, true, true);
        v = bv3[10];
        if (!v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, true, false);
        v = bv3[10];
        if (!v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, false);
        v = bv3[10];
        if (!v || changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, false, true);
        v = bv3[10];
        if (v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
        changed = bv3.set_bit_conditional(10, true, false);
        v = bv3[10];
        if (!v || !changed) {
            cout << "Conditional bit set failed." << endl;
            exit(1);
        }
    }


    {
        bvect bv(0);
        bv.resize(100);
        bv[10] = true;
        bv.resize(1000001);
        bv[100000] = 1;

        if (bv.size() != 1000001)
        {
            cout << "Resize failed" << endl;
            exit(1);
        }
        if (bv.count() != 2)
        {
            cout << "Resize count failed" << endl;
            exit(1);
        }

        bv.resize(100);
        if (bv.size() != 100)
        {
            cout << "Resize failed" << endl;
            exit(1);
        }
        if (bv.count() != 1)
        {
            cout << "Resize count failed" << endl;
            exit(1);
        }
        
        bv.resize(60000100);
        bv.invert();
        bv.clear(true);


        if (bv.size() != 60000100)
        {
            cout << "Resize failed" << endl;
            exit(1);
        }
        if (bv.count() != 0)
        {
            cout << "Resize count failed" << endl;
            exit(1);
        }
    }

}


template<class A, class B> void CompareMiniSet(const A& ms,
                                          const B& bvm)
{
    for (unsigned i = 0; i < bm::set_total_blocks; ++i)
    {
        bool ms_val = ms.test(i)!=0;
        bool bvm_val = bvm.is_bit_true(i)!=0;
        if (ms_val != bvm_val)
        {
            printf("MiniSet comparison error: %u\n",i);
            exit(1);
        }
    }
}

void MiniSetTest()
{
    cout << "----------------------- MiniSetTest" << endl;
    {
    bm::miniset<bm::block_allocator, bm::set_total_blocks> ms;
    bvect_mini bvm(bm::set_total_blocks);


    CompareMiniSet(ms, bvm);


    ms.set(1);
    bvm.set_bit(1);

    CompareMiniSet(ms, bvm);

    unsigned i;

    for (i = 1; i < 10; i++)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < 10; i++)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);


    for (i = 1; i < 10; i+=3)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < 5; i+=3)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);
    }


    {
    bm::miniset<bm::block_allocator, bm::set_total_blocks> ms;
    bvect_mini bvm(bm::set_total_blocks);


    ms.set(1);
    bvm.set_bit(1);

    CompareMiniSet(ms, bvm);

    unsigned i;
    for (i = 1; i < bm::set_total_blocks; i+=3)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < bm::set_total_blocks/2; i+=3)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);
    }


    {
    bm::bvmini<bm::set_total_blocks> ms(0);
    bvect_mini bvm(bm::set_total_blocks);


    CompareMiniSet(ms, bvm);


    ms.set(1);
    bvm.set_bit(1);

    CompareMiniSet(ms, bvm);

    unsigned i;

    for (i = 1; i < 10; i++)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < 10; i++)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);


    for (i = 1; i < bm::set_total_blocks; i+=3)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < bm::set_total_blocks/2; i+=3)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);
    }


    {
    bm::miniset<bm::block_allocator, bm::set_total_blocks> ms;
    bvect_mini bvm(bm::set_total_blocks);


    ms.set(1);
    bvm.set_bit(1);

    CompareMiniSet(ms, bvm);

    unsigned i;
    for (i = 1; i < 15; i+=3)
    {
        ms.set(i);
        bvm.set_bit(i);
    }
    CompareMiniSet(ms, bvm);

    for (i = 1; i < 7; i+=3)
    {
        ms.set(i, false);
        bvm.clear_bit(i);
    }
    CompareMiniSet(ms, bvm);
    }


    cout << "----------------------- MiniSetTest ok" << endl;
}


unsigned CalcBitCount32(unsigned b)
{
    b = (b & 0x55555555) + (b >> 1 & 0x55555555);
    b = (b & 0x33333333) + (b >> 2 & 0x33333333);
    b = b + ((b >> 4) & 0x0F0F0F0F);
    b = b + (b >> 8);
    b = b + ((b >> 16) & 0x0000003F);
    return b;
}


void PrintGapLevels(const gap_word_t* glevel)
{
    cout << "Gap levels:" << endl;
    unsigned i;
    for (i = 0; i < bm::gap_levels; ++i)
    {
        cout << glevel[i] << ",";
    }
    cout << endl;
}

void OptimGAPTest()
{
    gap_word_t    glevel[bm::gap_levels];
    ::memcpy(glevel, gap_len_table<true>::_len, bm::gap_levels * sizeof(gap_word_t));

    {
    gap_word_t  length[] = { 2, 2, 5, 5, 10, 11, 12 };
    unsigned lsize = sizeof(length) / sizeof(gap_word_t);

    bm::improve_gap_levels(length, length + lsize, glevel);

    PrintGapLevels(glevel);
    }

    {
    gap_word_t  length[] = { 3, 5, 15, 15, 100, 110, 120 };
    unsigned lsize = sizeof(length) / sizeof(gap_word_t);

    bm::improve_gap_levels(length, length + lsize, glevel);
    PrintGapLevels(glevel);
    }

    {
    gap_word_t  length[] = { 15, 80, 5, 3, 100, 110, 95 };
    unsigned lsize = sizeof(length) / sizeof(gap_word_t);

    bm::improve_gap_levels(length, length + lsize, glevel);
    PrintGapLevels(glevel);
    }

    {
    gap_word_t  length[] = 
    { 16,30,14,24,14,30,18,14,12,16,8,38,28,4,20,18,28,22,32,14,12,16,10,8,14,18,14,8,
      16,30,8,8,58,28,18,4,26,14,52,12,18,10,14,18,22,18,20,70,12,6,26,6,8,22,12,4,8,8,
      8,54,18,6,8,4,4,10,4,4,4,4,4,6,22,14,38,40,56,50,6,10,8,18,82,16,6,18,20,12,12,
      16,8,14,14,10,16,12,10,16,14,12,18,14,18,34,14,12,18,18,10,20,10,18,8,14,14,22,16,
      10,10,18,8,20,14,10,14,12,12,14,16,16,6,10,14,6,10,10,10,10,12,4,8,8,8,10,10,8,
      8,12,10,10,14,14,14,8,4,4,10,10,4,10,4,8,6,52,104,584,218
    };
    unsigned lsize = sizeof(length) / sizeof(gap_word_t);

    bm::improve_gap_levels(length, length + lsize, glevel);
    PrintGapLevels(glevel);
    }

    {
    gap_word_t  length[] = {
     30,46,26,4,4,68,72,6,10,4,6,14,6,42,198,22,12,4,6,24,12,8,18,4,6,10,6,4,6,6,12,6
    ,6,4,4,78,38,8,52,4,8,10,6,8,8,6,10,4,6,6,4,10,6,8,16,22,28,14,10,10,16,10,20,10
    ,14,12,8,18,4,8,10,6,10,4,6,12,16,12,6,4,8,4,14,14,6,8,4,10,10,8,8,6,8,6,8,4,8,4
    ,8,10,6,4,6 
    };
    unsigned lsize = sizeof(length) / sizeof(gap_word_t);

    bm::improve_gap_levels(length, length + lsize, glevel);
    PrintGapLevels(glevel);

    }

}

#if defined(BMSSE2OPT) || defined(BMSSE42OPT)

#if(_MSC_VER)
#  define BM_ALIGN16 __declspec(align(16))
#  define BM_ALIGN16ATTR
#else // GCC
#  define BM_ALIGN16
#  define BM_ALIGN16ATTR __attribute__((aligned (16)))
#endif

#else // NO SSE

#  define BM_ALIGN16ATTR
#  define BM_ALIGN16

#endif 


void BitCountChangeTest()
{
    cout << "---------------------------- BitCountChangeTest " << endl;

    unsigned i;
    for(i = 0xFFFFFFFF; i; i <<= 1) 
    { 
        unsigned a0 = bm::bit_count_change(i);
        unsigned a1 = BitCountChange(i);
        
        if (a0 != a1)
        {
            cout << hex 
                 << "Bit count change test failed!" 
                 << " i = " << i << " return = " 
                 << a0 << " check = " << a1
                 << endl;
            exit(1);
        }
    }

    cout << "---------------------------- STEP 2 " << endl;

    for(i = 0xFFFFFFFF; i; i >>= 1) 
    { 
        unsigned a0 = bm::bit_count_change(i);
        unsigned a1 = BitCountChange(i);
        
        if (a0 != a1)
        {
            cout << "Bit count change test failed!" 
                 << " i = " << i << " return = " 
                 << a0 << " check = " << a1
                 << endl;
            exit(1);
        }
    }

    cout << "---------------------------- STEP 3 " << endl;

    for (i = 0; i < 0xFFFFFFF; ++i)
    {
        unsigned a0 = bm::bit_count_change(i);
        unsigned a1 = BitCountChange(i);
        
        if (a0 != a1)
        {
            cout << "Bit count change test failed!" 
                 << " i = " << i << " return = " 
                 << a0 << " check = " << a1
                 << endl;
            exit(1);
        }
    }
  

    bm::word_t  BM_ALIGN16 arr[16] BM_ALIGN16ATTR = {0,};
    arr[0] = (bm::word_t)(1 << 31);
    arr[1] = 1; //(bm::word_t)(1 << 31);
    
    bm::id_t cnt;
    bm::id_t bc, bc1;
    
    cnt = bm::bit_count_change(arr[1]);
    cout << cnt << endl;
    if (cnt != 2)
    {
        cout << "0.count_change() failed " << cnt << endl;
        exit(1);
    }
    
    cnt = bm::bit_block_calc_count_change(arr, arr+4, &bc);
    
    bc1 = bit_block_calc_count(arr, arr+4);
    if (bc != bc1)
    {
        cout << "1. bitcount comparison failed " << endl;
    }
    
    if (cnt != 3)
    {
        cout << "1. count_intervals() failed " << cnt << endl;
        exit(1);
    }

    ::memset(arr, 0, sizeof(arr));

    arr[0] = arr[1] = arr[2] = 0xFFFFFFFF;
    arr[3] = (bm::word_t)(0xFFFFFFFF >> 1);
    
    cnt = bm::bit_block_calc_count_change(arr, arr+4, &bc);
    cout << cnt << endl;

    bc1 = bit_block_calc_count(arr, arr+4);
    if (bc != bc1)
    {
        cout << "1.1 bitcount comparison failed " << endl;
    }
    
    // this test is not correct for both 32 and 64 bit mode because of loop unroll
    if (cnt != 2 && cnt != 3)
    {
        cout << "1.1 count_intervals() failed " << cnt << endl;
        exit(1);
    }
    
 
    cout << "---------------------------- STEP 4 " << endl;

    bvect   bv1;
    cnt = bm::count_intervals(bv1);
    
    if (cnt != 1)
    {
        cout << "1.count_intervals() failed " << cnt << endl;
        exit(1);
    }
    CheckIntervals(bv1, 65536);
    
    bv1.invert();

    cnt = count_intervals(bv1);
    cout << "Inverted cnt=" << cnt << endl;
    
    if (cnt != 2)
    {
        cout << "2.inverted count_intervals() failed " << cnt << endl;
        exit(1);
    }
    
    bv1.invert();
        
    for (i = 10; i < 100000; ++i)
    {
        bv1.set(i);
    }
    
    cnt = count_intervals(bv1);
    
    if (cnt != 3)
    {
        cout << "3.count_intervals() failed " << cnt << endl;
        exit(1);
    }
    cout << "-----" << endl;
    CheckIntervals(bv1, 65536*2);
    cout << "Optmization..." << endl; 
    bv1.optimize();
    cnt = count_intervals(bv1);
    
    if (cnt != 3)
    {
        cout << "4.count_intervals() failed " << cnt << endl;
        exit(1);
    }
    
    CheckIntervals(bv1, 65536*2);

	cout << "---------------------------- array GAP test " << endl;

	{
		bm::gap_word_t arr[] = { 0 };

		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 1)
		{
			cout << "Array gap test failed. 1. " << endl;
			exit(1);
		}

        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);
        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 1. " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 1. " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 65535 };

		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 2)
		{
			cout << "Array gap test failed. 1.1 " << endl;
			exit(1);
		}

        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 1.1 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 1. " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 0, 65535 };

		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 3)
		{
			cout << "Array gap test failed. 1.2 " << endl;
			exit(1);
		}

        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 1.2 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 1.2 " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 0, 1, 2, 65534, 65535 };

		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 3)
		{
			cout << "Array gap test failed. 1.3 " << endl;
			exit(1);
		}

        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 1.3 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 1.3 " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 0, 1, 2 };
		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 1)
		{
			cout << "Array gap test failed. 2. " << endl;
			exit(1);
		}
        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 2 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 2 " << endl;
			exit(1);
        }

	}

	{
		bm::gap_word_t arr[] = { 1, 2 };
		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 2)
		{
			cout << "Array gap test failed. 3. " << endl;
			exit(1);
		}
        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 3 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 3 " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 1, 2, 10 };
		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 4)
		{
			cout << "Array gap test failed. 4. " << endl;
			exit(1);
		}
        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 4 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 4 " << endl;
			exit(1);
        }
	}

	{
		bm::gap_word_t arr[] = { 1, 2, 10, 11 };
		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 4)
		{
			cout << "Array gap test failed. 5. " << endl;
			exit(1);
		}
        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 5 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 5 " << endl;
			exit(1);
        }

	}

	{
		bm::gap_word_t arr[] = { 1, 2, 10, 11, 256 };
		unsigned gap_count;

		gap_count = bit_array_compute_gaps(arr, sizeof(arr)/sizeof(arr[0]));
		if (gap_count != 6)
		{
			cout << "Array gap test failed. 6. " << endl;
			exit(1);
		}
        bm::gap_word_t gap[20] = {0};
        bm::gap_word_t gap_cntrl[20] = {0};

        gap_set_all(gap_cntrl, bm::gap_max_bits, 0);
        for (unsigned i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
        {
            unsigned is_set;
            gap_set_value(1, gap_cntrl, arr[i], &is_set);
        }
        unsigned gap_l_cntrl = gap_length(gap_cntrl);

        unsigned gap_len = gap_set_array(&gap[0], arr, sizeof(arr)/sizeof(arr[0]));
        unsigned gap_len1 = gap_length(gap);

		if (gap_len != gap_l_cntrl || gap_len1 != gap_l_cntrl)
		{
			cout << "Array gap test failed. 6 " << endl;
			exit(1);
		}
        int cmpres = gapcmp(gap, gap_cntrl);
        if (cmpres != 0)
        {
			cout << "Array gap cmp test failed. 6 " << endl;
			exit(1);
        }

	}


    cout << "---------------------------- BitCountChangeTest Ok." << endl;
}


void DNACompressionTest()
{
    const char seeds[] = 
        { 'A', 'C', 'G', 'T', 'A', 'C', 'G', 'A', 'N', 'A', 'C', 'G' };
    
    const unsigned arr_size = bm::set_block_size*4;
    const unsigned arr_plain_size = arr_size / 8;    
    
    unsigned char BM_ALIGN16 block1[arr_size] BM_ALIGN16ATTR = {0,};

    unsigned char BM_ALIGN16 tmatrix1[8][arr_plain_size] BM_ALIGN16ATTR;
    unsigned BM_ALIGN16 distance1[8][8] BM_ALIGN16ATTR;
    unsigned char pc_vector1[8] = {0,};
    unsigned pc_vector_stat1[bm::ibpc_end];
/*
    unsigned   BM_ALIGN16 tmatrix2[32][bm::set_block_plain_size] BM_ALIGN16ATTR;
    unsigned  
    BM_ALIGN16 distance2[bm::set_block_plain_cnt][bm::set_block_plain_cnt] BM_ALIGN16ATTR;
    unsigned char pc_vector2[32] = {0,};


    unsigned   BM_ALIGN16 tmatrix3[32][bm::set_block_plain_size] BM_ALIGN16ATTR;
    unsigned  
    BM_ALIGN16 distance3[bm::set_block_plain_cnt][bm::set_block_plain_cnt] BM_ALIGN16ATTR;
    unsigned char pc_vector3[32] = {0,};
*/
    
    // generate pseudo-random DNA sequence
    for (unsigned i = 0; i < arr_size; ++i)
    {
        unsigned letter_idx = rand() % sizeof(seeds);
        unsigned char l = seeds[letter_idx];
        unsigned char c = 0;
        switch (l)
        {
        case 'A':
            c = 0; break;
        case 'C':
            c = 1; break;
        case 'G':
            c = 2; break;
        case 'T':
            c = 3; break;
        case 'N':
            c = 4; break;
        default:
            cout << "Alphabet error!" << endl;
            exit(1);
        };
        block1[i] = c;
        //cout << block1[i];
    }
    cout << endl;
        
    bm::vect_bit_transpose<unsigned char, 
                           8, 
                           arr_plain_size>
                           (block1, arr_size, tmatrix1);
    
    bm::tmatrix_distance<unsigned char, 
                         8, 
                         arr_plain_size>
                         (tmatrix1, distance1);
    
    cout << "ALL count=" << sizeof(char)*8*arr_plain_size << endl;
    bm::bit_iblock_make_pcv<unsigned char, 8, arr_plain_size>(distance1, pc_vector1);
    
    bm::bit_iblock_pcv_stat(pc_vector1, pc_vector1 + 8, pc_vector_stat1);
    
    for (unsigned s = 0; s < bm::ibpc_end; ++s)
    {
        switch(s)
        {
        case bm::ibpc_uncompr:
            cout << "Uncompressed: "; 
            break;
        case bm::ibpc_all_zero:
            cout << "    All ZERO: "; 
            break;
        case bm::ibpc_all_one:
            cout << "     All ONE: "; 
            break;
        case bm::ibpc_equiv:
            cout << "       Equiv: "; 
            break;
        case bm::ibpc_close:
            cout << "     Similar: "; 
            break;
        default:
            //cout << "Oops!" << s << " "; 
            break;
        }
        cout << pc_vector_stat1[s] << endl;
    } // for
    

    // print out the pc_vector    
    for (unsigned j = 0; j < 8; ++j)
    {
        unsigned ibpc = pc_vector1[j] & 7;
        unsigned n_row = (pc_vector1[j] >> 3);
        cout << j << ":" << "->" << n_row << " ";
        
        switch(ibpc)
        {
        case bm::ibpc_uncompr:
            cout << "Uncompressed: "; 
            cout << " popcnt=" << distance1[j][j];
            break;
        case bm::ibpc_all_zero:
            cout << "ZERO";
            break;            
        case bm::ibpc_all_one:
            cout << "ONE: "; 
            break;
        case bm::ibpc_equiv:
            cout << "Equiv: "; 
            break;            
        case bm::ibpc_close:
            cout << " Similar: "; 
            cout << " popcnt="  << distance1[j][j]
                 << " Humming=" << distance1[j][n_row];            
            break;
        }
        cout << endl;
    }
/*
    cout << endl << "Second round." << endl << endl;

    bm::bit_iblock_reduce(tmatrix1, pc_vector1, pc_vector1+32, tmatrix2);
    bm::tmatrix_distance<unsigned, 
                         bm::set_block_plain_cnt, 
                         bm::set_block_plain_size>
                         (tmatrix2, distance2);    
    
    bm::bit_iblock_make_pcv(distance2, pc_vector2);
    
    // print out the pc_vector    
    for (unsigned j = 0; j < 32; ++j)
    {
        unsigned ibpc = pc_vector2[j] & 7;
        unsigned n_row = (pc_vector2[j] >> 3);
        cout << j << ":" << "->" << n_row << " ";
        
        switch(ibpc)
        {
        case bm::ibpc_uncompr:
            cout << "Uncompressed: "; 
            cout << " popcnt=" << distance2[j][j];
            break;
        case bm::ibpc_all_zero:
            cout << "ZERO";
            break;            
        case bm::ibpc_all_one:
            cout << "ONE: "; 
            break;
        case bm::ibpc_equiv:
            cout << "Equiv: "; 
            break;            
        case bm::ibpc_close:
            cout << " Similar: "; 
            cout << " popcnt="  << distance2[j][j]
                 << " Humming=" << distance2[j][n_row] << endl; 
             {
                const unsigned* r1 = tmatrix2[j];
                for (unsigned i = 0; i < bm::set_block_plain_size; ++i)
                {
                    cout << hex << r1[i] << " ";
                }
                cout << dec << endl << endl;                         
             }           
            break;
        }
        cout << endl;
    }


    cout << endl << "3rd round." << endl << endl;

    bm::bit_iblock_reduce(tmatrix2, pc_vector2, pc_vector2+32, tmatrix3);

    bm::tmatrix_distance<unsigned, 
                         bm::set_block_plain_cnt, 
                         bm::set_block_plain_size>
                         (tmatrix3, distance3);    
    
    bm::bit_iblock_make_pcv(distance3, pc_vector3);
    
    // print out the pc_vector    
    for (unsigned j = 0; j < 32; ++j)
    {
        unsigned ibpc = pc_vector3[j] & 7;
        unsigned n_row = (pc_vector3[j] >> 3);
        cout << j << ":" << "->" << n_row << " ";
        
        switch(ibpc)
        {
        case bm::ibpc_uncompr:
            cout << "Uncompressed: "; 
            cout << " popcnt=" << distance3[j][j];
            break;
        case bm::ibpc_all_zero:
            cout << "ZERO";
            break;            
        case bm::ibpc_all_one:
            cout << "ONE: "; 
            break;
        case bm::ibpc_equiv:
            cout << "Equiv: "; 
            break;            
        case bm::ibpc_close:
            cout << " Similar: "; 
            cout << " popcnt="  << distance3[j][j]
                 << " Humming=" << distance3[j][n_row] << endl; 
             {
                const unsigned* r1 = tmatrix3[j];
                for (unsigned i = 0; i < bm::set_block_plain_size; ++i)
                {
                    cout << hex << r1[i] << " ";
                }
                cout << dec << endl << endl;                         
             }           
            break;
        }
        cout << endl;
    }
*/    
    
}

void BitBlockTransposeTest()
{
    DNACompressionTest();
   

    bm::word_t BM_ALIGN16 block1[bm::set_block_size] BM_ALIGN16ATTR = {0,};
    bm::word_t BM_ALIGN16 block2[bm::set_block_size] BM_ALIGN16ATTR = {0xFF,};
    unsigned   BM_ALIGN16 tmatrix1[32][bm::set_block_plain_size] BM_ALIGN16ATTR;


    cout << "---------------------------- BitTransposeTest" << endl;

    cout << "Transpose 1" << endl;

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        block1[i] = 1;
    }

    bm::vect_bit_transpose<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (block1, bm::set_block_size, tmatrix1);

    bm::vect_bit_trestore<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (tmatrix1, block2);

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        if (block1[i] != block2[i])
        {
            cout << "Bit transpose error! " << i << endl; exit(1);
        }
    }

    {
    unsigned BM_ALIGN16 distance[bm::set_block_plain_cnt][bm::set_block_plain_cnt];
    bm::tmatrix_distance<unsigned, 
                         bm::set_block_plain_cnt, 
                         bm::set_block_plain_size>
                         (tmatrix1, distance);
    
    PrintDistanceMatrix(distance);

    // distance matrix verification:
    {
    for (unsigned i = 0; i < bm::set_block_plain_cnt; ++i)
    {
        const unsigned* row = distance[i];
        for (unsigned j = i; j < bm::set_block_plain_cnt; ++j)
        {
            if (i == j)
            {
                if (distance[0][0] != 2048)
                {
                    cout << "Self distance(bitcount) is incorrect!" << endl;
                    exit(1);
                }
            }
            else
            {
                if (i == 0)
                {
                    if (row[j] != 2048) // max distance
                    {
                        cout << "Incorrect max distance!" << endl; exit(1);
                    }
                }
                else
                {
                    if (row[j] != 0) // max distance
                    {
                        cout << "Incorrect min distance!" << endl; exit(1);
                    }
                }
            }
        }
    }
    }

    }

    cout << "Transpose 2" << endl;

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        block1[i] = 1 | (1 << 17);
    }

    bm::vect_bit_transpose<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (block1, bm::set_block_size, tmatrix1);
    bm::vect_bit_trestore<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (tmatrix1, block2);


    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        if (block1[i] != block2[i])
        {
            cout << "Bit transpose error! " << i << endl; exit(1);
        }
    }

    cout << "Transpose 3" << endl;

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        block1[i] = ~1;
    }

    bm::vect_bit_transpose<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (block1, bm::set_block_size, tmatrix1);
    bm::vect_bit_trestore<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (tmatrix1, block2);

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        if (block1[i] != block2[i])
        {
            cout << "Bit transpose error! " << i << endl; exit(1);
        }
    }

    cout << "Transpose 4" << endl;

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        block1[i] = i;
    }

    bm::vect_bit_transpose<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (block1, bm::set_block_size, tmatrix1);
    bm::vect_bit_trestore<unsigned, 
                           bm::set_block_plain_cnt, 
                           bm::set_block_plain_size>
                           (tmatrix1, block2);

    for (unsigned i = 0; i < bm::set_block_size; ++i)
    {
        if (block1[i] != block2[i])
        {
            cout << "Bit transpose error! " << i << endl; exit(1);
        }
    }
/*    
    cout << "Transpose 5 - random" << endl;

    for (unsigned c = 0; c < 10000; ++c)
    {
        if ((c % 100) == 0) cout << ".";

        for (unsigned i = 0; i < bm::set_block_size; ++i)
        {
            block1[i] = rand();
        }

        bm::vect_bit_transpose<unsigned, 
                               bm::set_block_plain_cnt, 
                               bm::set_block_plain_size>
                               (block1, bm::set_block_size, tmatrix1);

        bm::vect_bit_trestore<unsigned, 
                               bm::set_block_plain_cnt, 
                               bm::set_block_plain_size>
                               (tmatrix1, block2);


        for (unsigned i = 0; i < bm::set_block_size; ++i)
        {
            if (block1[i] != block2[i])
            {
                cout << "Bit transpose error! " << i << endl; exit(1);
            }
        }
    }
 */   
    cout << "Transpose GAP block 1" << endl;
    
    {
    gap_vector   gapv(0);
    gap_vector   gapv1(9);
    gapv.set_bit(1);
    gapv.set_bit(2);
    gapv.set_bit(10);
    gapv.set_bit(65000);
    

    gap_transpose_engine<bm::gap_word_t, bm::word_t, bm::set_block_size> gte;
    
    if (sizeof(gte.tmatrix_) != (2048 * sizeof(bm::word_t)))
    {
        cout << "TMatrix recalculation error!" << sizeof(gte.tmatrix_) << endl;
        exit(1);
    }
    gte.transpose(gapv.get_buf());//, block1);


    gte.compute_distance_matrix();
    gte.reduce();
    gte.restore();
    
    unsigned glen = *(gapv.get_buf()) >> 3;
    PrintGap(gapv.get_buf());
    PrintDGap((gap_word_t*) block1, glen-1);
    PrintDGapGamma((gap_word_t*) block1, glen-1);
    
    PrintTMatrix(gte.tmatrix_, gte.eff_cols_, true);
    
    bm::gap_word_t gap_head = *gapv.get_buf();
    gte.trestore(gap_head, gapv1.get_buf());//, block2);
    
    if (gapv.compare(gapv1))
    {
        cout << "GAP block transpose error!" << endl;
        PrintGap(gapv.get_buf());
        PrintGap(gapv1.get_buf());
        exit(1);
    }
    
    }

    cout << "Transpose GAP block 2" << endl;

    {
    gap_vector   gapv(0);
    gap_vector   gapv1(0);

    unsigned gcnt = 5;
    for (unsigned i = 0; i < 65500; i+= 50)
    {
        for (unsigned j = 0; j < gcnt ; ++j)
        {
            gapv.set_bit(i);

            if (++i > 65500) 
                break;
        }
        gcnt += 2;
    }

    gap_transpose_engine<bm::gap_word_t, bm::word_t, bm::set_block_size> gte;    
    gte.transpose(gapv.get_buf());


    gte.compute_distance_matrix();
    gte.reduce();
    gte.restore();
    
    unsigned glen = *(gapv.get_buf()) >> 3;
    cout << glen << endl;

    bm::gap_word_t gap_head = *gapv.get_buf();
    gte.trestore(gap_head, gapv1.get_buf());
    
    if (gapv.compare(gapv1))
    {
        cout << "GAP block transpose error!" << endl;
        PrintGap(gapv.get_buf());
        PrintGap(gapv1.get_buf());
        exit(1);
    }

    }
    

    cout << endl << "---------------------------- BitTransposeTest ok" << endl;
}

/*
#define POWER_CHECK(w, mask) \
    (bm::bit_count_table<true>::_count[(w&mask) ^ ((w&mask)-1)])

void BitListTest()
{
    unsigned bits[64] = {0,};

    unsigned w = 0;

    w = (1 << 3) | 1;


    int bn3 = POWER_CHECK(w, 1 << 3) - 1;
    int bn2 = POWER_CHECK(w, 1 << 2) - 1;
    int bn0 = POWER_CHECK(w, 1 << 0) - 1;

    bit_list(w, bits+1);
  
}
*/

void ResizeTest()
{
    {{
    bvect bv(0);
    assert(bv.any() == false);
    assert(bv.count() == 0);
    }}

    {{
    bvect bv1(10);
    bvect bv2(bv1);
    assert(bv1.size() == bv2.size());
    }}

    {{
    bvect bv(10);
    assert(bv.any() == false);
    assert(bv.count() == 0);
    bv.invert();
    unsigned cnt = bv.count();
    assert(cnt == 10);
    }}

    {{
    bvect bv1(10);
    bv1.set(1);
    bvect bv2(0);

    assert(bv1.size() == 10);
    assert(bv2.size() == 0);
    assert(bv1.count() == 1);
    assert(bv2.count() == 0);
    
    bv1.swap(bv2);

    assert(bv2.size() == 10);
    assert(bv2.count() == 1);
    assert(bv1.size() == 0);
    assert(bv1.count() == 0);

    }}

    {{
    bvect bv1;
    bv1.set(65536);
    bv1.set(100);
    assert(bv1.size() == bm::id_max);
    assert(bv1.count() == 2);
    bv1.resize(101);
    assert(bv1.size() == 101);
    assert(bv1.count() == 1);
    {{
    bm::id_t f = bv1.get_first();
    assert(f == 100);
    f = bv1.get_next(f);
    assert(f == 0);
    }}

    bv1.resize(10);
    assert(bv1.size() == 10);
    assert(bv1.count() == 0);
    bm::id_t f = bv1.get_first();
    assert(f == 0);
    }}

    {{
    bvect bv;
    print_stat(bv);
    bv.set(100);
    bv.set(65536 + 10);
    print_stat(bv);
    bv.set_range(0, 65536*10, false);
    print_stat(bv);
    }}

    // test logical operations

    {{
    bvect bv1(65536 * 10);
    bvect bv2(65536 * 100);
    bv1.set(5);
    bv2.set(5);
    bv2.set(65536 * 2);
    bv2 &= bv1;
    assert(bv2.size() == 65536 * 100);
    assert(bv2.count() == 1);
    assert(bv2.get_first() == 5);
    }}

    {{
    bvect bv1(10);
    bvect bv2;
    bv1.set(5);
    bv2.set(5);
    bv2.set(65536 * 2);
    bv1 &= bv2;
    assert(bv1.size() == bv2.size());
    assert(bv1.count() == 1);
    assert(bv1.get_first() == 5);
    }}

    {{
    bvect bv1(10);
    bvect bv2;
    bv1.set(5);
    bv2.set(6);
    bv2.set(65536 * 2);
    bv1 |= bv2;
    assert(bv1.size() == bv2.size());
    assert(bv1.count() == 3);
    }}

    // comparison test

    {{
    int cmp;
    bvect bv1(10);
    bvect bv2;
    bv2.set(65536 * 2);

    cmp = bv1.compare(bv2);
    assert(cmp < 0);

    bv1.set(5);
    assert(cmp < 0);
    cmp = bv1.compare(bv2);
    assert(cmp > 0);
    cmp = bv2.compare(bv1);
    assert(cmp < 0);

    }}

    // inserter

    {{
    bvect bv1(10);
    {
        bvect::insert_iterator it(bv1);
        *it = 100 * 65536;
    }
    assert(bv1.size() ==  100 * 65536 + 1);
    }}

    // serialization

    {{
    bvect bv1(10);
    bv1.set(5);
    struct bvect::statistics st1;
    bv1.calc_stat(&st1);

    unsigned char* sermem = new unsigned char[st1.max_serialize_mem];
    unsigned slen2 = bm::serialize(bv1, sermem);
    cout << slen2 << endl;

    bvect bv2(0);
    bm::deserialize(bv2, sermem);
    delete [] sermem;

    assert(bv2.size() == 10);
    assert(bv2.count() == 1);
    assert(bv2.get_first() == 5);
    
    }}

    {{
    bvect bv1(10);
    bv1.set(5);
    unsigned int arg[] = { 10, 65536, 65537, 65538 * 10000 };
    unsigned* it1 = arg;
    unsigned* it2 = arg + 4;
    combine_or(bv1, it1, it2);
    assert(bv1.size() == 65538 * 10000 + 1);
    bvect::enumerator en = bv1.first();
    while (en.valid())
    {
        cout << *en << " ";
        ++en;
    }
    }}

}

void ExportTest()
{
    cout << "---------------------------- ExportTest..." << endl;

    {
    char buf[20] = {0,};

    buf[0] = 1;
    buf[1] = 1;
    buf[2]= (char)(1 << 1);

    bvect bv1;
    export_array(bv1, buf + 0, buf + 20);

    assert(bv1.count() == 3);
    assert(bv1.test(0));
    assert(bv1.test(8));
    assert(bv1.test(17));
    }

    {
    char buf[65536*10] = {0,};

    buf[0] = 1;
    buf[1] = 1;
    buf[2]= (char)(1 << 1);

    bvect bv1;
    export_array(bv1, buf + 0, buf + 65536*10);

    assert(bv1.count() == 3);
    assert(bv1.test(0));
    assert(bv1.test(8));
    assert(bv1.test(17));
    }

    {
    short buf[20] = {0,};

    buf[0] = 1;
    buf[1] = 1;
    buf[2]= (char)(1 << 1);

    bvect bv1;
    export_array(bv1, buf + 0, buf + 20);

    assert(bv1.count() == 3);
    assert(bv1.test(0));
    assert(bv1.test(16));
    assert(bv1.test(33));
    }

    {
    int buf[20] = {0,};

    buf[0] = 1;
    buf[1] = 1;
    buf[2]= (char)(1 << 1);

    bvect bv1;
    export_array(bv1, buf + 0, buf + 20);

    assert(bv1.count() == 3);
    assert(bv1.test(0));
    assert(bv1.test(32));
    assert(bv1.test(65));
    }


    cout << "---------------------------- ExportTest Ok." << endl;
}


void TestRecomb()
{
    bm::word_t b1[bm::set_block_size]= {0,};
    bm::word_t b2[bm::set_block_size]= {0,};
    bm::word_t br[bm::set_block_size]= {0,};
 
    b1[0] = 1;
    b1[1] = 1;
    b2[0] = 1;

    bitblock_get_adapter bga1(b1);
    bitblock_get_adapter bga2(b2);
    bitblock_store_adapter bbsa(br);
    bm::bit_AND<bm::word_t> and_func;
    bit_recomb<bitblock_get_adapter,
               bitblock_get_adapter,
               bm::bit_AND<bm::word_t>,
               bitblock_store_adapter>
           (bga1, bga2,and_func, bbsa);
/*
    bit_recomb(bitblock_get_adapter(b1),
               bitblock_get_adapter(b2),
               bit_AND<bm::word_t>(),
               bitblock_store_adapter(br)
               );

    assert(br[0] == 1);
    for (int i = 1; i < bm::set_block_size; ++i)
    {
        assert(br[i] == 0);
    }

    bitblock_sum_adapter sa;
    bit_recomb(bitblock_get_adapter(b1),
               bitblock_get_adapter(b2),
               bit_COUNT_AND<bm::word_t>(),
               sa
               );
    assert(sa.sum() == 1);
*/
}


void BitForEachTest()
{
    cout << "---------------------------- BitForEachTest..." << endl;

	unsigned bit_list1[32];
	unsigned bit_list2[32];

	for (unsigned i = 0; i < 65536*50; ++i)
	{
		unsigned bits1 = bm::bit_list(i, bit_list1);
		unsigned bits2 = bm::bit_list_4(i, bit_list2);
		if (bits1 != bits2)
		{
			cout << "Bit for each test failed bit_cnt criteria!" << endl;
			exit(1);
		}
		for (unsigned j = 0; j < bits1; ++j)
		{
			if (bit_list1[j] != bit_list2[j])
			{
				cout << "Bit for each check failed for " << j << endl;
				exit(1);
			}
		}

	} // for

    cout << "---------------------------- BitForEachTest Ok." << endl;
}

void Log2Test()
{
    cout << "---------------------------- Log2 Test..." << endl;

    cout << "Stage 1" << endl;
    for (unsigned  i = 1; i <= 65535; ++i)
    {
        unsigned l1 = bm::ilog2<unsigned short>(i);
        unsigned l2 = iLog2(i);
        unsigned l3 = ilog2_LUT<unsigned short>(i);
        unsigned l4 = l3;//bm::bsr_asm32(i);
        if (l1 != l2 || l2 != l3 || l2 != l4)
        {
            cout << "Log2 error for " << i << endl;
            cout << l2 << " " << l3 << endl;;
            exit(1);
        }
    }
    cout << "Stage 2" << endl;
    for (unsigned  i = 1; i <= 100*65535; ++i)
    {
        unsigned l1 = bm::ilog2<unsigned>(i);
        unsigned l2 = iLog2(i);
        unsigned l3 = ilog2_LUT<unsigned>(i);
        unsigned l4 = l3; //bm::bsr_asm32(i);
        if (l1 != l2 || l2 != l3 || l2 != l4)
        {
            cout << "Log2 error for " << i << endl;
            cout << l2 << " " << l3 << endl;;
            exit(1);
        }
    }
    cout << "---------------------------- Log2 Test Ok." << endl;
}

/*
void BitEncoderTest()
{
    cout << "---------------------------- BitEncoderTest..." << endl;

    cout << "Bit encoder stage 1" << endl;
    
    unsigned char buf1[2048] = {0, };
    {
    encoder enc(buf1, sizeof(buf1));
    bit_out<encoder> bout(enc);

    bout.put_bit(1);
    bout.put_zero_bit();
    bout.put_zero_bits(8);
    bout.put_bit(1);
    bout.put_bit(0);
    bout.put_bit(1);
    bout.flush();

    decoder dec(buf1);
    bit_in<decoder> bin(dec);
    unsigned bit_value;

    bit_value = bin.get_bit();
    if (bit_value != 1) { cout << "BitEncoder failed!" << endl; exit(1);}
    bit_value = bin.get_bit();
    if (bit_value != 0) { cout << "BitEncoder failed!" << endl; exit(1);}
    unsigned zeros = bin.eat_zero_bits();
    if (zeros != 8) { cout << "BitEncoder failed!" << endl; exit(1);}
    bit_value = bin.get_bit();
    if (bit_value != 1) { cout << "BitEncoder failed!" << endl; exit(1);}
    zeros = bin.eat_zero_bits();
    if (zeros != 1) { cout << "BitEncoder failed!" << endl; exit(1);}
    bit_value = bin.get_bit();
    if (bit_value != 1) { cout << "BitEncoder failed!" << endl; exit(1);}
    
    }
    
    cout << "Bit encoder stage 2" << endl;
    
    {
    unsigned char  bits[2048] = {0, };

    {
    encoder enc(buf1, sizeof(buf1));
    bit_out<encoder> bout(enc);
        
        for (unsigned i = 0; i < 2048; ++i)
        {
            unsigned val = rand() % 2;
            if (val > 1 ) val = 1;
            bits[i] = (unsigned char) val;
            
            bout.put_bit(val);
        }
    }
  
    {
    decoder dec(buf1);
    bit_in<decoder> bin(dec);
    
        for (unsigned i = 0; i < 2048; ++i)
        {
            unsigned val = bits[i];
            unsigned bit_value = bin.get_bit();
            if (val != bit_value)
            {
                cout << "Bit " << i << " incorrect!" << endl;
                exit(1);
            }
        }
    }

    }
    cout << "---------------------------- BitForEncoderTest Ok." << endl;
}
*/

void GammaEncoderTest()
{
    cout << "---------------------------- GammaEncoderTest" << endl;
    
    
    unsigned char buf1[2048 * 4] = {0, };
    
    cout << "Stage 1" << endl;

    {
    encoder enc(buf1, sizeof(buf1));
    typedef bit_out<encoder>  TBitIO;
    bit_out<encoder> bout(enc);
    gamma_encoder<bm::gap_word_t, TBitIO> gamma(bout);     
    gamma(65534);
    }

    {
    decoder dec(buf1);
    typedef bit_in<decoder> TBitIO;
    bit_in<decoder> bin(dec);
    gamma_decoder<bm::gap_word_t, TBitIO> gamma(bin);
    
    gap_word_t value = gamma();
    if (value != 65534)
        {
            cout << "Gamma decoder error for value=" << value << endl;
            exit(1);
        }             
    }


    {
    encoder enc(buf1, sizeof(buf1));
    typedef bit_out<encoder>  TBitIO;
    bit_out<encoder> bout(enc);
    gamma_encoder<bm::gap_word_t, TBitIO> gamma(bout);
     
    for (gap_word_t i = 1; i < 15; ++i)
    {
        gamma(i);
    } 
    }    
    
    {
    decoder dec(buf1);
    typedef bit_in<decoder> TBitIO;
    bit_in<decoder> bin(dec);
    gamma_decoder<bm::gap_word_t, TBitIO> gamma(bin);
    
    for (gap_word_t i = 1; i < 15; ++i)
    {
        gap_word_t value = gamma();
        if (value != i)
        {
            cout << "Gamma decoder error for " << i << " value=" << value << endl;
            exit(1);
        }
    }     
    
    }

    cout << "Stage 2" << endl;

    for (unsigned i = 0; i < 256; ++i)
    {
        gap_word_t short_block[64] = {0,};
        
        {
        encoder enc(buf1, sizeof(buf1));
        typedef bit_out<encoder>  TBitIO;
        bit_out<encoder> bout(enc);
        gamma_encoder<bm::gap_word_t, TBitIO> gamma(bout);
         

        for (unsigned j = 0; j < 64; ++j)
        {
            gap_word_t a = rand() % 65535;
            if (!a) a = 65535; // 0 is illegal
            gap_word_t value = short_block[j] = a;
            gamma(value);
        } // for
        }

        {
        decoder dec(buf1);
        typedef bit_in<decoder> TBitIO;
        bit_in<decoder> bin(dec);
        gamma_decoder<bm::gap_word_t, TBitIO> gamma(bin);
        
        for (unsigned j = 0; j < 64; ++j)
        {
            gap_word_t value = short_block[j];
            gap_word_t r = gamma();
            if (r != value)
            {
                cout << "Gamma encoding failure for value=" << value << " gamma=" << r << endl;
                exit(1);
            }
        } // for
        }
    }


    cout << "Stage 3" << endl;

    unsigned code_value = 65535;
    for (unsigned i = 0; i < 10000; ++i)
    {
        gap_word_t short_block[1000] = {0,};
        
        {
        encoder enc(buf1, sizeof(buf1));
        typedef bit_out<encoder>  TBitIO;
        bit_out<encoder> bout(enc);
        gamma_encoder<bm::gap_word_t, TBitIO> gamma(bout);
         
        for (unsigned j = 0; j < 1000; ++j)
        {
            gap_word_t a = code_value;
            if (!a) 
            {
                code_value = a = 65535;
            }

            gap_word_t value = short_block[j] = a;
            gamma(value);
            --code_value;
        } // for
        }

        {
        decoder dec(buf1);
        typedef bit_in<decoder> TBitIO;
        bit_in<decoder> bin(dec);
        gamma_decoder<bm::gap_word_t, TBitIO> gamma(bin);
        
        for (unsigned j = 0; j < 1000; ++j)
        {
            gap_word_t value = short_block[j];
            gap_word_t r = gamma();
            if (r != value)
            {
                cout << "Gamma encoding failure for value=" << value << " gamma=" << r << endl;
                exit(1);
            }
        } // for
        }
    }


    cout << "---------------------------- GammaEncoderTest Ok." << endl;

}

void LoadBVDump(const char* filename, const char* filename_out=0, bool validate=false)
{
    ifstream bv_file (filename, ios::in | ios::binary);
    if (!bv_file.good())
    {
        cout << "Cannot open file: " << filename << endl;
        exit(1);
    }

    ofstream* bv_file_out = 0;

    if (filename_out)
    {
        bv_file_out = new ofstream(filename_out, ios::out | ios::binary);
        if (!bv_file_out->good())
        {
            cout << "Cannot create file: " << filename_out << endl;
            exit(1);
        }    
    }


    unsigned buffer_size = 1024*1024;
    unsigned char* buffer = new unsigned char[buffer_size];

    unsigned count = 0;
    clock_t start = clock();
    unsigned total_out_size = 0;

    for (;1; ++count)
    {
        unsigned bv_size;
        bv_file.read((char*)&bv_size, sizeof(bv_size));
        if (!bv_file.good())
            break;
        if (bv_size == 0)
        {
            cout << "Warning:Zero vector in dump..." << endl;
            continue;
        }
        if (buffer_size < bv_size)
        {
            delete [] buffer;
            buffer_size = bv_size;
            buffer = new unsigned char[buffer_size];
        }
        bv_file.read((char*)buffer, bv_size);
        {
            bvect bv;
            bm::deserialize(bv, (unsigned char*)buffer);

            bvect::statistics st1;
            bv.calc_stat(&st1);

            if (st1.max_serialize_mem > buffer_size)
            {
                delete [] buffer;
                buffer_size = st1.max_serialize_mem;
                buffer = new unsigned char[buffer_size];
            }

            unsigned blob_size = bm::serialize(bv, buffer, BM_NO_GAP_LENGTH|BM_NO_BYTE_ORDER);
            total_out_size += blob_size;

            if (blob_size > bv_size)
            {
                //print_stat(bv);
                //cout << count << ". -" << blob_size-bv_size << endl;
                //exit(1);
            }
            
            if (validate)
            {
                bvect bv_control;
                bm::deserialize(bv_control, (unsigned char*)buffer);
                if (bv_control != bv)
                {
                    cout << "Serialization error!" << endl;
                    exit(1);
                }
            }
            
            if (bv_file_out)
            {
                bv_file_out->write((char*)&blob_size, sizeof(blob_size));
                bv_file_out->write((char*)buffer, blob_size);
            }

        }
        if (count % 1000 == 0)
        {
            cout << count << " out=" << total_out_size << endl;
        }
        //cout << count << ": size=" << bv_size << endl;
    }

    delete [] buffer;
    cout << "Total vectors:" << count << endl;
    cout << "Total out size:" << total_out_size << endl;

    clock_t finish = clock();
    clock_t elapsed_clocks = finish - start;
    double duration = (double)(elapsed_clocks) / CLOCKS_PER_SEC;

    cout << endl
         << "Serialization duration = " << duration 
         << endl;

    bv_file_out->close();
    delete bv_file_out;

}

void GroupByTest(const char* filename, const char* query_filename)
{
	bvect bv_query;

    unsigned count = 0;
	unsigned group_by_count = 0;

    clock_t start = clock();

	// load the query vector
	{
		ifstream bv_file (query_filename, ios::in | ios::binary);
		if (!bv_file.good())
		{
			cout << "Cannot open file: " << query_filename << endl;
			exit(1);
		}
	    unsigned buffer_size = 400*1024*1024;
		unsigned char* buffer = new unsigned char[buffer_size];

        unsigned bv_size=0;
        bv_file.read((char*)&bv_size, sizeof(bv_size));
        if (bv_size == 0)
        {
            cout << "Warning:Zero vector in query dump..." << endl;
            return;
        }
        bv_file.read((char*)buffer, bv_size);
        bm::deserialize(bv_query, (unsigned char*)buffer);
        
		delete [] buffer;

	}


    ifstream bv_file (filename, ios::in | ios::binary);
    if (!bv_file.good())
    {
        cout << "Cannot open file: " << filename << endl;
        exit(1);
    }


    unsigned buffer_size = 100*1024*1024;
    unsigned char* buffer = new unsigned char[buffer_size];

    for (;1; ++count)
    {
        unsigned bv_size;
        bv_file.read((char*)&bv_size, sizeof(bv_size));
        if (!bv_file.good())
            break;
        if (bv_size == 0)
        {
            cout << "Warning:Zero vector in dump..." << endl;
            continue;
        }
        if (buffer_size < bv_size)
        {
            delete [] buffer;
            buffer_size = bv_size;
            buffer = new unsigned char[buffer_size];
        }
        bv_file.read((char*)buffer, bv_size);
        bvect bv;
		if (1)
        {
            bv.clear(true);
            bm::deserialize(bv, (unsigned char*)buffer);

            unsigned bc = bm::count_and(bv, bv_query);
            if (bc)
            {
                ++group_by_count;
            }

/*            
			bv &= bv_query;
			if (bv.any())
			{
				++group_by_count;
			}
*/            

        }
	
	
#if 0
//print_stat(bv_query);
//exit(1);
		{
		bvect bv(BM_GAP);
        operation_deserializer<bvect>::deserialize(bv,
                                                   bv_query,
                                                   (unsigned char*)buffer,
                                                   0,
												   bm::set_AND);
		// control			
		if (0)
        {
            bvect bv_control(BM_GAP);
            bm::deserialize(bv_control, (unsigned char*)buffer);
			bv_control &= bv_query;
			if (bv_control != bv)
			{
				cerr << "Group by control failure" << endl;
				cerr << bv_control.count() << endl;
				cerr << bv.count() << endl;
				exit(1);
			}
        }				


		if (bv.any())
		{
			++group_by_count;
		}
		}
#endif

        if (count % 1000 == 0)
        {
            cout << count << endl;
        }
    }

    delete [] buffer;
    cout << "Total vectors:" << count << endl;
    cout << "Group by vectors:" << group_by_count << endl;

    clock_t finish = clock();
    clock_t elapsed_clocks = finish - start;
    double duration = (double)(elapsed_clocks) / CLOCKS_PER_SEC;

    cout << endl
         << "Test duration = " << duration 
         << endl;
}



void LoadVectors(const char* dir_name, unsigned from, unsigned to)
{
    vector<bvect*>   bv_list;
    vector<unsigned> sz_list;

    unsigned total_size = 0;
    unsigned total_new_size = 0;

    for(; from <= to; ++from)
    {
        char fname[1024];
        sprintf(fname, "%s/%i", dir_name, from);
        bvect* bv = new bvect;

        unsigned fsize = 0;
        LoadBVector(fname, *bv, &fsize);
        //bv->optimize();
        //print_stat(*bv);


        // get new size
        unsigned blob_size = 0;
        {
        bvect::statistics st1;
        bv->calc_stat(&st1);

        unsigned char* blob = new unsigned char[st1.max_serialize_mem];
        blob_size = bm::serialize(*bv, blob, BM_NO_GAP_LENGTH|BM_NO_BYTE_ORDER);

        if (st1.max_serialize_mem < blob_size)
        {
            printf("BLOB size prediction error!\n");
            exit(1);
        }

        //if (from >= 27)
        {
            bvect bv_control;
            bm::deserialize(bv_control, (unsigned char*)blob);
            if (bv_control != *bv)
            {
                cout << "Serialization error!" << endl;
                exit(1);
            }
        }
                
        delete [] blob;

        }

        cout << fname << "    " 
             << " old=" << fsize << " new=" << blob_size 
             << " diff=" << (int)fsize - (int) blob_size
             << endl;

        bv_list.push_back(bv);
        sz_list.push_back(fsize);

        total_size += fsize;
        total_new_size += blob_size;
    } // for

    cout << "Total size = " << total_size / (1024*1024) << "Mb" << endl;
    cout << "  New size = " << total_new_size / (1024*1024) << "Mb" << endl;
    cout << "Total diff = " << (total_size - total_new_size) / (1024*1024) << "Mb" << endl;

    vector<unsigned char*> bv_blobs;

    cout << "Running serialization benchmark..." << endl;
    {
    clock_t start = clock();

        for (size_t i = 0; i < bv_list.size(); ++i)
        {
            const bvect* bv = bv_list[i];
            bvect::statistics st1;
            bv->calc_stat(&st1);
            unsigned char* blob = new unsigned char[st1.max_serialize_mem*2];
            bv_blobs.push_back(blob);

            for (int j = 0; j < (int)(400/(i?i:1)); ++j)
            {
                //unsigned blob_size = 
                    bm::serialize(*bv, blob);
            }
            // delete [] blob;
        }

    clock_t finish = clock();
    clock_t elapsed_clocks = finish - start;
    double duration = (double)(elapsed_clocks) / CLOCKS_PER_SEC;

    cout << endl
         << "Serialization duration = " << duration 
         << endl;
    }

    cout << "Running de-serialization benchmark..." << endl;
    {
    clock_t start = clock();

        for (size_t i = 0; i < bv_blobs.size(); ++i)
        {
            const unsigned char* blob = bv_blobs[i];
            for (int j = 0; j < (int)(400/(i?i:1)); ++j)
            {
                 bvect bv;
                 bm::deserialize(bv, (unsigned char*)blob);
            }
            // delete [] blob;
        }

    clock_t finish = clock();
    clock_t elapsed_clocks = finish - start;
    double duration = (double)(elapsed_clocks) / CLOCKS_PER_SEC;

    cout << endl
         << "DeSerialization duration = " << duration 
         << endl;
    }




    for (size_t i = 0; i < bv_list.size(); ++i)
    {
        delete bv_list[i];
    }
    for (size_t i = 0; i < bv_blobs.size(); ++i)
    {
        delete [] bv_blobs[i];
    }

}

void deser_test()
{
    const char* file_names[] = {"1", "2", "3", "4"};
    const int file_lengths[] = {413760, 127855, 128640, 415192};

    string base_path("c:\\dev\\deser\\");

    bm::bvector<> bv(bm::BM_GAP);
    bm::bvector<> bv2(bm::BM_GAP);

    for(unsigned i = 0; i < sizeof(file_names) / sizeof(file_names[0]); ++i) {
        string fname(base_path);
        fname.append(file_names[i]);

        ifstream f(fname.c_str(), ios::in | ios::binary);

        if (!f.good()) {
            std::cerr << "Bad file name " << fname.c_str() << endl;
            return;
        }

        vector<char> v;
        v.resize(file_lengths[i]);
        f.read(&v[0], file_lengths[i]);

        if (i == 3)
        {
            cout << " ";
        }
        bm::deserialize(bv, (unsigned char*)&v[0]);

        bm::bvector<> bv_temp(bm::BM_GAP);
        bm::deserialize(bv_temp, (unsigned char*)&v[0]);

        bv2 |= bv_temp;

    }
    cerr << bv.count() << " (correct answer = " << bv2.count() << ")" << endl;
    cerr << abs((int)(bv.count() - bv2.count())) << endl;

    bv ^= bv2;
    bv.optimize();

    cerr << bv.count() << endl;

//    print_stat(bv);
    cout << endl;

    bv2.optimize();
//    print_stat(bv2, 126);

}



int main(void)
{
    time_t      start_time = time(0);
    time_t      finish_time;

    //deser_test();
    //return 0;

    TestRecomb();

    OptimGAPTest();

    CalcBeginMask();
    CalcEndMask();

    if (0)
    {
	    GroupByTest("C:/dev/group-by-sets/sets/geo_organization.bvdump",
		            "C:/dev/group-by-sets/single/pubmed.dat");
	    exit(1);
    }


/*
    LoadBVDump("C:/dev/group-by-sets/sets/geo_organization.dat", 
		       "C:/dev/group-by-sets/sets/geo_organization.bvdump", 
			   true); // validate!
    LoadBVDump("C:/dev/group-by-sets/sets/exec_time_msec_bit.dat", 
		       "C:/dev/group-by-sets/sets/exec_time_msec_bit.bvdump", 
			   true);
    LoadBVDump("C:/dev/group-by-sets/sets/geo_country.dat", 
		       "C:/dev/group-by-sets/sets/geo_country.bvdump", 
			   true);
    LoadBVDump("C:/dev/group-by-sets/sets/log_displayeduids.dat", 
		       "C:/dev/group-by-sets/sets/log_displayeduids.bvdump", 
			   true);

    //LoadVectors("c:/dev/bv_perf", 3, 27);
    exit(1);
*/                                                                                                        


     ExportTest();
     ResizeTest();

     MiniSetTest();

     SyntaxTest();

     SetTest();
 
     BitCountChangeTest();
   
     Log2Test();

	 BitForEachTest();

//     BitEncoderTest();
  
     GammaEncoderTest();


     EnumeratorTest();

     BasicFunctionalityTest();

     ClearAllTest();

     GAPCheck();

     MaxSTest();

     GetNextTest();

     SimpleRandomFillTest();
     
     RangeRandomFillTest();

     AndOperationsTest();   
          
     OrOperationsTest();

     XorOperationsTest();

     SubOperationsTest();

     WordCmpTest();

     ComparisonTest();

     BitBlockTransposeTest();

     MutationTest();

     MutationOperationsTest();

     SerializationTest();

     DesrializationTest2();

     BlockLevelTest();

     StressTest(100, 0); // OR
     StressTest(100, 1); // SUB
     StressTest(100, 2); // XOR
     StressTest(100, 3); // AND

     StressTest(800);

    finish_time = time(0);


    cout << "Test execution time = " << finish_time - start_time << endl;

#ifdef MEM_DEBUG
    cout << "Number of BLOCK allocations = " <<  dbg_block_allocator::na_ << endl;
    cout << "Number of PTR allocations = " <<  dbg_ptr_allocator::na_ << endl << endl;

    if(dbg_block_allocator::balance() != 0)
    {
        cout << "Block memory leak! " << endl;
        cout << dbg_block_allocator::balance() << endl;
        exit(1);
    }

    if(dbg_ptr_allocator::balance() != 0)
    {
        cout << "Ptr memory leak! " << endl;
        cout << dbg_ptr_allocator::balance() << endl;
        exit(1);
    }
#endif

    return 0;
}

