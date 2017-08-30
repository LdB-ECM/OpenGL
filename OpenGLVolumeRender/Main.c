/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++}
{					OPENGL MDI APP DEMO VERSION 1.0					        }
{       Writen by Leon de Boer, Perth, Western Australia, 2016.				}
{	  	contact: ldeboer@gateway.net.au										}
{																			}
{       Copyright released on Code Project Open License (CPOL) and use      }
{       and/or abuse is freely given :-)									}
{																			}
{        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND		}
{++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*==========================================================================
                        BACKGROUND TO CODE

 This code is is C syntax rather than C++ or C# code and contains no objects
 or sophisticated elements. That is not because those techniques don't have
 merit or a place but simply because this code is targetted as a learning 
 tool to the widest audience. Anyone proficient in C++ or C# could easily 
 convert this code to those formats.
 
 ==========================================================================*/

#define _WIN32_WINNT 0x0500
#include <windows.h>		// Standard windows headers
#include <tchar.h>			// Unicode support	.. we will use TCHAR rather than char	
#include <commctrl.h>		// Common controls dialogs unit
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include "glext.h"

// This is the lazy adding libraries via #pragma rather than in linker includes in visual studio
// If you are not on visual studio you will need to comment out the #pragma statements and
// add the libraries to the includes in your compiler linker 
#pragma comment(lib,"comctl32.lib") 
#pragma comment(lib,"OpenGl32.lib")
#pragma comment(lib,"GLU32.lib")


PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = 0;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = 0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture = 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0;


/***************************************************************************
                   APP SPECIFIC INTERNAL CONSTANTS
 ***************************************************************************/

/*--------------------------------------------------------------------------}
;{                   MAIN MENU COMMAND VALUE CONSTANTS			            }
;{-------------------------------------------------------------------------*/
#define IDC_BMPLOAD1 101								// App menu to load bitmap to window 1
#define IDC_BMPLOAD2 102								// App menu to load bitmap to window 2
#define IDC_EXIT 105									// App menu command to exit application

/*--------------------------------------------------------------------------}
;{                      APPLICATION STRING CONSTANTS			            }
;{-------------------------------------------------------------------------*/
static const TCHAR* OPENGLCHILD_CLASSNAME = _T("OpenGl_Child");
static const TCHAR* DATABASE_PROPERTY = _T("OurDataStructure");

/*---------------------------------------------------------------------------
					  GLDrawScene Function ProtoType
---------------------------------------------------------------------------*/
typedef struct OpenGLData* GLDataPtr;
typedef void (*DrawGLSceneFunc) (GLDataPtr, HDC);

/*---------------------------------------------------------------------------
                   OPENGL CHILD DATA RECORD DEFINITION
 ---------------------------------------------------------------------------*/
typedef struct OpenGLData {
	HGLRC Rc;											// Our render context
	DrawGLSceneFunc glDrawFunc;							// Pointer to draw function

	GLuint frameBuf;									// Frame buffer
	GLuint fboDepth;
	int fboWidth;
	int fboHeight;

	GLuint glTexture;									// Our texture to draw
	GLfloat	xrot;										// X Rotation
	GLfloat	yrot;										// Y Rotation
} GLDATABASE;



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						COMMON DIALOG CALL ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*--------------------------------------------------------------------------

 This uses the file open common dialog to select a file. It will return a 
 non zero value if successful. If the dialog was unsucessful or cancelled 
 zero will be returned.

 16Apr16 LdB
 --------------------------------------------------------------------------*/
int OpenFileDialog(TCHAR* Name,					// Buffer to receive selected file name in 
				   unsigned short NameBufSize,	// Size of that buffer in CHARACTERS unicode/wide is supported 
				   TCHAR* DescString,			// Description string "XYZ filetype:"
				   TCHAR* Ext,					// Extension type string "*.bmp", "*.jpg" etc 
				   TCHAR* Title,				// Title string to put on dialog
				   HWND AppWnd)					// Handle to application window
{
	int i;
	TCHAR FileName[256], DefExt[256], Filter[400];
	OPENFILENAME OpenFN;

	InitCommonControls();											// Initialize common dialogs
	Name[0] = 0;													// Preset name clear
	_tcscpy_s(&FileName[0], _countof(FileName), _T("*."));			// Tranfer "*."
	_tcscat_s(&FileName[0], _countof(FileName), Ext);				// Create "*.xxx" extension
	_tcscpy_s(Filter, _countof(Filter), DescString);				// Tranfer description string
	i = _tcslen(Filter);											// Fetch that string length in TCHAR
	_tcscpy_s(&Filter[i + 1], _countof(Filter) - i - 1, &FileName[0]);	// Transfer "*.ext"
	i += (_tcslen(&FileName[0]) + 1);								// Advance to beyond end
	_tcscpy_s(&Filter[i + 1], _countof(Filter) - i - 1, _T("\0"));	// Must end with two 0 entries
	_tcscpy_s(&Filter[i + 2], _countof(Filter) - i - 2, _T("\0"));	// Must end with two 0 entries
	_tcscpy_s(DefExt, _countof(DefExt), Ext);						// Default ext name
	memset((void*) &OpenFN, 0, sizeof(OpenFN));					    // Zero unused fields
	OpenFN.lStructSize = sizeof(OPENFILENAME);					    // Size of structure
	OpenFN.hInstance = GetModuleHandle(NULL);						// Pointer to instance
	OpenFN.hwndOwner = AppWnd;										// Owner window
	OpenFN.lpstrFilter = &Filter[0];								// Filter
	OpenFN.nFilterIndex = 1;										// 1st Filter String
	OpenFN.lpstrFile = &FileName[0];								// Return result
	OpenFN.nMaxFile = _countof(FileName);							// Max name length
	OpenFN.lpstrDefExt = Ext;										// Default extension
	OpenFN.lpstrFileTitle = &FileName[0];							// Default file title
	OpenFN.nMaxFileTitle = _countof(FileName);						// Max size of file title
	OpenFN.lpstrTitle = Title;	                     			    // Window title
	OpenFN.lpfnHook = NULL;											// No hook handler
	OpenFN.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES;               // Set flag masks
	if (GetOpenFileName(&OpenFN) != 0){
		_tcscpy_s(Name, NameBufSize, FileName);						// Return the name
		return OpenFN.nFilterIndex;									// Return filter type
	} else return 0;												// Dialog cancelled
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
							  OPENGL ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static BOOL SetupOpenGL3v3 (HWND Wnd) {
	//  We need to make sure the window create in a suitable DC format
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, //Flags
		PFD_TYPE_RGBA,												// The kind of framebuffer. RGBA or palette.
		32,															// Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,															// Number of bits for the depthbuffer
		0,															// Number of bits for the stencilbuffer
		0,															// Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	BOOL success = FALSE;											// Preset failure flag
	HDC tempDC = GetDC(Wnd);										// Get a temporary DC from window
	int letWindowsChooseThisPixelFormat = ChoosePixelFormat(tempDC, &pfd); // Let windows select an appropriate pixel format
	if (SetPixelFormat(tempDC, letWindowsChooseThisPixelFormat, &pfd)) { // Try to set that pixel format
		HGLRC  ourOpenGLRC = wglCreateContext(tempDC);
		if (ourOpenGLRC != 0) {
			if (wglMakeCurrent(tempDC, ourOpenGLRC)){
				success = TRUE;										// Now change sucess preset to true
				
				// Okay we are need to set all the OpenGL3.3 function pointers
				// The text names here are ASCII do not change to unicode/Wide
				
				glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
				if (glGenFramebuffers == 0) success = FALSE;		// Pointer is NULL so fail
				glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
				if (glDeleteFramebuffers == 0) success = FALSE;		// Pointer is NULL so fail
				glBindFramebuffer =	(PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
				if (glBindFramebuffer == 0) success = FALSE;		// Pointer is NULL so fail
				glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");
				if (glBindFramebufferEXT == 0) success = FALSE;		// Pointer is NULL so fail
				glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");
				if (glFramebufferTexture2D == 0) success = FALSE;	// Pointer is NULL so fail
				glBlitFramebuffer =	(PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
				if (glBlitFramebuffer == 0) success = FALSE;		// Pointer is NULL so fail
				glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
				if (glCheckFramebufferStatusEXT == 0) success = FALSE;// Pointer is NULL so fail
				glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) wglGetProcAddress("glFramebufferTexture");
				if (glFramebufferTexture == 0) success = FALSE;		// Pointer is NULL so fail
				glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
				if (glFramebufferTexture2DEXT == 0) success = FALSE;// Pointer is NULL so fail
				glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
				if (glDrawBuffers == 0) success = FALSE;			// Pointer is NULL so fail
				glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) wglGetProcAddress("glGenRenderbuffersEXT");
				if (glGenRenderbuffersEXT == 0) success = FALSE;	// Pointer is NULL so fail
				glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
				if (glBindRenderbufferEXT == 0) success = FALSE;    // Pointer is NULL so fail
				glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) wglGetProcAddress("glFramebufferRenderbufferEXT");
				if (glFramebufferRenderbufferEXT == 0) success = FALSE;//Pointer is NULL so fail
				glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) wglGetProcAddress("glRenderbufferStorageEXT");
				if (glRenderbufferStorageEXT == 0) success = FALSE;	// Pointer is NULL so fail
			}
			wglDeleteContext(ourOpenGLRC);							// We are done with context	
		}  
	}
	ReleaseDC(Wnd, tempDC);											// Release the window device context we are done
	return (success);												// Return result
}


