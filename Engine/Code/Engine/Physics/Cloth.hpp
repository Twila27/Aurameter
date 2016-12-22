#pragma once


#include "Engine/EngineCommon.hpp"


//-----------------------------------------------------------------------------
class EphanovParticle;


//-----------------------------------------------------------------------------
enum ConstraintType 
{
	STRETCH, 
	SHEAR, 
	BEND, 
	NUM_CONSTRAINT_TYPES
};


//-----------------------------------------------------------------------------
struct ClothConstraint
{
	ConstraintType type;
	EphanovParticle* const p1;
	EphanovParticle* const p2;
	double restDistance; //How far apart p1, p2 are when cloth at rest.
	ClothConstraint( ConstraintType type, EphanovParticle* const p1, EphanovParticle* const p2, double restDistance )
		: type( type ), p1( p1 ), p2( p2 ), restDistance( restDistance ) {}
};


//-----------------------------------------------------------------------------
class Cloth

{
public:
	//CONTRUCTORS//////////////////////////////////////////////////////////////////////////
	Cloth( const Vector3f& originTopLeftPosition,
		   EphanovParticleType EphanovParticleRenderType, float EphanovParticleMass, float EphanovParticleRadius,
		   int numRows, int numCols,
		   unsigned int numConstraintSolverIterations,
		   double baseDistanceBetweenEphanovParticles = 1.f,
		   double ratioDistanceStructuralToShear = sqrt( 2.f ),
		   double ratioDistanceStructuralToBend = 2.f,
		   const Vector3f& initialGlobalVelocity = Vector3f::ZERO )
		: m_originalTopLeftPosition( originTopLeftPosition )
		, m_currentTopLeftPosition( originTopLeftPosition )
		, m_numRows( numRows )
		, m_numCols( numCols )
		, m_numConstraintSolverIterations( numConstraintSolverIterations )
		, m_baseDistanceBetweenEphanovParticles( baseDistanceBetweenEphanovParticles )
		, m_ratioDistanceStructuralToShear( ratioDistanceStructuralToShear )
		, m_ratioDistanceStructuralToBend( ratioDistanceStructuralToBend )
		, m_EphanovParticleTemplate( EphanovParticleRenderType, EphanovParticleMass, -1.f, EphanovParticleRadius )
	{
		m_clothEphanovParticles.reserve( numRows * numCols );
		for ( int i = 0; i < numRows * numCols; i++ )
			m_clothEphanovParticles.push_back( EphanovParticle( EphanovParticleRenderType, EphanovParticleMass, 1.f, EphanovParticleRadius ) ); //Doesn't assign a dynamics state.
																											 //Needs a positive secondsToLive or else expiration logic will say it's already invisible/dead.

		AssignEphanovParticleStates( static_cast<float>( baseDistanceBetweenEphanovParticles ), originTopLeftPosition.y, initialGlobalVelocity );

		AddConstraints( baseDistanceBetweenEphanovParticles, ratioDistanceStructuralToShear, ratioDistanceStructuralToBend );

		GetEphanovParticle( 0, 0 )->SetIsPinned( true );
		GetEphanovParticle( 0, numCols - 1 )->SetIsPinned( true );
	}
	~Cloth() {
		for ( ClothConstraint* cc : m_clothConstraints ) delete cc;
	}

	//FUNCTIONS//////////////////////////////////////////////////////////////////////////
	EphanovParticle* const GetEphanovParticle( int rowStartTop, int colStartLeft )
	{
		if ( rowStartTop > m_numRows )
			return nullptr;
		if ( colStartLeft > m_numCols )
			return nullptr;
		return &m_clothEphanovParticles[ ( rowStartTop * m_numRows ) + colStartLeft ]; //Row-major.
	}

	//-----------------------------------------------------------------------------------
	bool IsDead() //Returns whether the corners still exist.
	{
		return GetEphanovParticle( 0, 0 )->IsExpired() && GetEphanovParticle( 0, m_numCols - 1 )->IsExpired();
	}

