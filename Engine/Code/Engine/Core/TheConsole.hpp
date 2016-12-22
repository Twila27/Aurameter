#pragma once

#include <map>
#include <vector>
#include <utility>
#include <string.h>
#include "Engine/Renderer/Rgba.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/TheRenderer.hpp"
#include "Engine/Memory/UntrackedAllocator.hpp"


//--------------------------------------------------------------------------------------------------------------
class TheConsole;
class BitmapFont;
class Command;


//--------------------------------------------------------------------------------------------------------------
extern TheConsole* g_theConsole;


//--------------------------------------------------------------------------------------------------------------
typedef void( ConsoleCommandCallback )( Command& );
struct ConsoleCommandEntry
{
	ConsoleCommandEntry( const std::string& name, ConsoleCommandCallback* cb )
		: name( name ), callback( cb ) {}
	std::string name;
	ConsoleCommandCallback* callback;
};
typedef std::pair< std::string, ConsoleCommandEntry* > ConsoleCommandRegistryPair;
typedef std::map < std::string, ConsoleCommandEntry*, std::less<std::string>, UntrackedAllocator<ConsoleCommandRegistryPair> > ConsoleCommandRegistryMap;
static ConsoleCommandRegistryMap s_theConsoleCommands; //Only exists in this .cpp unit, hence below wrapper.


//--------------------------------------------------------------------------------------------------------------
//Command implementations. Register by calling g_theConsole->RegisterCommand().
void ShowHelp( Command& args );
void ClearLog( Command& args );
void CloseConsole( Command& args );
void SetColor( Command& args );
void FindString( Command& args );


//--------------------------------------------------------------------------------------------------------------
typedef std::vector< std::pair< std::string, Rgba > > CommandHistory;


//--------------------------------------------------------------------------------------------------------------
class TheConsole
{
public:

	TheConsole( double consoleX, double consoleY, double consoleWidth, double screenHeight,
				bool showPromptBox = true,
				const Rgba& textColor = Rgba(), bool isVisible = false, float textScale = .25f,
				double maxConsoleHeightCoverageNormalized = 0.3, BitmapFont* font = g_theRenderer->GetDefaultFont() )
		: m_currentColor( textColor )
		, m_isVisible( isVisible )
		, m_currentScale( textScale )
		, m_consoleX( consoleX )
		, m_consoleY( consoleY )
		, m_consoleWidth( consoleWidth )
		, m_maxConsoleHeightAsScreenPercentage( screenHeight * maxConsoleHeightCoverageNormalized )
		, m_currentFont( font )
		, m_hasFontChanged( false )
		, m_caretAlphaCounter( 0.f )
		, m_replacerPos( 0 )
		, m_caretPosInInputString( 0 )
		, m_bottommostLogIndexToRender( 0 )
		, m_maxStoredLinesToShow( 0 )
		, m_shouldPulseSearchResult( false )
		, m_searchResultToPulse( "" )
		, m_showPromptBox( showPromptBox )
	{
		DEFAULT_COLOR = m_currentColor;

		RegisterCommand( "Help", ShowHelp );
		RegisterCommand( "ClearLog", ClearLog );
		RegisterCommand( "Close", CloseConsole );
		RegisterCommand( "SetConsoleColor", SetColor );
		RegisterCommand( "Find", FindString );
	}
	void RegisterCommand( const std::string& name, ConsoleCommandCallback* cb );
	void RunCommand( const std::string& fullCommandString );
	void Printf( const char* format, ... ); //Trigger this on VK_ENTER from command prompt.
	void SetTextColor( const Rgba& newColor = DEFAULT_COLOR ) { m_currentColor = newColor; }
	void SetFont( BitmapFont* newFont ) { m_currentFont = newFont; m_hasFontChanged = true; }
	void ShowConsole() { m_isVisible = true; }
	void HideConsole() { m_isVisible = false; HidePrompt(); }
	bool IsVisible() const { return m_isVisible; }
	void Render();

	void RenderPreviousTextEntries( float consoleX, float spacingPx, float heightOfOneLinePx );

	void Update( float deltaSeconds );
	void UpdatePromptForChar( unsigned char ch );
	void UpdatePromptForKeydown( unsigned char ch );

	void LowerShownLines();
	void RaiseShownLines();

	void ClearConsoleLog() { m_storedText.clear(); m_replacerPos = 0; }
	void ShouldPulseSearchResults( bool newVal, const std::string& term );
	void AttemptAutocomplete( const ConsoleCommandRegistryMap::iterator* indexOfLastResult = nullptr );
	void ShowPrompt() { m_showPromptBox = true; }
	bool ShowingPrompt() const { return m_showPromptBox; }
	void HidePrompt() { m_showPromptBox = false; }

	void CleanupEntries();
	std::string RemoveLineNumber( const std::string& storedLineWithLineNumber );
	static Rgba DEFAULT_COLOR;

private:

	CommandHistory m_storedText;

	Vector2f m_currentLogBoxTopLeft;
	Vector2f m_currentPromptBoxTopLeft;

	double m_consoleX;
	double m_consoleY;
	double m_consoleWidth;
	double m_maxConsoleHeightAsScreenPercentage;

	bool m_showPromptBox;

	std::string m_currentPromptString;
	BitmapFont* m_currentFont;
	bool m_hasFontChanged;
	Rgba m_currentColor;
	float m_currentScale;
	bool m_isVisible;
	bool m_shouldPulseSearchResult;
	std::string m_searchResultToPulse;
	std::string m_lastAutocompleteInput;

	float m_caretAlphaCounter;
	int m_replacerPos; //i.e. which line gets added to the command prompt if you hit up or down next.
	int m_caretPosInInputString;
	int m_bottommostLogIndexToRender; //i.e. bottom-most.
	unsigned int m_maxStoredLinesToShow;
};