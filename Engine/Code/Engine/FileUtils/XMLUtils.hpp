//================================================================================================================================
// XMLUtilities.hpp
//================================================================================================================================


#pragma once
#ifndef __included_XMLUtilities__
#define __included_XMLUtilities__

#include <string>
#include "ThirdParty/Parsers/xmlparser/xmlParser.h"
#include "Engine/String/StringUtils.hpp"


//================================================================================================================================
//
//================================================================================================================================
std::string			GetXMLErrorDescForErrorCode( XMLError xmlErrorCode );
bool				GetXMLNodeByNameSearchingFromPosition( const XMLNode& parentNode, const std::string& childName, int& position_inout, XMLNode& childNode_out );
std::string			GetXMLAttributeAsString( const XMLNode& node, const std::string& attributeName, bool& wasAttributePresent_out );
void				DestroyXMLDocument( XMLNode& xmlDocumentToDestroy );


///-----------------------------------------------------------------------------------
///
///-----------------------------------------------------------------------------------
template< typename ValueType >
ValueType GetXMLAttributeOfType( const XMLNode& node, const std::string& propertyName, bool& wasPropertyPresent_out )
{
	ValueType	outValue;
	std::string	valueAsString = GetXMLAttributeAsString( node, propertyName, wasPropertyPresent_out );
	
	SetTypeFromUnwrappedString( outValue, valueAsString );
	return outValue;
}


///-----------------------------------------------------------------------------------
///
///-----------------------------------------------------------------------------------
template< typename ValueType >
ValueType ReadXMLAttribute( const XMLNode& node, const std::string& propertyName, const ValueType& defaultValue )
{
	bool		wasPropertyPresent = false;

	ValueType	outValue = GetXMLAttributeOfType< ValueType >( node, propertyName, wasPropertyPresent );
	if ( !wasPropertyPresent )
		outValue = defaultValue;

	return outValue;
}


///-----------------------------------------------------------------------------------
///
///-----------------------------------------------------------------------------------
template< typename ValueType >
void WriteXMLAttribute( XMLNode& node, const std::string& propertyName, const ValueType& value, const ValueType& defaultValue )
{
	if ( value == defaultValue )
		return;

	std::string	valueAsString = GetTypedObjectAsString( value );
	
	node.addAttribute( propertyName.c_str(), valueAsString.c_str() );
}




#endif // __included_XMLUtilities__