	//-----------------------------------------------------------------------------------
	float GetPercentageConstraintsLeft( ConstraintType constraintType = NUM_CONSTRAINT_TYPES )
	{
		return static_cast<float>( GetNumConstraints( constraintType ) ) / static_cast<float>( m_originalNumConstraints );
	}

	//-----------------------------------------------------------------------------------
	int GetNumConstraints( ConstraintType constraintType = NUM_CONSTRAINT_TYPES )
	{
		if ( constraintType == NUM_CONSTRAINT_TYPES )
			return static_cast<int>( m_clothConstraints.size() );

		int typeCount = 0;
		for ( ClothConstraint* cc : m_clothConstraints )
		{
			if ( constraintType == cc->type )
				++typeCount;
		}
		return typeCount;
	}

	//-----------------------------------------------------------------------------------
	void Update( float /*deltaSeconds*/ )
	{
		float fixedTimeStep = .001f;

		for ( int EphanovParticleIndex = 0; EphanovParticleIndex < m_numRows * m_numCols; EphanovParticleIndex++ )
			if ( ( m_clothEphanovParticles[ EphanovParticleIndex ].GetIsPinned() == false ) || m_clothEphanovParticles[ EphanovParticleIndex ].IsExpired() ) //What happens if you add if ( isExpired() ) ?
				m_clothEphanovParticles[ EphanovParticleIndex ].StepAndAge( fixedTimeStep );

		//In future could remove this to a RemoveConstraintForEphanovParticle(EphanovParticle* p) that finds and erases all constraints referencing p, to not loop per frame.
		for ( auto constraintIter = m_clothConstraints.begin(); constraintIter != m_clothConstraints.end(); )
		{
			ClothConstraint* cc = *constraintIter;
			if ( cc->p1->IsExpired() && cc->p2->IsExpired() ) // Tried to do || instead, creates awkward stretching...
			{
				constraintIter = m_clothConstraints.erase( constraintIter );
			}
			else ++constraintIter;
		}

		SatisfyConstraints( fixedTimeStep );

		//Old way of pinning the corners. Now handled by EphanovParticle::m_isPinned member to let you pin things arbitrarily.

		//		if ( GetEphanovParticle( 0, 0 )->IsExpired() == false )
		//			GetEphanovParticle( 0, 0 )->SetPosition( m_currentTopLeftPosition );
		//		if ( GetEphanovParticle( 0, m_numCols - 1 )->IsExpired() == false )
		//			GetEphanovParticle( 0, m_numCols - 1 )->SetPosition( CalcTopRightPosFromTopLeft() );
	}

