#include "Engine/Renderer/Sprite.hpp"
#include "Engine/Renderer/ResourceDatabase.hpp"
#include "Engine/Renderer/SpriteRenderer.hpp"
#include "Engine/Math/MatrixStack.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC int Sprite::s_BASE_SPRITE_ID = 1;


//--------------------------------------------------------------------------------------------------------------
Sprite::~Sprite()
{
	SpriteRenderer::Unregister( this );
}


//--------------------------------------------------------------------------------------------------------------
STATIC Sprite* Sprite::Create( ResourceID resourceID )
{
	return Sprite::Create( resourceID, Vector2f::ZERO );
}


//--------------------------------------------------------------------------------------------------------------
STATIC Sprite* Sprite::Create( ResourceID resourceID, const Vector2f& worldPos )
{
	return Sprite::Create( resourceID, worldPos, Rgba::WHITE );
}


//--------------------------------------------------------------------------------------------------------------
STATIC Sprite* Sprite::Create( ResourceID resourceID, const Vector2f& worldPos, const Rgba& tint )
{
	Sprite* s = new Sprite();
	s->m_spriteResource = ResourceDatabase::Instance()->GetSpriteResource( resourceID );
	ASSERT_RETURN( s->m_spriteResource );

	// If you enable it by default and are doing multi-threading, 
	// you'll get frames where the renderer's rendering it halfway through its initialization 
	// (if there are other init steps handled by other threads).
	s->m_spriteID = s_BASE_SPRITE_ID++;
	s->m_layerID = 0;
	s->m_overrideMaterial = nullptr;

	s->m_transform.m_position = worldPos;
	s->m_transform.m_scale = 1.f;
	s->m_transform.m_rotationAngleDegrees = 0.f;
	s->m_tint = tint;

	return s;
}


//--------------------------------------------------------------------------------------------------------------
STATIC Sprite* Sprite::Create( Sprite* other )
{
	if ( other == nullptr )
		return nullptr;

	return Sprite::Create( other->GetResourceID(), other->m_transform.m_position, other->GetTint() );
}


//--------------------------------------------------------------------------------------------------------------
ResourceID Sprite::GetResourceID() const
{
	return m_spriteResource->GetID();
}


//--------------------------------------------------------------------------------------------------------------
Vector2f Sprite::GetPivotSpriteRelative() const
{
	return m_virtualPivot;
}


//--------------------------------------------------------------------------------------------------------------
Matrix4x4f Sprite::GetTransformSRT() const
{
	Matrix4x4f S, R, T;

	S.ClearToScaleMatrix( GetScale() );
	FIXME( "Why do 2 of the axes to rotate around have no effect, using either 2D or 3D rotation matrix functions below?" );
	R.ClearToRotationMatrix( GetRotationDegrees() + SPRITE_ANGLE_CORRECTION, R.GetOrdering() );
//	R.ClearToRotationMatrix_MyBasis( GetRotationDegrees() + SPRITE_ANGLE_CORRECTION, 0.f, 0.f, R.GetOrdering() );
	T.ClearToTranslationMatrix( Vector3f( m_transform.m_position.x, m_transform.m_position.y, 0.f ), T.GetOrdering() );

	return S * R * T;
}


