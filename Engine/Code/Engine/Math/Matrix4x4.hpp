#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"


enum Ordering : uint32_t { ROW_MAJOR, COLUMN_MAJOR, NUM_ORDERINGS };
	//Row-major: the first 4 values are the first row of the matrix. Multiply by vertex*(Model*View*Projection transforms) unless you transpose by sending GL_TRUE when setting MVP's uniforms.
	//Col-major: the first 4 values are the first col of the matrix. Multiply by (Model*View*Projection transforms)*vertex unless you transpose by sending GL_TRUE when setting MVP's uniforms.

template <typename T>
class Matrix4x4
{
public:
	//Be very careful about pushing matrices to the card when data members like this one imply a non-zero stride betweem matrices in an array.
	T m_data[ 16 ];


private:
	Ordering m_ordering;


public:
	Matrix4x4( Ordering ordering = ROW_MAJOR ) : m_ordering( ordering ) { SetAllValuesAssumingSameOrdering( Matrix4x4::IDENTITY ); }
	Matrix4x4( const T values[ 16 ], Ordering ordering = ROW_MAJOR ) : m_ordering( ordering ) { SetAllValuesAssumingSameOrdering( values ); }
	Matrix4x4( T in0, T in1, T in2, T in3, T in4, T in5, T in6, T in7, T in8, T in9, T in10, T in11, T in12, T in13, T in14, T in15, Ordering ordering = ROW_MAJOR );
	inline T& operator[]( int index );
	template <typename T> inline friend void mult( const Matrix4x4<T>& lhsRowMajor, const Matrix4x4<T>& rhsRowMajor, Matrix4x4<T>& res ); //Reverse args if col-major.
	inline Matrix4x4<T> operator*( const Matrix4x4<T>& rhs )
	{
		Matrix4x4<T> out( m_ordering );
		mult( *this, rhs, out );
		return out;
	}

	static const Matrix4x4<T> IDENTITY;

	Ordering GetOrdering() const { return m_ordering; }
	void ToggleOrdering() { m_ordering = ( m_ordering == ROW_MAJOR ) ? COLUMN_MAJOR : ROW_MAJOR; }


	//Packed based on the current value of m_ordering.
	void GetTranspose( Matrix4x4<T>& out_transpose ) const;
	void GetAllValues( Matrix4x4<T>& out_copy, Ordering* orderingReturned = nullptr ) const; //Will use its m_ordering if one isn't supplied. e.g. If given row-major, while currently col-major, will return transpose.
	void GetTranslation( Vector3<T>& out_translation ) const;
	void GetTranslation( Matrix4x4<T>& out_translation, bool makeOtherCellsIdentity = false, Ordering* ordering = nullptr ) const; //Will use its m_ordering if one isn't supplied.
	void GetBasisDirectionI( Vector3<T>& out_direction ) const;
	void GetBasisDirectionJ( Vector3<T>& out_direction ) const;
	void GetBasisDirectionK( Vector3<T>& out_direction ) const;
	void SetBasisDirectionI( const Vector3<T>& direction );
	void SetBasisDirectionJ( const Vector3<T>& direction );
	void SetBasisDirectionK( const Vector3<T>& direction );
	void GetBasis( Vector3<T>& out_iDir, Vector3<T>& out_jDir, Vector3<T>& out_kDir, Vector3<T>& out_translation ) const;
	void SetBasis( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, const Vector3<T>& translation, Ordering ordering );
	void GetRotation( Vector3<T>& out_iDir, Vector3<T>& out_jDir, Vector3<T>& out_kDir ) const;
	void GetRotation( Matrix4x4<T>& out_rotation, bool makeOtherCellsIdentity = false, Ordering* ordering = nullptr ) const; //Will use its m_ordering if one isn't supplied.
	void GetInverseAssumingOrthonormality( Matrix4x4<T>& out_inverse ) const;

