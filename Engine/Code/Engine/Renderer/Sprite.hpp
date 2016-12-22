#pragma once


#include "Engine/EngineCommon.hpp"
#include "Engine/Renderer/Rgba.hpp"
class SpriteResource;
class Material;
class Texture;


//-----------------------------------------------------------------------------
struct Transform2D
{
	Vector2f m_position;
	Vector2f m_scale;
	float m_rotationAngleDegrees;
};


//-----------------------------------------------------------------------------
class Sprite
{
public:

	Transform2D m_transform;

	~Sprite();
	static Sprite* Create( ResourceID resourceID );
	static Sprite* Create( ResourceID resourceID, const Vector2f& worldPos );
	static Sprite* Create( ResourceID resourceID, const Vector2f& worldPos, const Rgba& tint );
	static Sprite* Create( Sprite* other );

	virtual void Update( float deltaSeconds ) { UNREFERENCED( deltaSeconds ); } //Overridden to update animations w/o requiring a separate container for them.

	ResourceID GetResourceID() const;
	Vector2f GetPivotSpriteRelative() const;
	float GetVirtualWidth() const { return m_virtualDimensions.x; }
	float GetVirtualHeight() const { return m_virtualDimensions.y; }
	AABB2f GetVirtualBoundsInWorld() const;
	Material* GetMaterial() const;
	Rgba GetTint() const { return m_tint; }
	void SetTint( const Rgba& newTint ) { m_tint = newTint; }
	unsigned int GetDiffuseTextureID() const;
	Texture* GetDiffuseTexture() const;
	Vector2f GetScale() const { return m_transform.m_scale; }
	float GetRotationDegrees() const { return m_transform.m_rotationAngleDegrees; }
	RenderLayerID GetLayerID() const { return m_layerID; }

	Matrix4x4f GetTransformSRT() const;

	void SetVirtualSize( float unitXY ) { SetVirtualSize( unitXY, unitXY ); }
	void SetVirtualSize( const Vector2f& newSize ) { SetVirtualSize( newSize.x, newSize.y ); }
	void SetVirtualSize( float unitX, float unitY );
	void SetLayerID( RenderLayerID newLayerID, const char* newName = nullptr );
	void SetParent( Sprite* newParent ) { m_parent = newParent; }
	void SetMaterial( Material* newMat ) { m_overrideMaterial = newMat; }

	bool IsEnabled() const { return m_enabled; }
	void Enable();
	void Disable();

	bool IsVisible() const { return m_shown; }
	void Show() { m_shown = true; }
	void Hide() { m_shown = false; }


protected:
	Sprite() : m_parent( nullptr ), m_spriteResource( nullptr ), m_overrideMaterial( nullptr ), m_enabled( false ) {}
	bool m_enabled; //.active in Unity, doesn't render when false.
	bool m_shown; //for visibility culling, even when enabled.
	Sprite* m_parent;
	RenderLayerID m_layerID;
	int m_spriteID; //Copy SD4 style.
	Vector2f m_virtualDimensions; //Because layers can have custom virtual sizes, 2 sprites can share a SpriteResource but need different virtual dimensions.
	Vector2f m_virtualPivot;
	Rgba m_tint;
	SpriteResource const* m_spriteResource;
	Material* m_overrideMaterial;

	static int s_BASE_SPRITE_ID;
};