/*-[ InitGL ]---------------------------------------------------------------

 Initializes the OpenGL system for the provided window handle, this is a one 
 time call made for the window and the function returns the created Render
 context for the window. The responsibility to delete the render context when
 completed is placed on the caller. In this demo initialization will be called 
 from WM_CREATE from each MDICLIENT window. Failure of initialization will
 return a render context handle equal to zero.

 16Apr16 LdB
 --------------------------------------------------------------------------*/
static HGLRC InitGL (HWND Wnd) {
	HGLRC ourOpenGLRC = 0;											// Preset render context to zero

	//  We need to make sure the window create in a suitable DC format
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, //Flags
		PFD_TYPE_RGBA,												// The kind of framebuffer. RGBA or palette.
		32,															// Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,															// Number of bits for the depthbuffer
		8,															// Number of bits for the stencilbuffer
		0,															// Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC ourWindowHandleToDeviceContext = GetDC(Wnd);				// Get a DC for our window
	int letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd); // Let windows select an appropriate pixel format
	if (SetPixelFormat(ourWindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd)) { // Try to set that pixel format
		ourOpenGLRC = wglCreateContext(ourWindowHandleToDeviceContext);
		if (ourOpenGLRC != 0) {
			wglMakeCurrent(ourWindowHandleToDeviceContext, ourOpenGLRC); // Make our render context current
			glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
			glShadeModel(GL_SMOOTH);								// Enable Smooth Shading
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);					// Black Background
			glClearDepth(1.0f);										// Depth Buffer Setup
			glEnable(GL_DEPTH_TEST);								// Enables Depth Testing
			glDepthFunc(GL_LEQUAL);									// The Type Of Depth Testing To Do
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Really Nice Perspective Calculations
		}
	}
	ReleaseDC(Wnd, ourWindowHandleToDeviceContext);					// Release the window device context we are done
	return (ourOpenGLRC);											// Return the render context
}


/*-[ ReSizeGLScene ]--------------------------------------------------------

 Rescales the OpenGL system for a given size of screen, called at anytime
 the Application resizes the window . It will call once after InitGL as
 in this demo it is called from WM_WINDOWPOSCHANGING/WM_WINDOWPOSCHANGED
 from the MDICLIENT windows.

 15Apr16 LdB
 --------------------------------------------------------------------------*/
