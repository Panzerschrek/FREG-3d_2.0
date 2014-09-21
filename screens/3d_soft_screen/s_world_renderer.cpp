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

#include "s_texture_manager.h"

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

    texture_manager= new s_TextureManager();
}

s_WorldRenderer::~s_WorldRenderer()
{
    for( int y= 0; y< chunk_matrix_size[1]; y++ )
        for( int x= 0; x< chunk_matrix_size[0]; x++ )
            delete chunks[ x + y*chunk_matrix_size[0] ];

    delete chunks;
    delete texture_manager;
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
void SetTextureRaw( const unsigned char* data, int size_x_log2, int size_y_log2 );
void SetConstantLight( int l );
void SetConstantTime( fixed16_t time );
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
    static const unsigned char clear_color[4]= { 223,194,117,64 };
    PRast_ClearColorBuffer( clear_color );
    PRast_ClearDepthBuffer( 0 );
    Draw::SetConstantTime( QTime::currentTime().msecsSinceStartOfDay() * 10000 );

    m_Mat4 rotate_z, rotate_x, perspective, shift;
    shift.Identity();
    m_Vec3 tr= -cam_pos;
    shift.Translate( tr );

    rotate_z.RotateZ( cam_ang.z );
    rotate_x.RotateX( cam_ang.x );
    perspective.Identity();


    float width_f= float( screen_size_x ) * 65536.0f;
    float height_f= float( screen_size_y ) * 65536.0f;
    float width2_f= float( screen_size_x ) * 0.5f * 65536.0f;
    float height2_f= float( screen_size_y ) * 0.5f * 65536.0f;

    perspective[0]= -1.0f * width2_f;
    perspective[5]= height2_f * float( screen_size_x ) / float( screen_size_y ) ;


    view_matrix= shift * rotate_z * rotate_x * perspective;

    char buffer[500];

    int light_normal_table[]= { 230, 230, 240, 240, 255, 255 };
    float texels_per_pixel= (0.0000152587890625f * 2.0f * 32.0f * 1.3f) / float( screen_size_x );

    view_matrix.Transpose();
    int total_quads = 0;
    int tex_size_log2= FastIntLog2( texture_manager->GetTextureSize() );
    int max_mip= tex_size_log2-1;
    for( int c= 0; c<  chunk_matrix_size[0] * chunk_matrix_size[1]; c++ )
    {
        s_WorldQuad* q= chunks[c]->quad_buffer[1].quads;
        int quad_count= chunks[c]->quad_buffer[1].quad_count;
        for( int i= 0; i< quad_count; i++, q++ )
        {
            //back face culling
            static const float normal_signs[]= { 1.0f, -1.0f };
            int vec_component_index= q->normal>>1;
            float dst_to_quad_plane= normal_signs[q->normal&1] *
                    ( q->coord[ vec_component_index ] - cam_pos.ToArr()[ vec_component_index ] );
            if(  dst_to_quad_plane < 0.0f )
                continue;


            m_Vec3 transformed_vertices[4];
            for( int j= 0; j< 4; j++ )
            {
                m_Vec3 tmp_v(q->coord+3*j);
                transformed_vertices[j]= view_matrix * tmp_v;
                float inv_z= 1.0f / transformed_vertices[j].z;
                //transformed_vertices[j].x= ( transformed_vertices[j].x * inv_z + 1.0f ) * width2_f;
               // transformed_vertices[j].y= ( transformed_vertices[j].y * inv_z + 1.0f ) * height2_f;
                transformed_vertices[j].x=  transformed_vertices[j].x * inv_z + width2_f;
                transformed_vertices[j].y=  transformed_vertices[j].y * inv_z + height2_f;
                transformed_vertices[j].z*= 65536.0f;
            }
            const float zmin_f= float(PSR_MIN_ZMIN);
            if( transformed_vertices[0].z < zmin_f || transformed_vertices[1].z < zmin_f ||
                    transformed_vertices[2].z < zmin_f || transformed_vertices[3].z < zmin_f )
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
            if(  transformed_vertices[3].x < 0.0f ||  transformed_vertices[3].y < 0.0f ||
                 transformed_vertices[3].x > width_f ||  transformed_vertices[3].y > height_f )
                continue;

            //mip level calculating
            m_Vec3 vec_to_quad( cam_pos.x - q->coord[0], cam_pos.y - q->coord[1], cam_pos.z - q->coord[2] );
            float inv_angle_cos= vec_to_quad.Length() / dst_to_quad_plane;
            int mip= FastIntLog2Clamp0( int( inv_angle_cos * transformed_vertices[0].z * texels_per_pixel ) );
            if( mip > max_mip) mip = max_mip;

            int tex_coord_shift= tex_size_log2 - mip;
            Draw::SetTextureRaw( texture_manager->GetTextureData( q->tex_id, mip ), tex_coord_shift, tex_coord_shift );

            Draw::SetConstantLight( (light_normal_table[q->normal] * q->light )>>4 );
            using namespace VertexProcessing;

            triangle_in_vertex_xy[0]= fixed16_t(transformed_vertices[0].x);
            triangle_in_vertex_xy[1]= fixed16_t(transformed_vertices[0].y);
            triangle_in_vertex_xy[2]= fixed16_t(transformed_vertices[1].x);
            triangle_in_vertex_xy[3]= fixed16_t(transformed_vertices[1].y);
            triangle_in_vertex_xy[4]= fixed16_t(transformed_vertices[2].x);
            triangle_in_vertex_xy[5]= fixed16_t(transformed_vertices[2].y);
            triangle_in_vertex_z[0]= fixed16_t( transformed_vertices[0].z);
            triangle_in_vertex_z[1]= fixed16_t( transformed_vertices[1].z);
            triangle_in_vertex_z[2]= fixed16_t( transformed_vertices[2].z);
            triangle_in_tex_coord[0]= q->tc[0]<<tex_coord_shift;
            triangle_in_tex_coord[1]= q->tc[1]<<tex_coord_shift;
            triangle_in_tex_coord[2]= q->tc[2]<<tex_coord_shift;
            triangle_in_tex_coord[3]= q->tc[3]<<tex_coord_shift;
            triangle_in_tex_coord[4]= q->tc[4]<<tex_coord_shift;
            triangle_in_tex_coord[5]= q->tc[5]<<tex_coord_shift;

            if( DrawWorldTriangleToBuffer( buffer ) != 0 )
                DrawWorldTriangle( buffer );


            triangle_in_vertex_xy[2]= fixed16_t(transformed_vertices[3].x);
            triangle_in_vertex_xy[3]= fixed16_t(transformed_vertices[3].y);
            triangle_in_vertex_z[1]= fixed16_t( transformed_vertices[3].z);
            triangle_in_tex_coord[2]= q->tc[6]<<tex_coord_shift;
            triangle_in_tex_coord[3]= q->tc[7]<<tex_coord_shift;

            if( DrawWorldTriangleToBuffer( buffer ) != 0 )
                DrawWorldTriangle( buffer );

            total_quads++;
        }//for chunk quads

    }//for chunks
    frame_count++;
    //if( (rand()&16) == 0 )
    //    printf( "quads: %d\n", total_quads );
}
