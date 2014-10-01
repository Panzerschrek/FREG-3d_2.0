#include <QTime>

#include <QImage>
#include <QPainter>

#include "s_world_renderer.h"
#include "../../world.h"
#include "../../shred.h"

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
                ch->neighbors[2]= chunks[ x + (y+1)*chunk_matrix_size[0] ];
            else
                ch->neighbors[2]= nullptr;
            if( y != 0 )
                ch->neighbors[3]= chunks[ x + (y-1)*chunk_matrix_size[0] ];
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
        chunks[i]->GetTransparency();

    for( int i= 0; i< chunk_matrix_size[0] * chunk_matrix_size[1]; i++ )
    {
        s_ChunkInfo* ch= chunks[i];
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


void s_WorldRenderer::InitClipPlanes()
{
    fov_x= m_Math::FM_PI2;
    fov_y= 2.0f * atanf( tanf(fov_x*0.5f) * float(screen_size_x) / float(screen_size_y) );
   // fov_x= fov_y= 3.1414926535f * 0.5f;

    m_Mat4 normal_mat, rotate_z, rotate_x;
    rotate_z.Identity();rotate_z.RotateZ( -cam_ang.z );//check
    rotate_x.RotateX( m_Math::FM_PI2 - cam_ang.x );
    normal_mat= rotate_x * rotate_z;

    Plane* plane= clip_planes + 0;
    plane->v[0]= 0.0f;
    plane->v[1]= 1.0f;
    plane->v[2]= 0.0f;
    *((m_Vec3*)plane->v)= *((m_Vec3*)plane->v) * normal_mat;
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2] + PSR_MIN_ZMIN_FLOAT * 1.5f;

    //top clip plane
    float fov_multipler= 0.5f * 1.05f;
    plane= clip_planes + 1;
    plane->v[0]= 0.0f;
    plane->v[1]= sinf( fov_x * fov_multipler );
    plane->v[2]= -cosf( fov_x * fov_multipler );
    *((m_Vec3*)plane->v)= *((m_Vec3*)plane->v) * normal_mat;
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2];
    //bottom clip plane
    plane= clip_planes + 2;
    plane->v[0]= 0.0f;
    plane->v[1]= sinf( fov_x * fov_multipler );
    plane->v[2]= cosf( fov_x * fov_multipler );
    *((m_Vec3*)plane->v)= *((m_Vec3*)plane->v) * normal_mat;
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2];
    //left plane
    plane= clip_planes + 3;
    plane->v[0]= cosf( fov_y * fov_multipler );
    plane->v[1]= sinf( fov_y * fov_multipler );
    plane->v[2]= 0.0f;
    *((m_Vec3*)plane->v)= *((m_Vec3*)plane->v) * normal_mat;
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2];
    //right plane
    plane= clip_planes + 4;
    plane->v[0]= -cosf( fov_y * fov_multipler );
    plane->v[1]= sinf( fov_y * fov_multipler );
    plane->v[2]= 0.0f;
    *((m_Vec3*)plane->v)= *((m_Vec3*)plane->v) * normal_mat;
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2];


    /*plane= clip_planes + 0;
    plane->v[0]= cosf( 0.1f );
    plane->v[1]= 0.0f;
    plane->v[2]= sinf( 0.1f );
    plane->v[3]= cam_pos.x * plane->v[0] + cam_pos.y * plane->v[1] + cam_pos.z * plane->v[2];*/

}

extern "C" void PRast_AddFullscreenExponentialFog( float fog_half_distance, const unsigned char* color );
namespace Draw
{
void SetConstantColor( const unsigned char* color );
void SetTexture( const Texture* t );
void SetTextureLod( const Texture* t, int lod );
void SetTextureRaw( const unsigned char* data, int size_x_log2, int size_y_log2 );
void SetConstantLight( int l );
void SetConstantTime( fixed16_t time );
}


#define DotProduct(x,y) ((x)[0]*(y)[0] + (x)[1]*(y)[1] + (x)[2]*(y)[2])
struct ClipVertex
{
    float coord[3];
    float tex_coord[2];
};