static void ReSizeGLScene (HWND Wnd) {
	GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY); // Fetch the data base
	if (db == 0) return;											// Cant resize .. no render context
	HDC Dc = GetWindowDC(Wnd);										// Get the window DC
	RECT r;
	GetWindowRect(Wnd, &r);											// Fetch the window size
	int Width = r.right - r.left;									// Window width
	int Height = r.bottom - r.top;									// Window height
	if (Height == 0) Height = 1;									// Stop divid by zero
	wglMakeCurrent(Dc, db->Rc);										// Make our render context current
	glViewport(0, 0, Width, Height);								// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix
	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat) Width / (GLfloat) Height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
	ReleaseDC(Wnd, Dc);												// Release the window DC
}


/*-[ DrawGLScene ]----------------------------------------------------------

This is where all the OpenGL drawing is done for each frame. In this demo
it will be called from WM_PAINT messages to the window

15Apr16 LdB
--------------------------------------------------------------------------*/

GLfloat dOrthoSize = 1.0f;

// Macro to draw the quad.
// Performance can be achieved by making a call list.
// To make it simple i am not using that now :-)
#define MAP_3DTEXT( TexIndex ) \
            glTexCoord3f(0.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(-dOrthoSize,-dOrthoSize,TexIndex);\
        glTexCoord3f(1.0f, 0.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(dOrthoSize,-dOrthoSize,TexIndex);\
        glTexCoord3f(1.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(dOrthoSize,dOrthoSize,TexIndex);\
        glTexCoord3f(0.0f, 1.0f, ((float)TexIndex+1.0f)/2.0f);  \
        glVertex3f(-dOrthoSize,dOrthoSize,TexIndex);


void initFrameBufferDepthBuffer(GLDATABASE* db) {

	glGenRenderbuffersEXT(1, &db->fboDepth); // Generate one render buffer and store the ID in fbo_depth  
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, db->fboDepth); // Bind the fbo_depth render buffer  

	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, db->fboWidth, db->fboHeight); // Set the render buffer storage to be a depth component, with a width and height of the window  

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, db->fboDepth); // Set the render buffer of this buffer to the depth buffer  

	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // Unbind the render buffer  
}

void initFrameBufferTexture(GLDATABASE* db) {
	glBindTexture(GL_TEXTURE_2D, db->glTexture); // Bind the texture fbo_texture  

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, db->fboWidth, db->fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window  

	// Setup the basic texture parameters  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Unbind the texture  
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initFrameBuffer(GLDATABASE* db) {
	initFrameBufferDepthBuffer(db); // Initialize our frame buffer depth buffer  

	initFrameBufferTexture(db); // Initialize our frame buffer texture  

	//glGenFramebuffersEXT(1, db->frameBuf); // Generate one frame buffer and store the ID in fbo  
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, db->frameBuf); // Bind our frame buffer  

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, db->glTexture, 0); // Attach the texture fbo_texture to the color buffer in our frame buffer  

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, db->fboDepth); // Attach the depth buffer fbo_depth to our frame buffer  

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer  

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) // If the frame buffer does not report back as complete  
	{
		//std::cout << "Couldn't create frame buffer" << std::endl; // Output an error to the console  
		//exit(0); // Exit the application  
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our frame buffer  
}

void DrawGLSceneBitmap (GLDATABASE* db, HDC Dc) {							
	if ((db == 0) || (db->glTexture == 0)) return;					// Cant draw .. no render context
	wglMakeCurrent(Dc, db->Rc);										// Make our render context current

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // Clear the background of our window to red 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear the colour buffer (more buffers later on) 
	glLoadIdentity(); // Load the Identity Matrix to reset our drawing locations 

	glTranslatef(0.0f, 0.0f, -2.0f);

	glBindTexture(GL_TEXTURE_2D, db->glTexture); // Bind our frame buffer texture 

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f); // The bottom left corner 

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 0.0f); // The top left corner 

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 0.0f); // The top right corner 

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f); // The bottom right corner 

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind any textures 

	//glutSwapBuffers();
}

