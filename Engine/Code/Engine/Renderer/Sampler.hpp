#pragma once
#include <map>


enum SamplerFilter
{
	FILTER_NEAREST, //Closest Pixel
	FILTER_LINEAR, //Averages 4 Nearest Pixels
	FILTER_NEAREST_MIPMAP_NEAREST,
	FILTER_NEAREST_MIPMAP_LINEAR,
	FILTER_LINEAR_MIPMAP_NEAREST, //Blend two mipmaps using nearest.
	FILTER_LINEAR_MIPMAP_LINEAR,
	NUM_SAMPLER_FILTERS
};


enum SamplerWrapMode
{
	WRAP_REPEAT,
	WRAP_MIRRORED_REPEAT,
	WRAP_CLAMP_TO_EDGE,
	WRAP_CLAMP_TO_BORDER,
	NUM_SAMPLER_WRAP_MODES
};


struct SamplerOptions
{
	void Reset()
	{
		m_minFilter = SamplerFilter::FILTER_NEAREST_MIPMAP_LINEAR;
		m_magFilter = SamplerFilter::FILTER_LINEAR; //can only be linear or nearest
		m_uWrap = SamplerWrapMode::WRAP_REPEAT;
		m_vWrap = SamplerWrapMode::WRAP_REPEAT;
	}

	SamplerFilter m_minFilter;
	SamplerFilter m_magFilter;

	SamplerWrapMode m_uWrap;
	SamplerWrapMode m_vWrap;
};

//Not doing the CreateOrGet static registry design pattern because unlike Textures, a Sampler is just an inexpensive bundle of uint modes.
class Sampler
{
public:

	Sampler();
	Sampler( const SamplerOptions* options );
	Sampler( SamplerFilter minFilter, SamplerFilter magFilter, SamplerWrapMode uWrapMode, SamplerWrapMode vWrapMode );
	inline unsigned int GetSamplerID() const { return m_openglSamplerID; }
	inline void Reset() { m_samplerOptions.Reset(); }


private:

	unsigned int RegisterSampler( SamplerFilter minFilter, SamplerFilter magFilter, SamplerWrapMode uWrap, SamplerWrapMode vWrap ) const;

	//	unsigned int m_samplerType; //e.g. sampler2D in GLSL.
	unsigned int m_openglSamplerID;
	SamplerOptions m_samplerOptions;
};