	//Below methods WILL NOT affect any unparameterized cells.
	void Translate( const Vector3<T>& translation );
//	void RotateAroundRightAxisI( T degrees ); //TODO
//	void RotateAroundUpAxisJ( T degrees ); //TODO
//	void RotateAroundForwardAxisK( T degrees ); //TODO
	void Rotate( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering ); //No clamping or wrapping safeguards in place yet!
	
	template <typename VecType> Vector4<VecType> TransformVector( const Vector4<VecType>& v ) const;
	template <typename T> inline friend Matrix4x4<T> MatrixLerp( const Matrix4x4<T>& a, const Matrix4x4<T>& b, float t ); //Quaternion slerp if you use Quaternions.

	void SetRows( const Vector4<T>& firstRow, const Vector4<T>& secondRow, const Vector4<T>& thirdRow, const Vector4<T>& fourthRow );
	void SetToTranspose( bool toggleOrdering = false );
	void SetAllValuesAssumingSameOrdering( const T* inArray );
	void SetAllValuesAssumingSameOrdering( const Matrix4x4<T>& inMatrix );
	void SetScale( const Vector3<T>& scale );
	void SetRotation( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering );
	void SetTranslation( const Vector3<T>& translation, Ordering ordering ); //Only changes 3 values, use ClearToTranslationMatrix to blank rest.
	void SetToLookFrom( const Vector3<T>& translation, Ordering ordering );
	void SetToLookAt( const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering );
	void SetToLookAt( const Vector3<T>& fromPos, const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering );

	//Below methods WILL blank all unparameterized cells to Identity matrix values.
	void ClearToIdentityMatrix();
	void ClearToLookFromMatrix( const Vector3<T>& fromPos, Ordering ordering );
	void ClearToLookAtMatrix( const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering );
	void ClearToLookAtMatrix( const Vector3<T>& fromPos, const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering );
	void ClearToScaleMatrix( const Vector2<T>& scale );
	void ClearToScaleMatrix( const Vector3<T>& scale );
	void ClearToRotationMatrix( T angleDegrees, Ordering ordering ); //2D rotation or 3D rotation around forward k-axis.
	void ClearToRotationMatrix_MyBasis( T yaw, T pitch, T roll, Ordering ordering );
	void ClearToRotationMatrix_ForsethBasis( T yaw, T pitch, T roll, Ordering ordering );
	void ClearToRotationMatrix_ForsethBasis( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering );
	void ClearToTranslationMatrix( const Vector3<T>& translation, Ordering ordering );
	void ClearToPerspectiveProjection( const Matrix4x4<T>& changeOfBasis, T fovDegreesY, T aspect, T zNear, T zFar, Ordering ordering );
	void ClearToOrthogonalProjection( T width, T height, T zNear, T zFar, Ordering ordering ); //BOTTOM-LEFT IS (0,0) == OGL CENTER FOR SCREEN/NDC SPACE!
	void ClearToOrthogonalProjection( T xLeft, T xRight, T yBottom, T yTop, T zNear, T zFar, Ordering ordering );

	void PrintDebugMatrix( Ordering* orderingReturned = nullptr ) const; //Will use its m_ordering if one isn't supplied. e.g. If given row-major, while currently col-major, will return transpose.
};

