#include <foundation/math.h>


namespace foundation {

// class Matrix3x3
// //////////////////////////////////////////////////////////////////


Matrix3x3 &
Matrix3x3::operator *=( const Matrix3x3 &other )
{
    *this = *this * other;
    return *this;
}


Matrix3x3 
Matrix3x3::inversed() const
{
    Matrix3x3 result;
    const Matrix3x3 &m = *this;
    float OneOverDeterminant = 1.0f  / (
        +m[ 0 ][ 0 ] * ( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 2 ][ 1 ] * m[ 1 ][ 2 ] )
        - m[ 1 ][ 0 ] * ( m[ 0 ][ 1 ] * m[ 2 ][ 2 ] - m[ 2 ][ 1 ] * m[ 0 ][ 2 ] )
        + m[ 2 ][ 0 ] * ( m[ 0 ][ 1 ] * m[ 1 ][ 2 ] - m[ 1 ][ 1 ] * m[ 0 ][ 2 ] ) );

    result[ 0 ][ 0 ] = +( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 2 ][ 1 ] * m[ 1 ][ 2 ] ) * OneOverDeterminant;
    result[ 1 ][ 0 ] = -( m[ 1 ][ 0 ] * m[ 2 ][ 2 ] - m[ 2 ][ 0 ] * m[ 1 ][ 2 ] ) * OneOverDeterminant;
    result[ 2 ][ 0 ] = +( m[ 1 ][ 0 ] * m[ 2 ][ 1 ] - m[ 2 ][ 0 ] * m[ 1 ][ 1 ] ) * OneOverDeterminant;
    result[ 0 ][ 1 ] = -( m[ 0 ][ 1 ] * m[ 2 ][ 2 ] - m[ 2 ][ 1 ] * m[ 0 ][ 2 ] ) * OneOverDeterminant;
    result[ 1 ][ 1 ] = +( m[ 0 ][ 0 ] * m[ 2 ][ 2 ] - m[ 2 ][ 0 ] * m[ 0 ][ 2 ] ) * OneOverDeterminant;
    result[ 2 ][ 1 ] = -( m[ 0 ][ 0 ] * m[ 2 ][ 1 ] - m[ 2 ][ 0 ] * m[ 0 ][ 1 ] ) * OneOverDeterminant;
    result[ 0 ][ 2 ] = +( m[ 0 ][ 1 ] * m[ 1 ][ 2 ] - m[ 1 ][ 1 ] * m[ 0 ][ 2 ] ) * OneOverDeterminant;
    result[ 1 ][ 2 ] = -( m[ 0 ][ 0 ] * m[ 1 ][ 2 ] - m[ 1 ][ 0 ] * m[ 0 ][ 2 ] ) * OneOverDeterminant;
    result[ 2 ][ 2 ] = +( m[ 0 ][ 0 ] * m[ 1 ][ 1 ] - m[ 1 ][ 0 ] * m[ 0 ][ 1 ] ) * OneOverDeterminant;
    return result;
}


Matrix3x3 Matrix3x3::rotated( float angleInRadian, const Vector3 &rotationAxis ) const
{
    const float a = angleInRadian;
    const float c = cosf( a );
    const float s = sinf( a );

    Vector3 axis = rotationAxis.normalized();
    Vector3 temp = axis * ( 1.0f - c );

    Matrix3x3 rot;
    rot[ 0 ][ 0 ] = c + temp[ 0 ] * axis[ 0 ];
    rot[ 0 ][ 1 ] = 0 + temp[ 0 ] * axis[ 1 ] + s * axis[ 2 ];
    rot[ 0 ][ 2 ] = 0 + temp[ 0 ] * axis[ 2 ] - s * axis[ 1 ];

    rot[ 1 ][ 0 ] = 0 + temp[ 1 ] * axis[ 0 ] - s * axis[ 2 ];
    rot[ 1 ][ 1 ] = c + temp[ 1 ] * axis[ 1 ];
    rot[ 1 ][ 2 ] = 0 + temp[ 1 ] * axis[ 2 ] + s * axis[ 0 ];

    rot[ 2 ][ 0 ] = 0 + temp[ 2 ] * axis[ 0 ] + s * axis[ 1 ];
    rot[ 2 ][ 1 ] = 0 + temp[ 2 ] * axis[ 1 ] - s * axis[ 0 ];
    rot[ 2 ][ 2 ] = c + temp[ 2 ] * axis[ 2 ];

    Matrix3x3 result;
    result[ 0 ] = cols[ 0 ] * rot.cols[ 0 ].x + cols[ 1 ] * rot.cols[ 0 ].y + cols[ 2 ] * rot.cols[ 0 ].z;
    result[ 1 ] = cols[ 0 ] * rot.cols[ 1 ].x + cols[ 1 ] * rot.cols[ 1 ].y + cols[ 2 ] * rot.cols[ 1 ].z;
    result[ 2 ] = cols[ 0 ] * rot.cols[ 2 ].x + cols[ 1 ] * rot.cols[ 2 ].y + cols[ 2 ] * rot.cols[ 2 ].z;
    return result;
}


Matrix3x3 Matrix3x3::scaled( const Vector3 &scaleFactor ) const
{
    Matrix3x3 result;
    result.cols[ 0 ] = cols[ 0 ] * scaleFactor.x;
    result.cols[ 1 ] = cols[ 1 ] * scaleFactor.y;
    result.cols[ 2 ] = cols[ 2 ] * scaleFactor.z;
    return result;
}

// class Matrix4x4
// //////////////////////////////////////////////////////////////////

