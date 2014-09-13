#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H
#include "../../header.h"
#include "psr/psr.h"

//s_ prefix means softwre rendering types
class Shred;

enum BlockNormal
{
	NORMAL_X_POS,
	NORMAL_X_NEG,
    NORMAL_Y_POS,
	NORMAL_Y_NEG,
	NORMAL_Z_POS,
	NORMAL_Z_NEG
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
};


#endif//DATA_STRUCTURES_H
