#pragma once

namespace foundation
{
	class Allocator;

    template<typename T>
    class UniquePtr
    {
    public:
        UniquePtr();
        UniquePtr( T *value, Allocator &allocator );
//        template<typename T2> UniquePtr( UniquePtr<T2> &&other );
        UniquePtr( UniquePtr &&other );
        UniquePtr &operator =( UniquePtr &&other );
        ~UniquePtr();

        T *get();
        T *release();
        T *operator ->() const;
        T &operator *() const;
        void reset();

        explicit operator bool() const;

    private:
        using DestructorFn = void (*)( T &ptr );
        T *value_;
        Allocator *allocator_;
        DestructorFn destructor_;
    };
}