Matrix4x4 Matrix4x4::inversed() const
{
    const Matrix4x4 &m = *this;
    float coef00 = m.cols[ 2 ].z * m.cols[ 3 ].w - m.cols[ 3 ].z * m.cols[ 2 ].w;
    float coef02 = m.cols[ 1 ].z * m.cols[ 3 ].w - m.cols[ 3 ].z * m.cols[ 1 ].w;
    float coef03 = m.cols[ 1 ].z * m.cols[ 2 ].w - m.cols[ 2 ].z * m.cols[ 1 ].w;

    float coef04 = m.cols[ 2 ].y * m.cols[ 3 ].w - m.cols[ 3 ].y * m.cols[ 2 ].w;
    float coef06 = m.cols[ 1 ].y * m.cols[ 3 ].w - m.cols[ 3 ].y * m.cols[ 1 ].w;
    float coef07 = m.cols[ 1 ].y * m.cols[ 2 ].w - m.cols[ 2 ].y * m.cols[ 1 ].w;

    float coef08 = m.cols[ 2 ].y * m.cols[ 3 ].z - m.cols[ 3 ].y * m.cols[ 2 ].z;
    float coef10 = m.cols[ 1 ].y * m.cols[ 3 ].z - m.cols[ 3 ].y * m.cols[ 1 ].z;
    float coef11 = m.cols[ 1 ].y * m.cols[ 2 ].z - m.cols[ 2 ].y * m.cols[ 1 ].z;

    float coef12 = m.cols[ 2 ].x * m.cols[ 3 ].w - m.cols[ 3 ].x * m.cols[ 2 ].w;
    float coef14 = m.cols[ 1 ].x * m.cols[ 3 ].w - m.cols[ 3 ].x * m.cols[ 1 ].w;
    float coef15 = m.cols[ 1 ].x * m.cols[ 2 ].w - m.cols[ 2 ].x * m.cols[ 1 ].w;

    float coef16 = m.cols[ 2 ].x * m.cols[ 3 ].z - m.cols[ 3 ].x * m.cols[ 2 ].z;
    float coef18 = m.cols[ 1 ].x * m.cols[ 3 ].z - m.cols[ 3 ].x * m.cols[ 1 ].z;
    float coef19 = m.cols[ 1 ].x * m.cols[ 2 ].z - m.cols[ 2 ].x * m.cols[ 1 ].z;

    float coef20 = m.cols[ 2 ].x * m.cols[ 3 ].y - m.cols[ 3 ].x * m.cols[ 2 ].y;
    float coef22 = m.cols[ 1 ].x * m.cols[ 3 ].y - m.cols[ 3 ].x * m.cols[ 1 ].y;
    float coef23 = m.cols[ 1 ].x * m.cols[ 2 ].y - m.cols[ 2 ].x * m.cols[ 1 ].y;

    Vector4 fac0 = { coef00, coef00, coef02, coef03 };
    Vector4 fac1 = { coef04, coef04, coef06, coef07 };
    Vector4 fac2 = { coef08, coef08, coef10, coef11 };
    Vector4 fac3 = { coef12, coef12, coef14, coef15 };
    Vector4 fac4 = { coef16, coef16, coef18, coef19 };
    Vector4 fac5 = { coef20, coef20, coef22, coef23 };

    Vector4 vec0 = { m.cols[ 1 ][ 0 ], m.cols[ 0 ][ 0 ], m.cols[ 0 ][ 0 ], m.cols[ 0 ][ 0 ] };
    Vector4 vec1 = { m.cols[ 1 ][ 1 ], m.cols[ 0 ][ 1 ], m.cols[ 0 ][ 1 ], m.cols[ 0 ][ 1 ] };
    Vector4 vec2 = { m.cols[ 1 ][ 2 ], m.cols[ 0 ][ 2 ], m.cols[ 0 ][ 2 ], m.cols[ 0 ][ 2 ] };
    Vector4 vec3 = { m.cols[ 1 ][ 3 ], m.cols[ 0 ][ 3 ], m.cols[ 0 ][ 3 ], m.cols[ 0 ][ 3 ] };

    Vector4 inv0 = vec1 * fac0 - vec2 * fac1 + vec3 * fac2;
    Vector4 inv1 = vec0 * fac0 - vec2 * fac3 + vec3 * fac4;
    Vector4 inv2 = vec0 * fac1 - vec1 * fac3 + vec3 * fac5;
    Vector4 inv3 = vec0 * fac2 - vec1 * fac4 + vec2 * fac5;

    Vector4 signA = { +1.0f, -1.0f, +1.0f, -1.0f };
    Vector4 signB = { -1.0f, +1.0f, -1.0f, +1.0f};
    Matrix4x4 inverse;
    inverse.cols[0] = inv0 * signA;
    inverse.cols[1] = inv1 * signB;
    inverse.cols[2] = inv2 * signA;
    inverse.cols[3] = inv3 * signB;

    Vector4 row0 = { inverse[ 0 ][ 0 ], inverse[ 1 ][ 0 ], inverse[ 2 ][ 0 ], inverse[ 3 ][ 0 ] };

    Vector4 dot0 = m[ 0 ] * row0;
    float dot1 = ( dot0.x + dot0.y ) + ( dot0.z + dot0.w );
    float oneOverDeterminant = 1.0f / dot1;
    inverse.cols[ 0 ] *= oneOverDeterminant;
    inverse.cols[ 1 ] *= oneOverDeterminant;
    inverse.cols[ 2 ] *= oneOverDeterminant;
    inverse.cols[ 3 ] *= oneOverDeterminant;
    return inverse;
}

} // namespace foundation {
