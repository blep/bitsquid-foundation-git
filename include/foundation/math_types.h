#pragma once

#include "types.h"

namespace foundation
{
	struct Vector2
	{
		float x, y;
	};

	struct Vector3
	{
        union { float x, r, s; };
        union { float y, g, t; };
        union { float z, b, p; };

        static Vector3 zero();
        static Vector3 axisX();
        static Vector3 axisY();
        static Vector3 axisZ();
        static Vector3 uniform( float value );

        float operator[]( int index ) const;
        float &operator[]( int index );

        Vector3 &operator +=( const Vector3 &other );
        Vector3 &operator -=( const Vector3 &other );
        Vector3 &operator *=( float factor );
        float length() const;
        float lengthSquared() const;
        void normalize();
        Vector3 normalized() const;
        float dot( const Vector3 &other ) const;
        Vector3 crossProduct( const Vector3 &other ) const;

        Vector3 operator -() const;

        bool operator ==( const Vector3 &other ) const;
        bool operator !=( const Vector3 &other ) const;

    };

	struct Vector4
	{
        union { float x, r, s; };
        union { float y, g, t; };
        union { float z, b, p; };
        union { float w, a, u; };

        static Vector4 zero();
        static Vector4 uniform( float value );

        float operator[]( int index ) const;
        float &operator[]( int index );

        Vector4 &operator +=( const Vector4 &other );
        Vector4 &operator -=( const Vector4 &other );
        Vector4 &operator *=( float factor );
        Vector4 &operator *=( const Vector4 &factor );
        float length() const;
        float lengthSquared() const;
        void normalize();
        Vector4 normalized() const;
        float dot( const Vector4 &other ) const;

        Vector4 operator -() const;

        bool operator ==( const Vector4 &other ) const;
        bool operator !=( const Vector4 &other ) const;
    };

	struct Quaternion
	{
		float x, y, z, w;
	};

	struct Matrix3x3
	{
        using ColType = Vector3;
        Vector3 cols[3];

        static Matrix3x3 zero();
        static Matrix3x3 identity();

        const Vector3 &operator[]( int index ) const;
        Vector3 &operator[]( int index );
        Vector3 row( int index ) const;
        Matrix3x3 &operator +=( const Matrix3x3 &other );
        Matrix3x3 &operator *=( const Matrix3x3 &other );
        Matrix3x3 transposed() const;
        Matrix3x3 inversed() const;

        Matrix3x3 operator +( const Matrix3x3 &other ) const;
        Matrix3x3 operator *( const Matrix3x3 &other ) const;
        friend Vector3 operator *( const Vector3 &v, const Matrix3x3 &m );

        Matrix3x3 rotated( float angleInRadian, const Vector3 &rotationAxis ) const;
        Matrix3x3 scaled( const Vector3 &scaleFactor ) const;
    };

	struct Matrix4x4
	{
        using ColType = Vector4;
        Vector4 cols[4];

        static Matrix4x4 zero();
        static Matrix4x4 identity();

        const Vector4 &operator[]( int index ) const;
        Vector4 &operator[]( int index );
        Vector4 row( int index ) const;
        Matrix4x4 &operator +=( const Matrix4x4 &other );
        Matrix4x4 &operator *=( const Matrix4x4 &other );
        Matrix4x4 transposed() const;
        Matrix4x4 inversed() const;
        Matrix4x4 operator +( const Matrix4x4 &other ) const;
        Matrix4x4 operator -( const Matrix4x4 &other ) const;
        Matrix4x4 operator *( const Matrix4x4 &other ) const;
        friend Vector4 operator *( const Vector3 &v, const Matrix4x4 &m );
        friend Vector4 operator *( const Vector4 &v, const Matrix4x4 &m );
    };

	struct AABB
	{
		Vector3 min, max;
	};

	struct OOBB
	{
		Matrix4x4 tm;
		AABB aabb;
	};
}