#ifndef S_TEXTURE_MANAGER_H
#define S_TEXTURE_MANAGER_H
#include "../3d_soft_screen/data_structures.h"


#define S_TEXTURE_MAX_SIZE_LOG2 9

class s_TextureManager
{
	public:
    s_TextureManager();
    ~s_TextureManager();
	
    //static methods need for mesh generation
    static unsigned short GetTextureId( int kind, int sub );

    const unsigned char* GetTextureData( int texture_id, int mip ) const;
    int GetTextureSize() const;
	private:

    int texture_size;
    int texture_data_size;//size of data of one texture ( with mips )
    int mip_offset[S_TEXTURE_MAX_SIZE_LOG2];//offset of mip data, from curent texture mip data
    int texture_count;
    bool text_antialiasing;

    unsigned char* textures_data;

    static unsigned short tex_id_table[ (LAST_KIND << 6) | LAST_SUB ];
};


inline unsigned short s_TextureManager::GetTextureId( int kind, int sub )
{
    return tex_id_table[ (kind<<6) | sub ];
}

inline int s_TextureManager::GetTextureSize()const
{
    return texture_size;
}


inline const unsigned char* s_TextureManager::GetTextureData( int texture_id, int mip ) const
{
    return textures_data + texture_data_size * texture_id + mip_offset[mip];
}
#endif//S_TEXTURE_MANAGER_H
