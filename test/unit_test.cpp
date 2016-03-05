#include <foundation/queue.h>
#include <foundation/string_stream.h>
#include <foundation/murmur_hash.h>
#include <foundation/hash.h>
#include <foundation/temp_allocator.h>
#include <foundation/array.h>
#include <foundation/memory.h>
#include <foundation/hstring.h>
#include <foundation/math.h>

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
				array::push_back(v, i*3);
            ASSERT( array::size( v ) == 100 );
            for (int i = 0; i<100; ++i)
                ASSERT(v[i] == i*3);
            // for each support
            int index = 0;
            for ( auto value: v )
            {
                ASSERT( value == index*3 );
                ++index;
            }
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
            // swap
            Array<int> v1( scratch );
            Array<int> v2( scratch );
            array::push_back( v1, 1234 );
            array::push_back( v2, 4567 );
            array::swap(v1, v2);
            ASSERT( v1[ 0 ] == 4567 );
            ASSERT( v2[ 0 ] == 1234 );

            // move ctor
            Array<int> v3( std::move( v1 ) );
            ASSERT( begin(v1) == nullptr );
            ASSERT( size(v3) == 1 );
            ASSERT( v3[0] == 4567 );

            // move assign
            Array<int> v4( scratch );
            array::push_back( v4, 123 );
            array::push_back( v4, 456 );
            v4 = std::move( v2 );
            ASSERT( begin( v2 ) == nullptr );
            ASSERT( size( v4 ) == 1 );
            ASSERT( v4[ 0 ] == 1234 );

            std::swap(v1, v2);
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
            ASSERT( hash::get( h, 0, 99 ) == 99 );
            int v99 = 99;
            ASSERT( *hash::try_get( h, 0, &v99 ) == 99 );
            ASSERT( hash::try_get( h, 0 ) == nullptr );
            ASSERT(!hash::has(h, 0));
			hash::remove(h, 0);
			hash::set(h, 1000, 123);
			ASSERT(hash::get(h,1000,0) == 123);
			ASSERT(hash::get(h,2000,99) == 99);
            ASSERT( *hash::try_get( h, 1000, &v99 ) == 123 );
            ASSERT( *hash::try_get( h, 1000 ) == 123 );

			for (int i=0; i<100; ++i)
				hash::set(h, i, i*i);
			for (int i=0; i<100; ++i)
            {
                ASSERT( hash::get( h, i, 0 ) == i*i );
                ASSERT( *hash::try_get( h, i ) == i*i );
            }
            // remove
            hash::remove(h, 1000);
			ASSERT(!hash::has(h, 1000));
			hash::remove(h, 2000);
			ASSERT(hash::get(h,1000,0) == 0);
			for (int i=0; i<100; ++i)
				ASSERT(hash::get(h,i,0) == i*i);
            // clear
            hash::clear(h);
			for (int i=0; i<100; ++i)
				ASSERT(!hash::has(h,i));
		}
        // for each support
        {
            TempAllocator128 ta;
            Hash<int> h( ta );
            uint64_t expectedSum = 0;
            for (int i = 0; i<100; ++i)
            {
                hash::set( h, i, i*i );
                expectedSum += i + i*i;
            }
            uint64_t actualSum = 0;
            for (const Hash<int>::Entry &entry : h)
                actualSum += entry.key + entry.value;
            ASSERT( expectedSum == actualSum );
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

        {
            int64_t int64s[2] = { 0x1234567887654321ll, 0x7877665544332211 };
            uint64_t h = murmur3_hash_64( int64s, sizeof(int64s), 0 );
            ASSERT( h == 0x28b8dc7a31a58300ull );
            uint64_t h2 = murmur3_hash_64_mix( int64s[0], int64s[1], 0 );
            ASSERT( h2 == 0x28b8dc7a31a58300ull );
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

    static void test_hstring32()
    {
        HString32 h1( "mesh" );
        ASSERT( h1.hash() == 0x6e4fa4ca );
        HString32 h1b( 0x6e4fa4ca );
        HString32 h2( 0xbe4fa4ca );
        ASSERT( h1 == h1b );
        ASSERT( h1 != h2 );
        ASSERT( h1 < h2 );
    }

    static void test_hstring64()
    {
        HString64 h1( "cube_one_color" );
        ASSERT( h1.hash() == 0x2968710fbc4b49f9ull );
        HString64 h1b( 0x2968710fbc4b49f9ull );
        HString64 h2( 0x8968710fbc4b49f9ull );
        ASSERT( h1 == h1b );
        ASSERT( h1 != h2 );
        ASSERT( h1 < h2 );
    }

    static bool isVector3Eq( Vector3 actual, Vector3 expected )
    {
        return actual == expected;
    }

    static bool isCloseTo( float value, float expected, float tolerance=1e-6 )
    {
        return fabs(expected-value) <= tolerance;
    }

    static bool isVector3NearlyEq( Vector3 actual, Vector3 expected, float tolerance=1e-6 )
    {
        return isCloseTo(actual.x, expected.x, tolerance ) 
            && isCloseTo(actual.y, expected.y, tolerance ) 
            && isCloseTo(actual.z, expected.z, tolerance );
    }

    static void test_vector3()
    {
#define ASSERT_XYZ_EQ( v, ex, ey, ez )  \
        ASSERT( isVector3Eq( v, Vector3{ex, ey, ez} ) )
#define ASSERT_XYZ_NEARLY_EQ(v, ex, ey, ez) \
        ASSERT( isVector3NearlyEq( v, Vector3{ex, ey, ez} ) )
        const Vector3 v123 = {1.5f, 2.5f, 3.5f};
        ASSERT_XYZ_EQ( v123, 1.5f, 2.5f, 3.5f );
        ASSERT( v123[ 0 ] == 1.5f );
        ASSERT( v123[ 1 ] == 2.5f );
        ASSERT( v123[ 2 ] == 3.5f );
        const Vector3 v100 = { 1.0f, 0.0f, 0.0f };
        const Vector3 v101 = { 1.0f, 0.0f, 1.0f };
        const Vector3 v111 = { 1.0f, 1.0f, 1.0f };
        const Vector3 v010 = { 0.0f, 1.0f, 0.0f };
        const Vector3 v001 = { 0.0f, 0.0f, 1.0f };
        const Vector3 v423 = { 4.0f, 2.0f, 3.0f };
        // zero()
        ASSERT_XYZ_EQ( Vector3::zero(), 0.0f, 0.0f, 0.0f );
        // vector3()
        ASSERT_XYZ_EQ( vector3( -1.0f, -2.0f, 3.0f ), -1.0f, -2.0f, 3.0f );
        // axisX()
        ASSERT_XYZ_EQ( Vector3::axisX(), 1.0f, 0.0f, 0.0f );
        // axisY()
        ASSERT_XYZ_EQ( Vector3::axisY(), 0.0f, 1.0f, 0.0f );
        // axisZ()
        ASSERT_XYZ_EQ( Vector3::axisZ(), 0.0f, 0.0f, 1.0f );
        // uniform()
        ASSERT_XYZ_EQ( Vector3::uniform(3.0), 3.0f, 3.0f, 3.0f );
        // operator ==
        ASSERT( v123 == v123 );
        ASSERT( v100 == v100 );
        ASSERT( v010 == v010 );
        ASSERT( v001 == v001 );
        // operator !=
        ASSERT( v123 != v100 );
        ASSERT( v123 != v010 );
        ASSERT( v123 != v001 );
        ASSERT( v100 != v010 );
        ASSERT( v100 != v111 );
        // operator +=
        Vector3 r1 = v123;
        r1 += v423;
        ASSERT_XYZ_EQ( r1, 5.5f, 4.5f, 6.5f );
        // operator -=
        Vector3 r2 = v123;
        r2 -= v423;
        ASSERT_XYZ_EQ( r2, -2.5f, 0.5f, 0.5f );
        // operator *=
        Vector3 r3 = v123;
        r3 *= 4.0f;
        ASSERT_XYZ_EQ( r3, 6.0f, 10.0f, 14.0f );
        // operator +
        ASSERT_XYZ_EQ( v123 + v423, 5.5f, 4.5f, 6.5f );
        ASSERT_XYZ_EQ( v123 + v100, 2.5f, 2.5f, 3.5f );
        ASSERT_XYZ_EQ( v123 + v010, 1.5f, 3.5f, 3.5f );
        ASSERT_XYZ_EQ( v123 + v001, 1.5f, 2.5f, 4.5f );
        // operator -
        ASSERT_XYZ_EQ( v123 - v423, -2.5f, 0.5f, 0.5f );
        ASSERT_XYZ_EQ( v123 - v100, 0.5f, 2.5f, 3.5f );
        ASSERT_XYZ_EQ( v123 - v010, 1.5f, 1.5f, 3.5f );
        ASSERT_XYZ_EQ( v123 - v001, 1.5f, 2.5f, 2.5f );
        // unary negation
        ASSERT_XYZ_EQ( -v123, -1.5f, -2.5f, -3.5f );
        // dot()
        ASSERT( v100.dot( v010 ) == 0.0f );
        ASSERT( v010.dot( v001 ) == 0.0f );
        ASSERT( v100.dot( v101 ) == 1.0f );
        // length()
        ASSERT( Vector3::zero().length() == 0.0f );
        ASSERT( v100.length() == 1.0f );
        ASSERT( v100.length() == 1.0f );
        ASSERT( v010.length() == 1.0f );
        Vector3 v122 = {1.0f, 2.0f, 2.0f};
        ASSERT( v122.length() == 3.0f );
        Vector3 v184 = { 1.0f, 8.0f, 4.0f };
        ASSERT( v184.length() == 9.0f );
        ASSERT( (-v184).length() == 9.0f );
        ASSERT( isCloseTo( v123.length(), 4.55521678957215f ) );
        // lengthSquared()
        ASSERT( Vector3::zero().lengthSquared() == 0.0f );
        ASSERT( v100.lengthSquared() == 1.0f );
        ASSERT( v122.lengthSquared() == 9.0f );
        ASSERT( v184.lengthSquared() == 81.0f );
        ASSERT( ( -v184 ).lengthSquared() == 81.0f );
        ASSERT( v123.lengthSquared() == 20.75f );
        // normalize()
        Vector3 n1 = v100;
        n1.normalize();
        ASSERT_XYZ_EQ( n1, 1.0f, 0.0f, 0.0f );
        Vector3 n2 = v122;
        n2.normalize();
        ASSERT_XYZ_EQ( n2, 1.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f );
        Vector3 n3 = v123;
        n3.normalize();
        ASSERT_XYZ_NEARLY_EQ( n3, 1.5f / 4.55521678957215f, 2.5f / 4.55521678957215f, 3.5f / 4.55521678957215f );
        // normalized()
        ASSERT_XYZ_EQ( v100.normalized(), 1.0f, 0.0f, 0.0f );
        ASSERT_XYZ_EQ( v122.normalized(), 1.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f );
        ASSERT_XYZ_NEARLY_EQ( v123.normalized(), 1.5f / 4.55521678957215f, 2.5f / 4.55521678957215f, 3.5f / 4.55521678957215f );
        // cross()
        ASSERT_XYZ_EQ( v100.crossProduct( v010 ), 0.0f, 0.0f, 1.0f );
        ASSERT_XYZ_EQ( v100.crossProduct( v001 ), 0.0f, -1.0f, 0.0f );
        ASSERT_XYZ_EQ( v123.crossProduct( v423 ), 0.5f, 9.5f, -7.0f );
        ASSERT_XYZ_EQ( v101.crossProduct( v111 ), -1.0f, 0.0f, 1.0f );
        static_assert( sizeof( Vector3 ) == 3 * sizeof( float ), "bad memory layout for Vector3" );
    }

    static bool isMatrix3x3Eq( const Matrix3x3 &m, const Vector3 &col0, const Vector3 &col1, const Vector3 &col2 )
    {
        return m[ 0 ] == col0  &&  m[ 1 ] == col1  &&  m[ 2 ] == col2;
    }

    static bool isMatrix3x3NearlyEq( const Matrix3x3 &m, const Vector3 &col0, const Vector3 &col1, const Vector3 &col2, float tolerance=1e-3 )
    {
        return isVector3NearlyEq( m[ 0 ], col0, tolerance )
            && isVector3NearlyEq( m[ 1 ], col1, tolerance )
            && isVector3NearlyEq( m[ 2 ], col2, tolerance );
    }

    static void test_matrix3x3()
    {
#define ASSERT_M3x3_EQ(m, col0, col1, col2) \
    ASSERT( isMatrix3x3Eq( m, col0, col1, col2 ) )
#define ASSERT_M3x3_NEARLY_EQ(m, col0, col1, col2) \
    ASSERT( isMatrix3x3NearlyEq( m, col0, col1, col2 ) )

        ASSERT_M3x3_EQ( Matrix3x3::zero(),
                        vector3( 0.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 0.0f ) );

        ASSERT_M3x3_EQ( Matrix3x3::identity(),
                        vector3( 1.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 1.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 1.0f ) );
        // matrix3x3()
        const Matrix3x3 m1to9 = matrix3x3( vector3( 1.0f, 2.0f, 3.0f ),
                                           vector3( 4.0f, 5.0f, 6.0f ),
                                           vector3( 7.0f, 8.0f, 9.0f ) );
        ASSERT_M3x3_EQ( m1to9,
                        vector3( 1.0f, 2.0f, 3.0f ), 
                        vector3( 4.0f, 5.0f, 6.0f ), 
                        vector3( 7.0f, 8.0f, 9.0f ) );

        // non-const operator []
        Matrix3x3 m1 = Matrix3x3::identity();
        m1[ 0 ][ 2 ] = 3.0f;
        m1[ 1 ][ 0 ] = 4.0f;
        m1[ 2 ][ 1 ] = 5.0f;
        ASSERT_M3x3_EQ( m1,
                        vector3( 1.0f, 0.0f, 3.0f ),
                        vector3( 4.0f, 1.0f, 0.0f ),
                        vector3( 0.0f, 5.0f, 1.0f ) );
        // const operator[]
        const Matrix3x3 &cm1 = m1;
        ASSERT( cm1[ 2 ].y == 5.0f );
        // row()
        ASSERT_XYZ_EQ( cm1.row( 0 ), 1.0f, 4.0f, 0.0f );
        ASSERT_XYZ_EQ( cm1.row( 1 ), 0.0f, 1.0f, 5.0f );
        ASSERT_XYZ_EQ( cm1.row( 2 ), 3.0f, 0.0f, 1.0f );
        // operator +=
        Matrix3x3 m2 = m1to9;
        const Matrix3x3 m10to90 = matrix3x3( vector3( 10.0f, 20.0f, 30.0f ),
                                             vector3( 40.0f, 50.0f, 60.0f ),
                                             vector3( 70.0f, 80.0f, 90.0f ) );
        m2 += m10to90;
        ASSERT_M3x3_EQ( m2, 
                        vector3( 11.0f, 22.0f, 33.0f ),
                        vector3( 44.0f, 55.0f, 66.0f ),
                        vector3( 77.0f, 88.0f, 99.0f ) );
        // operator *= (matrix)
        Matrix3x3 m3 = Matrix3x3::identity();
        m3 *= Matrix3x3::identity();
        ASSERT_M3x3_EQ( m3,
                        vector3( 1.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 1.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 1.0f ) );
        Matrix3x3 m4 = Matrix3x3::identity();
        m4 *= Matrix3x3::zero();
        ASSERT_M3x3_EQ( m4,
                        vector3( 0.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 0.0f ) );
        Matrix3x3 m5 = m1to9;
        m5 *= m10to90;
        ASSERT( m5[ 0 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 0 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 0 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 2 ] ) );
        // transposed()
        ASSERT_M3x3_EQ( m1to9.transposed(),
                        vector3( 1.0f, 4.0f, 7.0f ),
                        vector3( 2.0f, 5.0f, 8.0f ),
                        vector3( 3.0f, 6.0f, 9.0f ) );
        // inverse()
        ASSERT_M3x3_EQ( Matrix3x3::identity().inversed(),
                        vector3( 1.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 1.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 1.0f ) );
        // Notes: m1to9 is a singlar matrix
        const Matrix3x3 m7 = matrix3x3( vector3( 0.6f, 0.2f, 0.3f ),
                                        vector3( 0.2f, 0.7f, 0.5f ),
                                        vector3( 0.3f, 0.5f, 0.7f ) );
        const Matrix3x3 m7_inv = matrix3x3( vector3( 2.1238938053097347f, 0.08849557522123908f, -0.9734513274336286f ),
                                            vector3( 0.08849557522123908f, 2.9203539823008855f, -2.123893805309736f ),
                                            vector3( -0.9734513274336286f, -2.1238938053097356f, 3.3628318584070813f ) );
        const Matrix3x3 mii7 = m7 * m7_inv;
        ASSERT_M3x3_NEARLY_EQ( mii7,
                        vector3( 1.0f, 0.0f, 0.0f ),
                        vector3( 0.0f, 1.0f, 0.0f ),
                        vector3( 0.0f, 0.0f, 1.0f ) );
        ASSERT_M3x3_NEARLY_EQ( m7.inversed(), m7_inv[0], m7_inv[ 1 ], m7_inv[ 2 ] );
        // operator +
        ASSERT_M3x3_EQ( m1to9 + m10to90,
                        vector3( 11.0f, 22.0f, 33.0f ),
                        vector3( 44.0f, 55.0f, 66.0f ),
                        vector3( 77.0f, 88.0f, 99.0f ) );
        // operator * (matrix, matrix)
        Matrix3x3 m6 = m1to9 * m10to90;
        ASSERT( m6[ 0 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 0 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 0 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 2 ] ) );
        // operator * (vector, matrix)
        Vector3 v123 = { 1.5f,2.5f,3.5f };
        ASSERT_XYZ_EQ( v123 * Matrix3x3::identity(), 1.5f, 2.5f, 3.5f );
        ASSERT_XYZ_NEARLY_EQ( v123 * m1to9, v123.dot( m1to9[0] ), v123.dot( m1to9[ 1 ] ), v123.dot( m1to9[ 2 ] ) );
        // rotated()
        const float pi = 3.14159265358979323846f;
        Matrix3x3 rotateX180 = Matrix3x3::identity().rotated( pi, vector3(10.0f, 0.0f, 0.0f) );
        ASSERT_XYZ_NEARLY_EQ( vector3( 5.0f, 0.0f, 0.0f ) * rotateX180, 5.0f, 0.0f, 0.0f );
        ASSERT_XYZ_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * rotateX180, 5.0f, -2.0f, 3.0f );
        Matrix3x3 rotateY45 = Matrix3x3::identity().rotated( pi / 2.0f, vector3( 0.0f, 10.0f, 0.0f ) );
        ASSERT_XYZ_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * rotateY45, 3.0f, 2.0f, 5.0f );
        Matrix3x3 rotateZm45 = Matrix3x3::identity().rotated( pi/2.0f, vector3( 0.0f, 0.0f, 4.0f ) );
        ASSERT_XYZ_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * rotateZm45, 2.0f, -5.0f, -3.0f );
        Matrix3x3 rotatePlan = Matrix3x3::identity().rotated( pi, vector3( 0.0f, 1.0f, -1.0f ) ); // plan defined by (0,0)(1,0),(0,1)
        ASSERT_XYZ_NEARLY_EQ( vector3( 0.0f, 1.0f, 1.0f ) * rotatePlan, 0.0f, -1.0f, -1.0f );
        ASSERT_XYZ_NEARLY_EQ( vector3( 1.0f, 0.0f, 0.0f ) * rotatePlan, -1.0f, 0.0f, 0.0f );
        ASSERT_XYZ_NEARLY_EQ( vector3( 1.0f, 1.0f, 1.0f ) * rotatePlan, -1.0f, -1.0f, -1.0f );

        // scaled()
        ASSERT_M3x3_EQ( m1to9.scaled( vector3(2.0f, -1.5f, 3.0f) ),
                        vector3( 1.0f*2.0f, 2.0f*2.0f, 3.0f*2.0f ),
                        vector3( 4.0f*-1.5f, 5.0f*-1.5f, 6.0f*-1.5f ),
                        vector3( 7.0f*3.0f, 8.0f*3.0f, 9.0f*3.0f ) );
        static_assert( sizeof( Matrix3x3 ) == 3 * 3 * sizeof( float ), "bad memory layout for Matrix3x3" );
    }


    static bool isVector4Eq( const Vector4 &actual, const Vector4 &expected )
    {
        return actual == expected;
    }

    static bool isVector4NearlyEq( const Vector4 &actual, const Vector4 &expected, float tolerance = 1e-6 )
    {
        return isCloseTo( actual.x, expected.x, tolerance )
            && isCloseTo( actual.y, expected.y, tolerance )
            && isCloseTo( actual.z, expected.z, tolerance )
            && isCloseTo( actual.w, expected.w, tolerance );
    }

    static void test_vector4()
    {
#define ASSERT_XYZW_EQ( v, ex, ey, ez, ew )  \
        ASSERT( isVector4Eq( v, Vector4{ex, ey, ez, ew} ) )
#define ASSERT_XYZW_NEARLY_EQ(v, ex, ey, ez, ew) \
        ASSERT( isVector4NearlyEq( v, Vector4{ex, ey, ez, ew} ) )
        const Vector4 v1234 = { 1.5f, 2.5f, 3.5f, 4.5f };
        ASSERT_XYZW_EQ( v1234, 1.5f, 2.5f, 3.5f, 4.5f );
        ASSERT( v1234[ 0 ] == 1.5f );
        ASSERT( v1234[ 1 ] == 2.5f );
        ASSERT( v1234[ 2 ] == 3.5f );
        ASSERT( v1234[ 3 ] == 4.5f );
        const Vector4 v1000 = { 1.0f, 0.0f, 0.0f, 0.0f };
        const Vector4 v1010 = { 1.0f, 0.0f, 1.0f, 0.0f };
        const Vector4 v1111 = { 1.0f, 1.0f, 1.0f, 1.0f };
        const Vector4 v0100 = { 0.0f, 1.0f, 0.0f, 0.0f };
        const Vector4 v0010 = { 0.0f, 0.0f, 1.0f, 0.0f };
        const Vector4 v0001 = { 0.0f, 0.0f, 0.0f, 1.0f };
        const Vector4 v4231 = { 4.0f, 2.0f, 3.0f, 1.0f };
        // zero()
        ASSERT_XYZW_EQ( Vector4::zero(), 0.0f, 0.0f, 0.0f, 0.0f );
        // vector4()
        ASSERT_XYZW_EQ( vector4( -1.0f, -2.0f, 3.0f, 7.0f ), -1.0f, -2.0f, 3.0f, 7.0f );
        // uniform()
        ASSERT_XYZW_EQ( Vector4::uniform( 3.0 ), 3.0f, 3.0f, 3.0f, 3.0f );
        // operator ==
        ASSERT( v1234 == v1234 );
        ASSERT( v1000 == v1000 );
        ASSERT( v0100 == v0100 );
        ASSERT( v0010 == v0010 );
        ASSERT( v0001 == v0001 );
        // operator !=
        ASSERT( v1234 != v1000 );
        ASSERT( v1234 != v0100 );
        ASSERT( v1234 != v0010 );
        ASSERT( v1234 != v0001 );
        ASSERT( v1000 != v0100 );
        ASSERT( v1000 != v1111 );
        // operator +=
        Vector4 r1 = v1234;
        r1 += v4231;
        ASSERT_XYZW_EQ( r1, 5.5f, 4.5f, 6.5f, 5.5f );
        // operator -=
        Vector4 r2 = v1234;
        r2 -= v4231;
        ASSERT_XYZW_EQ( r2, -2.5f, 0.5f, 0.5f, 3.5f );
        // operator *= (float)
        Vector4 r3 = v1234;
        r3 *= 4.0f;
        ASSERT_XYZW_EQ( r3, 6.0f, 10.0f, 14.0f, 18.0f );
        // operator *= (vector)
        Vector4 r4 = v1234;
        r4 *= v4231;
        ASSERT_XYZW_EQ( r4, 1.5f*4.0f, 2.5f*2.0f, 3.5f*3.0f, 4.5f*1.0f );
        // operator * (vector)
        ASSERT_XYZW_EQ( v1234 * v4231, 1.5f*4.0f, 2.5f*2.0f, 3.5f*3.0f, 4.5f*1.0f );
        // operator +
        ASSERT_XYZW_EQ( v1234 + v4231, 5.5f, 4.5f, 6.5f, 5.5f );
        ASSERT_XYZW_EQ( v1234 + v1000, 2.5f, 2.5f, 3.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 + v0100, 1.5f, 3.5f, 3.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 + v0010, 1.5f, 2.5f, 4.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 + v0001, 1.5f, 2.5f, 3.5f, 5.5f );
        // operator -
        ASSERT_XYZW_EQ( v1234 - v4231, -2.5f, 0.5f, 0.5f, 3.5f );
        ASSERT_XYZW_EQ( v1234 - v1000, 0.5f, 2.5f, 3.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 - v0100, 1.5f, 1.5f, 3.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 - v0010, 1.5f, 2.5f, 2.5f, 4.5f );
        ASSERT_XYZW_EQ( v1234 - v0001, 1.5f, 2.5f, 3.5f, 3.5f );
        // unary negation
        ASSERT_XYZW_EQ( -v1234, -1.5f, -2.5f, -3.5f, -4.5f );
        // dot()
        ASSERT( v1000.dot( v0100 ) == 0.0f );
        ASSERT( v0100.dot( v0010 ) == 0.0f );
        ASSERT( v1000.dot( v1010 ) == 1.0f );
        ASSERT( v1010.dot( v1111 ) == 2.0f );
        // length()
        ASSERT( Vector4::zero().length() == 0.0f );
        ASSERT( v1000.length() == 1.0f );
        ASSERT( v1000.length() == 1.0f );
        ASSERT( v0100.length() == 1.0f );
        ASSERT( v1111.length() == 2.0f );
        Vector4 v124a = { 1.0f, 2.0f, 4.0f, 10.0f };
        ASSERT( v124a.length() == 11.0f );
        ASSERT( ( -v124a ).length() == 11.0f );
        ASSERT( isCloseTo( v1234.length(), 6.4031242374328485f ) );
        // lengthSquared()
        ASSERT( Vector4::zero().lengthSquared() == 0.0f );
        ASSERT( v1000.lengthSquared() == 1.0f );
        ASSERT( v1111.lengthSquared() == 4.0f );
        ASSERT( v124a.lengthSquared() == 121.0f );
        ASSERT( ( -v124a ).lengthSquared() == 121.0f );
        ASSERT( v1234.lengthSquared() == 41.0f );
        // normalize()
        Vector4 n1 = v1000;
        n1.normalize();
        ASSERT_XYZW_EQ( n1, 1.0f, 0.0f, 0.0f, 0.0f );
        Vector4 n2 = v1111;
        n2.normalize();
        ASSERT_XYZW_EQ( n2, 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f );
        Vector4 n3 = v1234;
        n3.normalize();
        ASSERT_XYZW_NEARLY_EQ( n3, 1.5f / 6.4031242374328485f, 2.5f / 6.4031242374328485f, 3.5f / 6.4031242374328485f, 4.5f / 6.4031242374328485f );
        // normalized()
        ASSERT_XYZW_EQ( v1000.normalized(), 1.0f, 0.0f, 0.0f, 0.0f );
        ASSERT_XYZW_EQ( v1111.normalized(), 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f );
        ASSERT_XYZW_NEARLY_EQ( v1234.normalized(), 1.5f / 6.4031242374328485f, 2.5f / 6.4031242374328485f, 3.5f / 6.4031242374328485f, 4.5f / 6.4031242374328485f );
        static_assert( sizeof( Vector4 ) == 4 * sizeof( float ), "bad memory layout for Vector4" );
    }

    static bool isMatrix4x4Eq( const Matrix4x4 &m, const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3 )
    {
        return m[ 0 ] == col0  &&  m[ 1 ] == col1  &&  m[ 2 ] == col2  &&  m[ 3 ] == col3;
    }

    static bool isMatrix4x4NearlyEq( const Matrix4x4 &m, const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3, float tolerance = 1e-3 )
    {
        return isVector4NearlyEq( m[ 0 ], col0, tolerance )
            && isVector4NearlyEq( m[ 1 ], col1, tolerance )
            && isVector4NearlyEq( m[ 2 ], col2, tolerance )
            && isVector4NearlyEq( m[ 3 ], col3, tolerance );
    }

    static void test_matrix4x4()
    {
#define ASSERT_M4x4_EQ(m, col0, col1, col2, col3) \
    ASSERT( isMatrix4x4Eq( m, col0, col1, col2, col3 ) )
#define ASSERT_M4x4_NEARLY_EQ(m, col0, col1, col2, col3) \
    ASSERT( isMatrix4x4NearlyEq( m, col0, col1, col2, col3 ) )

        ASSERT_M4x4_EQ( Matrix4x4::zero(),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ) );

        ASSERT_M4x4_EQ( Matrix4x4::identity(),
                        vector4( 1.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 1.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        // matrix4x4()
        const Matrix4x4 m1to9 = matrix4x4( vector4( 1.0f, 2.0f, 3.0f, 4.0f ),
                                           vector4( 5.0f, 6.0f, 7.0f, 8.0f ),
                                           vector4( 9.0f, 10.0f, 11.0f, 12.0f ),
                                           vector4( 13.0f, 14.0f, 15.0f, 16.0f ) );
        ASSERT_M4x4_EQ( m1to9,
                        vector4( 1.0f, 2.0f, 3.0f, 4.0f ),
                        vector4( 5.0f, 6.0f, 7.0f, 8.0f ),
                        vector4( 9.0f, 10.0f, 11.0f, 12.0f ),
                        vector4( 13.0f, 14.0f, 15.0f, 16.0f ) );

        // non-const operator []
        Matrix4x4 m1 = Matrix4x4::identity();
        m1[ 0 ][ 2 ] = 3.0f;
        m1[ 1 ][ 0 ] = 4.0f;
        m1[ 2 ][ 1 ] = 5.0f;
        m1[ 3 ][ 2 ] = 6.0f;
        ASSERT_M4x4_EQ( m1,
                        vector4( 1.0f, 0.0f, 3.0f, 0.0f ),
                        vector4( 4.0f, 1.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 5.0f, 1.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 6.0f, 1.0f ) );
        // const operator[]
        const Matrix4x4 &cm1 = m1;
        ASSERT( cm1[ 3 ].z == 6.0f );
        // row()
        ASSERT_XYZW_EQ( cm1.row( 0 ), 1.0f, 4.0f, 0.0f, 0.0f );
        ASSERT_XYZW_EQ( cm1.row( 1 ), 0.0f, 1.0f, 5.0f, 0.0f );
        ASSERT_XYZW_EQ( cm1.row( 2 ), 3.0f, 0.0f, 1.0f, 6.0f );
        ASSERT_XYZW_EQ( cm1.row( 3 ), 0.0f, 0.0f, 0.0f, 1.0f );
        // operator +=
        Matrix4x4 m2 = m1to9;
        const Matrix4x4 m10to90 = matrix4x4( vector4( 100.0f, 200.0f, 300.0f, 400.0f ),
                                             vector4( 500.0f, 600.0f, 700.0f, 800.0f ),
                                             vector4( 900.0f, 1000.0f, 1100.0f, 1200.0f ),
                                             vector4( 1300.0f, 1400.0f, 1500.0f, 1600.0f ) );
        m2 += m10to90;
        ASSERT_M4x4_EQ( m2,
                        vector4( 101.0f, 202.0f, 303.0f, 404.0f ),
                        vector4( 505.0f, 606.0f, 707.0f, 808.0f ),
                        vector4( 909.0f, 1010.0f, 1111.0f, 1212.0f ),
                        vector4( 1313.0f, 1414.0f, 1515.0f, 1616.0f ) );
        // operator *= (matrix)
        Matrix4x4 m3 = Matrix4x4::identity();
        m3 *= Matrix4x4::identity();
        ASSERT_M4x4_EQ( m3,
                        vector4( 1.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 1.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        Matrix4x4 m4 = Matrix4x4::identity();
        m4 *= Matrix4x4::zero();
        ASSERT_M4x4_EQ( m4,
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
        Matrix4x4 m5 = m1to9;
        m5 *= m10to90;
        ASSERT( m5[ 0 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 3 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 3 ] ) );
        ASSERT( m5[ 0 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 3 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 3 ] ) );
        ASSERT( m5[ 0 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 3 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 3 ] ) );
        ASSERT( m5[ 0 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 0 ] ) );
        ASSERT( m5[ 1 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 1 ] ) );
        ASSERT( m5[ 2 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 2 ] ) );
        ASSERT( m5[ 3 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 3 ] ) );
        // transposed()
        ASSERT_M4x4_EQ( m1to9.transposed(),
                        vector4( 1.0f, 5.0f, 9.0f, 13.0f ),
                        vector4( 2.0f, 6.0f, 10.0f, 14.0f ),
                        vector4( 3.0f, 7.0f, 11.0f, 15.0f ),
                        vector4( 4.0f, 8.0f, 12.0f, 16.0f ) );
        // operator +
        ASSERT_M4x4_EQ( m1to9 + m10to90,
                        vector4( 101.0f, 202.0f, 303.0f, 404.0f ),
                        vector4( 505.0f, 606.0f, 707.0f, 808.0f ),
                        vector4( 909.0f, 1010.0f, 1111.0f, 1212.0f ),
                        vector4( 1313.0f, 1414.0f, 1515.0f, 1616.0f ) );
        // operator -
        ASSERT_M4x4_EQ( m10to90 - m1to9,
                        vector4( 99.0f, 198.0f, 297.0f, 396.0f ),
                        vector4( 495.0f, 594.0f, 693.0f, 792.0f ),
                        vector4( 891.0f, 990.0f, 1089.0f, 1188.0f ),
                        vector4( 1287.0f, 1386.0f, 1485.0f, 1584.0f ) );
        // operator * (matrix, matrix)
        Matrix4x4 m6 = m1to9 * m10to90;
        ASSERT( m6[ 0 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 3 ][ 0 ] == m1to9.row( 0 ).dot( m10to90[ 3 ] ) );
        ASSERT( m6[ 0 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 3 ][ 1 ] == m1to9.row( 1 ).dot( m10to90[ 3 ] ) );
        ASSERT( m6[ 0 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 3 ][ 2 ] == m1to9.row( 2 ).dot( m10to90[ 3 ] ) );
        ASSERT( m6[ 0 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 0 ] ) );
        ASSERT( m6[ 1 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 1 ] ) );
        ASSERT( m6[ 2 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 2 ] ) );
        ASSERT( m6[ 3 ][ 3 ] == m1to9.row( 3 ).dot( m10to90[ 3 ] ) );
        // operator * (vector4, matrix)
        Vector4 v1234 = { 1.5f,2.5f,3.5f,4.5f };
        ASSERT_XYZW_EQ( v1234 * Matrix4x4::identity(), 1.5f, 2.5f, 3.5f, 4.5f );
        ASSERT_XYZW_NEARLY_EQ( v1234 * m1to9, v1234.dot( m1to9[ 0 ] ), v1234.dot( m1to9[ 1 ] ), v1234.dot( m1to9[ 2 ] ), v1234.dot( m1to9[ 3 ] ) );
        // operator * (vector3, matrix)
        Vector4 v1230 = { 1.5f,2.5f,3.5f,1.0f };
        Vector3 v123 = { 1.5f,2.5f,3.5f };
        ASSERT_XYZW_EQ( v123 * Matrix4x4::identity(), 1.5f, 2.5f, 3.5f, 1.0f );
        ASSERT_XYZW_NEARLY_EQ( v123 * m1to9, v1230.dot( m1to9[ 0 ] ), v1230.dot( m1to9[ 1 ] ), v1230.dot( m1to9[ 2 ] ), v1230.dot( m1to9[ 3 ] ) );
        // rotated()
        //const float pi = 3.14159265358979323846f;
        //Matrix4x4 rotateX180 = Matrix4x4::identity().rotated( pi, vector4( 10.0f, 0.0f, 0.0f ) );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 5.0f, 0.0f, 0.0f ) * rotateX180, 5.0f, 0.0f, 0.0f );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 5.0f, 2.0f, -3.0f ) * rotateX180, 5.0f, -2.0f, 3.0f );
        //Matrix4x4 rotateY45 = Matrix4x4::identity().rotated( pi / 2.0f, vector4( 0.0f, 10.0f, 0.0f ) );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 5.0f, 2.0f, -3.0f ) * rotateY45, 3.0f, 2.0f, 5.0f );
        //Matrix4x4 rotateZm45 = Matrix4x4::identity().rotated( pi / 2.0f, vector4( 0.0f, 0.0f, 4.0f ) );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 5.0f, 2.0f, -3.0f ) * rotateZm45, 2.0f, -5.0f, -3.0f );
        //Matrix4x4 rotatePlan = Matrix4x4::identity().rotated( pi, vector4( 0.0f, 1.0f, -1.0f ) ); // plan defined by (0,0)(1,0),(0,1)
        //ASSERT_XYZW_NEARLY_EQ( vector4( 0.0f, 1.0f, 1.0f ) * rotatePlan, 0.0f, -1.0f, -1.0f );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 1.0f, 0.0f, 0.0f ) * rotatePlan, -1.0f, 0.0f, 0.0f );
        //ASSERT_XYZW_NEARLY_EQ( vector4( 1.0f, 1.0f, 1.0f ) * rotatePlan, -1.0f, -1.0f, -1.0f );

        // scaled()
        //ASSERT_M4x4_EQ( m1to9.scaled( vector4( 2.0f, -1.5f, 3.0f ) ),
        //                vector4( 1.0f*2.0f, 2.0f*2.0f, 3.0f*2.0f ),
        //                vector4( 4.0f*-1.5f, 5.0f*-1.5f, 6.0f*-1.5f ),
        //                vector4( 7.0f*3.0f, 8.0f*3.0f, 9.0f*3.0f ) );

        // inverse()
        ASSERT_M4x4_EQ( Matrix4x4::identity().inversed(),
                        vector4( 1.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 1.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        // Notes: m1to9 is a singlar matrix
        const Matrix4x4 m7 = matrix4x4( vector4( 0.6f, 0.2f, 0.3f, 0.4f ),
                                        vector4( 0.2f, 0.7f, 0.5f, 0.3f ),
                                        vector4( 0.3f, 0.5f, 0.7f, 0.2f ),
                                        vector4( 0.4f, 0.3f, 0.2f, 0.6f ) );
        const Matrix4x4 m7_inv = matrix4x4( vector4( 3.9649122807017556f, 1.4035087719298254f, -1.9298245614035099f, -2.7017543859649136f ),
                                            vector4( 1.4035087719298251f, 3.8596491228070184f, -2.807017543859651f, -1.9298245614035092f ),
                                            vector4( -1.9298245614035097f, -2.8070175438596507f, 3.8596491228070198f, 1.4035087719298251f ),
                                            vector4( -2.701754385964913f, -1.9298245614035092f, 1.4035087719298256f, 3.964912280701755f ) );
        Matrix4x4 actualM7Inv = m7.inversed();
        ASSERT_M4x4_NEARLY_EQ( actualM7Inv,
                               vector4( 3.9649122807017556f, 1.4035087719298254f, -1.9298245614035099f, -2.7017543859649136f ),
                               vector4( 1.4035087719298251f, 3.8596491228070184f, -2.807017543859651f, -1.9298245614035092f ),
                               vector4( -1.9298245614035097f, -2.8070175438596507f, 3.8596491228070198f, 1.4035087719298251f ),

                               vector4( -2.701754385964913f, -1.9298245614035092f, 1.4035087719298256f, 3.964912280701755f ) );
        //3.9649122807017556	1.4035087719298254 -1.9298245614035099  -2.7017543859649136
        //1.4035087719298251	3.8596491228070184 -2.807017543859651   -1.9298245614035092
        //-1.9298245614035097   -2.8070175438596507	3.8596491228070198	1.4035087719298251
        //-2.701754385964913    -1.9298245614035092	1.4035087719298256	3.964912280701755
        const Matrix4x4 mii7 = m7 * m7_inv;
        ASSERT_M4x4_NEARLY_EQ( mii7,
                               vector4( 1.0f, 0.0f, 0.0f, 0.0f ),
                               vector4( 0.0f, 1.0f, 0.0f, 0.0f ),
                               vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
                               vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        ASSERT_M4x4_NEARLY_EQ( m7.inversed(), m7_inv[ 0 ], m7_inv[ 1 ], m7_inv[ 2 ], m7_inv[ 3 ] );
        // matrix4x4( rotation, scale, translate)
        const float pi = 3.14159265358979323846f;
        Matrix4x4 m8 = matrix4x4( Matrix3x3::identity(), Vector3::uniform(1.0f), Vector3::zero() );
        ASSERT_M4x4_EQ( m8,
                        vector4( 1.0f, 0.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 1.0f, 0.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 1.0f, 0.0f ),
                        vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
        Matrix4x4 mtranslate1 = matrix4x4( Matrix3x3::identity(), Vector3::uniform( 1.0f ), Vector3{ 2.0f, 3.0f, 4.0f } );
        ASSERT_XYZW_EQ( Vector3::zero() * mtranslate1, 2.0f, 3.0f, 4.0f, 1.0f );
        Matrix4x4 mscale1 = matrix4x4( Matrix3x3::identity(), Vector3{ 2.0f, 3.0f, 4.0f }, Vector3::zero() );
        ASSERT_XYZW_EQ( Vector3::zero() * mscale1, 0.0f, 0.0f, 0.0f, 1.0f );
        ASSERT_XYZW_EQ( vector3(-1.0f, 5.0f, 7.0f) * mscale1, -1.0f*2.0f, 5.0f*3.0f, 7.0f*4.0f, 1.0f );
        Matrix3x3 rotateY45 = Matrix3x3::identity().rotated( pi / 2.0f, vector3( 0.0f, 10.0f, 0.0f ) );
        Matrix4x4 mrotate1 = matrix4x4( rotateY45, Vector3::uniform( 1.0f ), Vector3::zero() );
        ASSERT_XYZW_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * mrotate1, 3.0f, 2.0f, 5.0f, 1.0f );
        ASSERT_XYZW_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * ( mrotate1*mtranslate1 ), 3.0f + 2.0f, 2.0f + 3.0f, 5.0f + 4.0f, 1.0f );
        ASSERT_XYZW_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * ( mrotate1*mscale1*mtranslate1 ),
                               3.0f*2.0f + 2.0f, 2.0f*3.0f + 3.0f, 5.0f*4.0f + 4.0f, 1.0f );
        ASSERT_XYZW_NEARLY_EQ( vector3( 5.0f, 2.0f, -3.0f ) * ( mrotate1*mtranslate1*mscale1 ),
                               (3.0f + 2.0f)*2.0f, (2.0f + 3.0f)*3.0f, (5.0f + 4.0f)*4.0f, 1.0f );
        static_assert( sizeof(Matrix4x4) == 4*4*sizeof(float), "bad memory layout for Matrix4x4" );
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
    test_hstring32();
    test_hstring64();
    test_vector3();
    test_matrix3x3();
    test_vector4();
    test_matrix4x4();
    return 0;
}