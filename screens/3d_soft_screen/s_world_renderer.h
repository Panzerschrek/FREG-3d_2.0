#ifndef S_WORLD_RENDERE_H
#define S_WORLD_RENDERE_H

#include "data_structures.h"
#include "../math_lib/vec.h"
#include "../math_lib/matrix.h"
class World;

class s_TextureManager;

class s_WorldRenderer
{
public:

    s_WorldRenderer( World* w);
    ~s_WorldRenderer();

    void SetCamPos(const m_Vec3& cam );
    void SetCamAng(const m_Vec3& ang );
    void SetViewportSize( int width, int height );

    void Draw();//draw single frame

private:
    void BuildWorld();//initial world building




    //vectors, matrices
    m_Vec3 cam_pos, cam_ang;
    m_Mat4 view_matrix;

    int frame_count;

    int screen_size_x, screen_size_y;

    World* world;

    s_TextureManager* texture_manager;

    //matrix of pointers to ChunkInfo. Each chunk allocates separatly
    //matrix will be shifted, when world will be shifted, new chunkInfo will be allocated and old deleted
    int chunk_matrix_size[2];
    s_ChunkInfo** chunks;
};



inline void s_WorldRenderer::SetCamPos(const m_Vec3& cam )
{
    cam_pos= cam;
}
inline void s_WorldRenderer::SetCamAng(const m_Vec3& ang )
{
    cam_ang= ang;
}

inline void s_WorldRenderer::SetViewportSize( int width, int height )
{
    screen_size_x= width;
    screen_size_y= height;
}

#endif//S_WORLD_RENDERE_H
