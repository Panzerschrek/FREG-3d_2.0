#ifndef S_WORLD_RENDERE_H
#define S_WORLD_RENDERE_H

#include "data_structures.h"

class World;

class s_WorldRenderer
{
public:

    s_WorldRenderer( const World* w);
    ~s_WorldRenderer();

    void Draw();//draw single frame

private:
    void BuildWorld();//initial world building

    int frame_count;

    const World* world;

    //matrix of pointers to ChunkInfo. Each chunk allocates separatly
    //matrix will be shifted, when world will be shifted, new chunkInfo will be allocated and old deleted
    int chunk_matrix_size[2];
    s_ChunkInfo** chunks;
};


#endif//S_WORLD_RENDERE_H