typedef Matrix4x4<float> Matrix4x4f;
typedef Matrix4x4<double> Matrix4x4d;


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline void mult( const Matrix4x4<T>& whatToTransformIfRowMajor, const Matrix4x4<T>& whatToTransformByIfRowMajor, Matrix4x4<T>& out_result ) //Reverse args if col-major.
{
	const Matrix4x4<T>& l = whatToTransformIfRowMajor;
	const Matrix4x4<T>& r = whatToTransformByIfRowMajor;

	ASSERT_OR_DIE( &l != &out_result && &r != &out_result, "Cannot Mult-in-Place: Will Invalidate Entry Reuse in Later Multiplications!" );
	ASSERT_OR_DIE( l.GetOrdering() == r.GetOrdering(), "Cannot Mult Row-Major by Col-Major!" );

	//Credit to Guildhall's Math/Physics instructor, Anton Ephanov:

	//assume that the arguments are valid float[16] buffers
	out_result.m_data[ 0 ] = l.m_data[ 0 ] * r.m_data[ 0 ] + l.m_data[ 1 ] * r.m_data[ 4 ] + l.m_data[ 2 ] * r.m_data[ 8 ] + l.m_data[ 3 ] * r.m_data[ 12 ];
	out_result.m_data[ 1 ] = l.m_data[ 0 ] * r.m_data[ 1 ] + l.m_data[ 1 ] * r.m_data[ 5 ] + l.m_data[ 2 ] * r.m_data[ 9 ] + l.m_data[ 3 ] * r.m_data[ 13 ];
	out_result.m_data[ 2 ] = l.m_data[ 0 ] * r.m_data[ 2 ] + l.m_data[ 1 ] * r.m_data[ 6 ] + l.m_data[ 2 ] * r.m_data[ 10 ] + l.m_data[ 3 ] * r.m_data[ 14 ];
	out_result.m_data[ 3 ] = l.m_data[ 0 ] * r.m_data[ 3 ] + l.m_data[ 1 ] * r.m_data[ 7 ] + l.m_data[ 2 ] * r.m_data[ 11 ] + l.m_data[ 3 ] * r.m_data[ 15 ];

	out_result.m_data[ 4 ] = l.m_data[ 4 ] * r.m_data[ 0 ] + l.m_data[ 5 ] * r.m_data[ 4 ] + l.m_data[ 6 ] * r.m_data[ 8 ] + l.m_data[ 7 ] * r.m_data[ 12 ];
	out_result.m_data[ 5 ] = l.m_data[ 4 ] * r.m_data[ 1 ] + l.m_data[ 5 ] * r.m_data[ 5 ] + l.m_data[ 6 ] * r.m_data[ 9 ] + l.m_data[ 7 ] * r.m_data[ 13 ];
	out_result.m_data[ 6 ] = l.m_data[ 4 ] * r.m_data[ 2 ] + l.m_data[ 5 ] * r.m_data[ 6 ] + l.m_data[ 6 ] * r.m_data[ 10 ] + l.m_data[ 7 ] * r.m_data[ 14 ];
	out_result.m_data[ 7 ] = l.m_data[ 4 ] * r.m_data[ 3 ] + l.m_data[ 5 ] * r.m_data[ 7 ] + l.m_data[ 6 ] * r.m_data[ 11 ] + l.m_data[ 7 ] * r.m_data[ 15 ];

	out_result.m_data[ 8 ] = l.m_data[ 8 ] * r.m_data[ 0 ] + l.m_data[ 9 ] * r.m_data[ 4 ] + l.m_data[ 10 ] * r.m_data[ 8 ] + l.m_data[ 11 ] * r.m_data[ 12 ];
	out_result.m_data[ 9 ] = l.m_data[ 8 ] * r.m_data[ 1 ] + l.m_data[ 9 ] * r.m_data[ 5 ] + l.m_data[ 10 ] * r.m_data[ 9 ] + l.m_data[ 11 ] * r.m_data[ 13 ];
	out_result.m_data[ 10 ] = l.m_data[ 8 ] * r.m_data[ 2 ] + l.m_data[ 9 ] * r.m_data[ 6 ] + l.m_data[ 10 ] * r.m_data[ 10 ] + l.m_data[ 11 ] * r.m_data[ 14 ];
	out_result.m_data[ 11 ] = l.m_data[ 8 ] * r.m_data[ 3 ] + l.m_data[ 9 ] * r.m_data[ 7 ] + l.m_data[ 10 ] * r.m_data[ 11 ] + l.m_data[ 11 ] * r.m_data[ 15 ];

	out_result.m_data[ 12 ] = l.m_data[ 12 ] * r.m_data[ 0 ] + l.m_data[ 13 ] * r.m_data[ 4 ] + l.m_data[ 14 ] * r.m_data[ 8 ] + l.m_data[ 15 ] * r.m_data[ 12 ];
	out_result.m_data[ 13 ] = l.m_data[ 12 ] * r.m_data[ 1 ] + l.m_data[ 13 ] * r.m_data[ 5 ] + l.m_data[ 14 ] * r.m_data[ 9 ] + l.m_data[ 15 ] * r.m_data[ 13 ];
	out_result.m_data[ 14 ] = l.m_data[ 12 ] * r.m_data[ 2 ] + l.m_data[ 13 ] * r.m_data[ 6 ] + l.m_data[ 14 ] * r.m_data[ 10 ] + l.m_data[ 15 ] * r.m_data[ 14 ];
	out_result.m_data[ 15 ] = l.m_data[ 12 ] * r.m_data[ 3 ] + l.m_data[ 13 ] * r.m_data[ 7 ] + l.m_data[ 14 ] * r.m_data[ 11 ] + l.m_data[ 15 ] * r.m_data[ 15 ];
}


