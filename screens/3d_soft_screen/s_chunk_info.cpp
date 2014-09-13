#include "data_structures.h"
#include "../../Shred.h"
#include "../../blocks/Block.h"

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
                unsigned char block_t= shred->GetBlock( x, y, z )->Transparent() &3;
                *t= block_t;
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

    for( x= 0; x< SHRED_WIDTH  - 1 ; x++ )
    {
        for( y= 0; y< SHRED_WIDTH  - 1; y++ )
        {
            unsigned char* t_up= transparency + BLOCK_LINEAR_ADDR(x,y,0+1);
            unsigned char* t_x=  transparency + BLOCK_LINEAR_ADDR(x+1,y,0);
            unsigned char* t_y=  transparency + BLOCK_LINEAR_ADDR(x,y+1,0);

            unsigned char block_t= *(transparency + BLOCK_LINEAR_ADDR(x,y,0) );
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

    //TODO add calcualting quad count on chunk borders
}


void s_ChunkInfo::GenChunk()
{
    //something unfinished:
    //TODO: set up textures, generate chunk borders

    const fixed16_t tex_size= 65536;

    int x, y, z;
    s_WorldQuad* quad= quad_buffer[1].quads;

    int X= 0, Y= 0;

    for( x= 0; x< SHRED_WIDTH  - 1 ; x++ )
    {
        for( y= 0; y< SHRED_WIDTH  - 1; y++ )
        {
            unsigned char* t_up= transparency + BLOCK_LINEAR_ADDR(x,y,min_geometry_height+1);
            unsigned char* t_x=  transparency + BLOCK_LINEAR_ADDR(x+1,y,0);
            unsigned char* t_y=  transparency + BLOCK_LINEAR_ADDR(x,y+1,0);

            unsigned char block_t= *(transparency + BLOCK_LINEAR_ADDR(x,y,0) );
            for( z= min_geometry_height; z<= max_geometry_height; z++ )
            {
                if( block_t != *t_up )
                {
                    quad->coord[2]= quad->coord[5]= quad->coord[8]= quad->coord[11]= float(z+1);
                    //vertex 0 - (x,y)
                    //vertex 1 (x,y+1)
                    //vertex 2(x+1,y+1)
                    //vertex 3 (x+1,y)
                    quad->coord[3]= quad->coord[0]= float(X+x);
                    quad->coord[9]= quad->coord[6]= float(X+x+1);
                    quad->coord[10]= quad->coord[1]= float(Y+y);
                    quad->coord[7]= quad->coord[4]= float(Y+y+1);
                    quad->normal= (block_t > *t_up) ? NORMAL_Z_POS : NORMAL_Z_NEG;

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_size;
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_size;

                    quad++;
                }//if is up face
                if( block_t != *t_x )
                {
                    quad->coord[0]= quad->coord[3]= quad->coord[6]= quad->coord[9]= float(X+x+1);

                    quad->coord[4]= quad->coord[1]= float(Y+y);
                    quad->coord[10]= quad->coord[7]= float(Y+y+1);
                    quad->coord[11]= quad->coord[2]= float(z);
                    quad->coord[8]= quad->coord[5]= float(z+1);
                    quad->normal= (block_t > *t_x) ? NORMAL_X_POS : NORMAL_X_NEG;

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_size;
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_size;

                    quad++;
                }//if is x+ face
                if( block_t != *t_y )
                {
                    quad->coord[1]= quad->coord[4]= quad->coord[7]= quad->coord[10]= float(Y+y+1);
                    quad->coord[5]= quad->coord[2]= float(z);
                    quad->coord[11]= quad->coord[8]= float(z+1);
                    quad->coord[9]= quad->coord[0]= float(X+x);
                    quad->coord[6]= quad->coord[3]= float(X+x+1);
                    quad->normal= (block_t > *t_y) ? NORMAL_Y_POS : NORMAL_Y_NEG;

                    quad->tc[0]= quad->tc[2]= 0;
                    quad->tc[4]= quad->tc[6]= tex_size;
                    quad->tc[1]= quad->tc[7]= 0;
                    quad->tc[3]= quad->tc[5]= tex_size;

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
