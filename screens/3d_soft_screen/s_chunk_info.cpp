#include "data_structures.h"
#include "../../Shred.h"
#include "../../blocks/Block.h"

#include "s_texture_manager.h"

#define CHUNK_WIDTH_LOG2 4
#define CHUNK_HEIGHT_LOG2 7

#define BLOCK_LINEAR_ADDR(x,y,z) ( ((x)<<(CHUNK_HEIGHT_LOG2+CHUNK_WIDTH_LOG2)) | ((y)<<(CHUNK_HEIGHT_LOG2)) | (z) )



s_ChunkInfo::s_ChunkInfo(Shred* s):
shred(s)
{
    for( int i= 0; i< 2; i++ )
    {
       quad_buffer[i].allocated_quad_count=
                quad_buffer[i].quad_count= 0;

       quad_buffer[i].quads= nullptr;
    }
}

void s_ChunkInfo::GetTransparency()
{
    for( int x= 0; x< SHRED_WIDTH; x++ )
        for( int y= 0; y< SHRED_WIDTH; y++ )
        {
            unsigned char* t= transparency + BLOCK_LINEAR_ADDR(x,y,0);
            for( int z= 0; z< HEIGHT; z++, t++ )
            {
                Block* b= shred->GetBlock( x, y, z );
                if( b == NULL )
                    *t= 0;
                else
                    *t= b->Transparent() &3;
            }
        }//for y
}

void s_ChunkInfo::GetTransparencyInCube( int x_min, int x_max, int y_min, int y_max, int z_min, int z_max )
{
    for( int x= x_min; x<= x_max; x++ )
        for( int y= y_min; y<= y_max; y++ )
        {
            unsigned char* t= transparency + BLOCK_LINEAR_ADDR(x,y,z_min);
            for( int z= z_min; z<= z_max; z++, t++ )
            {
                unsigned char block_t= shred->GetBlock( x, y, z )->Transparent() &3;
                *t= block_t;
            }
        }//for y
}

s_ChunkInfo::~s_ChunkInfo()
{
    for( int i= 0; i< 2; i++ )
    {
        if( quad_buffer[i].quads != nullptr )
            delete[]  quad_buffer[i].quads;
    }
}


void s_ChunkInfo::GetQuadCount()
{
    int x, y, z;

    quad_buffer[1].quad_count= 0;

    min_geometry_height= HEIGHT;
    max_geometry_height= 0;

    for( x= 0; x< SHRED_WIDTH ; x++ )
    {
        for( y= 0; y< SHRED_WIDTH; y++ )
        {
            unsigned char* t_up= transparency + BLOCK_LINEAR_ADDR(x,y,0+1);
            unsigned char* t_x;
            unsigned char* t_y;
            unsigned char block_t= *(transparency + BLOCK_LINEAR_ADDR(x,y,0) );

            if( x == SHRED_WIDTH-1 )
            {
                if( neighbors[0] != nullptr )
                    t_x= neighbors[0]->transparency + BLOCK_LINEAR_ADDR(0,y,0);
                else
                    t_x= t_up-1;
            }
            else
                t_x= transparency + BLOCK_LINEAR_ADDR(x+1,y,0);
            if( y == SHRED_WIDTH-1 )
            {
                if( neighbors[2] != nullptr )
                    t_y= neighbors[2]->transparency + BLOCK_LINEAR_ADDR(x,0,0);
                else
                    t_y= t_up-1;
            }
            else
                t_y= transparency + BLOCK_LINEAR_ADDR(x,y+1,0);
            for( z= 0; z< HEIGHT  - 2; z++ )
            {
                if( block_t != *t_up )
                {
                    if( z < min_geometry_height )
                        min_geometry_height= z;
                    else if( z > max_geometry_height )
                        max_geometry_height= z;
                    quad_buffer[1].quad_count++;
                }
                if( block_t != *t_x )
                {
                    if( z < min_geometry_height )
                        min_geometry_height= z;
                    else if( z > max_geometry_height )
                        max_geometry_height= z;
                    quad_buffer[1].quad_count++;
                }
                if( block_t != *t_y )
                {
                    if( z < min_geometry_height )
                        min_geometry_height= z;
                    else if( z > max_geometry_height )
                        max_geometry_height= z;
                    quad_buffer[1].quad_count++;
                }

                block_t= *t_up;
                t_up++;
                t_x++;
                t_y++;
            }//for z
        }//for y
    }//for x
}