void DrawGLSceneBitmap1 (GLDATABASE* db, HDC Dc) {
	if ((db == 0) || (db->glTexture == 0)) return;					// Cant draw .. no render context
	wglMakeCurrent(Dc, db->Rc);										// Make our render context current

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear The Screen And The Depth Buffer
	glLoadIdentity();												// Reset The View
	glTranslatef(0.0f, 0.0f, -5.0f);

	glRotatef(db->xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(db->yrot, 0.0f, 1.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, db->glTexture);

	glBegin(GL_QUADS);
	// Front Face
	glNormal3f(0.0f, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	// Back Face
	glNormal3f(0.0f, 0.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	// Top Face
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	// Bottom Face
	glNormal3f(0.0f, -1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	// Right face
	glNormal3f(1.0f, 0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, -1.0f, 1.0f);
	// Left Face
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
}

void DrawGLSceneVolumeRender (GLDATABASE* db, HDC Dc) {
	if ((db == 0) || (db->glTexture == 0)) return;					// Cant draw .. no render context
	wglMakeCurrent(Dc, db->Rc);										// Make our render context current

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear The Screen And The Depth Buffer
	glLoadIdentity();												// Reset The View
	glTranslatef(0.0f, 0.0f, -5.0f);

	glRotatef(db->xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(db->yrot, 0.0f, 1.0f, 0.0f);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, db->glTexture);

	for (float fIndx = -1.0f; fIndx <= 1.0f; fIndx += 0.01f)
	{
		glBegin(GL_QUADS);
		MAP_3DTEXT(fIndx);
		glEnd();
	}

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                      DATA CONVERSION ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*-[ BMP2GLTexture ]--------------------------------------------------------

 This converts a bitmap file from the filename path and converts it to an
 OpenGL texture. If the filename is invalid of the file not a bitmap the
 routine will return a GLuint of zero.

 15Apr16 LdB
 --------------------------------------------------------------------------*/
GLuint BMP2GLTexture (TCHAR* fileName, int* bmpWth, int* bmpHt) 
{	
	HBITMAP hBMP;                                                   // Handle Of The Bitmap
	BITMAP  BMP;                                                    // Bitmap Structure

	hBMP = (HBITMAP) LoadImage(GetModuleHandle(NULL), fileName, 
		IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);	// Load the bitmap from file
	if (!hBMP) return (0);											// If bitmap does not exist return false
	GetObject(hBMP, sizeof(BMP), &BMP);								// Get The bitmap details
	if (bmpWth) *bmpWth = BMP.bmWidth;								// If user provides width ptr return it
	if (bmpHt) *bmpHt = BMP.bmHeight;								// If user provides hight ptr return it
	int P2Width = (BMP.bmWidth) >> 2;								// Divid width by 4
	if ((P2Width << 2) < (BMP.bmWidth)) P2Width++;					// Inc by 1 if width x 4 is less than original
	P2Width = P2Width << 2;											// Power of two width
	long imageSize = (long) P2Width * (long) BMP.bmHeight * sizeof(RGBQUAD);
	BYTE* lpPixels = (BYTE*) malloc(imageSize);						// Create the pixel buffer					

	// Create and fill BITMAPINFO structure to pass to GetDIBits
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = BMP.bmWidth;
	bmi.bmiHeader.biHeight = BMP.bmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = 0;
	bmi.bmiHeader.biSizeImage = imageSize;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	
	// Put DIBBits into memory buffer
	HDC screenDC = GetDC(GetDesktopWindow());						// Grab a screen DC from the desktop
	GetDIBits(screenDC, hBMP, 0, BMP.bmHeight, lpPixels, 
		&bmi, DIB_RGB_COLORS);										// Create a DIB from bitmap consistent with screen DC
	ReleaseDC(GetDesktopWindow(), screenDC);						// Release the screen DC done with it now

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);							// Pixel Storage Mode (Word Alignment / 4 Bytes)
	GLuint texture;
	glGenTextures(1, &texture);										// Create a GL texture

	// Create Nearest Filtered Texture
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, lpPixels);

	free(lpPixels);													// Free allocated pixel memory
	DeleteObject(hBMP);												// Delete The Object
	return (texture);												// Return the texture
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                       READ AND PROCESS A FILE ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

BOOL ProcessBmpFile (TCHAR* szFilename, HWND Wnd) {
	if (_tcsstr(szFilename, _T(".bmp")) || _tcsstr(szFilename, _T(".BMP"))){
		GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY);
		if (db != 0) {
			HDC Dc = GetWindowDC(Wnd);								// Get a window context
			wglMakeCurrent(Dc, db->Rc);								// Make sure our render context current
			// Now check if texture exists and if so delete it
			if (db->glTexture) {
				glDeleteTextures(1, &db->glTexture);				// Delete the texture
				db->glTexture = 0;									// Zero the texture in database
			}
			// Create new texture
			db->glTexture = BMP2GLTexture(&szFilename[0], 0 , 0);	// Load the texure
			ReleaseDC(Wnd, Dc);										// Release the window context
			db->glDrawFunc = &DrawGLSceneBitmap;
			return (TRUE);
		}
	} 
	return (FALSE);
}


BOOL ProcessMedFile (TCHAR* szFilename, HWND Wnd, int nWidth_i, int nHeight_i, int nSlices_i)
{
	GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY);	// Fetch data base
	if (db != 0) {
		HDC Dc = GetWindowDC(Wnd);									// Get a window context
		wglMakeCurrent(Dc, db->Rc);									// Make sure our render context current

		HANDLE Handle = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ,
			0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);			// Open the file
		if (Handle == INVALID_HANDLE_VALUE) return 0;				// Invalid handle


		// Total size of image
		long imageSize = (long) nWidth_i * (long) nHeight_i * (long) nSlices_i;

		// Holds the luminance buffer
		char* chBuffer = malloc(imageSize);
		if (!chBuffer)
		{
			return FALSE;
		}

		// Holds the RGBA buffer
		char* pRGBABuffer = malloc(imageSize * 4);
		if (!pRGBABuffer)
		{
			return FALSE;
		}

		long Actual;
		ReadFile(Handle, chBuffer, imageSize, &Actual, 0);				// Read image data from file

		CloseHandle(Handle);											// We are done with file
		if (imageSize != Actual) {										// Check read worked
			return (FALSE);												// Read failed
		}

		// Convert the data to RGBA data.
		// Here we are simply putting the same value to R, G, B and A channels.
		// Usually for raw data, the alpha value will be constructed by a threshold value given by the user 

		for (int nIndx = 0; nIndx < imageSize; nIndx++)
		{
			pRGBABuffer[nIndx * 4] = chBuffer[imageSize-nIndx];
			pRGBABuffer[nIndx * 4 + 1] = chBuffer[imageSize-nIndx];
			pRGBABuffer[nIndx * 4 + 2] = chBuffer[imageSize-nIndx];
			pRGBABuffer[nIndx * 4 + 3] = chBuffer[imageSize-nIndx];
		}

		// If this function is getting called again for another data file.
		// Deleting and creating texture is not a good idea, 
		// we can use the glTexSubImage3D for better performance for such scenario.
		// I am not using that now :-)
		if (0 != db->glTexture)
		{
			glDeleteTextures(1, &db->glTexture);
		}
		glGenTextures(1, &db->glTexture);

		glBindTexture(GL_TEXTURE_3D, db->glTexture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		PFNGLTEXIMAGE3DPROC glTexImage3D =
			(PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, nWidth_i, nHeight_i, nSlices_i, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, pRGBABuffer);
		glBindTexture(GL_TEXTURE_3D, 0);

		free(chBuffer);
		free(pRGBABuffer);
		ReleaseDC(Wnd, Dc);											// Release the window context
		db->glDrawFunc = &DrawGLSceneVolumeRender;
		return TRUE;
	}
	return FALSE;
}


#define HB_LHSTATE_PROP  _T("LASTHOVERSTATE")
#define HB_GLUINT_PROP   _T("GLUINT")
#define HB_COMMAND_PROP  _T("COMMAND")

/*---------------------------------------------------------------------------
 Handler for the hover buttons subclassed off the standard button handler.
 It deals with normal button stuff as well as tab move to next window with
 WS_TABSTOP set.
 17May06 LdB
- -------------------------------------------------------------------------*/
static LRESULT FAR PASCAL GLHoverButtonHandler(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam){
	RECT R;
	long state, lstate, command;

	switch (Msg){
		case WM_GETDLGCODE:											// WM_GETDLGCODE MESSAGE
			return DLGC_DEFPUSHBUTTON;								// Default push button code returned
		/*case WM_CHAR:												// WM_CHAR MESSAGE
			if (wParam == VK_TAB){									// If tab key
				if ((GetKeyState(VK_SHIFT) & 0x8000) != 0){			// Move focus back/fwd dependant on shift
					SelectNext(hWnd, GetParent(hWnd), TRUE);
				}
				else SelectNext(hWnd, GetParent(hWnd), FALSE);
				return 0;											// Return message handled
			};
			break;*/
		case WM_PAINT: {											// WM_PAINT MESSAGE
				PAINTSTRUCT Ps;

			    BeginPaint(hWnd, &Ps);								// Begin paint
				
				GLuint gltex = (GLuint) GetProp(hWnd, HB_GLUINT_PROP);// Get saved gltexture
				HWND pw = GetParent(hWnd);	
				GLDATABASE* db = (GLDATABASE*) GetProp(pw, DATABASE_PROPERTY); // Fetch the data base	
			
				/*if (gltex != 0){									// Safety incase something wrong

					RECT r;
					GetWindowRect(hWnd, &r);
					int width = r.right - r.left;
					int height = r.bottom - r.top;


					HDC Dc = GetWindowDC(pw);								// Get a window context
					wglMakeCurrent(Dc, db->Rc);								// Make sure our render context current

					GLuint fbo = 0;
					glGenFramebuffers(1, &fbo);

					glBindFramebuffer(GL_FRAMEBUFFER, fbo);
					glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						GL_TEXTURE_2D, gltex, 0);
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
						GL_TEXTURE_2D, db->glTexture, 0);
					glDrawBuffer(GL_COLOR_ATTACHMENT1);
					
					lstate = HandleToLong(GetProp(hWnd, HB_LHSTATE_PROP));// Get saved last state
					if ((lstate == 0) && (GetFocus() == hWnd)) lstate = 1;// If we have focus now set lstate to 1

					glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
						GL_COLOR_BUFFER_BIT, GL_NEAREST);

					glDeleteFramebuffers(1, &fbo);

					ReleaseDC(pw, Dc);


				}*/

				if (gltex != 0){									// Safety incase something wrong
					
					PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D =
						(PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");

					RECT r;
					GetWindowRect(hWnd, &r);
					int width = r.right - r.left;
					int height = r.bottom - r.top;
					glEnable(GL_TEXTURE_2D);
					glGenTextures(1, &gltex);
					glBindTexture(GL_TEXTURE_2D, gltex);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gltex, 0);
					//glDisable(GL_TEXTURE_2D);
					lstate = HandleToLong(GetProp(hWnd, HB_LHSTATE_PROP));// Get saved last state
					if ((lstate == 0) && (GetFocus() == hWnd)) lstate = 1;// If we have focus now set lstate to 1

				}
				//BitBlt(Ps.hdc, 0, 0, R.right - R.left, R.bottom - R.top,
				//	MemDc, 0, lstate*(R.bottom - R.top), SRCCOPY);	// Transfer appropriate bitmap


				EndPaint(hWnd, &Ps);								// End paint routine
			}
			return 0;												// Return message handled
		case WM_ERASEBKGND:											// WM_ERASEBKGND MESSAGE
			return 1;												// Return message handled
		case WM_SETFOCUS:											// WM_SETFOCUS MESSAGE
			InvalidateRect(hWnd, NULL, TRUE);						// Invalidate area for total redraw
			return 0;												// return message handled
		case WM_KILLFOCUS:											// WM_KILLFOCUS MESSAGE
			InvalidateRect(hWnd, NULL, TRUE);						// Invalidate area for total redraw
			return 0;												// return message handled
		case WM_LBUTTONDBLCLK:										// WM_LBUTTONDBLCLK MESSAGE
		case WM_LBUTTONDOWN:										// WM_LBUTTONDOWN MESSAGE
			//if (CheckFocusCanMove(GetParent(hWnd)) == 1)return 0;	// Focus to this window
			SetProp(hWnd, HB_LHSTATE_PROP, (HANDLE) 2);				// Set hoverstate to 2
			InvalidateRect(hWnd, NULL, TRUE);						// Invalidtae area for total redraw
			return 0;
		case WM_LBUTTONUP:											// WM_LBUTTONUP MESSAGE
			lstate = HandleToLong(GetProp(hWnd, HB_LHSTATE_PROP));	// Get last hover state
			SetProp(hWnd, HB_LHSTATE_PROP, (HANDLE) 0);				// Set hoverstate to 0
			command = HandleToLong(GetProp(hWnd, HB_COMMAND_PROP));	// Get command message
			if (command == 0) command = WM_COMMAND;					// No property mean WM_COMMAND
			if (lstate == 2) PostMessage(GetParent(hWnd), command,
				(WPARAM) GetMenu(hWnd), (LPARAM) hWnd);				// If button was down post command
			InvalidateRect(hWnd, NULL, TRUE);						// Invalidate area for total redraw
			break;
		case WM_MOUSEMOVE:											// WM_MOUSEMOVE MESSAGE
			GetClientRect(hWnd, &R);								// Get client area
			lstate = HandleToLong(GetProp(hWnd, HB_LHSTATE_PROP));	// Get last hover state
			if (((R.left <= LOWORD(lParam)) && (LOWORD(lParam) <= R.right))
				&& ((R.top <= HIWORD(lParam)) && (HIWORD(lParam) <= R.bottom))){ // Mouse in button area
				if (GetCapture() != hWnd) {							// If we dont have mouse capture
					SetCapture(hWnd);								// Set us to have mouse capture
				};
				if (lstate == 2) {									// If left button down
					state = 2;										// Keep state as 2 (down state)
				}
				else {
					state = 1;										// Otherwise report as 1 (hover state)
				};
			}
			else {
				state = 0;											// Set new state to 0 (normal state)
				ReleaseCapture();									// Release the mouse capture
			};
			if (lstate != state){									// last and current states differ
				SetProp(hWnd, HB_LHSTATE_PROP, (HANDLE)state);		// Set new hover state
				InvalidateRect(hWnd, NULL, TRUE);					// Invalidate area for total redraw
			};
			break;
		case WM_DESTROY: {											// WM_DESTROY MESSAGE
				GLuint gltex = (GLuint) GetProp(hWnd, HB_GLUINT_PROP); // Get glTexture
				if (gltex != 0) glDeleteTextures(1, &gltex);		// Delete the texture
				RemoveProp(hWnd, HB_GLUINT_PROP);					// Remove the GL texture property
				RemoveProp(hWnd, HB_LHSTATE_PROP);					// Remove last hover state property
				RemoveProp(hWnd, HB_COMMAND_PROP);					// Removae any command property
			}
			break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);				// Call default handler
};

HWND CreateGLHoverButton (HWND Parent, 
					      TCHAR* BmpFileName, 
						  int x, 
						  int y, 
						  long Cmd,
						  long CmdId, 
						  BOOL HorzBmp, 
						  TCHAR* tooltip, 
						  WNDPROC proc)
{
	int bw, bh;
	/*-----------------------------------------------}
	{              LOAD BITMAP FROM FILE             }
	{-----------------------------------------------*/
	if (BmpFileName == 0) return 0;									// No valid filename exit
	GLuint btnTexture = BMP2GLTexture(BmpFileName, &bw, &bh);		// Convert the button bitmap to a glTexture
	if (btnTexture == 0) return 0;									// Bitmap file did not load

	/*-----------------------------------------------}
	{    ADJUST BITMAP IN DIRECTION IT IS TILED      }
	{-----------------------------------------------*/
	if (HorzBmp == TRUE){											// 3x bitmap is horizontal
		bw /= 3;													// Bitmap width is 1/3 actual bitmap width
	} else {														// 3x bitmap is vertical
		bh /= 3;													// Bitmap height is 1/3 actual bitmap height
	}

	/*-----------------------------------------------}
	{           CREATE THE BUTTON WINDOW             }
	{-----------------------------------------------*/
	HWND LWnd = CreateWindowEx(0, _T("BUTTON"), NULL,
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | BS_OWNERDRAW | WS_TABSTOP,
		x, y, bw, bh, Parent, LongToHandle(CmdId), 0, NULL);		// Create the window

	/*-----------------------------------------------}
	{        SET INITIAL PROPERTIES TO WINDOW        }
	{-----------------------------------------------*/
	//SetProp(LWnd, "VALIDATE", (HANDLE) (LONG_PTR) proc);				// Set validate function
	SetProp(LWnd, HB_LHSTATE_PROP, 0);								// Set last hoverstate property
	SetProp(LWnd, HB_GLUINT_PROP, (HANDLE) btnTexture);				// Set button texture property                                                                                         // Hold this handle for later
	if (Cmd != 0) SetProp(LWnd, HB_COMMAND_PROP, LongToHandle(Cmd));// Set command if specified
	SetWindowLong(LWnd, GWL_WNDPROC, (LONG) GLHoverButtonHandler);// Set new handler

	/*-----------------------------------------------}
	{        IF WE HAVE TOOLTIP THEN SET IT UP       }
	{-----------------------------------------------*/
	if (tooltip != 0){												// Check we have a tooltip
		InitCommonControls();										// Check common controls are initialized
		HWND TTWnd = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS,
			NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			LWnd, 0, 0, NULL);										// Create tooltip window
		SetWindowPos(TTWnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);				// Set window position
		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);								// Cize of structure
		ti.uFlags = TTF_SUBCLASS;									// Class
		ti.hwnd = LWnd;												// Parent window
		ti.hinst = 0;												// This instance
		ti.uId = 0;													// No uid
		ti.lpszText = tooltip;										// Transfer the text pointer
		GetClientRect(LWnd, &ti.rect);								// Tooltip to cover whole window
		SendMessage(TTWnd, TTM_ADDTOOL, 0, (LPARAM) &ti);			// Send tooltip to window
	};
	return LWnd;													// Return window
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					 	 MDI CHILD LEVEL ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
const int CIRCLEWIDTH = 700;
const int CIRCLEHEIGHT = 500;
const int CIRCLEPIXEL = 3;

/*--------------------------------------------------------------------------
						OpenGL MDICHILD handler
 --------------------------------------------------------------------------*/
static LRESULT CALLBACK OpenGLChildHandler (HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg){
		case WM_CREATE:	{											// WM_CREATE MESSAGE
				// Drag and drop functionality added
				DragAcceptFiles(Wnd, TRUE);

				RECT r;
				GetWindowRect(Wnd, &r);

				GLDATABASE* db = (GLDATABASE*) malloc(sizeof(GLDATABASE)); // Allocate structure
				db->Rc = InitGL(Wnd);								// Initialize OpenGL and get render context
				db->glDrawFunc = 0;									// Zero the glDrawFunc

				glGenFramebuffers(1, &db->frameBuf);				// Create a frameBuffer (fbo)
				db->fboWidth = r.right - r.left;
				db->fboHeight = r.bottom - r.top;
				db->fboDepth = 0;

				db->glTexture = 0;									// Zero the texture
				db->xrot = 0.0f;									// Zero x rotation
				db->yrot = 0.0f;									// Zero y rotation
				SetProp (Wnd, DATABASE_PROPERTY, (HANDLE) db);		// Set the database structure to a property on window
				ReSizeGLScene (Wnd);								// Rescale the OpenGL window

				CreateGLHoverButton(Wnd,
					_T("Button.bmp"),
					10,
					10,
					0,
					100,
					TRUE,
					0,
					0);
			}
			break;
		case WM_DESTROY: {											// WM_DESTROY MESSAGE
				GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY); // Fetch the data base
				if (db != 0) {
					if (db->Rc != 0) {								// Render context valid
						HDC Dc = GetWindowDC(Wnd);					// Get a window context
						wglMakeCurrent(Dc, db->Rc);					// Make sure our render context current
						if (db->glTexture != 0) {
							glDeleteTextures(1, &db->glTexture);	// If valid delete the texture							
						}
						if (db->frameBuf){	
							glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//Bind 0, which means render to back buffer, as a result, fb is unbound
							glDeleteFramebuffers(1, &db->frameBuf); // Delete any valid frame buffer
						}
						if (db->Rc != 0) wglDeleteContext(db->Rc);	// If valid delete the rendering context
						ReleaseDC(Wnd, Dc);							// Release the window context			
					}
					free(db);										// Release the data structure memory
				}
			}
			break;
		case WM_PAINT: {											// WM_PAINT MESSAGE
				PAINTSTRUCT Ps;
				GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY);// Fetch the data base
				BeginPaint (Wnd, &Ps);								// Begin paint
				if (db->glDrawFunc) db->glDrawFunc(db, Ps.hdc);		// Draw the OpenGL scene		
				SwapBuffers(Ps.hdc);								// Swap buffers
				EndPaint(Wnd, &Ps);									// End paint
				return 0;
			}
			break;
		case WM_TIMER: {											// WM_TIMER MESSAGE
				GLDATABASE* db = (GLDATABASE*) GetProp(Wnd, DATABASE_PROPERTY);// Fetch the data base
				db->xrot += 1.0f;									// Inc x rotation
				db->yrot += 1.0f;									// Inc y rotation
				InvalidateRect(Wnd, 0, TRUE);						// We need a redraw now so invalidate us			
			}
			break;
		case WM_WINDOWPOSCHANGED:									// WM_WINDOWPOSCHANGED
			// Check if window size has changed .. window move doesnt change aspect ratio
			if ((lParam == 0) || ((((PWINDOWPOS) lParam)->flags & SWP_NOSIZE) == 0)){
				ReSizeGLScene(Wnd);									// Rescale the GL window							
				InvalidateRect(Wnd, 0, TRUE);						// We need a redraw now so invalidate us
			}
			break;
		case WM_ERASEBKGND:											// WM_ERASEBKGND MESSAGE
			return 1;
		case WM_DROPFILES: {										// WM_DROPFILES ... Added for drag and drop support
				TCHAR szFilename[MAX_PATH];
				DragQueryFile((HDROP) wParam, 0, &szFilename[0], MAX_PATH);
				DragFinish((HDROP) wParam);
				if (ProcessBmpFile(szFilename, Wnd)) {				// Process the bitmap
					InvalidateRect(Wnd, 0, TRUE);					// Force redraw of window
				}
			}
			return 0;
	}
	return DefWindowProc(Wnd, Msg, wParam, lParam);					// Pass unprocessed message to DefWindowProc
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						APPLICATION LEVEL ROUTINES
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*--------------------------------------------------------------------------
                         Application handler 
 --------------------------------------------------------------------------*/
