#include <QTime>

#include <QImage>
#include <QPainter>

#include "s_world_renderer.h"
#include "../../world.h"

#include "psr/rasterization.h"
#include "psr/rendering_commands.h"
#include "psr/vertex_processing.h"
#include "psr/fixed.h"
#include "psr/fast_math.h"
#include "psr/texture.h"



static Texture tmp_tex;


s_WorldRenderer::s_WorldRenderer( World *w):
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

    QImage img( 16, 16, QImage::Format_RGB32 );
    QPainter p(&img);
    p.fillRect( 0, 0, 16, 16, Qt::darkGreen );
    p.drawText( 4, 13, QString("%") );

    tmp_tex.Create( 16, 16, false, nullptr, img.constBits(), Texture::RESIZE_NO, false );
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
    world->Lock();

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

    world->Unlock();
}

namespace Draw
{
void SetConstantColor( const unsigned char* color );
void SetTexture( const Texture* t );
void SetTextureLod( const Texture* t, int lod );
void SetConstantLight( int l );
}


void s_WorldRenderer::Draw()
{

    if( frame_count == 0 )
    {
        int i= rand();
        fixed16_t inv= Fixed16Invert( i );
        printf( "invert of %d is %d\n", i, inv );
        BuildWorld();
    }
    static const unsigned char clear_color[4]= { 64,64,64,64 };
    PRast_ClearColorBuffer( clear_color );
    PRast_ClearDepthBuffer( 0 );
    Draw::SetTexture( &tmp_tex );


    /*int vertices[]= {
        65536*32, 65536*64, 65536,
        65536*4, 65536*128, 65536,
        65536*64, 65536*128, 65536,
        65536*128, 65536*364, 65536 };
   static const unsigned char triangle_color[]= { 24, 24, 24, 0 };
   Draw::SetConstantColor( triangle_color );
   DrawWorldTriangleSimple( (char*)vertices );
*/
    m_Mat4 rotate_z, rotate_x, perspective, shift;
    shift.Identity();
    m_Vec3 tr= -cam_pos;
    shift.Translate( tr );

    rotate_z.RotateZ( cam_ang.z );
    rotate_x.RotateX( cam_ang.x );
    perspective.Identity();
    perspective[0]= 1.0f;
    perspective[5]= float( screen_size_x ) / float( screen_size_y ) ;

    view_matrix= shift * rotate_z * rotate_x * perspective;

    float width_f= float( screen_size_x ) * 65536.0f;
    float height_f= float( screen_size_y ) * 65536.0f;
    float width2_f= float( screen_size_x ) * 0.5f * 65536.0f;
    float height2_f= float( screen_size_y ) * 0.5f * 65536.0f;

    char buffer[500];

    int light_normal_table[]= { 230, 230, 240, 240, 255, 255 };
    float texels_per_pixel= 2.0f * 16.0f / float( screen_size_x );

    for( int c= 0; c<  chunk_matrix_size[0] * chunk_matrix_size[1]; c++ )
    {
        s_WorldQuad* q= chunks[c]->quad_buffer[1].quads;
        int quad_count= chunks[c]->quad_buffer[1].quad_count;
        for( int i= 0; i< quad_count; i++, q++ )
        {
            m_Vec3 transformed_vertices[4];
            for( int j= 0; j< 4; j++ )
            {
                //transformed_vertices[1]= *((m_Vec3*)(q->coord+3*j)) * view_matrix;
                m_Vec3 tmp_v(q->coord+3*j);
                transformed_vertices[j]= tmp_v * view_matrix;
                float inv_z= 1.0f / transformed_vertices[j].z;
                transformed_vertices[j].x= ( transformed_vertices[j].x * inv_z + 1.0f ) * width2_f;
                transformed_vertices[j].y= ( transformed_vertices[j].y * inv_z + 1.0f ) * height2_f;
                transformed_vertices[j].z*= 65536.0f;
            }
            if( transformed_vertices[0].z < 1.0f || transformed_vertices[1].z < 1.0f ||
                    transformed_vertices[2].z < 1.0f || transformed_vertices[3].z < 1.0f )
                continue;
            if(  transformed_vertices[0].x < 0.0f ||  transformed_vertices[0].y < 0.0f ||
                 transformed_vertices[0].x > width_f ||  transformed_vertices[0].y > height_f )
                continue;
            if(  transformed_vertices[1].x < 0.0f ||  transformed_vertices[1].y < 0.0f ||
                 transformed_vertices[1].x > width_f ||  transformed_vertices[1].y > height_f )
                continue;
            if(  transformed_vertices[2].x < 0.0f ||  transformed_vertices[2].y < 0.0f ||
                 transformed_vertices[2].x > width_f ||  transformed_vertices[2].y > height_f )
                continue;

            int mip= FastIntLog2Clamp0( int(1.5f * transformed_vertices[0].z * texels_per_pixel * 0.0000152587890625f) );
            if( mip > 3) mip = 3;

            Draw::SetTextureLod( &tmp_tex, mip );
            Draw::SetConstantLight( light_normal_table[q->normal] );
            using namespace VertexProcessing;
            /*int r= i&7;
            int g= (i>>3)&7;
            int b= (i>>6)&3;
            unsigned char color[4];
            color[0]= b*85;
            color[1]= g*36;
            color[2]= r*36;
            Draw::SetConstantColor( color );*/

            triangle_in_vertex_xy[0]= fixed16_t(transformed_vertices[0].x);
            triangle_in_vertex_xy[1]= fixed16_t(transformed_vertices[0].y);
            triangle_in_vertex_xy[2]= fixed16_t(transformed_vertices[1].x);
            triangle_in_vertex_xy[3]= fixed16_t(transformed_vertices[1].y);
            triangle_in_vertex_xy[4]= fixed16_t(transformed_vertices[2].x);
            triangle_in_vertex_xy[5]= fixed16_t(transformed_vertices[2].y);
            triangle_in_vertex_z[0]= fixed16_t( transformed_vertices[0].z);
            triangle_in_vertex_z[1]= fixed16_t( transformed_vertices[1].z);
            triangle_in_vertex_z[2]= fixed16_t( transformed_vertices[2].z);
            triangle_in_tex_coord[0]= q->tc[0]>>mip;
            triangle_in_tex_coord[1]= q->tc[1]>>mip;
            triangle_in_tex_coord[2]= q->tc[2]>>mip;
            triangle_in_tex_coord[3]= q->tc[3]>>mip;
            triangle_in_tex_coord[4]= q->tc[4]>>mip;
            triangle_in_tex_coord[5]= q->tc[5]>>mip;

            if( DrawWorldTriangleSimpleToBuffer( buffer ) != 0 )
                DrawWorldTriangleSimple( buffer );


            triangle_in_vertex_xy[2]= fixed16_t(transformed_vertices[3].x);
            triangle_in_vertex_xy[3]= fixed16_t(transformed_vertices[3].y);
            triangle_in_vertex_z[1]= fixed16_t( transformed_vertices[3].z);
            triangle_in_tex_coord[2]= q->tc[6]>>mip;
            triangle_in_tex_coord[3]= q->tc[7]>>mip;

            if( DrawWorldTriangleSimpleToBuffer( buffer ) != 0 )
                DrawWorldTriangleSimple( buffer );

        }

    }//for chunks
    frame_count++;
}