void s_ChunkInfo::GenChunk()
{
    const fixed16_t tex_size= 65536;

    int x, y, z;
    int tex_coord_sign[2];

    s_WorldQuad* quad= quad_buffer[1].quads;
    Block* b;

    int X= shred->Latitude() * SHRED_WIDTH, Y= shred->Longitude()  * SHRED_WIDTH;
    //int X= 0, Y= 0;

    int x_block_x, y_block_y;
    for( x= 0; x< SHRED_WIDTH; x++ )
    {
        x_block_x= (x+1) & (SHRED_WIDTH-1);
        for( y= 0; y< SHRED_WIDTH; y++ )
        {
            unsigned char* t_up= transparency + BLOCK_LINEAR_ADDR(x,y,min_geometry_height+1);
            unsigned char* t_x;
            unsigned char* t_y;
            unsigned char block_t= *(transparency + BLOCK_LINEAR_ADDR(x,y,min_geometry_height) );
            Shred *x_block_shred, *y_block_shred;

            y_block_y= (y+1) & (SHRED_WIDTH-1);
            if( x == SHRED_WIDTH-1 )
            {
                if( neighbors[0] != nullptr )
                {
                    t_x= neighbors[0]->transparency + BLOCK_LINEAR_ADDR(0,y,min_geometry_height);
                    x_block_shred= neighbors[0]->shred;
                }
                else
                    t_x= t_up-1;
            }
            else
            {
                t_x=  transparency + BLOCK_LINEAR_ADDR(x+1,y,min_geometry_height);
                x_block_shred= shred;
            }
            if( y == SHRED_WIDTH-1 )
            {
                if( neighbors[2] != nullptr )
                {
                    t_y= neighbors[2]->transparency + BLOCK_LINEAR_ADDR(x,0,min_geometry_height);
                    y_block_shred= neighbors[2]->shred;
                }
                else
                    t_y= t_up-1;
            }
            else
            {
                t_y=  transparency + BLOCK_LINEAR_ADDR(x,y+1,min_geometry_height);
                y_block_shred= shred;
            }
            for( z= min_geometry_height; z<= max_geometry_height; z++ )
            {
                if( block_t != *t_up )
                {
                    if( block_t > *t_up )
                    {
                        quad->normal= NORMAL_Z_POS;
                        quad->light= shred->Lightmap( x, y, z )&15;
                        b= shred->GetBlock( x, y, z+1 );
                        tex_coord_sign[1]= tex_size;
                    }
                    else
                    {
                        quad->normal= NORMAL_Z_NEG;
                        quad->light= shred->Lightmap( x, y, z+1 )&15;
                        b= shred->GetBlock( x, y, z );
                        tex_coord_sign[1]= -tex_size;
                    }
                    quad->coord[2]= quad->coord[5]= quad->coord[8]= quad->coord[11]= float(z+1);
                    //vertex 0 - (x,y)
                    //vertex 1 (x,y+1)
                    //vertex 2(x+1,y+1)
                    //vertex 3 (x+1,y)
                    quad->coord[3]= quad->coord[0]= float(X+x);
                    quad->coord[9]= quad->coord[6]= float(X+x+1);
                    quad->coord[10]= quad->coord[1]= float(Y+y);
                    quad->coord[7]= quad->coord[4]= float(Y+y+1);

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_size;
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_coord_sign[1];
                    quad->tex_id= s_TextureManager::GetTextureId( b->Kind(), b->Sub() );

                    quad++;
                }//if is up face
                if( block_t != *t_x )
                {
                    if( block_t > *t_x )
                    {
                        quad->normal= NORMAL_X_POS;
                        quad->light= shred->Lightmap( x, y, z )&15;
                        b= x_block_shred->GetBlock( x_block_x, y, z );
                        tex_coord_sign[0]= tex_size;
                    }
                    else
                    {
                        quad->normal= NORMAL_X_NEG;
                        quad->light= x_block_shred->Lightmap( x_block_x, y, z )&15;
                        b= shred->GetBlock( x, y, z );
                        tex_coord_sign[0]= -tex_size;
                    }
                    quad->coord[0]= quad->coord[3]= quad->coord[6]= quad->coord[9]= float(X+x+1);

                    quad->coord[4]= quad->coord[1]= float(Y+y);
                    quad->coord[10]= quad->coord[7]= float(Y+y+1);
                    quad->coord[11]= quad->coord[2]= float(z);
                    quad->coord[8]= quad->coord[5]= float(z+1);

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_coord_sign[0];
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_size;
                    quad->tex_id= s_TextureManager::GetTextureId( b->Kind(), b->Sub() );

                    quad++;
                }//if is x+ face
                if( block_t != *t_y )
                {
                    if( block_t > *t_y )
                    {
                        quad->normal= NORMAL_Y_POS;
                        quad->light= shred->Lightmap( x, y, z )&15;
                        b= y_block_shred->GetBlock( x, y_block_y, z );
                        tex_coord_sign[0]= -tex_size;
                    }
                    else
                    {
                        quad->normal= NORMAL_Y_NEG;
                        quad->light= y_block_shred->Lightmap( x, y_block_y, z )&15;
                        b= shred->GetBlock( x, y, z );
                        tex_coord_sign[0]= tex_size;
                    }
                    quad->coord[1]= quad->coord[4]= quad->coord[7]= quad->coord[10]= float(Y+y+1);
                    quad->coord[11]= quad->coord[2]= float(z);
                    quad->coord[5]= quad->coord[8]= float(z+1);
                    quad->coord[3]= quad->coord[0]= float(X+x);
                    quad->coord[9]= quad->coord[6]= float(X+x+1);

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_coord_sign[0];
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_size;
                    quad->tex_id= s_TextureManager::GetTextureId( b->Kind(), b->Sub() );

                    quad++;
                }//if is y+ chunk

                block_t= *t_up;
                t_up++;
                t_x++;
                t_y++;
            }//for z
        }//for y
    }//for x

    //TODO add generation of quad in chunk borders
}