//--------------------------------------------------------------------------------------------------------------
AABB2f Sprite::GetVirtualBoundsInWorld() const
{
	//Maybe add static variables here to check against, and if the same, return a cached class member m_virtBounds?

	//Recall that position is the displacement from world origin to sprite pivot.
	AABB2f virtualBounds;

	Vector2f pivot = GetPivotSpriteRelative(); //Should be the center of the sprite.

	//If the position (offset between pivot and world origin) were (0,0), considering +x right, +y up in 2D:
	Vector2f originallyBottomLeftVert = -pivot;
	Vector2f originallyTopRightVert = Vector2f( GetVirtualWidth() - pivot.x, GetVirtualHeight() - pivot.y );
	//I say originally because, after rotation, it may not be as such anymore.

	//x, y, 0, 1.
	Vector4f BL = Vector4f( originallyBottomLeftVert.x, originallyBottomLeftVert.y, 0.f, 1.f );
	Vector4f TR = Vector4f( originallyTopRightVert.x, originallyTopRightVert.y, 0.f, 1.f );

	//-----------------------------------------------------------------------------

	if ( SpriteRenderer::IsParentingEnabled() && m_parent != nullptr )
	{
		Matrix4x4Stack matrixStack = Matrix4x4Stack( ROW_MAJOR ); //Match the below S, R matrices' ordering.

		//Walk up until an root/no-parent Sprite is found, then back down to this node, pushing onto the stack each step back down.
		std::vector<Sprite*> ancestry;
		Sprite* next = m_parent;
		int stepsToRoot = 1;
		while ( next != nullptr )
		{
			ancestry.push_back( next );
			++stepsToRoot;
			next = next->m_parent;
		}

		for ( std::vector<Sprite*>::reverse_iterator ancestorIter = ancestry.rbegin(); ancestorIter != ancestry.rend(); ++ancestorIter )
		{
			Sprite* ancestor = *ancestorIter;
			matrixStack.Push( ancestor->GetTransformSRT() );
		}

		Matrix4x4f top = matrixStack.Peek();
		BL = top.TransformVector( BL );
		TR = top.TransformVector( TR );
	}
	//final return value == vertex of child * top of matrix stack * child’s SRT.

	//-----------------------------------------------------------------------------

	//First scale, then rotate, then add position (world origin to sprite pivot, in virtual units).
	originallyBottomLeftVert = GetTransformSRT().TransformVector( BL ).xy(); //GOAL
	originallyTopRightVert = GetTransformSRT().TransformVector( TR ).xy(); //GOAL

	virtualBounds.mins = originallyBottomLeftVert;
	virtualBounds.maxs = originallyTopRightVert;

	return virtualBounds;
}


//--------------------------------------------------------------------------------------------------------------
Material* Sprite::GetMaterial() const
{
	if ( m_overrideMaterial == nullptr )
		return m_spriteResource->GetMaterial();
	else
		return m_overrideMaterial;
}


//--------------------------------------------------------------------------------------------------------------
Texture* Sprite::GetDiffuseTexture() const
{
	return m_spriteResource->GetDiffuseTexture();
}


//--------------------------------------------------------------------------------------------------------------
unsigned int Sprite::GetDiffuseTextureID() const
{
	return m_spriteResource->GetDiffuseTextureID();
}


//--------------------------------------------------------------------------------------------------------------
void Sprite::SetVirtualSize( float unitX, float unitY )
{
	m_virtualDimensions.x = unitX; 
	m_virtualDimensions.y = unitY;

	m_virtualPivot = m_virtualDimensions * 0.5f; //i.e. pivot == center by default.
}


//--------------------------------------------------------------------------------------------------------------
void Sprite::SetLayerID( RenderLayerID newLayerID, const char* newName /*= nullptr*/ )
{
	if ( m_layerID == newLayerID )
		return;

	if ( m_enabled )
	{
		SpriteRenderer::Unregister( this );
		m_layerID = newLayerID; //This line has to happen between Unregister and Register, since Unregister will look up the layer by its ID.
		SpriteRenderer::Register( this, newName );
	}
	else
	{
		m_layerID = newLayerID;
		SpriteRenderer::CreateOrGetLayer( newLayerID, newName );
	}
}


//--------------------------------------------------------------------------------------------------------------
void Sprite::Enable()
{
	if ( !m_enabled )
	{
		m_enabled = true;
		SpriteRenderer::Register( this );
	}
	else ERROR_RECOVERABLE( "Enabling an already enabled sprite!" );
}

//--------------------------------------------------------------------------------------------------------------
void Sprite::Disable()
{
	if ( m_enabled )
	{
		m_enabled = false;
		SpriteRenderer::Unregister( this );
	}
	else ERROR_RECOVERABLE( "Disabling an already disabled sprite!" );
}