	//-----------------------------------------------------------------------------------
	void Render( bool showCloth = true, bool showConstraints = false, bool showEphanovParticles = false )
	{
		//Render the cloth "fabric" by taking every 4 EphanovParticle positions (r,c) to (r+1,c+1) in to make a quad.
		Vector3f EphanovParticleStateTopLeft; //as 0,0 is top left. 
		Vector3f EphanovParticleStateTopRight;
		Vector3f EphanovParticleStateBottomLeft;
		Vector3f EphanovParticleStateBottomRight;
		AABB3f bounds;

		if ( showCloth )
		{
			for ( int r = 0; ( r + 1 ) < m_numRows; r++ )
			{
				for ( int c = 0; ( c + 1 ) < m_numCols; c++ )
				{
					if ( GetEphanovParticle( r, c )->IsExpired() && GetEphanovParticle( r, c )->IsExpired() && GetEphanovParticle( r, c )->IsExpired() && GetEphanovParticle( r, c )->IsExpired() )
						continue; //Don't draw a quad for a EphanovParticle that's been shot.

					GetEphanovParticle( r, c )->GetPosition( EphanovParticleStateTopLeft );
					GetEphanovParticle( r, c + 1 )->GetPosition( EphanovParticleStateTopRight );
					GetEphanovParticle( r + 1, c )->GetPosition( EphanovParticleStateBottomLeft );
					GetEphanovParticle( r + 1, c + 1 )->GetPosition( EphanovParticleStateBottomRight );

					Vector2f currentU = Vector2f::UNIT_X - ( Vector2f::UNIT_X * ( ( (float)( c + 1 ) / (float)( m_numCols - 1 ) ) ) );
					Vector2f currentV = Vector2f::UNIT_Y * ( (float)r / (float)( m_numRows - 1 ) );
					Vector2f nextU = Vector2f::UNIT_X - ( Vector2f::UNIT_X * ( ( (float)c / (float)( m_numCols - 1 ) ) ) );
					Vector2f nextV = Vector2f::UNIT_Y * ( (float)( r + 1 ) / (float)( m_numRows - 1 ) );
					Vertex3D_PCT quad[ 4 ] =
					{
						Vertex3D_PCT( EphanovParticleStateBottomLeft, Rgba::WHITE, nextU + nextV ),
						Vertex3D_PCT( EphanovParticleStateBottomRight, Rgba::WHITE, currentU + nextV ),
						Vertex3D_PCT( EphanovParticleStateTopRight, Rgba::WHITE, currentU + currentV ),
						Vertex3D_PCT( EphanovParticleStateTopLeft, Rgba::WHITE, nextU + currentV )
					};
					TODO( "Look into Anthony's TheRenderer:DrawVertexArray() to see how it handles drawing this with a texture!" );
					g_theRenderer->DrawVertexArray3D_PCT( TheRenderer::AS_QUADS, quad, 4 ); //Can't use AABB, cloth quads deform from being axis-aligned.
				}
			}
		}

		if ( showConstraints )
		{
			for ( ClothConstraint* cc : m_clothConstraints )
			{
				Vector3f EphanovParticlePosition1;
				Vector3f EphanovParticlePosition2;
				cc->p1->GetPosition( EphanovParticlePosition1 );
				cc->p2->GetPosition( EphanovParticlePosition2 );

				switch ( cc->type )
				{
				case STRETCH:	g_theRenderer->DrawLine( EphanovParticlePosition1, EphanovParticlePosition2, Rgba::RED ); break;
				case SHEAR:		g_theRenderer->DrawLine( EphanovParticlePosition1, EphanovParticlePosition2, Rgba::GREEN ); break;
				case BEND:		g_theRenderer->DrawLine( EphanovParticlePosition1, EphanovParticlePosition2, Rgba::BLUE ); break;
				}
			}
		}

		if ( !showEphanovParticles )
			return;

		for ( int EphanovParticleIndex = 0; EphanovParticleIndex < m_numRows * m_numCols; EphanovParticleIndex++ )
			if ( m_clothEphanovParticles[ EphanovParticleIndex ].IsExpired() == false )
				m_clothEphanovParticles[ EphanovParticleIndex ].Render();
	}

	//-----------------------------------------------------------------------------------
	inline void MoveClothByOffset( const Vector3f& offset )
	{
		for ( int c = 0; c < m_numCols; c++ )
		{
			Vector3f currentPosition;
			EphanovParticle* currentEphanovParticle = GetEphanovParticle( 0, c );
			currentEphanovParticle->GetPosition( currentPosition );
			currentEphanovParticle->SetPosition( currentPosition + offset );
		}
		GetEphanovParticle( 0, 0 )->GetPosition( m_currentTopLeftPosition );
	}

	//-----------------------------------------------------------------------------------
	inline Vector3f GetCurrentTopLeftPosition()
	{
		return m_currentTopLeftPosition;
	}

	//-----------------------------------------------------------------------------------
	inline Vector3f GetOriginalTopLeftPosition()
	{
		return m_originalTopLeftPosition;
	}

	//-----------------------------------------------------------------------------------
	inline void SetTopLeftPosition( const Vector3f& offset )
	{
		m_currentTopLeftPosition = offset;
	}

