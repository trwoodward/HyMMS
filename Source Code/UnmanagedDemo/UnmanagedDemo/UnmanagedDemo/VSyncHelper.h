#ifndef  _VSYNC_H_
#define _VSYNC_H_
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

//From Beginning OpenGL Game Programming by Astle and Hawkins, 
//see Project report for full reference
typedef BOOL (APIENTRY *PFNWGLSWAPINTERVALFARPROC)( int );
PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

void setVSync(int interval=1)
{
	const char *extensions = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) == 0 )
		return;
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress( "wglSwapIntervalEXT" );

		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT(interval);
	}
}

#endif // ! _VSYNC_H_