const int MAX_TMP_VERTICES= 24;
static ClipVertex* s_active_vertices[ MAX_TMP_VERTICES ];
static bool s_vertex_plane_pos[ MAX_TMP_VERTICES ];
static ClipVertex s_tmp_vertices[ MAX_TMP_VERTICES * 3 ];
static int s_active_vertex_count;
static int s_tmp_vertex_stack_pos;


int ClipWorldQuadByPlane( const Plane* plane )
{
    const float* normal= plane->v;

        int discarded_vertex_count= 0;
        for( int i= 0; i< s_active_vertex_count; i++ )
        {
            if( DotProduct( normal, s_active_vertices[i]->coord ) > plane->v[3] )
                s_vertex_plane_pos[i]= true;
            else
            {
                discarded_vertex_count++;
                s_vertex_plane_pos[i]= false;// false means discarded
            }
        }

        if( discarded_vertex_count == s_active_vertex_count )
            return 0;
        else if( discarded_vertex_count == 0 )
            return s_active_vertex_count;


        int splitted_edge[2];
        //0 - index of discarded vertex
        //1 - index of passed vertex
        for( int i= 0; i< s_active_vertex_count; i++ )
        {
            int next_vertex_index= i+1; if( next_vertex_index == s_active_vertex_count ) next_vertex_index= 0;
            if( !s_vertex_plane_pos[i] )//if back vertex
            {
                if( s_vertex_plane_pos[next_vertex_index] )//if front vertex
                    splitted_edge[0]= i;
            }
            else//if fron vertex
            {
                if(!s_vertex_plane_pos[next_vertex_index])//if back vertex
                    splitted_edge[1]= i;
            }
        }

        ClipVertex* new_v[2]={  s_tmp_vertices + s_tmp_vertex_stack_pos, s_tmp_vertices + s_tmp_vertex_stack_pos + 1 };
        s_tmp_vertex_stack_pos+= 2;
        for( int i= 0; i< 2; i++ )
        {
            int v0_ind= splitted_edge[i];
            int v1_ind= splitted_edge[i]+1; if( v1_ind == s_active_vertex_count ) v1_ind= 0;
            float* v0= s_active_vertices[v0_ind]->coord, *v1= s_active_vertices[v1_ind]->coord;

            float k0= fabsf( DotProduct(v0,normal) - plane->v[3] );
            float k1= fabsf( DotProduct(v1,normal) - plane->v[3] );
            float inv_dst_sum= 1.0f / ( k0 + k1 );
            k0*= inv_dst_sum;
            k1*= inv_dst_sum;

            float* out_pos= new_v[i]->coord;
            out_pos[0]= k1 * v0[0] + k0 * v1[0];
            out_pos[1]= k1 * v0[1] + k0 * v1[1];
            out_pos[2]= k1 * v0[2] + k0 * v1[2];
            new_v[i]->tex_coord[0]= k1 * s_active_vertices[v0_ind]->tex_coord[0] + k0 * s_active_vertices[v1_ind]->tex_coord[0];
            new_v[i]->tex_coord[1]= k1 * s_active_vertices[v0_ind]->tex_coord[1] + k0 * s_active_vertices[v1_ind]->tex_coord[1];
        }

        if( discarded_vertex_count == 2 )
        {
            s_active_vertices[ splitted_edge[0] ]= new_v[0];
            int new_v_ind= splitted_edge[1]+1; if( new_v_ind == s_active_vertex_count ) new_v_ind= 0;
            s_active_vertices[new_v_ind]= new_v[1];
            return s_active_vertex_count;
        }
        else if( discarded_vertex_count > 2 )
        {
            ClipVertex* tmp_vertices[MAX_TMP_VERTICES];
            for( int i= 0; i< s_active_vertex_count; i++ )
                tmp_vertices[i]= s_active_vertices[i];

            s_active_vertices[0]= new_v[1];
            s_active_vertices[1]= new_v[0];
            int new_vertex_count= s_active_vertex_count - discarded_vertex_count + 2;
            for( int i= 2, j= splitted_edge[0]+1; i < new_vertex_count; i++, j++ )
            {
                s_active_vertices[i]= tmp_vertices[j%s_active_vertex_count];
            }
            return new_vertex_count;
        }
        else//discard one vertex
        {
            int discarded_v_ind= splitted_edge[0];
            for( int i= s_active_vertex_count; i> discarded_v_ind; i-- )
                s_active_vertices[i]= s_active_vertices[i-1];

            s_active_vertices[discarded_v_ind  ]= new_v[1];
            s_active_vertices[discarded_v_ind+1]= new_v[0];
            return s_active_vertex_count + 1;
        }
}

