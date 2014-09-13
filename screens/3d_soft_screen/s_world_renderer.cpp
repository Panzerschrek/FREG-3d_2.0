#include "s_world_renderer.h"
#include "../../world.h"

#include "psr/rasterization.h"
#include "psr/rendering_commands.h"

s_WorldRenderer::s_WorldRenderer(const World *w):
world(w),
chunk_matrix_size{w->NumShreds(),w->NumShreds() },


frame_count(0)
{

    chunks= new s_ChunkInfo*[ chunk_matrix_size[0]*chunk_matrix_size[1] ];

    for( int y= 0; y< chunk_matrix_size[1]; y++ )
        for( int x= 0; x< chunk_matrix_size[0]; x++ )
        {
            chunks[ x + y*chunk_matrix_size[0] ]= new s_ChunkInfo( world->GetShredByPos(x,y) );
        }

    //setup chunk neighbors
    for( int y= 0; y< chunk_matrix_size[1]; y++ )
        for( int x= 0; x< chunk_matrix_size[0]; x++ )
        {
            s_ChunkInfo* ch= chunks[ x + y*chunk_matrix_size[0] ];
            if( x != chunk_matrix_size[0]-1 )
                ch->neighbors[0]= chunks[ x + 1 + y*chunk_matrix_size[0] ];
            else
                ch->neighbors[0]= nullptr;
            if( x != 0 )
                ch->neighbors[1]= chunks[ x - 1 + y*chunk_matrix_size[0] ];
            else
                ch->neighbors[1]= nullptr;

            if( y != chunk_matrix_size[1]-1 )
                ch->neighbors[2]= chunks[ (y+1)*chunk_matrix_size[0] ];
            else
                ch->neighbors[2]= nullptr;
            if( y != 0 )
                ch->neighbors[3]= chunks[ (y-1)*chunk_matrix_size[0] ];
            else
                ch->neighbors[3]= nullptr;
        }
}

s_WorldRenderer::~s_WorldRenderer()
{
    for( int y= 0; y< chunk_matrix_size[1]; y++ )
        for( int x= 0; x< chunk_matrix_size[0]; x++ )
            delete chunks[ x + y*chunk_matrix_size[0] ];

    delete chunks;
}


void s_WorldRenderer::BuildWorld()
{
    for( int i= 0; i< chunk_matrix_size[0] * chunk_matrix_size[1]; i++ )
    {
        s_ChunkInfo* ch= chunks[i];
        ch->GetTransparency();
        ch->GetQuadCount();

        //allocate memory for front and back quads
        ch->quad_buffer[1].allocated_quad_count= ch->quad_buffer[1].quad_count + ( ch->quad_buffer[1].quad_count>>2 );//+25%
        ch->quad_buffer[1].quads= new s_WorldQuad[ ch->quad_buffer[1].allocated_quad_count ];

        ch->quad_buffer[0].quad_count= 0;
        ch->quad_buffer[0].allocated_quad_count= ch->quad_buffer[1].allocated_quad_count;
        ch->quad_buffer[0].quads= new s_WorldQuad[ ch->quad_buffer[0].allocated_quad_count ];

        ch->GenChunk();
    }//for all chunks
}

namespace Draw
{
void SetConstantColor( const unsigned char* color );
}


void s_WorldRenderer::Draw()
{
    unsigned char clear_color[4];
    clear_color[0]= 255;
    clear_color[1]= 255;
    clear_color[2]= 255;

    PRast_ClearColorBuffer( clear_color );

    int vertices[]= {
        65536*32, 65536*64, 65536,
        65536*4, 65536*128, 65536,
        65536*64, 65536*128, 65536,
        65536*128, 65536*364, 65536 };

    static const unsigned char triangle_color[]= { 32, 255, 255, 0 };
    Draw::SetConstantColor( triangle_color );

    DrawWorldTriangleSimple( (char*)vertices );
}
