#pragma once

#include "types.h"
#include "memory_types.h"
#include <new> // placement new

namespace foundation
{
	/// Base class for memory allocators.
	///
	/// Note: Regardless of which allocator is used, prefer to allocate memory in larger chunks
	/// instead of in many small allocations. This helps with data locality, fragmentation,
	/// memory usage tracking, etc.
	class Allocator
	{
	public:
		/// Default alignment for memory allocations.
		static const uint32_t DEFAULT_ALIGN = 4;

		Allocator() {}
		virtual ~Allocator() {}
		
		/// Allocates the specified amount of memory aligned to the specified alignment.
		virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;

		/// Frees an allocation previously made with allocate().
		virtual void deallocate(void *p) = 0;

		static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;

		/// Returns the amount of usable memory allocated at p. p must be a pointer
		/// returned by allocate() that has not yet been deallocated. The value returned
		/// will be at least the size specified to allocate(), but it can be bigger.
		/// (The allocator may round up the allocation to fit into a set of predefined
		/// slot sizes.)
		///
		/// Not all allocators support tracking the size of individual allocations.
		/// An allocator that doesn't suppor it will return SIZE_NOT_TRACKED.
		virtual uint32_t allocated_size(void *p) = 0;

		/// Returns the total amount of memory allocated by this allocator. Note that the 
		/// size returned can be bigger than the size of all individual allocations made,
		/// because the allocator may keep additional structures.
		///
		/// If the allocator doesn't track memory, this function returns SIZE_NOT_TRACKED.
		virtual uint32_t total_allocated() = 0;

	private:
		/// Allocators cannot be copied.
	    Allocator(const Allocator& other);
	    Allocator& operator=(const Allocator& other);
	};

	/// Creates a new object of type T using the allocator a to allocate the memory.
	#define MAKE_NEW(a, T, ...)		(new ((a).allocate(sizeof(T), alignof(T))) T(__VA_ARGS__))

    /// Returns a UniquePtr owning a new object of type T using the allocator a to allocate the memory
    #define MAKE_UNIQUE(alloc, T, ...) foundation::UniquePtr< T >( new (foundation::allocate<T>( alloc )) T(__VA_ARGS__), alloc )

	/// Frees an object allocated with MAKE_NEW.
	#define MAKE_DELETE(a, T, p)	do { if (p) { ::foundation::memory::destruct( *p ); a.deallocate(p); } } while (0)

	/// Functions for accessing global memory data.
	namespace memory_globals {
		/// Initializes the global memory allocators. scratch_buffer_size is the size of the
		/// memory buffer used by the scratch allocators.
		void init(uint32_t scratch_buffer_size = 4*1024*1024);

		/// Returns a default memory allocator that can be used for most allocations.
		///
		/// You need to call init() for this allocator to be available.
		Allocator &default_allocator();

		/// Returns a "scratch" allocator that can be used for temporary short-lived memory
		/// allocations. The scratch allocator uses a ring buffer of size scratch_buffer_size
		/// to service the allocations.
		///
		/// If there is not enough memory in the buffer to match requests for scratch
		/// memory, memory from the default_allocator will be returned instead.
		Allocator &default_scratch_allocator();

		/// Shuts down the global memory allocators created by init().
		void shutdown();
	}

	namespace memory {
		inline void *align_forward(void *p, uint32_t align);
		inline void *pointer_add(void *p, uint32_t bytes);
		inline const void *pointer_add(const void *p, uint32_t bytes);
		inline void *pointer_sub(void *p, uint32_t bytes);
		inline const void *pointer_sub(const void *p, uint32_t bytes);
        template<typename T, typename O>
        inline const T *ptr_from_byte_offset( const void *basePtr, O offsetInByte );
        template<typename T, typename O>
        inline T *ptr_from_byte_offset( void *basePtr, O offsetInByte );
        template<typename T> 
        inline void destruct( T &self );
    }

	// ---------------------------------------------------------------
	// Inline function implementations
	// ---------------------------------------------------------------

	// Aligns p to the specified alignment by moving it forward if necessary
	// and returns the result.
	inline void *memory::align_forward(void *p, uint32_t align)
	{
		uintptr_t pi = uintptr_t(p);
		const uint32_t mod = pi % align;
		if (mod)
			pi += (align - mod);
		return (void *)pi;
	}

