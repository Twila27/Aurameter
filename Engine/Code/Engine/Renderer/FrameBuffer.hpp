#pragma once
#include <vector>
#include "Engine/Renderer/Texture.hpp"
class Texture;
enum TextureFormat;

class FrameBuffer
{
public:

	FrameBuffer( size_t numColorRenderTargets, Texture** colorRenderTargets, Texture* depthStencilTarget = nullptr );
		//Can overload to take in a vector<Texture2D*> instead if you don't know the size, but you normally would, hence the flat C-style array.
	FrameBuffer( unsigned int width, unsigned int height, const std::vector<TextureFormat>& colorFormats, const TextureFormat* depthStencilFormat = nullptr );
	~FrameBuffer();
	
	unsigned int GetWidth() const { return m_pixelWidth; }
	unsigned int GetHeight() const { return m_pixelHeight; }
	unsigned int GetFrameBufferID() const { return m_fboID; }
	unsigned int GetNumColorRenderTargets() const { return m_colorRenderTargets.size(); }
	unsigned int GetColorTextureID( unsigned int colorRenderTargetIndex ) const { return m_colorRenderTargets[colorRenderTargetIndex]->GetTextureID(); }
	unsigned int GetDepthStencilTextureID() const { return m_depthStencilRenderTarget->GetTextureID(); }
	bool HasDepthStencilRenderTarget() const { return m_depthStencilRenderTarget != nullptr; }

private:

	unsigned int m_fboID;
	std::vector<Texture*> m_colorRenderTargets; //1+ of these.
	Texture* m_depthStencilRenderTarget;

	unsigned int m_pixelWidth;
	unsigned int m_pixelHeight;
};