	//-----------------------------------------------------------------------------------
	void ResetForces( bool keepGravity = true )
	{
		for ( EphanovParticle& p : m_clothEphanovParticles )
		{
			p.ResetForces( keepGravity );
		}
	}

	//-----------------------------------------------------------------------------------
	void AddForce( Force* force )
	{
		EphanovParticle templateEphanovParticle = EphanovParticle( EphanovParticle_AABB3, 1.f, -1.f, 1.f );
		LinearDynamicsState* lds = new LinearDynamicsState();
		templateEphanovParticle.SetEphanovParticleState( lds );
		templateEphanovParticle.AddForce( force );

		for ( EphanovParticle& p : m_clothEphanovParticles )
			p.CloneForcesFromEphanovParticle( &templateEphanovParticle );
	}
	//-----------------------------------------------------------------------------------
	void RemoveAllConstraints()
	{
		for ( ClothConstraint* cc : m_clothConstraints )
		{
			if ( cc != nullptr )
				delete cc;
			cc = nullptr;
		}
		m_clothConstraints.clear();
	}

private:
	//-----------------------------------------------------------------------------------
	void AssignEphanovParticleStates( float baseDistance, float nonPlanarDepth, const Vector3f& velocity = Vector3f::ZERO ) //Note: 0,0 == top-left, so +x is right, +y is down.
	{
		//FORCES ASSIGNED HERE RIGHT NOW:
		LinearDynamicsState* lds = new LinearDynamicsState(); //Need its forces to stay valid over cloth lifetime, EphanovParticle will handle cleanup.
		m_EphanovParticleTemplate.SetEphanovParticleState( lds );
		//TheGame::instance->m_cloth->ResetForces( true );
		//TheGame::instance->m_cloth->AddForce( new ConstantWindForce( 5000.f, Vector3f::UP ) );
		m_EphanovParticleTemplate.AddForce( new GravityForce( 9.81f, Vector3f( 0, 0, -1 ) ) );
		//m_EphanovParticleTemplate.AddForce( new SpringForce( 0, Vector3f::ZERO, .72f, .72f ) );
		//m_EphanovParticleTemplate.AddForce( new ConstantWindForce( 30.f, Vector3f::UP ) );
		//m_EphanovParticleTemplate.AddForce( new WormholeForce( m_currentTopLeftPosition, 2.f, Vector3f::ONE ) );

		for ( int r = 0; r < m_numRows; r++ )
		{
			for ( int c = 0; c < m_numCols; c++ )
			{
				Vector3f startPosition( c * baseDistance, -r * baseDistance, nonPlanarDepth ); //BASIS CHANGE GOES HERE!
				startPosition += m_currentTopLeftPosition;
				EphanovParticle* const currentEphanovParticle = GetEphanovParticle( r, c );

				currentEphanovParticle->SetEphanovParticleState( new LinearDynamicsState( startPosition, velocity ) ); //EphanovParticle will handle state cleanup.
				currentEphanovParticle->CloneForcesFromEphanovParticle( &m_EphanovParticleTemplate );
			}
		}
	}

	//-----------------------------------------------------------------------------------
	Vector3f CalcTopRightPosFromTopLeft()
	{
		m_currentTopRightPosition = m_currentTopLeftPosition;
		m_currentTopRightPosition.x += ( ( m_numCols - 1 ) * static_cast<float>( m_baseDistanceBetweenEphanovParticles ) ); //Might need to change direction per engine basis.
		return m_currentTopRightPosition;
	}

	//-----------------------------------------------------------------------------------
	void SetDistancesForConstraints( ConstraintType affectedType, double newRestDistance )
	{
		for ( unsigned int constraintIndex = 0; constraintIndex < m_clothConstraints.size(); constraintIndex++ )
			if ( m_clothConstraints[ constraintIndex ]->type == affectedType )
				m_clothConstraints[ constraintIndex ]->restDistance = newRestDistance;
	}

