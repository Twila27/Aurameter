#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/String/StringUtils.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
template <>
STATIC const Matrix4x4<int> Matrix4x4<int>::IDENTITY = Matrix4x4(
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
);
STATIC const Matrix4x4<float> Matrix4x4<float>::IDENTITY = Matrix4x4(
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
);
STATIC const Matrix4x4<double> Matrix4x4<double>::IDENTITY = Matrix4x4(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
);


//--------------------------------------------------------------------------------------------------------------
template <typename T>
Matrix4x4<T>::Matrix4x4( T in0, T in1, T in2, T in3, T in4, T in5, T in6, T in7, 
					  T in8, T in9, T in10, T in11, T in12, T in13, T in14, T in15, Ordering ordering /*= ROW_MAJOR*/ )
{
	if ( ordering == ROW_MAJOR )
	{
		m_data[ 0 ] = in0;		m_data[ 1 ] = in1;		m_data[ 2 ] = in2;		m_data[ 3 ] = in3;
		m_data[ 4 ] = in4;		m_data[ 5 ] = in5;		m_data[ 6 ] = in6;		m_data[ 7 ] = in7;
		m_data[ 8 ] = in8;		m_data[ 9 ] = in9;		m_data[ 10 ] = in10;	m_data[ 11 ] = in11;
		m_data[ 12 ] = in12;	m_data[ 13 ] = in13;	m_data[ 14 ] = in14;	m_data[ 15 ] = in15;
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		m_data[ 0 ] = in0;		m_data[ 1 ] = in4;		m_data[ 2 ] = in8;		m_data[ 3 ] = in12;
		m_data[ 4 ] = in1;		m_data[ 5 ] = in5;		m_data[ 6 ] = in9;		m_data[ 7 ] = in13;
		m_data[ 8 ] = in2;		m_data[ 9 ] = in6;		m_data[ 10 ] = in10;	m_data[ 11 ] = in14;
		m_data[ 12 ] = in3;		m_data[ 13 ] = in7;		m_data[ 14 ] = in11;	m_data[ 15 ] = in15;
	}

	m_ordering = ordering;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
inline T& Matrix4x4<T>::operator[]( int index )
{
	ASSERT_OR_DIE(index > -1 && index < 16, Stringf("Index %d into Matrix4x4::operator[] Out of Bounds", &index) );

	return m_data[ index ];
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetAllValues( Matrix4x4<T>& out_copy, Ordering* orderingReturned /*= nullptr*/ ) const
{
	const Ordering orderToUse = ( orderingReturned == nullptr ) ? m_ordering : *orderingReturned;

	if ( orderToUse == ROW_MAJOR )
	{
		if ( m_ordering == ROW_MAJOR )
			out_copy.SetAllValuesAssumingSameOrdering( this->m_data );
		else
			this->GetTranspose( out_copy );
	}
	if ( orderToUse == COLUMN_MAJOR )
	{
		if ( m_ordering == COLUMN_MAJOR )
			out_copy.SetAllValuesAssumingSameOrdering( this->m_data );
		else
			this->GetTranspose( out_copy );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetTranspose( Matrix4x4<T>& out_transpose ) const
{
	out_transpose.SetAllValuesAssumingSameOrdering( this->m_data );
	out_transpose.SetToTranspose();
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetTranslation( Vector3<T>& out_translation ) const
{
	if ( m_ordering == ROW_MAJOR ) //Implies translation == three bottom row cells, from the left.
	{
		out_translation.x = m_data[ 12 ];
		out_translation.y = m_data[ 13 ];
		out_translation.z = m_data[ 14 ];
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies translation == three right column cells, from the top.
	{
		out_translation.x = m_data[ 3 ];
		out_translation.y = m_data[ 7 ];
		out_translation.z = m_data[ 11 ];
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetTranslation( Matrix4x4<T>& out_translation, bool makeOtherCellsIdentity /*= false*/, Ordering* ordering /*= nullptr*/ ) const
{
	const Ordering orderToUse = ( ordering == nullptr ) ? m_ordering : *ordering;

	Vector3<T> currentTranslation;
	GetTranslation( currentTranslation );

	if ( makeOtherCellsIdentity )
		out_translation.ClearToTranslationMatrix( currentTranslation, orderToUse );
	else 
		out_translation.SetTranslation( currentTranslation, orderToUse );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetBasisDirectionI( Vector3<T>& out_direction ) const
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.i == first row.
	{
		out_direction.x = m_data[ 0 ];
		out_direction.y = m_data[ 1 ];
		out_direction.z = m_data[ 2 ];
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.i == first column.
	{
		out_direction.x = m_data[ 0 ];
		out_direction.y = m_data[ 4 ];
		out_direction.z = m_data[ 8 ];
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetBasisDirectionJ( Vector3<T>& out_direction ) const
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.j == second row.
	{
		out_direction.x = m_data[ 4 ];
		out_direction.y = m_data[ 5 ];
		out_direction.z = m_data[ 6 ];
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.j == second column.
	{
		out_direction.x = m_data[ 1 ];
		out_direction.y = m_data[ 5 ];
		out_direction.z = m_data[ 9 ];
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetBasisDirectionK( Vector3<T>& out_direction ) const
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.k == third row.
	{
		out_direction.x = m_data[ 8 ];
		out_direction.y = m_data[ 9 ];
		out_direction.z = m_data[ 10 ];
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.k == third column.
	{
		out_direction.x = m_data[ 2 ];
		out_direction.y = m_data[ 6 ];
		out_direction.z = m_data[ 10 ];
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetBasisDirectionI( const Vector3<T>& direction )
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.k == third row.
	{
		m_data[ 0 ] = direction.x;
		m_data[ 1 ] = direction.y;
		m_data[ 2 ] = direction.z;
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.k == third column.
	{
		m_data[ 0 ] = direction.x;
		m_data[ 4 ] = direction.y;
		m_data[ 8 ] = direction.z;
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetBasisDirectionJ( const Vector3<T>& direction )
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.k == third row.
	{
		m_data[ 4 ] = direction.x;
		m_data[ 5 ] = direction.y;
		m_data[ 6 ] = direction.z;
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.k == third column.
	{
		m_data[ 1 ] = direction.x;
		m_data[ 5 ] = direction.y;
		m_data[ 9 ] = direction.z;
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetBasisDirectionK( const Vector3<T>& direction )
{
	if ( m_ordering == ROW_MAJOR ) //Implies basis.k == third row.
	{
		m_data[ 8 ] = direction.x;
		m_data[ 9 ] = direction.y;
		m_data[ 10 ] = direction.z;
	}
	else if ( m_ordering == COLUMN_MAJOR ) //Implies basis.k == third column.
	{
		m_data[ 2 ] = direction.x;
		m_data[ 6 ] = direction.y;
		m_data[ 10 ] = direction.z;
	}
}



//--------------------------------------------------------------------------------------------------------------
template<typename T>
void Matrix4x4<T>::GetBasis( Vector3<T>& out_iDir, Vector3<T>& out_jDir, Vector3<T>& out_kDir, Vector3<T>& out_translation ) const
{
	GetBasisDirectionI( out_iDir );
	GetBasisDirectionJ( out_jDir );
	GetBasisDirectionK( out_kDir );
	GetTranslation( out_translation );
}


//--------------------------------------------------------------------------------------------------------------
template<typename T>
void Matrix4x4<T>::SetBasis( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, const Vector3<T>& translation, Ordering ordering )
{
	SetBasisDirectionI( iDir );
	SetBasisDirectionJ( jDir );
	SetBasisDirectionK( kDir );
	SetTranslation( translation, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetRotation( Vector3<T>& out_iDir, Vector3<T>& out_jDir, Vector3<T>& out_kDir ) const
{
	GetBasisDirectionI( out_iDir );
	GetBasisDirectionJ( out_jDir );
	GetBasisDirectionK( out_kDir );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetRotation( Matrix4x4<T>& out_rotation, bool makeOtherCellsIdentity /*= false*/, Ordering* ordering /*= nullptr*/ ) const
{
	const Ordering orderToUse = ( ordering == nullptr ) ? m_ordering : *ordering;

	Vector3<T> iDir;
	Vector3<T> jDir;
	Vector3<T> kDir;
	GetRotation( iDir, jDir, kDir );

	if ( makeOtherCellsIdentity )
		out_rotation.ClearToRotationMatrix_ForsethBasis( iDir, jDir, kDir, orderToUse );
	else
		out_rotation.SetRotation( iDir, jDir, kDir, orderToUse );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::GetInverseAssumingOrthonormality( Matrix4x4<T>& out_inverse ) const
{	
	Vector3<T> currentTranslationToInvertByNegation; //Inverse of translation == translation * -1.
	GetTranslation( currentTranslationToInvertByNegation );
	currentTranslationToInvertByNegation = -currentTranslationToInvertByNegation;

	Matrix4x4<T> invertedTranslationToRotate( m_ordering );
	invertedTranslationToRotate.SetTranslation( currentTranslationToInvertByNegation, m_ordering );

	Matrix4x4<T> rotationToInvertViaTranspose( m_ordering ); //Inverse of rotation == rotation^T.
	GetRotation( rotationToInvertViaTranspose );
	rotationToInvertViaTranspose.SetToTranspose();

	//(Rc*Tc)^-1 == (Tc^-1)*(Rc^-1) == (-Tc)*(Rc^T), i.e. transform the translation by the rotation.
	if ( m_ordering == ROW_MAJOR )
		mult( invertedTranslationToRotate, rotationToInvertViaTranspose, out_inverse );
	else if ( m_ordering == COLUMN_MAJOR )
		mult( rotationToInvertViaTranspose, invertedTranslationToRotate, out_inverse ); 
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::Translate( const Vector3<T>& translation )
{
	if ( m_ordering == ROW_MAJOR )
	{
		m_data[ 12 ] += translation.x;
		m_data[ 13 ] += translation.y;
		m_data[ 14 ] += translation.z;
	}
	else if ( m_ordering == COLUMN_MAJOR )
	{
		m_data[ 3 ] += translation.x;
		m_data[ 7 ] += translation.y;
		m_data[ 11 ] += translation.z;
	}
}


//--------------------------------------------------------------------------------------------------------------
//template <typename T>
// void Matrix4x4<T>::RotateAroundRightAxisI( T degrees )
// {
// 	//Compute new unsigned i, j, k vectors using canonical rotation matrix.
// 	T sinOfAngle = SinDegrees( degrees );
// 	T cosOfAngle = CosDegrees( degrees );
// 
// 	Vector3<T> iDir = Vector3f::UNIT_X;
// 	Vector3<T> jDir = Vector3( 0, cosOfAngle, sinOfAngle );
// 	Vector3<T> kDir = Vector3( 0, sinOfAngle, cosOfAngle );
// 
// 	//Ordering determines where the canonical rotation matrix's negative goes.
// 	if ( m_ordering == ROW_MAJOR )
// 		kDir.y = -kDir.y;
// 	else if ( m_ordering == COLUMN_MAJOR )
// 		jDir.z = -jDir.z;
// 
// 	Rotate( iDir, jDir, kDir );
// }
//
//
//--------------------------------------------------------------------------------------------------------------
//template <typename T>
// void Matrix4x4<T>::RotateAroundUpAxisJ( T degrees )
// {
// 	//Compute new unsigned i, j, k vectors using canonical rotation matrix.
// 	T sinOfAngle = SinDegrees( degrees );
// 	T cosOfAngle = CosDegrees( degrees );
// 
// 	Vector3<T> iDir = Vector3( cosOfAngle,	0, sinOfAngle );
// 	Vector3<T> jDir = Vector3f::UNIT_Y;
// 	Vector3<T> kDir = Vector3( sinOfAngle, 0, cosOfAngle );
// 
// 	//Ordering determines where the canonical rotation matrix's negative goes.
// 	if ( m_ordering == ROW_MAJOR )
// 		iDir.z = -iDir.z;
// 	else if ( m_ordering == COLUMN_MAJOR )
// 		kDir.x = -kDir.x;
// 
// 	Rotate( iDir, jDir, kDir );
// }
//
//
//--------------------------------------------------------------------------------------------------------------
//template <typename T>
// void Matrix4x4::RotateAroundForwardAxisK( T degrees )
// {
// 	//Compute new unsigned i, j, k vectors using canonical rotation matrix.
// 	T sinOfAngle = SinDegrees( degrees );
// 	T cosOfAngle = CosDegrees( degrees );
// 
// 	Vector3<T> iDir = Vector3( cosOfAngle, sinOfAngle, 0 );
// 	Vector3<T> jDir = Vector3( sinOfAngle, cosOfAngle, 0 );
// 	Vector3<T> kDir = Vector3f::UNIT_Z;
// 
// 	//Ordering determines where the canonical rotation matrix's negative goes.
// 	if ( m_ordering == ROW_MAJOR )
// 		jDir.x = -jDir.x;
// 	else if ( m_ordering == COLUMN_MAJOR )
// 		iDir.y = -iDir.y;
// 
// 	Rotate( iDir, jDir, kDir );
// }


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::Rotate( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering )
{
	//Create a matrix out of the arguments, transform *this matrix by it.
	Matrix4x4<T> rotationTransform( ordering );
	Matrix4x4<T> product( ordering );

	rotationTransform.SetRotation( iDir, jDir, kDir, ordering );

	mult( *this, rotationTransform, product );

	this->SetAllValuesAssumingSameOrdering( product );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetRows( const Vector4<T>& firstRow, const Vector4<T>& secondRow, const Vector4<T>& thirdRow, const Vector4<T>& fourthRow )
{
	{
		if ( m_ordering == ROW_MAJOR )
		{
			m_data[ 0 ] = firstRow.x;
			m_data[ 1 ] = firstRow.y;
			m_data[ 2 ] = firstRow.z;
			m_data[ 3 ] = firstRow.w;

			m_data[ 4 ] = secondRow.x;
			m_data[ 5 ] = secondRow.y;
			m_data[ 6 ] = secondRow.z;
			m_data[ 7 ] = secondRow.w;

			m_data[ 8 ] = thirdRow.x;
			m_data[ 9 ] = thirdRow.y;
			m_data[ 10 ] = thirdRow.z;
			m_data[ 11 ] = thirdRow.w;

			m_data[ 12 ] = fourthRow.x;
			m_data[ 13 ] = fourthRow.y;
			m_data[ 14 ] = fourthRow.z;
			m_data[ 15 ] = fourthRow.w;
		}
		else //COLUMN_MAJOR
		{

			m_data[ 0 ] = firstRow.x;
			m_data[ 4 ] = firstRow.y;
			m_data[ 8 ] = firstRow.z;
			m_data[ 12 ] = firstRow.w;

			m_data[ 1 ] = secondRow.x;
			m_data[ 5 ] = secondRow.y;
			m_data[ 9 ] = secondRow.z;
			m_data[ 13 ] = secondRow.w;

			m_data[ 2 ] = thirdRow.x;
			m_data[ 6 ] = thirdRow.y;
			m_data[ 10 ] = thirdRow.z;
			m_data[ 14 ] = thirdRow.w;

			m_data[ 3 ] = fourthRow.x;
			m_data[ 7 ] = fourthRow.y;
			m_data[ 11 ] = fourthRow.z;
			m_data[ 15 ] = fourthRow.w;
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetToTranspose( bool toggleOrdering /*= false*/ )
{
	//Inner 3x3.
	std::swap( m_data[ 1 ], m_data[ 4 ] );
	std::swap( m_data[ 2 ], m_data[ 8 ] );
	std::swap( m_data[ 6 ], m_data[ 9 ] );

	//Outermost.
	std::swap( m_data[ 12 ], m_data[ 3 ] );
	std::swap( m_data[ 13 ], m_data[ 7 ] );
	std::swap( m_data[ 14 ], m_data[ 11 ] );

	if ( toggleOrdering )
		m_ordering = ( m_ordering == ROW_MAJOR ) ? COLUMN_MAJOR : ROW_MAJOR;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetAllValuesAssumingSameOrdering( const Matrix4x4<T>& inMatrix )
{
	inMatrix.GetAllValues( *this ); //Packs inMatrix's values INTO *this.
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetAllValuesAssumingSameOrdering( const T* inArray )
{
	m_data[ 0 ] = inArray[ 0 ];		m_data[ 1 ] = inArray[ 1 ];		m_data[ 2 ] = inArray[ 2 ];		m_data[ 3 ] = inArray[ 3 ];
	m_data[ 4 ] = inArray[ 4 ];		m_data[ 5 ] = inArray[ 5 ];		m_data[ 6 ] = inArray[ 6 ];		m_data[ 7 ] = inArray[ 7 ];
	m_data[ 8 ] = inArray[ 8 ];		m_data[ 9 ] = inArray[ 9 ];		m_data[ 10 ] = inArray[ 10 ];	m_data[ 11 ] = inArray[ 11 ];
	m_data[ 12 ] = inArray[ 12 ];	m_data[ 13 ] = inArray[ 13 ];	m_data[ 14 ] = inArray[ 14 ];	m_data[ 15 ] = inArray[ 15 ];
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetTranslation( const Vector3<T>& translation, Ordering ordering )
{
	if ( ordering == ROW_MAJOR )
	{
		m_data[ 12 ] = translation.x;
		m_data[ 13 ] = translation.y;
		m_data[ 14 ] = translation.z;
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		m_data[ 3 ] = translation.x;
		m_data[ 7 ] = translation.y;
		m_data[ 11 ] = translation.z;
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetRotation( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering )
{
	if ( ordering == ROW_MAJOR )
	{
		m_data[ 0 ] = iDir.x;	m_data[ 1 ] = iDir.y;	m_data[ 2 ] = iDir.z;
		m_data[ 4 ] = jDir.x;	m_data[ 5 ] = jDir.y;	m_data[ 6 ] = jDir.z;
		m_data[ 8 ] = kDir.x;	m_data[ 9 ] = kDir.y;	m_data[ 10 ] = kDir.z;
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		m_data[ 0 ] = iDir.x;	m_data[ 1 ] = jDir.x;	m_data[ 2 ] = kDir.x;
		m_data[ 4 ] = iDir.y;	m_data[ 5 ] = jDir.y;	m_data[ 6 ] = kDir.y;
		m_data[ 8 ] = iDir.z;	m_data[ 9 ] = jDir.z;	m_data[ 10 ] = kDir.z;
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetScale( const Vector3<T>& scale )
{
	m_data[ 0 ] = scale.x;
	m_data[ 5 ] = scale.y;
	m_data[ 10 ] = scale.z;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetToLookFrom( const Vector3<T>& translation, Ordering ordering )
{
	SetTranslation( translation, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetToLookAt( const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering )
{
	//Note the current matrix translation is being used here, since we have no parametric fromPos.
	Vector3<T> currentTranslation;
	this->GetTranslation( currentTranslation );
	SetToLookAt( currentTranslation, targetPos, globalUpDir, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::SetToLookAt( const Vector3<T>& fromPos, const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering )
{
//	Vector3<T> z = ( eye - at ).Normalized();  // Forward
//	Vector3<T> x = up.Cross( z ).Normalized(); // Right
//	Vector3<T> y = z.Cross( x );
//
//	Matrix4 m( x.x, x.y, x.z, -( x.Dot( eye ) ),
//			   y.x, y.y, y.z, -( y.Dot( eye ) ),
//			   z.x, z.y, z.z, -( z.Dot( eye ) ),
//			   0, 0, 0, 1 );
//	return m;

	Vector3<T> localForward = ( targetPos - fromPos );
	localForward.Normalize();

	Vector3<T> worldUp = globalUpDir;
	worldUp.Normalize();

	Vector3<T> localRight = CrossProduct( localForward, worldUp );
	localRight.Normalize();

	Vector3<T> localUp = CrossProduct( localRight, localForward );



//	//LookAt.
//	Vector3<T> localForward = ( targetPos - fromPos );
//	localForward.Normalize();
//
//	Vector3<T> worldUp = globalUpDir;
//	worldUp.Normalize();
//
//	Vector3<T> localRight = CrossProduct( localForward, worldUp ); //Reverse the operand order if it needs to be localLeft instead of right.
//	localRight.Normalize();
//
//	Vector3<T> localUp = CrossProduct( localRight, localForward );
//
	SetRotation( localRight, localUp, -localForward, ordering );
//
//	//LookFrom == translation stored lower right.
	SetTranslation( fromPos * -1.f, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToIdentityMatrix()
{
	m_data[ 0 ] = 1.0;	m_data[ 1 ] = 0.0;	m_data[ 2 ] = 0.0;	m_data[ 3 ] = 0.0;
	m_data[ 4 ] = 0.0;	m_data[ 5 ] = 1.0;	m_data[ 6 ] = 0.0;	m_data[ 7 ] = 0.0;
	m_data[ 8 ] = 0.0;	m_data[ 9 ] = 0.0;	m_data[ 10 ] = 1.0; m_data[ 11 ] = 0.0;
	m_data[ 12 ] = 0.0;	m_data[ 13 ] = 0.0; m_data[ 14 ] = 0.0; m_data[ 15 ] = 1.0;
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToTranslationMatrix( const Vector3<T>& translation, Ordering ordering )
{
	if ( ordering == ROW_MAJOR )
	{
		const float values[ 16 ] = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			translation.x, translation.y, translation.z, 1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const float values[ 16 ] = {
			1.f, 0.f, 0.f, translation.x,
			0.f, 1.f, 0.f, translation.y,
			0.f, 0.f, 1.f, translation.z,
			0.f, 0.f, 0.f, 1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToRotationMatrix( T angleDegrees, Ordering ordering ) //2D rotation or 3D rotation around forward k-axis.
{
	T sinDeg = SinDegrees( static_cast<T>( angleDegrees ) );
	T cosDeg = CosDegrees( static_cast<T>( angleDegrees ) );

	//Rotation X
	T values[ 16 ] =
	{
		1.f, 0.f,	 0.f, 0.f,
		0.f, cosDeg, sinDeg, 0.f,
		0.f, sinDeg, cosDeg, 0.f,
		0.f, 0.f,	 0.f, 1.f
	};

	//All that differs with matrix ordering is the negative on the sine.
	if ( ordering == ROW_MAJOR )
	{
		values[ 9 ] *= -1.f;
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		values[ 6 ] *= -1.f;
		SetAllValuesAssumingSameOrdering( values );
	}

	//Rotation Y
// 	T values[ 16 ] =
// 	{
// 		cosDeg, 0.f, sinDeg, 0.f,
// 		0.f,	1.f, 0.f, 0.f,
// 		sinDeg,	0.f, cosDeg, 0.f,
// 		0.f,	0.f, 0.f, 1.f
// 	};
// 
// 	//All that differs with matrix ordering is the negative on the sine.
// 	if ( ordering == ROW_MAJOR )
// 	{
// 		values[ 8 ] *= -1.f;
// 		SetAllValuesAssumingSameOrdering( values );
// 	}
// 	else if ( ordering == COLUMN_MAJOR )
// 	{
// 		values[ 2 ] *= -1.f;
// 		SetAllValuesAssumingSameOrdering( values );
// 	}

	//Rotation Z
// 	T values[ 16 ] =
// 	{
// 		cosDeg, sinDeg, 0.f, 0.f,
// 		sinDeg, cosDeg, 0.f, 0.f,
// 		0.f, 0.f, 1.f, 0.f,
// 		0.f, 0.f, 0.f, 1.f
// 	};
// 
// 	//All that differs with matrix ordering is the negative on the sine.
// 	if ( ordering == ROW_MAJOR )
// 	{
// 		values[ 4 ] *= -1.f;
// 		SetAllValuesAssumingSameOrdering( values );
// 	}
// 	else if ( ordering == COLUMN_MAJOR )
// 	{
// 		values[ 1 ] *= -1.f;
// 		SetAllValuesAssumingSameOrdering( values );
// 	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToRotationMatrix_MyBasis( T yaw, T pitch, T roll, Ordering ordering )
{
	//Applies a C*R*C^-1 detoured version of the Forseth rotation transform.

	T sx = SinDegrees( static_cast<T>( pitch ) ); //assumes pitch == around i-vector == x
	T cx = CosDegrees( static_cast<T>( pitch ) );

	T sy = SinDegrees( static_cast<T>( yaw ) ); //assumes yaw == around j-vector == y
	T cy = CosDegrees( static_cast<T>( yaw ) );

	T sz = SinDegrees( static_cast<T>( roll ) ); //assumes roll == around k-vector == z
	T cz = CosDegrees( static_cast<T>( roll ) );

	if ( ordering == ROW_MAJOR )
	{
		const T values[ 16 ] =
		{
			cx*cz,					sx,						-cx*sz,						0, //i(-g)h
			-( -sy*sz - cy*cz*sx ),	cx*cy,					-cz*sy + cy*sx*sz,			0, //(-c)a(-b)
			-( cy*sz - cz*sx*sy ),	cx*sy,					cy*cz + sx*sy*sz,			0, //f(-d)e
			0,						0,						0,							1
		};
// 		const float values[ 16 ] = //Forseth's hand-computed x-then-y-then-z rotation transform composition.
// 		{
// 			cy*cz + sx*sy*sz,		-cx*sz,				-cz*sy + cy*sx*sz,			0.f,
// 			cy*sz - cz*sx*sy,		cx*cz,				-sy*sz - cy*cz*sx,			0.f,
// 			cx*sy,					sx,					cx*cy,						0.f,
// 			0.f,					0.f,				0.f,						1.0f
// 		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const T values[ 16 ] =
		{
			cx*cy,					-( -cz*sy + cy*sx*sz ),	-sy*sz - cy*cz*sx,			0, //i(-g)h
			-( cx*sy ),				cy*cz + sx*sy*sz,		-( cy*sz - cz*sx*sy ),		0, //(-c)a(-b)
			sx,						-( cx*sz ),				cx*cz,						0, //f(-d)e
			0,						0,						0,							1
		};

// 		const float values[ 16 ] =
// 		{
// 			cy*cz + sx*sy*sz,	cy*sz - cz*sx*sy,		cx*sy,						0.f, //abc
// 			-cx*sz,				cx*cz,					sx,							0.f, //def
// 			-cz*sy + cy*sx*sz,	-sy*sz - cy*cz*sx,		cx*cy,						0.f, //ghi
// 			0.0f,				0.0f,					0.0f,						1.0f
// 		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToRotationMatrix_ForsethBasis( T yaw, T pitch, T roll, Ordering ordering )
{
	//EulerAngles angles = EulerAngles( roll, pitch, yaw ); //See this class for my engine conventions.

	//But below still uses the variable names from Forseth's left-handed y-up code. For him, x,y,z == pitch, yaw, roll
	T sx = SinDegrees( static_cast<T>( pitch ) ); //assumes pitch == around i-vector == x
	T cx = CosDegrees( static_cast<T>( pitch ) );

	T sy = SinDegrees( static_cast<T>( yaw ) ); //assumes yaw == around j-vector == y
	T cy = CosDegrees( static_cast<T>( yaw ) );

	T sz = SinDegrees( static_cast<T>( roll ) ); //assumes roll == around k-vector == z
	T cz = CosDegrees( static_cast<T>( roll ) );
	 
	if ( ordering == ROW_MAJOR )
	{
// 		const T values[ 16 ] = { //My hand-computed x-then-y-then-z rotation transform composition.
// 			cy*cz,					sx*sy*cz + cx*sz,		-cx*sy*cz + sx*sz,			0.f,
// 			-cy*sz,					-sx*sy*sz + cx*cz,		cx*sy*sz + sx*cz,			0.f,
// 			sy,						-sx*cy,					cx*cy,						0.f,
// 			0.f,					0.f,					0.f,						1.f
// 		};
		const T values[16] = //Forseth's hand-computed x-then-y-then-z rotation transform composition.
		{
			cy*cz + sx*sy*sz,		-cx*sz,				-cz*sy + cy*sx*sz,			0,
			cy*sz - cz*sx*sy,		cx*cz,				-sy*sz - cy*cz*sx,			0,
			cx*sy,					sx,					cx*cy,						0,
			0,						0,					0,							1
		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
// 		const float values[ 16 ] = {
// 			cy*cz,				-cy*sz,					sy,			0.f,
// 			sx*sy*cz + cx*sz,	-sx*sy*sz + cx*cz,		-sx*cy,		0.f,
// 			-cx*sy*cz + sx*sz,	cx*sy*sz + sx*cz,		cx*cy,		0.f,
// 			0.f,				0.f,					0.f,		1.f
// 		};
		const T values[16] =
		{
			cy*cz + sx*sy*sz,	cy*sz - cz*sx*sy,		cx*sy,						0,
			-cx*sz,				cx*cz,					sx,							0,
			-cz*sy + cy*sx*sz,	-sy*sz - cy*cz*sx,		cx*cy,						0,
			0,						0,					0,							1
		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToRotationMatrix_ForsethBasis( const Vector3<T>& iDir, const Vector3<T>& jDir, const Vector3<T>& kDir, Ordering ordering )
{
	if ( ordering == ROW_MAJOR )
	{
		const T values[16] = {
			iDir.x,		iDir.y,		iDir.z,		0.f,
			jDir.x,		jDir.y,		jDir.z,		0.f,
			kDir.x,		kDir.y,		kDir.z,		0.f,
			0.f,		0.f,		0.f,		1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const T values[16] = {
			iDir.x,		jDir.x,		kDir.x,		0.f,
			iDir.y,		jDir.y,		kDir.y,		0.f,
			iDir.z,		jDir.z,		kDir.z,		0.f,
			0.f,		0.f,		0.f,		1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToScaleMatrix( const Vector2<T>& scale )
{
	const T values[ 16 ] = {
		scale.x, 0.f, 0.f, 0.f,
		0.f, scale.y, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	SetAllValuesAssumingSameOrdering( values );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToScaleMatrix( const Vector3<T>& scale )
{
	const T values[ 16 ] = {
		scale.x, 0.f, 0.f, 0.f,
		0.f, scale.y, 0.f, 0.f,
		0.f, 0.f, scale.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
	SetAllValuesAssumingSameOrdering( values );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToLookFromMatrix( const Vector3<T>& fromPos, Ordering ordering )
{
	ClearToTranslationMatrix( fromPos, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToLookAtMatrix( const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering )
{
	//Note the current matrix translation is being used here, since we have no parametric fromPos.
	Vector3<T> currentTranslation;
	this->GetTranslation( currentTranslation );
	ClearToLookAtMatrix( currentTranslation, targetPos, globalUpDir, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToLookAtMatrix( const Vector3<T>& fromPos, const Vector3<T>& targetPos, const Vector3<T>& globalUpDir, Ordering ordering )
{
	ClearToIdentityMatrix();
	SetToLookAt( fromPos, targetPos, globalUpDir, ordering );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToPerspectiveProjection( const Matrix4x4<T>& changeOfBasis, T fovDegreesY, T aspect, T zNear, T zFar, Ordering ordering ) //Projects things into not a box (as in ortho) but a frustum. Breaks if zNear <= 0. NOTE INVERTED Z!
{
	ASSERT_OR_DIE( zNear > 0, "Found Unallowed Non-positive zNear!" );

	//Create the view frustum bounds.
	Matrix4x4<T> perspectiveProjection( ordering );
	float rads = ConvertDegreesToRadians( fovDegreesY );
	float size = 1.f / tan( rads / 2.0f ); //Half of the horizon of the view frustum. Used to scale everything else by the fovDegreesY.

	float w = size;
	float h = size;
	if ( aspect > 1.0f )
		w /= aspect;
	else
		h *= aspect;

	float tmp = 1.0f / ( zFar - zNear );

	//Simplifies from general perspective projection when (as assumed here) right = -left, top = -bottom (i.e. symmetric frustum).
	if ( ordering == ROW_MAJOR )
	{
		const T values[16] = {
			w,			0.f,		0.f,					0.f,
			0.f,		h,			0.f,					0.f,
			0.f,		0.f,		-( zFar + zNear )*tmp,	-1.f, //z inverted by not having - sign precede this row's TWO entries!
			0.f,		0.f,		-2.0f*zNear*zFar*tmp,	0.f 
		};
		perspectiveProjection.SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const T values[16] = {
			w,			0.f,		0.f,					0.f,
			0.f,		h,			0.f,					0.f,
			0.f,		0.f,		-( zFar + zNear )*tmp,	-2.0f*zNear*zFar*tmp,
			0.f,		0.f,		-1.f,					0.f //z inverted by not having - sign precede 3rd col's TWO entries!
		};
		perspectiveProjection.SetAllValuesAssumingSameOrdering( values );
	}

	//Combine change of basis and perspective matrices.
	if ( ordering == ROW_MAJOR )
		mult( changeOfBasis, perspectiveProjection, *this ); //Confirmed to be so if changeOfBasis is row-major, what about column
	else if ( ordering == COLUMN_MAJOR )
		mult( perspectiveProjection, changeOfBasis, *this );
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToOrthogonalProjection( T width, T height, T zNear, T zFar, Ordering ordering ) //Puts a box at your camera, projects things into it.
{
	//Simplifies from general perspective projection when (as assumed here) right = -left, top = -bottom (i.e. symmetric frustum).

	//WARNING: THE DEFAULT CENTER IS (0,0) IN OGL2 BECAUSE OF NDC [-1,1], so this ortho projection will have its bottom-left == center-screen.

	float tmp = 1.0f / ( zFar - zNear );

	if ( ordering == ROW_MAJOR )
	{
		const T values[ 16 ] = {
			2.f / width,	0.f,			0.f,			0.f,
			0.f,			2.f / height,	0.f,			0.f,
			0.f,			0.f,			2.f*tmp,		-( zFar + zNear )*tmp,
			0.f,			0.f,			0.f,			1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const T values[ 16 ] = {
			2.0f / width,	0.f,			0.f,					0.f,
			0.f,			2.f / height,	0.f,					0.f,
			0.f,			0.f,			2.f*tmp,				0.f,
			0.f,			0.f,			-( zFar + zNear )*tmp,	1.f
		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::ClearToOrthogonalProjection( T xLeft, T xRight, T yBottom, T yTop, T zNear, T zFar, Ordering ordering ) //Lets you map freely, e.g. mipmaps from 0 to 1 instead of above's -1 to 1.
{
	//Unsimplified version, won't assume right = -left, top = -bottom (i.e. symmetric frustum) like other version.

	//Note that each is still defining a "near" and "far of a plane in one dimension, not just sz.
	float sx = 1.0f / ( xRight - xLeft );
	float sy = 1.0f / ( yTop - yBottom );
	float sz = 1.0f / ( zFar - zNear );

	if ( ordering == ROW_MAJOR )
	{
		const T values[ 16 ] = {
			2.0f * sx,					0.0f,					0.0f,						0.0f,
			0.0f,						2.0f * sy,				0.0f,						0.0f,
			0.0f,						0.0f,					2.0f * sz,					0.0f,
			-( xRight + xLeft ) * sx, -( yTop + yBottom ) * sy, -( zFar + zNear ) * sz,		1.0f,
		};
		SetAllValuesAssumingSameOrdering( values );
	}
	else if ( ordering == COLUMN_MAJOR )
	{
		const T values[ 16 ] = {
			2.0f * sx,  0.0f,       0.0f,       -( xRight + xLeft ) * sx,
			0.0f,       2.0f * sy,  0.0f,       -( yTop + yBottom ) * sy,
			0.0f,       0.0f,       2.0f * sz,  -( zFar + zNear ) * sz,
			0.0f,       0.0f,       0.0f,       1.0f,
		};
		SetAllValuesAssumingSameOrdering( values );
	}
}


//--------------------------------------------------------------------------------------------------------------
template <typename T>
void Matrix4x4<T>::PrintDebugMatrix( Ordering* orderingReturned /*= nullptr*/ ) const
{
	const Ordering orderToUse = ( orderingReturned == nullptr ) ? m_ordering : *orderingReturned;

	if ( orderToUse == ROW_MAJOR )
	{
		DebuggerPrintf( "\nPrinting matrix using row-major ordering." );
		if ( m_ordering == ROW_MAJOR )
		{
			DebuggerPrintf( "\nMatrix was using row-major ordering.\n" );
			TODO( "Make these into a single call." );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 0 ], m_data[ 1 ], m_data[ 2 ], m_data[ 3 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 4 ], m_data[ 5 ], m_data[ 6 ], m_data[ 7 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 8 ], m_data[ 9 ], m_data[ 10 ], m_data[ 11 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 12 ], m_data[ 13 ], m_data[ 14 ], m_data[ 15 ] );
		}
		else
		{
			DebuggerPrintf( "\nMatrix was using column-major ordering.\n" );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 0 ], m_data[ 4 ], m_data[ 8 ], m_data[ 12 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 1 ], m_data[ 5 ], m_data[ 9 ], m_data[ 13 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 2 ], m_data[ 6 ], m_data[ 10 ], m_data[ 14 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 3 ], m_data[ 7 ], m_data[ 11 ], m_data[ 15 ] );
		}
	}
	else if ( orderToUse == COLUMN_MAJOR )
	{
		DebuggerPrintf( "Printing matrix using column-major ordering.\n" );
		if ( m_ordering == COLUMN_MAJOR )
		{
			DebuggerPrintf( "\nMatrix was using row-major ordering.\n" );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 0 ], m_data[ 1 ], m_data[ 2 ], m_data[ 3 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 4 ], m_data[ 5 ], m_data[ 6 ], m_data[ 7 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 8 ], m_data[ 9 ], m_data[ 10 ], m_data[ 11 ] );
			DebuggerPrintf( "%f %f %f %f", m_data[ 12 ], m_data[ 13 ], m_data[ 14 ], m_data[ 15 ] );
		}
		else
		{
			DebuggerPrintf( "\nMatrix was using column-major ordering.\n" );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 0 ], m_data[ 4 ], m_data[ 8 ], m_data[ 12 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 1 ], m_data[ 5 ], m_data[ 9 ], m_data[ 13 ] );
			DebuggerPrintf( "%f %f %f %f\n", m_data[ 2 ], m_data[ 6 ], m_data[ 10 ], m_data[ 14 ] );
			DebuggerPrintf( "%f %f %f %f", m_data[ 3 ], m_data[ 7 ], m_data[ 11 ], m_data[ 15 ] );
		}
	}
}


//--------------------------------------------------------------------------------------------------------------
template <>
void Matrix4x4<int>::PrintDebugMatrix( Ordering* orderingReturned /*= nullptr*/ ) const
{
	const Ordering orderToUse = ( orderingReturned == nullptr ) ? m_ordering : *orderingReturned;

	if ( orderToUse == ROW_MAJOR )
	{
		DebuggerPrintf( "\nPrinting matrix using row-major ordering." );
		if ( m_ordering == ROW_MAJOR )
		{
			DebuggerPrintf( "\nMatrix was using row-major ordering.\n" );
			TODO( "Make these into a single call." );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 0 ], m_data[ 1 ], m_data[ 2 ], m_data[ 3 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 4 ], m_data[ 5 ], m_data[ 6 ], m_data[ 7 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 8 ], m_data[ 9 ], m_data[ 10 ], m_data[ 11 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 12 ], m_data[ 13 ], m_data[ 14 ], m_data[ 15 ] );
		}
		else
		{
			DebuggerPrintf( "\nMatrix was using column-major ordering.\n" );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 0 ], m_data[ 4 ], m_data[ 8 ], m_data[ 12 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 1 ], m_data[ 5 ], m_data[ 9 ], m_data[ 13 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 2 ], m_data[ 6 ], m_data[ 10 ], m_data[ 14 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 3 ], m_data[ 7 ], m_data[ 11 ], m_data[ 15 ] );
		}
	}
	else if ( orderToUse == COLUMN_MAJOR )
	{
		DebuggerPrintf( "Printing matrix using column-major ordering.\n" );
		if ( m_ordering == COLUMN_MAJOR )
		{
			DebuggerPrintf( "\nMatrix was using row-major ordering.\n" );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 0 ], m_data[ 1 ], m_data[ 2 ], m_data[ 3 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 4 ], m_data[ 5 ], m_data[ 6 ], m_data[ 7 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 8 ], m_data[ 9 ], m_data[ 10 ], m_data[ 11 ] );
			DebuggerPrintf( "%d %d %d %d", m_data[ 12 ], m_data[ 13 ], m_data[ 14 ], m_data[ 15 ] );
		}
		else
		{
			DebuggerPrintf( "\nMatrix was using column-major ordering.\n" );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 0 ], m_data[ 4 ], m_data[ 8 ], m_data[ 12 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 1 ], m_data[ 5 ], m_data[ 9 ], m_data[ 13 ] );
			DebuggerPrintf( "%d %d %d %d\n", m_data[ 2 ], m_data[ 6 ], m_data[ 10 ], m_data[ 14 ] );
			DebuggerPrintf( "%d %d %d %d", m_data[ 3 ], m_data[ 7 ], m_data[ 11 ], m_data[ 15 ] );
		}
	}
}

template class Matrix4x4<float>;
