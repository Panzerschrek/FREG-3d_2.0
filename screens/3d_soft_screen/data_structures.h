#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H
#include "../../header.h"
#include "psr/psr.h"
#include "../math_lib/vec.h"

//s_ prefix means softwre rendering types
class Shred;

enum BlockNormal
{
    NORMAL_X_POS =0,
    NORMAL_X_NEG =1,
    NORMAL_Y_POS =2,
    NORMAL_Y_NEG =3,
    NORMAL_Z_POS =4,
    NORMAL_Z_NEG =5
};

/*
main struct for software rendering.
size is very huge, but we need perfomanse in vertex processing. 
*/
struct s_WorldQuad
{
	float coord[3*4];
	fixed16_t tc[2*4];
	fixed16_t light;
	unsigned int tex_id;
	BlockNormal normal;
		
};


struct Plane
{
   m_Vec3 toVec3()const
   {
       return m_Vec3( v[0], v[1], v[2] );
   }
   float& dst()
   {
       return v[3];
   }
   float v[4];
};


struct s_CommandBuffer
{
	char* buffer;
	char* current_pos;
	unsigned int allocated_size;
};

struct s_ChunkInfo
{
	//unlike OpenGL renderer, in software renderer each chunk contains his quads separatly and has ownership on his pointers.
	struct
	{
        s_WorldQuad* quads;
		int quad_count;
		int allocated_quad_count;
	}quad_buffer[2];
    //buffer 0 uses for rendering
    //buffer 1 used for quad generating
    //buffers swaps after chung geometry regeneration

    unsigned char transparency[ SHRED_WIDTH * SHRED_WIDTH * HEIGHT ];
    Shred* shred;

    //0 x+
    //1 x-
    //2 y+
    //3 y-
    s_ChunkInfo* neighbors[4];

    int min_geometry_height, max_geometry_height;

    s_ChunkInfo( Shred* s );
    ~s_ChunkInfo();
    void GetQuadCount();
    // generate chunk mesh. memory must be allocated in quad_buffer[1].quads
    void GenChunk();
    // parce chunk blocks and get transparency
    void GetTransparency();
    //update chunk transparency in cube [mins;maxs]. coordinates - chunk relative
    void GetTransparencyInCube( int x_min, int x_max, int y_min, int y_max, int z_min, int z_max );


    bool IsBehindPlane( const Plane* plane );
};


#endif//DATA_STRUCTURES_H