	//-----------------------------------------------------------------------------------
	void AddConstraints( double baseDistance, double ratioStructuralToShear, double ratioStructuralToBend )
	{
		double shearDist = baseDistance * ratioStructuralToShear;
		double bendDist = baseDistance * ratioStructuralToBend;

		std::set<ClothConstraint*> tmpSet;

		for ( int r = 0; r < m_numRows; r++ )
		{
			for ( int c = 0; c < m_numCols; c++ )
			{
				if ( ( r + 1 ) < m_numRows )
					tmpSet.insert( new ClothConstraint( STRETCH, GetEphanovParticle( r, c ), GetEphanovParticle( r + 1, c ), baseDistance ) );
				if ( ( r - 1 ) >= 0 )
					tmpSet.insert( new ClothConstraint( STRETCH, GetEphanovParticle( r, c ), GetEphanovParticle( r - 1, c ), baseDistance ) );
				if ( ( c + 1 ) < m_numCols )
					tmpSet.insert( new ClothConstraint( STRETCH, GetEphanovParticle( r, c ), GetEphanovParticle( r, c + 1 ), baseDistance ) );
				if ( ( c - 1 ) >= 0 )
					tmpSet.insert( new ClothConstraint( STRETCH, GetEphanovParticle( r, c ), GetEphanovParticle( r, c - 1 ), baseDistance ) );

				if ( ( r + 1 ) < m_numRows && ( c + 1 ) < m_numCols )
					tmpSet.insert( new ClothConstraint( SHEAR, GetEphanovParticle( r, c ), GetEphanovParticle( r + 1, c + 1 ), shearDist ) );
				if ( ( r - 1 ) >= 0 && ( c + 1 ) < m_numCols )
					tmpSet.insert( new ClothConstraint( SHEAR, GetEphanovParticle( r, c ), GetEphanovParticle( r - 1, c + 1 ), shearDist ) );
				if ( ( r + 1 ) < m_numRows && ( c - 1 ) >= 0 )
					tmpSet.insert( new ClothConstraint( SHEAR, GetEphanovParticle( r, c ), GetEphanovParticle( r + 1, c - 1 ), shearDist ) );
				if ( ( r - 1 ) >= 0 && ( c - 1 ) >= 0 )
					tmpSet.insert( new ClothConstraint( SHEAR, GetEphanovParticle( r, c ), GetEphanovParticle( r - 1, c - 1 ), shearDist ) );


				if ( ( r + 2 ) < m_numRows )
					tmpSet.insert( new ClothConstraint( BEND, GetEphanovParticle( r, c ), GetEphanovParticle( r + 2, c ), bendDist ) );
				if ( ( r - 2 ) >= 0 )
					tmpSet.insert( new ClothConstraint( BEND, GetEphanovParticle( r, c ), GetEphanovParticle( r - 2, c ), bendDist ) );
				if ( ( c + 2 ) < m_numCols )
					tmpSet.insert( new ClothConstraint( BEND, GetEphanovParticle( r, c ), GetEphanovParticle( r, c + 2 ), bendDist ) );
				if ( ( c - 2 ) >= 0 )
					tmpSet.insert( new ClothConstraint( BEND, GetEphanovParticle( r, c ), GetEphanovParticle( r, c - 2 ), bendDist ) );
			}
		}

		//Now that we know it's duplicate-free, store in the vector to get the ability to index.
		for ( ClothConstraint* cc : tmpSet )
			m_clothConstraints.push_back( cc );

		m_originalNumConstraints = m_clothConstraints.size();
	}

