#include "Engine/Renderer/Rgba.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/String/StringUtils.hpp"



STATIC const Rgba Rgba::BLACK = Rgba( 0.f, 0.f, 0.f );
STATIC const Rgba Rgba::DARK_GRAY = Rgba( 0.27f, 0.27f, 0.27f );
STATIC const Rgba Rgba::GRAY = Rgba( 0.5f, 0.5f, 0.5f );
STATIC const Rgba Rgba::WHITE = Rgba( 1.f, 1.f, 1.f );
STATIC const Rgba Rgba::RED = Rgba( 1.f, 0.f, 0.f );
STATIC const Rgba Rgba::GREEN = Rgba( 0.f, 1.f, 0.f );
STATIC const Rgba Rgba::BLUE = Rgba( 0.f, 0.f, 1.f );
STATIC const Rgba Rgba::MAGENTA = Rgba( 1.f, 0.f, 1.f );
STATIC const Rgba Rgba::YELLOW = Rgba( 1.f, 1.f, 0.f );
STATIC const Rgba Rgba::CYAN = Rgba( 0.f, 1.f, 1.f );
STATIC const Rgba Rgba::HALF_OPAQUE_BLACK = Rgba( 0.f, 0.f, 0.f, 0.5f );


//--------------------------------------------------------------------------------------------------------------
Rgba::Rgba()
	: red( 255 )
	, green( 255 )
	, blue( 255 )
	, alphaOpacity( 255 )
{
}


//--------------------------------------------------------------------------------------------------------------
Rgba::Rgba( float red, float green, float blue, float alphaOpacity /*=1.f*/ )
{
	this->red = static_cast<byte_t>( red * 255.f );
	this->green = static_cast<byte_t>( green * 255.f );
	this->blue = static_cast<byte_t>( blue * 255.f );
	this->alphaOpacity = static_cast<byte_t>( alphaOpacity * 255.f );
}


//--------------------------------------------------------------------------------------------------------------
Rgba::Rgba( byte_t red, byte_t green, byte_t blue, byte_t alphaOpacity /*= 255 */ )
	: red( red )
	, blue( blue )
	, green( green )
	, alphaOpacity( alphaOpacity )	
{
}


//--------------------------------------------------------------------------------------------------------------
Rgba::Rgba( const Rgba& other )
	: red( other.red )
	, green( other.green )
	, blue( other.blue )
	, alphaOpacity( other.alphaOpacity )
{
}


//--------------------------------------------------------------------------------------------------------------
bool Rgba::operator==( const Rgba& compareTo ) const
{
	return ( red == compareTo.red ) && ( green == compareTo.green ) && ( blue == compareTo.blue );
}


//--------------------------------------------------------------------------------------------------------------
bool Rgba::operator!=( const Rgba& compareTo ) const
{
	return !operator==( compareTo );
}


//--------------------------------------------------------------------------------------------------------------
const Rgba Rgba::operator*( const Rgba& componentwiseScaleBy ) const
{
	//Convert argument to floats to scale by.
	Vector4f argumentAsFloats = componentwiseScaleBy.GetAsFloats();
	Vector4f thisAsFloats = GetAsFloats();

	return Rgba( argumentAsFloats * thisAsFloats );
}


//--------------------------------------------------------------------------------------------------------------
const Rgba Rgba::operator+( const Rgba& componentwiseAddBy ) const
{	
	Rgba out;
	out.red = static_cast<byte_t>( Clamp( red + componentwiseAddBy.red, 0, 255 ) );
	out.green = static_cast<byte_t>( Clamp( green + componentwiseAddBy.green, 0, 255 ) );
	out.blue = static_cast<byte_t>( Clamp( blue + componentwiseAddBy.blue, 0, 255 ) );
	out.alphaOpacity = static_cast<byte_t>( Clamp( alphaOpacity + componentwiseAddBy.alphaOpacity, 0, 255 ) );

	return out;
}


//--------------------------------------------------------------------------------------------------------------
Vector4f Rgba::GetAsFloats() const
{
	Vector4f colorAsFloats;

	colorAsFloats.x = red / 255.f;
	colorAsFloats.y = green / 255.f;
	colorAsFloats.z = blue / 255.f;
	colorAsFloats.w = alphaOpacity / 255.f;

	return colorAsFloats;
}


//--------------------------------------------------------------------------------------------------------------
Rgba Rgba::AverageColors( const Rgba& a, const Rgba& b )
{
	Vector4f aFloats = a.GetAsFloats();
	Vector4f bFloats = b.GetAsFloats();
	return Rgba( ( aFloats + bFloats ) * .5f );
}


//--------------------------------------------------------------------------------------------------------------
std::string Rgba::ToString() const
{
	return Stringf( "%hhu,%hhu,%hhu,%hhu", red, green, blue, alphaOpacity );
}