int s_WorldRenderer::ClipWorldQuad( const s_WorldQuad* quad )
{
    for( int i= 0; i< 4; i++ )
    {
        s_active_vertices[i]= s_tmp_vertices+i;
        for( int j= 0; j< 3; j++ )
            s_active_vertices[i]->coord[j]= quad->coord[i*3+j];
        s_active_vertices[i]->tex_coord[0]= float(quad->tc[i*2+0]);
        s_active_vertices[i]->tex_coord[1]= float(quad->tc[i*2+1]);
    }
    s_active_vertex_count= 4;
    s_tmp_vertex_stack_pos= 4;

    for( int i= 0; i< 5; i++ )
    {
        s_active_vertex_count= ClipWorldQuadByPlane( clip_planes + i );
        if( s_active_vertex_count == 0 )
            break;
    }
    return s_active_vertex_count;
}

void s_WorldRenderer::Draw()
{

    if( frame_count == 0 )
    {
        BuildWorld();
    }
    static const unsigned char clear_color[4]= { 223,194,117,64 };
    PRast_ClearColorBuffer( clear_color );
    PRast_ClearDepthBuffer( 0 );
    Draw::SetConstantTime( QTime::currentTime().msecsSinceStartOfDay() * 10000 );

    InitClipPlanes();

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

    perspective[0]= -1.0f * width2_f / tanf( fov_y * 0.5f );
    perspective[5]= height2_f / tanf( fov_x * 0.5f );


    view_matrix= shift * rotate_z * rotate_x * perspective;

    char buffer[500];

    int light_normal_table[]= { 230, 230, 240, 240, 255, 255 };
    float texels_per_pixel= float(texture_manager->GetTextureSize()) * (0.0000152587890625f * 2.0f * 1.3f) / float( screen_size_x );

    view_matrix.Transpose();
    int total_quads = 0;
    int tex_size_log2= FastIntLog2( texture_manager->GetTextureSize() );
    int max_mip= tex_size_log2-1;
    int chunks_passed= 0;
    for( int c= 0; c<  chunk_matrix_size[0] * chunk_matrix_size[1]; c++ )
    {
        s_ChunkInfo* ch= chunks[c];

        bool chunk_clipped= false;
        for( int i= 0; i< 5; i++ )
            if( ch->IsBehindPlane( clip_planes+i ) )
            {
                chunk_clipped= true;
                break;
            }
        if( chunk_clipped )
            continue;
        chunks_passed++;

        s_WorldQuad* q= ch->quad_buffer[1].quads;
        int quad_count= ch->quad_buffer[1].quad_count;
        for( int i= 0; i< quad_count; i++, q++ )
        {
            m_Vec3 transformed_vertices[ MAX_TMP_VERTICES ];

            //back face culling
            static const float normal_signs[]= { 1.0f, -1.0f };
            int vec_component_index= q->normal>>1;
            float dst_to_quad_plane= normal_signs[q->normal&1] *
                    ( q->coord[ vec_component_index ] - cam_pos.ToArr()[ vec_component_index ] );
            if(  dst_to_quad_plane < 0.0f )
                continue;

            int polygon_vertex_count= ClipWorldQuad(q);
            if( polygon_vertex_count == 0 )
                continue;

            for( int v= 0; v< polygon_vertex_count; v++ )
            {
                m_Vec3 tmp_v( s_active_vertices[v]->coord );
                transformed_vertices[v]= view_matrix * tmp_v;
                float inv_z= 1.0f / transformed_vertices[v].z;
                transformed_vertices[v].x=  transformed_vertices[v].x * inv_z + width2_f;
                transformed_vertices[v].y=  transformed_vertices[v].y * inv_z + height2_f;
                transformed_vertices[v].z*= 65536.0f;
            }

            //mip level calculating
            m_Vec3 vec_to_quad( cam_pos.x - q->coord[0], cam_pos.y - q->coord[1], cam_pos.z - q->coord[2] );
            float inv_angle_cos= vec_to_quad.Length() / dst_to_quad_plane;
            int mip= FastIntLog2Clamp0( int( inv_angle_cos * transformed_vertices[0].z * texels_per_pixel ) );
            if( mip > max_mip) mip = max_mip;

            int tex_coord_shift= tex_size_log2 - mip;
            Draw::SetTextureRaw( texture_manager->GetTextureData( q->tex_id, mip ), tex_coord_shift, tex_coord_shift );

            Draw::SetConstantLight( (light_normal_table[q->normal] * q->light )>>4 );
            using namespace VertexProcessing;

            for( int v= 0; v< polygon_vertex_count-2; v++ )
            {
                const float zmin_f= float(PSR_MIN_ZMIN);
                if( transformed_vertices[0].z < zmin_f || transformed_vertices[v+1].z < zmin_f ||
                        transformed_vertices[v+2].z < zmin_f )
                    continue;
                /*if(  transformed_vertices[0].x < 0.0f ||  transformed_vertices[0].y < 0.0f ||
                     transformed_vertices[0].x > width_f ||  transformed_vertices[0].y > height_f )
                    continue;
                if(  transformed_vertices[v+1].x < 0.0f ||  transformed_vertices[v+1].y < 0.0f ||
                     transformed_vertices[v+1].x > width_f ||  transformed_vertices[v+1].y > height_f )
                    continue;
                if(  transformed_vertices[v+2].x < 0.0f ||  transformed_vertices[v+2].y < 0.0f ||
                     transformed_vertices[v+2].x > width_f ||  transformed_vertices[v+2].y > height_f )
                    continue;*/

                triangle_in_vertex_xy[0]= fixed16_t(transformed_vertices[0].x);
                triangle_in_vertex_xy[1]= fixed16_t(transformed_vertices[0].y);
                triangle_in_vertex_xy[2]= fixed16_t(transformed_vertices[v+1].x);
                triangle_in_vertex_xy[3]= fixed16_t(transformed_vertices[v+1].y);
                triangle_in_vertex_xy[4]= fixed16_t(transformed_vertices[v+2].x);
                triangle_in_vertex_xy[5]= fixed16_t(transformed_vertices[v+2].y);
                triangle_in_vertex_z[0]= fixed16_t( transformed_vertices[0].z);
                triangle_in_vertex_z[1]= fixed16_t( transformed_vertices[v+1].z);
                triangle_in_vertex_z[2]= fixed16_t( transformed_vertices[v+2].z);
                triangle_in_tex_coord[0]= fixed16_t( s_active_vertices[0]->tex_coord[0])<<tex_coord_shift;
                triangle_in_tex_coord[1]= fixed16_t( s_active_vertices[0]->tex_coord[1])<<tex_coord_shift;
                triangle_in_tex_coord[2]= fixed16_t( s_active_vertices[v+1]->tex_coord[0])<<tex_coord_shift;
                triangle_in_tex_coord[3]= fixed16_t( s_active_vertices[v+1]->tex_coord[1])<<tex_coord_shift;
                triangle_in_tex_coord[4]= fixed16_t( s_active_vertices[v+2]->tex_coord[0])<<tex_coord_shift;
                triangle_in_tex_coord[5]= fixed16_t( s_active_vertices[v+2]->tex_coord[1])<<tex_coord_shift;

                if( DrawWorldTriangleToBuffer( buffer ) != 0 )
                    DrawWorldTriangle( buffer );
            }


            /*m_Vec3 transformed_vertices[4];
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
*/
        }//for chunk quads

    }//for chunks
    frame_count++;
    if( (rand()&15) == 0 )
    {
       // printf( "quads: %d\n", total_quads );
        printf( "chunks: %d\n", chunks_passed );
    }


    //PRast_AddFullscreenExponentialFog( float( SHRED_WIDTH) * float(world->NumShreds()) * 0.5f * 0.3f, clear_color );
}
