#include <foundation/queue.h>
#include <foundation/string_stream.h>
#include <foundation/murmur_hash.h>
#include <foundation/hash.h>
#include <foundation/temp_allocator.h>
#include <foundation/array.h>
#include <foundation/memory.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <utility>

#define ASSERT(x) if ( !(x) ) reportAssertFailure( #x, __FILE__, __LINE__ )

namespace {
	using namespace foundation;

    void reportAssertFailure( const char *expr, const char *file, int line )
    {
        fprintf( stderr, "\n%s(%d): assertion failed:\n%s\n", file, line, expr );
        fflush( stderr );
        abort();
    }
	
	void test_memory() {
		memory_globals::init();
		Allocator &a = memory_globals::default_allocator();

		void *p = a.allocate(100);
		ASSERT(a.allocated_size(p) >= 100);
		ASSERT(a.total_allocated() >= 100);
		void *q = a.allocate(100);
		ASSERT(a.allocated_size(q) >= 100);
		ASSERT(a.total_allocated() >= 200);
		
		a.deallocate(p);
		a.deallocate(q);
		
		memory_globals::shutdown();
	}

	void test_array() {
		memory_globals::init();
		Allocator &a = memory_globals::default_allocator();

		{
			Array<int> v(a);

			ASSERT(array::size(v) == 0);
			array::push_back(v, 3);
			ASSERT(array::size(v) == 1);
			ASSERT(v[0] == 3);

			Array<int> v2(v);
			ASSERT(v2[0] == 3);
			v2[0] = 5;
			ASSERT(v[0] == 3);
			ASSERT(v2[0] == 5);
			v2 = v;
			ASSERT(v2[0] == 3);
			
			ASSERT(size_cast(array::end(v) - array::begin(v)) == array::size(v));
			ASSERT(*array::begin(v) == 3);
			array::pop_back(v);
			ASSERT(array::empty(v));

			for (int i=0; i<100; ++i)
				array::push_back(v, i);
			ASSERT(array::size(v) == 100);
		}

        {
            Allocator &scratch = memory_globals::default_scratch_allocator();
            Array<char> v( scratch );
            ASSERT( array::size( v ) == 0 );
            array::push_back( v, 'a' );
            ASSERT( array::size( v ) == 1 );
            ASSERT( v[ 0 ] == 'a' );
        }

        {
            Allocator &scratch = memory_globals::default_scratch_allocator();
            Array<int> v1( scratch );
            Array<int> v2( scratch );
            array::push_back( v1, 1234 );
            array::push_back( v2, 4567 );
            array::swap(v1, v2);
            ASSERT( v1[ 0 ] == 4567 );
            ASSERT( v2[ 0 ] == 1234 );
        }

		memory_globals::shutdown();
	}

	void test_scratch() {
		memory_globals::init(256*1024);
		Allocator &a = memory_globals::default_scratch_allocator();

		char *p = (char *)a.allocate(10*1024);

		char *pointers[512];
		for (int i=0; i<100; ++i)
			pointers[i] = (char *)a.allocate(1024);
		for (int i=0; i<100; ++i)
			a.deallocate(pointers[i]);

		a.deallocate(p);

		for (int i=0; i<100; ++i)
			pointers[i] = (char *)a.allocate(4*1024);
		for (int i=0; i<100; ++i)
			a.deallocate(pointers[i]);

        // Reserve all scratch space in a single block
        const uint32_t sizeOfHeader = 4;
        void *pExactMaxSize1 = a.allocate( 256 * 1024 - sizeOfHeader, 4 );
        a.deallocate( pExactMaxSize1 );

        // Reserve all scracth space twice and check delegation to backing allocator
        void *pExactMaxSize2 = a.allocate( 256 * 1024 - sizeOfHeader, 4 );
        void *pExactMaxSize3 = a.allocate( 256 * 1024 - sizeOfHeader, 4 );
        a.deallocate( pExactMaxSize2 );
        a.deallocate( pExactMaxSize3 );

        // Allocates small chuck beyond capacity
        for (int i = 0; i<512; ++i)
            pointers[ i ] = (char *)a.allocate( 1024 );
        for (int i = 0; i<512; ++i)
            a.deallocate( pointers[ 511-i ] );  // reverse order deallocation

        // Delegate allocation request too large to the backing allocator
        void *pTooBig = a.allocate( 2 * 256 * 1024, 4 );
        a.deallocate( pTooBig );

        // Check support for small alignment (array of char)
        void *pString = a.allocate( 12, 1 );
        a.deallocate( pString );

		memory_globals::shutdown();
	}

