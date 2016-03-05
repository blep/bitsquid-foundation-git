#pragma once

#include "math_types.h"
#include "assert.h"
#include <math.h>


namespace foundation {


// class Vector3
// //////////////////////////////////////////////////////////////////

inline Vector3 vector3( float x, float y, float z )
{
    Vector3 v = {x, y, z};
    return v;
}

inline Vector3 Vector3::zero()
{
    Vector3 v = { 0,0,0 };
    return v;
}

inline Vector3 Vector3::axisX() {
    Vector3 v = {1.0f, 0.0f, 0.0f};
    return v;
}

inline Vector3 Vector3::axisY() {
    Vector3 v = { 0.0f, 1.0f, 0.0f };
    return v;
}

inline Vector3 Vector3::axisZ() {
    Vector3 v = { 0.0f, 0.0f, 1.0f };
    return v;
}

inline Vector3 Vector3::uniform( float value )
{
    return Vector3{value, value, value};
}

inline float Vector3::operator[]( int index ) const
{
    FDT_ASSERT( index >= 0  &&  index < 3 );
    return (&x)[index];
}

inline float &Vector3::operator[]( int index )
{
    FDT_ASSERT( index >= 0 && index < 3 );
    return ( &x )[ index ];
}


inline Vector3 &Vector3::operator +=( const Vector3 &other )
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

inline Vector3 &Vector3::operator -=( const Vector3 &other )
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

inline Vector3 &Vector3::operator *=( float factor )
{
    x *= factor;
    y *= factor;
    z *= factor;
    return *this;
}

inline float Vector3::length() const
{
    return sqrtf( x*x + y*y + z*z );
}

inline float Vector3::lengthSquared() const
{
    return x*x + y*y + z*z;
}

inline void Vector3::normalize()
{
    float factor = 1.0f / length();
    *this *= factor;
}

inline Vector3 Vector3::normalized() const
{
    Vector3 copy( *this );
    copy.normalize();
    return copy;
}

inline float Vector3::dot( const Vector3 &other ) const
{
    return x * other.x + y * other.y + z * other.z;
}

inline Vector3 Vector3::crossProduct( const Vector3 &other ) const
{
    Vector3 result = { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
    return result;
}

inline bool Vector3::operator ==( const Vector3 &other ) const
{
    return x == other.x  &&  y == other.y  &&  z == other.z;
}

inline bool Vector3::operator !=( const Vector3 &other ) const
{
    return !(*this == other);
}

inline Vector3 Vector3::operator -() const
{
    Vector3 result;
    result.x = -x;
    result.y = -y;
    result.z = -z;
    return result;
}

inline Vector3 operator +( const Vector3 &a, const Vector3 &b )
{
    Vector3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline Vector3 operator -( const Vector3 &a, const Vector3 &b )
{
    Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline Vector3 operator *( const Vector3 &a, float factor )
{
    Vector3 result;
    result.x = a.x * factor;
    result.y = a.y * factor;
    result.z = a.z * factor;
    return result;
}


// class Matrix3x3
// //////////////////////////////////////////////////////////////////

inline Matrix3x3 matrix3x3( const Vector3 &col0, const Vector3 &col1, const Vector3 &col2 )
{
    Matrix3x3 m;
    m[0] = col0;
    m[1] = col1;
    m[2] = col2;
    return m;
}

inline Matrix3x3 Matrix3x3::zero()
{
    Matrix3x3 m = { Vector3{0.0f, 0.0f, 0.0f}, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f } };
    return m;
}

inline Matrix3x3 Matrix3x3::identity()
{
    Matrix3x3 m = { Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 1.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f } };
    return m;
}


inline const Vector3 &Matrix3x3::operator[]( int index ) const
{
    FDT_ASSERT( index >= 0  &&  index < 3 );
    return cols[index];
}

inline Vector3 &Matrix3x3::operator[]( int index )
{
    FDT_ASSERT( index >= 0 && index < 3 );
    return cols[ index ];
}


inline Vector3 Matrix3x3::row( int index ) const
{
    FDT_ASSERT( index >= 0 && index < 3 );
    Vector3 result;
    result.x = cols[ 0 ][ index ];
    result.y = cols[ 1 ][ index ];
    result.z = cols[ 2 ][ index ];
    return result;
}


inline Matrix3x3 &Matrix3x3::operator +=( const Matrix3x3 &other )
{
    cols[ 0 ] += other.cols[ 0 ];
    cols[ 1 ] += other.cols[ 1 ];
    cols[ 2 ] += other.cols[ 2 ];
    return *this;
}

inline Matrix3x3 Matrix3x3::transposed() const
{
    Matrix3x3 result;
    result.cols[ 0 ] = { cols[ 0 ].x, cols[ 1 ].x, cols[ 2 ].x };
    result.cols[ 1 ] = { cols[ 0 ].y, cols[ 1 ].y, cols[ 2 ].y };
    result.cols[ 2 ] = { cols[ 0 ].z, cols[ 1 ].z, cols[ 2 ].z };
    return result;
}


inline Matrix3x3 Matrix3x3::operator +( const Matrix3x3 &other ) const
{
    Matrix3x3 result;
    result[ 0 ] = cols[ 0 ] + other.cols[ 0 ];
    result[ 1 ] = cols[ 1 ] + other.cols[ 1 ];
    result[ 2 ] = cols[ 2 ] + other.cols[ 2 ];
    return result;
}


inline Matrix3x3 Matrix3x3::operator *( const Matrix3x3 &other ) const
{
    const float srcA00 = cols[ 0 ].x;
    const float srcA01 = cols[ 0 ].y;
    const float srcA02 = cols[ 0 ].z;
    const float srcA10 = cols[ 1 ].x;
    const float srcA11 = cols[ 1 ].y;
    const float srcA12 = cols[ 1 ].z;
    const float srcA20 = cols[ 2 ].x;
    const float srcA21 = cols[ 2 ].y;
    const float srcA22 = cols[ 2 ].z;

    const float srcB00 = other.cols[ 0 ].x;
    const float srcB01 = other.cols[ 0 ].y;
    const float srcB02 = other.cols[ 0 ].z;
    const float srcB10 = other.cols[ 1 ].x;
    const float srcB11 = other.cols[ 1 ].y;
    const float srcB12 = other.cols[ 1 ].z;
    const float srcB20 = other.cols[ 2 ].x;
    const float srcB21 = other.cols[ 2 ].y;
    const float srcB22 = other.cols[ 2 ].z;

    Matrix3x3 result;
    result.cols[ 0 ].x = srcA00 * srcB00 + srcA10 * srcB01 + srcA20 * srcB02;
    result.cols[ 0 ].y = srcA01 * srcB00 + srcA11 * srcB01 + srcA21 * srcB02;
    result.cols[ 0 ].z = srcA02 * srcB00 + srcA12 * srcB01 + srcA22 * srcB02;
    result.cols[ 1 ].x = srcA00 * srcB10 + srcA10 * srcB11 + srcA20 * srcB12;
    result.cols[ 1 ].y = srcA01 * srcB10 + srcA11 * srcB11 + srcA21 * srcB12;
    result.cols[ 1 ].z = srcA02 * srcB10 + srcA12 * srcB11 + srcA22 * srcB12;
    result.cols[ 2 ].x = srcA00 * srcB20 + srcA10 * srcB21 + srcA20 * srcB22;
    result.cols[ 2 ].y = srcA01 * srcB20 + srcA11 * srcB21 + srcA21 * srcB22;
    result.cols[ 2 ].z = srcA02 * srcB20 + srcA12 * srcB21 + srcA22 * srcB22;
    return result;
}

// In: column vector, column matrix
// Out: row vector
inline Vector3 operator *( const Vector3 &v, const Matrix3x3 &m )
{
    Vector3 result;
    result.x = m.cols[ 0 ].x * v.x + m.cols[ 0 ].y * v.y + m.cols[ 0 ].z * v.z;
    result.y = m.cols[ 1 ].x * v.x + m.cols[ 1 ].y * v.y + m.cols[ 1 ].z * v.z;
    result.z = m.cols[ 2 ].x * v.x + m.cols[ 2 ].y * v.y + m.cols[ 2 ].z * v.z;
    return result;
}



// class Vector4
// //////////////////////////////////////////////////////////////////

inline Vector4 vector4( float x, float y, float z, float w=1.0f )
{
    Vector4 v = { x, y, z, w };
    return v;
}

inline Vector4 Vector4::zero()
{
    Vector4 v = { 0,0,0,0 };
    return v;
}

inline Vector4 Vector4::uniform( float value )
{
    return Vector4{ value, value, value, value };
}


inline float Vector4::operator[]( int index ) const
{
    FDT_ASSERT( index >= 0 && index < 4 );
    return ( &x )[ index ];
}

inline float &Vector4::operator[]( int index )
{
    FDT_ASSERT( index >= 0 && index < 4 );
    return ( &x )[ index ];
}


inline Vector4 &Vector4::operator +=( const Vector4 &other )
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

inline Vector4 &Vector4::operator -=( const Vector4 &other )
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

inline Vector4 &Vector4::operator *=( float factor )
{
    x *= factor;
    y *= factor;
    z *= factor;
    w *= factor;
    return *this;
}

inline Vector4 &Vector4::operator *=( const Vector4 &factor )
{
    x *= factor.x;
    y *= factor.y;
    z *= factor.z;
    w *= factor.w;
    return *this;
}

inline float Vector4::length() const
{
    return sqrtf( x*x + y*y + z*z + w*w );
}

inline float Vector4::lengthSquared() const
{
    return x*x + y*y + z*z + w*w;
}

inline void Vector4::normalize()
{
    float factor = 1.0f / length();
    *this *= factor;
}

inline Vector4 Vector4::normalized() const
{
    Vector4 copy( *this );
    copy.normalize();
    return copy;
}

inline float Vector4::dot( const Vector4 &other ) const
{
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

inline bool Vector4::operator ==( const Vector4 &other ) const
{
    return x == other.x  &&  y == other.y  &&  z == other.z  &&  w == other.w;
}

inline bool Vector4::operator !=( const Vector4 &other ) const
{
    return !( *this == other );
}

inline Vector4 Vector4::operator -() const
{
    Vector4 result;
    result.x = -x;
    result.y = -y;
    result.z = -z;
    result.w = -w;
    return result;
}

inline Vector4 operator +( const Vector4 &a, const Vector4 &b )
{
    Vector4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline Vector4 operator -( const Vector4 &a, const Vector4 &b )
{
    Vector4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline Vector4 operator *( const Vector4 &a, float factor )
{
    Vector4 result;
    result.x = a.x * factor;
    result.y = a.y * factor;
    result.z = a.z * factor;
    result.w = a.w * factor;
    return result;
}

inline Vector4 operator *( const Vector4 &a, const Vector4 &b )
{
    Vector4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    return result;
}



// class Matrix4x4
// //////////////////////////////////////////////////////////////////

inline Matrix4x4 Matrix4x4::zero()
{
    Matrix4x4 m = { Vector4{ 0.0f, 0.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } };
    return m;
}

inline Matrix4x4 Matrix4x4::identity()
{
    Matrix4x4 m = { Vector4{ 1.0f, 0.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 1.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 1.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f } };
    return m;
}

inline Matrix4x4 matrix4x4( const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3 )
{
    Matrix4x4 m;
    m[ 0 ] = col0;
    m[ 1 ] = col1;
    m[ 2 ] = col2;
    m[ 3 ] = col3;
    return m;
}

inline Matrix4x4 matrix4x4( const Matrix3x3 &rotation, const Vector3 &scale, const Vector3 &translate )
{
    Vector3 rotX = rotation[ 0 ] * scale.x;
    Vector3 rotY = rotation[ 1 ] * scale.y;
    Vector3 rotZ = rotation[ 2 ] * scale.z;
    return matrix4x4(
        Vector4{ rotX.x, rotX.y, rotX.z, translate.x },
        Vector4{ rotY.x, rotY.y, rotY.z, translate.y },
        Vector4{ rotZ.x, rotZ.y, rotZ.z, translate.z },
        Vector4{ 0.0f, 0.0f, 0.0f, 1.0f } );
}


inline const Vector4 &Matrix4x4::operator[]( int index ) const
{
    FDT_ASSERT( index >= 0 && index < 4 );
    return cols[ index ];
}

inline Vector4 &Matrix4x4::operator[]( int index )
{
    FDT_ASSERT( index >= 0 && index < 4 );
    return cols[ index ];
}

inline Vector4 Matrix4x4::row( int index ) const
{
    FDT_ASSERT( index >= 0 && index < 4 );
    Vector4 result;
    result.x = cols[ 0 ][ index ];
    result.y = cols[ 1 ][ index ];
    result.z = cols[ 2 ][ index ];
    result.w = cols[ 3 ][ index ];
    return result;
}

inline Matrix4x4 &Matrix4x4::operator +=( const Matrix4x4 &other )
{
    cols[ 0 ] += other.cols[ 0 ];
    cols[ 1 ] += other.cols[ 1 ];
    cols[ 2 ] += other.cols[ 2 ];
    cols[ 3 ] += other.cols[ 3 ];
    return *this;
}

inline Matrix4x4 &Matrix4x4::operator *=( const Matrix4x4 &other )
{
    *this = *this * other;
    return *this;
}

inline Matrix4x4 Matrix4x4::transposed() const
{
    Matrix4x4 result;
    result.cols[ 0 ] = { cols[ 0 ].x, cols[ 1 ].x, cols[ 2 ].x, cols[ 3 ].x };
    result.cols[ 1 ] = { cols[ 0 ].y, cols[ 1 ].y, cols[ 2 ].y, cols[ 3 ].y };
    result.cols[ 2 ] = { cols[ 0 ].z, cols[ 1 ].z, cols[ 2 ].z, cols[ 3 ].z };
    result.cols[ 3 ] = { cols[ 0 ].w, cols[ 1 ].w, cols[ 2 ].w, cols[ 3 ].w };
    return result;
}

inline Matrix4x4 Matrix4x4::operator +( const Matrix4x4 &other ) const
{
    Matrix4x4 result;
    result[ 0 ] = cols[ 0 ] + other.cols[ 0 ];
    result[ 1 ] = cols[ 1 ] + other.cols[ 1 ];
    result[ 2 ] = cols[ 2 ] + other.cols[ 2 ];
    result[ 3 ] = cols[ 3 ] + other.cols[ 3 ];
    return result;
}

inline Matrix4x4 Matrix4x4::operator -( const Matrix4x4 &other ) const
{
    Matrix4x4 result;
    result[ 0 ] = cols[ 0 ] - other.cols[ 0 ];
    result[ 1 ] = cols[ 1 ] - other.cols[ 1 ];
    result[ 2 ] = cols[ 2 ] - other.cols[ 2 ];
    result[ 3 ] = cols[ 3 ] - other.cols[ 3 ];
    return result;
}

inline Matrix4x4 Matrix4x4::operator *( const Matrix4x4 &other ) const
{
    const float srcA00 = cols[ 0 ].x;
    const float srcA01 = cols[ 0 ].y;
    const float srcA02 = cols[ 0 ].z;
    const float srcA03 = cols[ 0 ].w;
    const float srcA10 = cols[ 1 ].x;
    const float srcA11 = cols[ 1 ].y;
    const float srcA12 = cols[ 1 ].z;
    const float srcA13 = cols[ 1 ].w;
    const float srcA20 = cols[ 2 ].x;
    const float srcA21 = cols[ 2 ].y;
    const float srcA22 = cols[ 2 ].z;
    const float srcA23 = cols[ 2 ].w;
    const float srcA30 = cols[ 3 ].x;
    const float srcA31 = cols[ 3 ].y;
    const float srcA32 = cols[ 3 ].z;
    const float srcA33 = cols[ 3 ].w;

    const float srcB00 = other.cols[ 0 ].x;
    const float srcB01 = other.cols[ 0 ].y;
    const float srcB02 = other.cols[ 0 ].z;
    const float srcB03 = other.cols[ 0 ].w;
    const float srcB10 = other.cols[ 1 ].x;
    const float srcB11 = other.cols[ 1 ].y;
    const float srcB12 = other.cols[ 1 ].z;
    const float srcB13 = other.cols[ 1 ].w;
    const float srcB20 = other.cols[ 2 ].x;
    const float srcB21 = other.cols[ 2 ].y;
    const float srcB22 = other.cols[ 2 ].z;
    const float srcB23 = other.cols[ 2 ].w;
    const float srcB30 = other.cols[ 3 ].x;
    const float srcB31 = other.cols[ 3 ].y;
    const float srcB32 = other.cols[ 3 ].z;
    const float srcB33 = other.cols[ 3 ].w;

    Matrix4x4 result;
    result.cols[ 0 ].x = srcA00 * srcB00 + srcA10 * srcB01 + srcA20 * srcB02 + srcA30 * srcB03;
    result.cols[ 0 ].y = srcA01 * srcB00 + srcA11 * srcB01 + srcA21 * srcB02 + srcA31 * srcB03;
    result.cols[ 0 ].z = srcA02 * srcB00 + srcA12 * srcB01 + srcA22 * srcB02 + srcA32 * srcB03;
    result.cols[ 0 ].w = srcA03 * srcB00 + srcA13 * srcB01 + srcA23 * srcB02 + srcA33 * srcB03;
    result.cols[ 1 ].x = srcA00 * srcB10 + srcA10 * srcB11 + srcA20 * srcB12 + srcA30 * srcB13;
    result.cols[ 1 ].y = srcA01 * srcB10 + srcA11 * srcB11 + srcA21 * srcB12 + srcA31 * srcB13;
    result.cols[ 1 ].z = srcA02 * srcB10 + srcA12 * srcB11 + srcA22 * srcB12 + srcA32 * srcB13;
    result.cols[ 1 ].w = srcA03 * srcB10 + srcA13 * srcB11 + srcA23 * srcB12 + srcA33 * srcB13;
    result.cols[ 2 ].x = srcA00 * srcB20 + srcA10 * srcB21 + srcA20 * srcB22 + srcA30 * srcB23;
    result.cols[ 2 ].y = srcA01 * srcB20 + srcA11 * srcB21 + srcA21 * srcB22 + srcA31 * srcB23;
    result.cols[ 2 ].z = srcA02 * srcB20 + srcA12 * srcB21 + srcA22 * srcB22 + srcA32 * srcB23;
    result.cols[ 2 ].w = srcA03 * srcB20 + srcA13 * srcB21 + srcA23 * srcB22 + srcA33 * srcB23;
    result.cols[ 3 ].x = srcA00 * srcB30 + srcA10 * srcB31 + srcA20 * srcB32 + srcA30 * srcB33;
    result.cols[ 3 ].y = srcA01 * srcB30 + srcA11 * srcB31 + srcA21 * srcB32 + srcA31 * srcB33;
    result.cols[ 3 ].z = srcA02 * srcB30 + srcA12 * srcB31 + srcA22 * srcB32 + srcA32 * srcB33;
    result.cols[ 3 ].w = srcA03 * srcB30 + srcA13 * srcB31 + srcA23 * srcB32 + srcA33 * srcB33;
    return result;
}

inline Vector4 operator *( const Vector3 &v, const Matrix4x4 &m )
{
    Vector4 result;
    result.x = m.cols[ 0 ].x * v.x + m.cols[ 0 ].y * v.y + m.cols[ 0 ].z * v.z + m.cols[ 0 ].w;
    result.y = m.cols[ 1 ].x * v.x + m.cols[ 1 ].y * v.y + m.cols[ 1 ].z * v.z + m.cols[ 1 ].w;
    result.z = m.cols[ 2 ].x * v.x + m.cols[ 2 ].y * v.y + m.cols[ 2 ].z * v.z + m.cols[ 2 ].w;
    result.w = m.cols[ 3 ].x * v.x + m.cols[ 3 ].y * v.y + m.cols[ 3 ].z * v.z + m.cols[ 3 ].w;
    return result;
}

inline Vector4 operator *( const Vector4 &v, const Matrix4x4 &m )
{
    Vector4 result;
    result.x = m.cols[ 0 ].x * v.x + m.cols[ 0 ].y * v.y + m.cols[ 0 ].z * v.z + m.cols[ 0 ].w * v.w;
    result.y = m.cols[ 1 ].x * v.x + m.cols[ 1 ].y * v.y + m.cols[ 1 ].z * v.z + m.cols[ 1 ].w * v.w;
    result.z = m.cols[ 2 ].x * v.x + m.cols[ 2 ].y * v.y + m.cols[ 2 ].z * v.z + m.cols[ 2 ].w * v.w;
    result.w = m.cols[ 3 ].x * v.x + m.cols[ 3 ].y * v.y + m.cols[ 3 ].z * v.z + m.cols[ 3 ].w * v.w;
    return result;
}




} // namespace foundation {