#include "zstring.h"

#ifndef _WIN32

#include <ctype.h>
#include <unistd.h>

// [BB] itoa is not available under Linux, so we supply a replacement here.
// Code taken from http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
/**
 * Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C"
 * with slight modification to optimize for specific architecture:
 */
	
void strreverse(char* begin, char* end) {
	char aux;
	while(end>begin)
		aux=*end, *end--=*begin, *begin++=aux;
}
	
char* itoa(int value, char* str, int base) {
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char* wstr=str;
	int sign;
	div_t res;
	
	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return str; }

	// Take care of sign
	if ((sign=value) < 0) value = -value;

	// Conversion. Number is reversed.
	do {
		res = div(value,base);
		*wstr++ = num[res.rem];
	}while(value=res.quot);
	if(sign<0) *wstr++='-';
	*wstr='\0';

	// Reverse string
	strreverse(str,wstr-1);
	
	return str;
}

/*
**  Portable, public domain replacements for strupr() by Bob Stout
*/

char *strupr(char *string)
{
      char *s;

      if (string)
      {
            for (s = string; *s; ++s)
                  *s = toupper(*s);
      }
      return string;
} 

void I_Sleep( int iMS )
{
	usleep( 1000 * iMS );
}

#endif

#ifdef NO_SERVER_GUI

#include <iostream>
#include "networkheaders.h"

// [BB] I collect dummy implementations of many functions, which are either
// GL or server console gui related, here. This way one doesn't have to make
// define constructions whenever the functions are called elsewhere in the code.

// ------------------- Server console related stuff ------------------- 
// [BB] Most of these functions just do nothing if we don't have a GUI.
// Only SERVERCONSOLE_Print is really needed.
void SERVERCONSOLE_UpdateTitleString( char *pszString ) {}
void SERVERCONSOLE_UpdateScoreboard( void ) {}
void SERVERCONSOLE_UpdateTotalOutboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateAverageOutboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdatePeakOutboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateCurrentOutboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateTotalInboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateAverageInboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdatePeakInboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateCurrentInboundDataTransfer( LONG lData ) {}
void SERVERCONSOLE_UpdateTotalUptime( LONG lData ) {}
void SERVERCONSOLE_SetCurrentMapname( char *pszString ) {}
void SERVERCONSOLE_SetupColumns( void ) {}
void SERVERCONSOLE_AddNewPlayer( LONG lPlayer ) {}
void SERVERCONSOLE_UpdatePlayerInfo( LONG lPlayer, ULONG ulUpdateFlags ) {}
void SERVERCONSOLE_RemovePlayer( LONG lPlayer ) {}
void SERVERCONSOLE_UpdateOperatingSystem( char *pszString ) {}
void SERVERCONSOLE_UpdateCPUSpeed( char *pszString ) {}
void SERVERCONSOLE_UpdateVendor( char *pszString ) {}
void SERVERCONSOLE_Print( char *pszString ) { std::cout << pszString; }

// ------------------- GL related stuff ------------------- 
#include "d_player.h"
#include "p_setup.h"
#include "doomtype.h"
#include "r_data.h"
#include "gl/gl_functions.h"
#include "gl/gl_texture.h"
#include "gl/gl_lights.h"

// [BB] We need the gl nodes even if we don't display anything on the screen, i.e. on the server.
#include "gl/gl_nodes.cpp"
/*
// [BB] This are the necessary things from gl_nodes.cpp
void gl_CheckNodes(MapData * map) {}
bool gl_LoadGLNodes(MapData * map) { return false; }
node_t * gamenodes = NULL;
int numgamenodes = 0;
subsector_t * gamesubsectors = NULL;
*/