static LRESULT CALLBACK AppHandler (HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg){
		case WM_CREATE:	{											// WM_CREATE MESSAGE
			if (!SetupOpenGL3v3(Wnd)){								// OpenGL3.3 setup failed
				MessageBox(0, _T("OpenGL version 3.3 not initialized"), _T("APPLICATION WILL CLOSE"), MB_OK);
				PostQuitMessage(0);									// Post quit message
				return (0);
			}

			// Drag and drop functionality added
			DragAcceptFiles(Wnd, TRUE);

			// We are going to manually build a menu for the application
			// You could do this by resource file but this is another way

			HMENU SubMenu, Menu;
			Menu = CreateMenu();									// Create main menu item
			// Create a submenu and populate it
			SubMenu = CreatePopupMenu();							// Create a submenu popup
			AppendMenu(SubMenu, MF_STRING, IDC_BMPLOAD1, _T("&Left window bitmap load"));
			AppendMenu(SubMenu, MF_STRING, IDC_BMPLOAD2, _T("&Right window bitmap load"));
			AppendMenu(SubMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(SubMenu, MF_STRING, IDC_EXIT, _T("E&xit"));
			// Append the above popup submenu into our menu
			AppendMenu(Menu, MF_POPUP, (UINT_PTR) SubMenu, _T("&File"));
			SetMenu(Wnd, Menu);

			// We now need to split our window in two
			RECT r;
			GetWindowRect(Wnd, &r);								// Get our client area
			HWND Child1 = CreateWindowEx(0,
				OPENGLCHILD_CLASSNAME, _T("OpenGL MDI Client"),
				WS_CHILD | WS_VISIBLE | WS_BORDER,
				0, 0,
				(r.right - r.left) / 2, r.bottom - r.top,
				Wnd, NULL, GetModuleHandle(0),
				NULL);											// Child1 window in parent
			SetProp(Wnd, _T("CHILD1"), (HANDLE) Child1);
			SetTimer(Child1,									// handle to child window 
					1,											// timer identifier 
					100,										// 100 ms interval 
					0);											// timer callback null
			HWND Child2 = CreateWindowEx(0,
				OPENGLCHILD_CLASSNAME, _T("OpenGL MDI Client"),
				WS_CHILD | WS_VISIBLE | WS_BORDER,
				(r.right - r.left) / 2, 0,
				(r.right - r.left) / 2, r.bottom - r.top,
				Wnd, NULL, GetModuleHandle(0),
				NULL);												// Create the MDI Child
			SetProp(Wnd, _T("CHILD2"), (HANDLE) Child2);
			//SetTimer(Child2,										// handle to child window 
			//	1,													// timer identifier 
			//	100,												// 100 ms interval 
			//	0);													// timer callback null
			return (0);												// Return message handled
		}
		case WM_DESTROY:											// WM_DESTROY MESSAGE
			PostQuitMessage(0);										// Post quit message
			return (0);												// Return message handled
		case WM_COMMAND: {
			switch LOWORD(wParam){
				case IDC_BMPLOAD1: {                                // Menu item: File-->Load Bitmap window 1
						TCHAR FileName[256];
						int i = OpenFileDialog(&FileName[0], _countof(FileName),
							_T("MED file:"), _T("MED"), _T("MED FILE TO LOAD"), Wnd);
						if (i != 0) {
							// Fetch the child window
							HWND Child = GetProp(Wnd, _T("CHILD1"));
							if (Child != 0){
								if (ProcessMedFile(FileName, Child, 256, 256, 109)) {
								//if (ProcessBmpFile(FileName, Child)) {// Process the bitmap
									InvalidateRect(Child, 0, TRUE);	// Force redraw of window
								}
							}
						}
					}
					break;
					case IDC_BMPLOAD2: {                            // Menu item: File-->Load Bitmap window 2
						TCHAR FileName[256];
						int i = OpenFileDialog(&FileName[0], _countof(FileName),
							_T("BMP file:"), _T("BMP"), _T("BMP FILE TO LOAD"), Wnd);
						if (i != 0) {
							// Fetch the child window
							HWND Child = GetProp(Wnd, _T("CHILD2"));
							if (Child != 0){
								if (ProcessBmpFile(FileName, Child)) {// Process the bitmap
									InvalidateRect(Child, 0, TRUE);	// Force redraw of window
								}
							}
						}
					}
					break;
				case IDC_EXIT:										// Menu item: File-->Exit
					DestroyWindow(Wnd);
					break;
			};// End of switch wParam case
			return DefWindowProc(Wnd, WM_COMMAND, wParam, lParam);
		}
		case WM_WINDOWPOSCHANGED:									// WM_WINDOWPOSCHANGED
			// Check if window size has changed .. window move doesnt change aspect ratio
			if ((lParam == 0) || ((((PWINDOWPOS) lParam)->flags & SWP_NOSIZE) == 0)){
				// Neew to resize the split window
				RECT r;
				GetWindowRect(Wnd, &r);								// Get new client area
				int clientWth = (r.right - r.left) / 2;
				int clientHt = r.bottom - r.top;
				HWND child = (HWND) GetProp(Wnd, _T("CHILD1"));		// Get child1 window
				if (child) SetWindowPos(child, 0, 0, 0, clientWth, clientHt, SWP_NOZORDER);
				child = (HWND) GetProp(Wnd, _T("CHILD2"));			// Get child2 window
				if (child) SetWindowPos(child, 0, clientWth, 0, clientWth, clientHt, SWP_NOZORDER);
			}
			break;
		case WM_ERASEBKGND:											// WM_ERASEBKGND MESSAGE
			return 1;
        default: 
			return DefWindowProc(Wnd, Msg, wParam, lParam);			// Default window handler
   };// end switch case
   return (0);
};


