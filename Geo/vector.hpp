#pragma once

namespace kato
{

template< typename T >
class vector
{
public:
	T x, y, z;

public:
	constexpr
	vector()
		: x{}, y{}, z{}
	{
	}
	constexpr
	vector( T const v1, T const v2, T const v3 )
		: x( v1 ), y( v2 ), z( v3 )
	{
	}
	constexpr
	vector( T const *const pv )
		: vector( pv[ 0 ], pv[ 1 ], pv[ 2 ] )
	{
	}
	vector( vector const &right ) = default;
	vector( vector &&right ) = default;
	vector &operator=( vector const &right ) = default;
	vector &operator=( vector &&right ) = default;
	~vector() = default;

	T inner_product( vector const &right ) const
	{
		return x * right.x + y * right.y + z * right.z;
	}
	vector cross_product( vector const &right ) const
	{
		return vector( y * right.z - z * right.y, z * right.x - x * right.z, x * right.y - y * right.x );
	}
	vector &operator+=( vector const &right )
	{
		x += right.x; y += right.y; z += right.z;
		return *this;
	}
	vector operator+( vector const &right ) const
	{
		return vector( x + right.x, y + right.y, z + right.z );
	}
	vector &operator-=( vector const &right )
	{
		x -= right.x; y -= right.y; z -= right.z;
		return *this;
	}
	vector operator-( vector const &right ) const
	{
		return vector( x - right.x, y - right.y, z - right.z );
	}
	vector &operator*=( T const right )
	{
		x *= right; y *= right; z *= right;
		return *this;
	}
	vector operator*( T const right ) const
	{
		return vector( x * right, y * right, z * right );
	}
	friend
	vector operator*( T const left, vector const &right )
	{
		return vector( left * right.x, left * right.y, left * right.z );
	}
	vector &operator/=( T const right )
	{
		x /= right; y /= right; z /= right;
		return *this;
	}
	vector operator/( T const right ) const
	{
		return vector( x / right, y / right, z / right );
	}
	bool operator==( vector const &right ) const
	{
		return x == right.x && y == right.y && z == right.z;
	}
	bool operator!=( vector const &right ) const
	{
		return !(*this == right);
	}

	T length() const
	{
		return std::sqrt( x * x + y * y + z * z );
	}
	vector normarize() const
	{
		return *this / length();
	}
};

template< typename T >
class vector2
{
public:
	T x, y;

public:
	constexpr
	vector2()
		: x{}, y{}
	{
	}
	constexpr
	vector2( T const v1, T const v2 )
		: x( v1 ), y( v2 )
	{
	}
	constexpr
	vector2( T const *const pv )
		: vector2( pv[ 0 ], pv[ 1 ] )
	{
	}
	vector2( vector2 const &right ) = default;
	vector2( vector2 &&right ) = default;
	vector2 &operator=( vector2 const &right ) = default;
	vector2 &operator=( vector2 &&right ) = default;
	~vector2() = default;

	T inner_product( vector2 const &right ) const
	{
		return x * right.x + y * right.y;
	}
	vector2 &operator+=( vector2 const &right )
	{
		x += right.x; y += right.y;
		return *this;
	}
	vector2 operator+( vector2 const &right ) const
	{
		return vector2( x + right.x, y + right.y );
	}
	vector2 &operator-=( vector2 const &right )
	{
		x -= right.x; y -= right.y;
		return *this;
	}
	vector2 operator-( vector2 const &right ) const
	{
		return vector2( x - right.x, y - right.y );
	}
	vector2 &operator*=( T const right )
	{
		x *= right; y *= right;
		return *this;
	}
	vector2 operator*( T const right ) const
	{
		return vector2( x * right, y * right );
	}
	friend
	vector2 operator*( T const left, vector2 const &right )
	{
		return vector2( left * right.x, left * right.y );
	}
	vector2 &operator/=( T const right )
	{
		x /= right; y /= right;
		return *this;
	}
	vector2 operator/( T const right ) const
	{
		return vector2( x / right, y / right );
	}
	bool operator==( vector2 const &right ) const
	{
		return x == right.x && y == right.y;
	}
	bool operator!=( vector2 const &right ) const
	{
		return !(*this == right);
	}

	T length() const
	{
		return std::sqrt( x * x + y * y );
	}
	vector2 normarize() const
	{
		return *this / length();
	}
};

using vectorf = vector< float >;
using vectord = vector< double >;

using vector2f = vector2< float >;
using vector2d = vector2< double >;

}