//--------------------------------------------------------------------------------------------------------------
template<typename T>
template<typename VecType>
inline Vector4<VecType> Matrix4x4<T>::TransformVector( const Vector4<VecType>& v ) const
{
	Vector4<VecType> result;

	//BUT REMEMBER, ORDERING IN MEMORY DOES NOT IMPLY MULTIPLY ON LEFT/RIGHT (basis handedness does it).
	if ( m_ordering == COLUMN_MAJOR ) //Treat vector as row vector, multiply M*v.
	{
		result.x = m_data[ 0 ] * v.x + m_data[ 1 ] * v.y + m_data[ 2 ] * v.z + m_data[ 3 ] * v.w;
		result.y = m_data[ 4 ] * v.x + m_data[ 5 ] * v.y + m_data[ 6 ] * v.z + m_data[ 7 ] * v.w;
		result.z = m_data[ 8 ] * v.x + m_data[ 9 ] * v.y + m_data[ 10 ] * v.z + m_data[ 11 ] * v.w;
		result.w = m_data[ 12 ] * v.x + m_data[ 13 ] * v.y + m_data[ 14 ] * v.z + m_data[ 15 ] * v.w;
	}
	else //ROW_MAJOR. Treat vector as column vector, multiply v*M.
	{
		result.x = m_data[ 0 ] * v.x + m_data[ 4 ] * v.y + m_data[ 8 ] * v.z + m_data[ 12 ] * v.w;
		result.y = m_data[ 1 ] * v.x + m_data[ 5 ] * v.y + m_data[ 9 ] * v.z + m_data[ 13 ] * v.w;
		result.z = m_data[ 2 ] * v.x + m_data[ 6 ] * v.y + m_data[ 10 ] * v.z + m_data[ 14 ] * v.w;
		result.w = m_data[ 3 ] * v.x + m_data[ 7 ] * v.y + m_data[ 11 ] * v.z + m_data[ 15 ] * v.w;
	}

	return result;
}


//--------------------------------------------------------------------------------------------------------------
template<typename T>
inline Matrix4x4<T> MatrixLerp( const Matrix4x4<T>& a, const Matrix4x4<T>& b, float t )
{
	ASSERT_OR_DIE( a.GetOrdering() == b.GetOrdering(), nullptr );

	Vector3<T> right0, up0, forward0, translation0; //i.e. row1, row2, row3, row4 if row-major. Each vector needs to get slerped between, but translation lerped.
	Vector3<T> right1, up1, forward1, translation1;

	a.GetBasis( right0, up0, forward0, translation0 );
	b.GetBasis( right1, up1, forward1, translation1 );

	Vector3<T> right, up, forward, translation;
	right = Slerp( right0, right1, t ); //Get formula online, takes an acos to work so it's slower but looks better. Quaternions is optimal though.
	up = Slerp( up0, up1, t );
	forward = Slerp( forward0, forward1, t );
	translation = Lerp( translation0, translation1, t );

	Matrix4x4<T> out( a.GetOrdering() );
	out.SetBasis( right, up, forward, translation, a.GetOrdering() );  //Sets rows if row-major, etc.
	return out;
}