/*--------------------------------------------------------------------------
 Application entry point
 --------------------------------------------------------------------------*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	TCHAR* AppClassName = _T("Windows Cipher");
	HACCEL hAccel = 0;												// no accelerator table in this example.
	MSG Msg;
	RECT R;
	HWND Wnd;
	WNDCLASSEX WndClass;

	// Initialize the common controls dll, specifying the type of control(s) required 
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccx.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&iccx);

	// MDI child class registration
	ZeroMemory(&WndClass, sizeof(WNDCLASSEX));						// Clear the class record
	WndClass.cbSize = sizeof(WNDCLASSEX);							// Size of this record
	WndClass.style = CS_HREDRAW | CS_VREDRAW;						// Set class styles
	WndClass.lpfnWndProc = OpenGLChildHandler;						// Handler for this class
	WndClass.cbClsExtra = 0;										// No extra class data
	WndClass.cbWndExtra = 0;										// No extra window data
	WndClass.hInstance = GetModuleHandle(NULL);						// This instance
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);					// Set icon
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);					// Set cursor
	WndClass.hbrBackground = GetStockObject(NULL_BRUSH);			// Set background brush
	WndClass.lpszMenuName = NULL;									// No menu yet
	WndClass.lpszClassName = OPENGLCHILD_CLASSNAME;					// Set class name
	RegisterClassEx(&WndClass);										// Register the class

	// Application class registration
	ZeroMemory(&WndClass, sizeof(WNDCLASSEX));						// Clear the class record
	WndClass.cbSize = sizeof(WNDCLASSEX);							// Size of this record
	WndClass.lpfnWndProc = AppHandler;								// Handler for this class
	WndClass.cbClsExtra = 0;										// No extra class data
	WndClass.cbWndExtra = 0;										// No extra window data
	WndClass.hInstance = GetModuleHandle(NULL);						// This instance
	WndClass.hIcon = LoadIcon(0, IDI_APPLICATION);					// Set icon
	WndClass.hCursor = LoadCursor(0, IDC_ARROW);					// Set cursor
	WndClass.hbrBackground = (HBRUSH) GetStockObject(NULL_BRUSH);	// Set background brush
	WndClass.lpszMenuName = 0;										// No menu
	WndClass.lpszClassName = AppClassName;							// Set class name
	RegisterClassEx(&WndClass);										// Register the class

	GetClientRect(GetDesktopWindow(), &R);							// Get desktop area					
   	Wnd = CreateWindowEx(0, AppClassName, _T("OpenGL Splitter Example"), 
		WS_VISIBLE | WS_OVERLAPPEDWINDOW, R.left+50, R.top+50, 
		R.right-R.left-100, R.bottom-R.top-100,
		0, 0, GetModuleHandle(NULL), 
		NULL);														// Create main window
	while (GetMessage(&Msg, 0, 0, 0)){								// Get messages
		if (!TranslateAccelerator(Wnd, hAccel, &Msg))				// All other keyboard msgs
		{
			TranslateMessage(&Msg);									// Translate each message
			DispatchMessage(&Msg);									// Dispatch each message
		}
	}
	return (0);
}

