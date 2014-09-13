#ifndef RASTERIZATION_H
#define RASTERIZATION_H
#include "psr.h"


#ifdef __cplusplus
extern "C"
{
#endif


void PRast_Init( int screen_width, int screen_height );
void PRast_Shutdown();
void PRast_SetFramebuffer( unsigned char* buff );
unsigned char* PRast_GetFrontBuffer();
unsigned char* PRast_SwapBuffers();
void PRast_SwapRedBlueLostAlphaInFramebuffer();
void PRast_SwapRedBlueInFramebuffer();
void PRast_MirrorFramebufferVertical();
void PRast_MakeGammaCorrection(const unsigned char* gamma_table);
void PRast_AddFullscreenExponentialFog( float half_distance, const unsigned char* color );

//common draw functions
void PRast_ClearColorBuffer( const unsigned char* color );
void PRast_ClearDepthBuffer( depth_buffer_t value );


extern void (*DrawWorldTriangleSimple)(char*);
extern void (*DrawWorldTriangle)(char*);

/*hack, for generation 1 mov instruction instead 3(4):
mov eax, dword ptr[src]
mov dword ptr [dst], eax
;instead
mov al, byte ptr[src]
mov byte ptr [dst], al
mov al, byte ptr[src+1]
mov byte ptr [dst+1], al
mov al, byte ptr[src+2]
mov byte ptr [dst+2], al
mov al, byte ptr[src+3]
mov byte ptr [dst+3], al
*/
#define Byte4Copy( dst, src ) *((int*)(dst))=  *((int*)(src))

#ifdef __cplusplus
//extern "C"
}
#endif


#endif//RASTERIZATION