bool s_ChunkInfo::IsBehindPlane( const Plane* plane )
{
    float shred_x= float( shred->Latitude() * SHRED_WIDTH );
    float shred_y= float( shred->Longitude() * SHRED_WIDTH );
    for( int x= 0; x< 2; x++ )
        for( int y= 0; y< 2; y++ )
            for( int z= 0; z< 2; z++ )
            {
                float v[3];
                v[0]= shred_x + float(x*SHRED_WIDTH);
                v[1]= shred_y + float(y*SHRED_WIDTH);
                v[2]= (z == 0) ?  float(min_geometry_height) : float(max_geometry_height+1);

                float dot= v[0] * plane->v[0] + v[1] * plane->v[1] + v[2] * plane->v[2];
                if( dot > plane->v[3] )
                    return false;
            }
    return true;

    /*float v[3];
    v[0]= shred->Latitude() * SHRED_WIDTH + float(SHRED_WIDTH/2);
    v[1]= shred->Longitude() * SHRED_WIDTH + float(SHRED_WIDTH/2);
    v[2]= float((min_geometry_height + max_geometry_height)>>1);
    float dot= v[0] * plane->v[0] + v[1] * plane->v[1] + v[2] * plane->v[2];
    if( dot > plane->v[3] )
        return false;
    return true;*/
}