	/// Returns the result of advancing p by the specified number of bytes
	inline void *memory::pointer_add(void *p, uint32_t bytes)
	{
		return (void*)((char *)p + bytes);
	}

	inline const void *memory::pointer_add(const void *p, uint32_t bytes)
	{
		return (const void*)((const char *)p + bytes);
	}

	/// Returns the result of moving p back by the specified number of bytes
	inline void *memory::pointer_sub(void *p, uint32_t bytes)
	{
		return (void*)((char *)p - bytes);
	}

	inline const void *memory::pointer_sub(const void *p, uint32_t bytes)
	{
		return (const void*)((const char *)p - bytes);
	}

    template<typename T, typename O>
    inline const T *memory::ptr_from_byte_offset( const void *basePtr, O offsetInByte )
    {
        const char *p = reinterpret_cast<const char *>( basePtr );
        p += offsetInByte;
        return reinterpret_cast<const T *>( p );
    }

    template<typename T, typename O>
    inline T *memory::ptr_from_byte_offset( void *basePtr, O offsetInByte )
    {
        char *p = reinterpret_cast<char *>( basePtr );
        p += offsetInByte;
        return reinterpret_cast<T *>( p );
    }

    template<typename T>
    inline void memory::destruct( T &self )
    {
        self.~T();
    }

    // ---------------------------------------------------------------
    // Inline UniquePtr function implementations
    // ---------------------------------------------------------------

    template<typename T> inline UniquePtr<T>::UniquePtr()
        : value_( nullptr )
        , allocator_( nullptr )
        , destructor_( nullptr )
    {
    }

    template<typename T> inline UniquePtr<T>::UniquePtr( T *value, Allocator &allocator )
        : value_( value )
        , allocator_( &allocator )
        , destructor_( &memory::destruct )
    {
    }

    template<typename T> inline UniquePtr<T>::UniquePtr( UniquePtr &&other )
        : value_( other.value_ )
        , allocator_( other.allocator_ )
        , destructor_( other.destructor_ )
    {
        other.value_ = nullptr;
        other.allocator_ = nullptr;
        other.destructor_ = nullptr;
    }

    template<typename T>
    inline UniquePtr<T> &UniquePtr<T>::operator =( UniquePtr &&other )
    {
        if (value_ != nullptr)
        {
            destructor_( *value_ );
            allocator_->deallocate( value_ );
        }
        value_ = other.value_;
        allocator_ = other.allocator_;
        destructor_ = other.destructor_;
        other.value_ = nullptr;
        other.allocator_ = nullptr;
        other.destructor_ = nullptr;
        return *this;
    }

    template<typename T>
    template<typename O>
    inline UniquePtr<T> &UniquePtr<T>::operator =( UniquePtr<O> &&other )
    {
        if (value_ != nullptr)
        {
            destructor_( *value_ );
            allocator_->deallocate( value_ );
        }
        value_ = other.value_;
        allocator_ = other.allocator_;
        destructor_ = &memory::destruct;
        other.value_ = nullptr;
        other.allocator_ = nullptr;
        other.destructor_ = nullptr;
        return *this;
    }

    template<typename T> inline UniquePtr<T>::~UniquePtr()
    {
        reset();
    }

    template<typename T> inline T *UniquePtr<T>::get()
    {
        return value_;
    }

    template<typename T> inline T *UniquePtr<T>::release()
    {
        T *value = value_;
        value_ = nullptr;
        allocator_ = nullptr;
        return value;
    }

    template<typename T> inline T *UniquePtr<T>::operator ->() const
    {
        return value_;
    }

    template<typename T> inline T &UniquePtr<T>::operator *() const
    {
        return *value_;
    }


    template<typename T> inline void UniquePtr<T>::reset()
    {
        if (value_ != nullptr)
        {
            destructor_( *value_ );
            allocator_->deallocate( value_ );
            value_ = nullptr;
            allocator_ = nullptr;
        }
    }

    template<typename T> inline UniquePtr<T>::operator bool() const
    {
        return value_ != nullptr;
    }

    template<typename T> inline T* allocate( Allocator &allocator )
    {
        return static_cast<T *>( allocator.allocate(sizeof(T), alignof(T)) );
    }

}