	//-----------------------------------------------------------------------------------
	void SatisfyConstraints( float deltaSeconds )
	{
		double norm = 0.0;
		for ( unsigned int numIteration = 0; numIteration < m_numConstraintSolverIterations; ++numIteration )
		{
			for ( unsigned int constraintIndex = 0; constraintIndex < m_clothConstraints.size(); constraintIndex++ )
			{
				ClothConstraint* currentConstraint = m_clothConstraints[ constraintIndex ];

				Vector3f EphanovParticlePosition1;
				Vector3f EphanovParticlePosition2;

				currentConstraint->p1->GetPosition( EphanovParticlePosition1 );
				currentConstraint->p2->GetPosition( EphanovParticlePosition2 );

				Vector3f currentDisplacement = EphanovParticlePosition2 - EphanovParticlePosition1;

				if ( currentDisplacement == Vector3f::ZERO )
					continue; //Skip solving for a step.
				double currentDistance = currentDisplacement.CalcFloatLength();

				float stiffness = 100.f;
				Vector3f halfCorrectionVector = currentDisplacement * stiffness * static_cast<float>( 0.5 * ( 1.0 - ( currentConstraint->restDistance / currentDistance ) ) );
				// Note last term is ( currDist - currConstraint.restDist ) / currDist, just divided through.

				norm += ( currentConstraint->restDistance - currentDistance ) * ( currentConstraint->restDistance - currentDistance );

				//Move p2 towards p1 (- along halfVec), p1 towards p2 (+ along halfVec).
				bool isPinnedEphanovParticle1 = currentConstraint->p1->GetIsPinned();
				bool isPinnedEphanovParticle2 = currentConstraint->p2->GetIsPinned();

				// 				if ( isPinnedEphanovParticle1 && isPinnedEphanovParticle2 )
				// 				{
				// 					return; //Neither need correction.
				// 				}
				// 				if ( isPinnedEphanovParticle1 && !isPinnedEphanovParticle2 )
				// 				{
				// 					currentConstraint->p2->Translate( -halfCorrectionVector * 1.f * deltaSeconds ); //Have to cover the full correction with one EphanovParticle.
				// 					return;
				// 				}
				// 				if ( !isPinnedEphanovParticle1 && isPinnedEphanovParticle2 )
				// 				{
				// 					currentConstraint->p1->Translate( halfCorrectionVector * 1.f * deltaSeconds ); //Have to cover the full correction with one EphanovParticle.
				// 					return;
				// 				}

				//Neither point is pinned, correct as normal.
				if ( !isPinnedEphanovParticle1 )
					currentConstraint->p1->Translate( halfCorrectionVector * ( isPinnedEphanovParticle2 ? 2.f : 1.f ) * deltaSeconds );

				if ( !isPinnedEphanovParticle2 )
					currentConstraint->p2->Translate( -halfCorrectionVector * ( isPinnedEphanovParticle1 ? 2.f : 1.f ) *deltaSeconds );
			}
		}
		DebuggerPrintf( "Error: %f\n", norm );
	}

	//MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
	EphanovParticle m_EphanovParticleTemplate; //Without this and CloneForces, adding forces will crash when they go out of scope.
	Vector3f m_originalTopLeftPosition;
	Vector3f m_currentTopLeftPosition; //m_clothEphanovParticles[0,0].position: MOVE THIS WITH WASD TO MOVE PINNED CORNERS!
	Vector3f m_currentTopRightPosition; //Update whenever WASD event occurs as an optimization, else just recalculating per-tick.
	int m_numRows;
	int m_numCols;
	unsigned int m_numConstraintSolverIterations; //Affects soggy: more is less sag.
	unsigned int m_originalNumConstraints;

	//Ratios stored with class mostly for debugging. Or maybe use > these to tell when break a cloth constraint?
	double m_baseDistanceBetweenEphanovParticles;
	double m_ratioDistanceStructuralToShear;
	double m_ratioDistanceStructuralToBend;

	std::vector<ClothConstraint*> m_clothConstraints; //TODO: make c-style after getting fixed-size formula given cloth dims?
public:
	std::vector<EphanovParticle> m_clothEphanovParticles; //A 1D array, use GetEphanovParticle for 2D row-col interfacing accesses. Vector in case we want to push more at runtime.
};
