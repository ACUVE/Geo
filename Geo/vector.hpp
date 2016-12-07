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

using vectorf = vector< float >;
using vectord = vector< double >;

}