	void test_temp_allocator() {
		memory_globals::init();
		{
			TempAllocator128 ta;
			Array<int> a(ta);
			for (int i=0; i<100; ++i)
				array::push_back(a, i);
			ta.allocate(2*1024);
		}
		memory_globals::shutdown();
	}

	void test_hash() {
		memory_globals::init();
		{
			TempAllocator128 ta;
			Hash<int> h(ta);
			ASSERT(hash::get(h,0,99) == 99);
			ASSERT(!hash::has(h, 0));
			hash::remove(h, 0);
			hash::set(h, 1000, 123);
			ASSERT(hash::get(h,1000,0) == 123);
			ASSERT(hash::get(h,2000,99) == 99);

			for (int i=0; i<100; ++i)
				hash::set(h, i, i*i);
			for (int i=0; i<100; ++i)
				ASSERT(hash::get(h,i,0) == i*i);
			hash::remove(h, 1000);
			ASSERT(!hash::has(h, 1000));
			hash::remove(h, 2000);
			ASSERT(hash::get(h,1000,0) == 0);
			for (int i=0; i<100; ++i)
				ASSERT(hash::get(h,i,0) == i*i);
			hash::clear(h);
			for (int i=0; i<100; ++i)
				ASSERT(!hash::has(h,i));
		}
		memory_globals::shutdown();
	}

	void test_multi_hash()
	{
		memory_globals::init();
		{
			TempAllocator128 ta;
			Hash<int> h(ta);

			ASSERT(multi_hash::count(h, 0) == 0);
			multi_hash::insert(h, 0, 1);
			multi_hash::insert(h, 0, 2);
			multi_hash::insert(h, 0, 3);
			ASSERT(multi_hash::count(h, 0) == 3);

			Array<int> a(ta);
			multi_hash::get(h, 0, a);
			ASSERT(array::size(a) == 3);
			std::sort(array::begin(a), array::end(a));
			ASSERT(a[0] == 1 && a[1] == 2 && a[2] == 3);

			multi_hash::remove(h, multi_hash::find_first(h, 0));
			ASSERT(multi_hash::count(h,0) == 2);
			multi_hash::remove_all(h, 0);
			ASSERT(multi_hash::count(h, 0) == 0);
		}
		memory_globals::shutdown();
	}

	void test_murmur_hash()
	{
		const char *s = "test_string";
        {
		    uint64_t h = murmur_hash_64(s, size_cast(strlen(s)), 0);
		    ASSERT(h == 0xe604acc23b568f83ull);
        }

        {
            uint32_t h = murmur3_hash_32( s, size_cast( strlen( s ) ), 0 );
            ASSERT( h == 0xda041198u );
        }

        {
            uint64_t h = murmur3_hash_64( s, size_cast( strlen( s ) ), 0 );
            ASSERT( h == 0x898a3df17f25c396ull );
        }
    }

	void test_pointer_arithmetic()
	{
		const char check = '\xfe';
		const unsigned test_size = 128;

		TempAllocator512 ta;
		Array<char> buffer(ta);
		array::set_capacity(buffer, test_size);
		memset(array::begin(buffer), 0, array::size(buffer));

		void* data = array::begin(buffer);
		for (unsigned i = 0; i != test_size; ++i) {
			buffer[i] = check;
			char* value = (char*)memory::pointer_add(data, i);
			ASSERT(*value == buffer[i]);
		}
	}