#ifdef _WIN32
typedef unsigned char byte;
void gl_DrawLine(int x1, int y1, int x2, int y2, int color) {}
void gl_RenderPlayerView (player_t* player) {}
void gl_RecreateAllAttachedLights() {}
void gl_DeleteAllAttachedLights() {}
void gl_CleanLevelData() {}
void gl_PreprocessLevel() {}
void gl_SetFogParams(int _fogdensity, PalEntry _outsidefogcolor, int _outsidefogdensity, int _skyfog) {}
void gl_DrawBuffer(byte * sbuffer, int width, int height, int x, int y, int dx, int dy, PalEntry * palette) {}
void gl_DrawSavePic(DCanvas * canvas, const char * Filename, int x, int y, int dx, int dy) {}
void gl_ScreenShot (const char* fname) {}
void gl_SetActorLights(AActor *actor) {}
void gl_DrawTexture(FTexInfo *texInfo) {}
void gl_ParseDefs(void) {}
void gl_RenderViewToCanvas(DCanvas * pic, int x, int y, int width, int height) {}
void StartGLMenu (void) {}

void FTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																	 intptr_t cm, int translation) {}
void FPNGTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																			intptr_t cm, int translation) {}
void FJPEGTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																			 intptr_t cm, int translation) {}
void FTGATexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																			intptr_t cm, int translation) {}
void FPCXTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																			intptr_t cm, int translation) {}
void FWarpTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int xx, int yy, 
									 intptr_t cm, int translation) {}
void FWarp2Texture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int xx, int yy, 
									 intptr_t cm, int translation) {}
void FMultiPatchTexture::CopyTrueColorPixels(BYTE * buffer, int buf_width, int buf_height, int x, int y, 
																						 intptr_t cm, int translation) {}

FGLTexture::~FGLTexture() {}
void FGLTexture::FlushAll() {}
void FGLTexture::Clean(bool) {}
const WorldTextureInfo * FGLTexture::Bind(int cm) { return NULL; }
const PatchTextureInfo * FGLTexture::BindPatch(int cm, int translation, const byte * translationtable) { return NULL; }
FGLTexture * FGLTexture::ValidateTexture(FTexture * tex) { return NULL; }
void FCanvasTexture::RenderGLView (AActor *viewpoint, int fov) {}

CVAR(Bool, gl_precache, false, CVAR_ARCHIVE)
CVAR(Bool, gl_nogl, false, CVAR_ARCHIVE)

IMPLEMENT_STATELESS_ACTOR (ADynamicLight, Any, -1, 0)
END_DEFAULTS

FCycler::FCycler(void) {}
void ADynamicLight::Serialize(class FArchive &) {}
void ADynamicLight::Destroy(void) {}
void ADynamicLight::Tick(void) {}
void ADynamicLight::PostBeginPlay(void) {}
void ADynamicLight::BeginPlay(void) {}
void ADynamicLight::Activate(class AActor *) {}
void ADynamicLight::Deactivate(class AActor *) {}
#endif //_WIN32
#endif

//
// I_ConsoleInput - [NightFang] - pulled from the old 0.99 code
//
#ifndef	WIN32
int		stdin_ready = 0;
int		do_stdin = 1;
#else
#include "conio.h"
#endif

char *I_ConsoleInput (void)
{
#ifndef	WIN32
	static 	char text[256];
	int	len;
	if (!stdin_ready || !do_stdin)
	{ return NULL; }

	stdin_ready = 0;

	len = read(0, text, sizeof(text));
	if (len < 1)
	{ return NULL; }

	text[len-1] = 0;

	return text;
#else
	// Windows code
	static char     text[256];
    static int              len;
    int             c;

    // read a line out
    while (_kbhit())
    {
		c = _getch();
        putch (c);
        if (c == '\r')
        {
			text[len] = 0;
            putch ('\n');
            len = 0;
            return text;
        }
        
		if (c == 8)
        {
			if (len)
            {
				putch (' ');
                putch (c);
                len--;
                text[len] = 0;
            }
            continue;
        }
    
		text[len] = c;
        len++;
        text[len] = 0;
        if (len == sizeof(text))
		    len = 0;
	}

    return NULL;
#endif
}
