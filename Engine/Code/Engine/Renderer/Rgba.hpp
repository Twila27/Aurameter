#pragma once
#include "Engine/Math/Vector4.hpp"


typedef unsigned char byte_t;
struct Rgba
{
	Rgba();
	Rgba( float red, float green, float blue, float alphaOpacity = 1.f );
	Rgba( const Vector4f& rgba ) : Rgba( rgba.x, rgba.y, rgba.z, rgba.w ) {}
	Rgba( const Vector4i& rgba ) : Rgba( (byte_t)rgba.x, (byte_t)rgba.y, (byte_t)rgba.z, (byte_t)rgba.w ) {}
	Rgba( byte_t red, byte_t green, byte_t blue, byte_t alphaOpacity = 255 );
	Rgba( const Rgba& other );
	bool operator==( const Rgba& compareTo ) const; //Currently NOT comparing alpha.
	bool operator!=( const Rgba& compareTo ) const; //Currently NOT comparing alpha.
	const Rgba operator*( const Rgba& componentwiseScaleBy ) const;
	const Rgba operator+( const Rgba& componentWiseAddBy ) const;
	Vector4f GetAsFloats() const;
	static Rgba AverageColors( const Rgba& a, const Rgba& b );
	std::string ToString() const;

	byte_t red;
	byte_t green;
	byte_t blue;
	byte_t alphaOpacity;

	static const Rgba BLACK;
	static const Rgba DARK_GRAY;
	static const Rgba GRAY;
	static const Rgba WHITE;
	static const Rgba RED;
	static const Rgba BLUE;
	static const Rgba GREEN;
	static const Rgba MAGENTA;
	static const Rgba YELLOW;
	static const Rgba CYAN;
	static const Rgba HALF_OPAQUE_BLACK;
};