	void test_string_stream()
	{
		memory_globals::init();
		{
			using namespace string_stream;

			TempAllocator1024 ta;
			Buffer ss(ta);

			ss << "Name"; 			tab(ss, 20);	ss << "Score\n";
			repeat(ss, 10, '-');	tab(ss, 20);	repeat(ss, 10, '-'); ss << "\n";
			ss << "Niklas"; 		tab(ss, 20);	printf(ss, "%.2f", 2.7182818284f); ss << "\n";
			ss << "Jim"; 			tab(ss, 20);	printf(ss, "%.2f", 3.14159265f); ss << "\n";

			ASSERT(
				0 == strcmp(c_str(ss),
					"Name                Score\n"
					"----------          ----------\n"
					"Niklas              2.72\n"
					"Jim                 3.14\n"
				)
			);
		}
		memory_globals::shutdown();
	}

	void test_queue()
	{
		memory_globals::init();
		{
			TempAllocator1024 ta;
			Queue<int> q(ta);

			queue::reserve(q, 10);
			ASSERT(queue::space(q) == 10);
			queue::push_back(q, 11);
			queue::push_front(q, 22);
			ASSERT(queue::size(q) == 2);
			ASSERT(q[0] == 22);
			ASSERT(q[1] == 11);
			queue::consume(q, 2);
			ASSERT(queue::size(q) == 0);
			int items[] = {1,2,3,4,5,6,7,8,9,10};
			queue::push(q,items,10);
			ASSERT(queue::size(q) == 10);
			for (int i=0; i<10; ++i)
				ASSERT(q[i] == i+1);
			queue::consume(q, size_cast(queue::end_front(q) - queue::begin_front(q)));
			queue::consume(q, size_cast(queue::end_front(q) - queue::begin_front(q)));
			ASSERT(queue::size(q) == 0);
		}
        memory_globals::shutdown();
    }

    struct IntWrapper
    {
        IntWrapper( int value, bool &alive )
            : value_( value )
            , alive_( alive )
        {
            alive_ = true;
        }

        ~IntWrapper()
        {
            alive_ = false;
            value_ = -1;
        }

        operator int() const
        {
            return value_;
        }

        int get() const
        {
            return value_;
        }


        int value_;
        bool &alive_;
    };

    static void test_unique_ptr()
    {
        memory_globals::init();
        {
            UniquePtr<int> a;
            ASSERT( !a );
            ASSERT( static_cast<bool>(a) == false );
            ASSERT( a.get() == nullptr );
            ASSERT( a.release() == nullptr );
            ASSERT( a.release() == nullptr );
            a.reset();
            ASSERT( a.get() == nullptr );

            TempAllocator1024 ta;
            bool bAlive = false;
            IntWrapper *pb = MAKE_NEW( ta, IntWrapper, 1234, bAlive );
            UniquePtr<IntWrapper> b( pb, ta );
            ASSERT( bAlive );
            ASSERT( b );
            ASSERT( !!b );
            ASSERT( b.get() == pb );
            ASSERT( b->get() == 1234 );

            IntWrapper *pb2 = b.release();
            ASSERT( bAlive );
            ASSERT( pb == pb2 );
            // move assignment
            b = std::move( UniquePtr<IntWrapper>( pb2, ta ) );
            ASSERT( b );
            ASSERT( b.get() == pb );
            ASSERT( bAlive );

            // reset with != null
            b.reset();
            ASSERT( b.get() == nullptr );
            ASSERT( !bAlive );

            // move constructor
            bool cAlive = false;
            UniquePtr<IntWrapper> c( MAKE_NEW( ta, IntWrapper, 5678, cAlive ), ta );
            ASSERT( cAlive );
            ASSERT( c->get() == 5678 );
            UniquePtr<IntWrapper> d{ std::move(c) };
            ASSERT( cAlive );
            ASSERT( d->get() == 5678 );
            ASSERT( !c );
            ASSERT( c.get() == nullptr );

            // check that UniquePtr call destructor
            bool eAlive = false;
            {
                UniquePtr<IntWrapper> e( MAKE_NEW( ta, IntWrapper, 5678, eAlive ), ta );
                ASSERT( eAlive );
            }
            ASSERT( !eAlive );
        }
        memory_globals::shutdown();
    }
}


int main(int, char **)
{
	test_memory();
	test_array();
	test_scratch();
	test_temp_allocator();
	test_hash();
	test_multi_hash();
	test_murmur_hash();
	test_pointer_arithmetic();
	test_string_stream();
	test_queue();
    test_unique_ptr();
	return 0;
}