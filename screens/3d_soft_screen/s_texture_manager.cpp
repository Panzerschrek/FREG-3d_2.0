#include "s_texture_manager.h"
#include "../../BlockManager.h"

#include <QImage>
#include <QPainter>

unsigned short s_TextureManager::tex_id_table[ LAST_KIND << 7 ];


void GenTextureMip( const unsigned char* in_data, unsigned char* out_data, int in_size_x, int in_size_y )
{
    unsigned char* dst= out_data;
    for( int y= 0; y< in_size_y; y+=2 )
        for( int x= 0; x< in_size_x; x+=2, dst+=4 )
        {
            int d_src= ( x + y * in_size_x )<<2;
            unsigned int color[4];
            color[0]= in_data[ d_src+0 ];
            color[1]= in_data[ d_src+1 ];
            color[2]= in_data[ d_src+2 ];
            color[3]= in_data[ d_src+3 ];
            d_src+=4;
            color[0]+= in_data[ d_src+0 ];
            color[1]+= in_data[ d_src+1 ];
            color[2]+= in_data[ d_src+2 ];
            color[3]+= in_data[ d_src+3 ];
            d_src+= in_size_x<<2;
            color[0]+= in_data[ d_src+0 ];
            color[1]+= in_data[ d_src+1 ];
            color[2]+= in_data[ d_src+2 ];
            color[3]+= in_data[ d_src+3 ];
            d_src-=4;
            color[0]+= in_data[ d_src+0 ];
            color[1]+= in_data[ d_src+1 ];
            color[2]+= in_data[ d_src+2 ];
            color[3]+= in_data[ d_src+3 ];

            dst[0]= color[0]>>2;
            dst[1]= color[1]>>2;
            dst[2]= color[2]>>2;
            dst[3]= color[3]>>2;
        }
}

void FlipTextureVertical( unsigned char* data, int size_x, int size_y )
{
    for( int y= 0; y < (size_y>>1); y++ )
    {
        unsigned char* d[2];
        d[0]= data + size_x * y * 4;
        d[1]= data + size_x * (size_y-1-y) * 4;
        for( int x= 0; x< size_x; x++ )
        {
            int c[2];
            c[0]= ((int*)(d[0]))[x];
            c[1]= ((int*)(d[1]))[x];
            ((int*)(d[1]))[x]= c[0];
            ((int*)(d[0]))[x]= c[1];
        }
    }
}

s_TextureManager::s_TextureManager():
    texture_count(0),
    texture_size(64),
    text_antialiasing(true),
    texture_data_size(0)
{
    //calculate data size for one texture, mip offsets
    for( int p = texture_size, m=0; p> 0; p>>=1, m++ )
    {
        mip_offset[m]= texture_data_size;
        texture_data_size+= 4 * p * p;
    }

    //get number of valid blocks and their textures and populate tex_id table
    for( int kind= 0; kind < LAST_KIND; kind++ )
        for( int sub= 0; sub< LAST_SUB; sub++ )
            if( BlockManager::IsValid( kind, sub ) )
            {
                tex_id_table[ (kind<<6) | sub ]= texture_count;
                texture_count++;
            }

    textures_data= new unsigned char[ texture_data_size * texture_count ];

    int img_size= text_antialiasing ? texture_size * 2 : texture_size;
    QImage img( img_size, img_size, QImage::Format_RGB32 );
    QColor col;
    QPainter painter(&img);
    QFont font( "Sans Mono", img_size * 11 / 16 );
    painter.setFont( font );
    QString str( "t" );


    //generate textures
    unsigned char* d= textures_data;
    for( int kind= 0; kind < LAST_KIND; kind++ )
        for( int sub= 0; sub< LAST_SUB; sub++ )
            if( BlockManager::IsValid( kind, sub ) )
            {
                col.setRgb( rand()&255, rand()&255, rand()&255 );
                painter.fillRect( 0, 0, img_size, img_size, col );

                col.setRgb( rand()&255, rand()&255, rand()&255 );
                painter.setPen( col );
                str[0]= 32 + rand()%(127-32);
                painter.drawText( img_size/8, img_size*3/4, str );

                if( text_antialiasing )
                    GenTextureMip( img.constBits(), d, texture_size * 2, texture_size * 2 );
                else
                    memcpy( d, img.constBits(), texture_size * texture_size * 4 );
                FlipTextureVertical( d, texture_size, texture_size );
                for( int p= texture_size>>1, i=1; p> 0; p>>= 1, i++ )
                    GenTextureMip( d + mip_offset[i-1], d + mip_offset[i], p<<1, p<<1 );

                d+= texture_data_size;
            }
}

s_TextureManager::~s_TextureManager()
{
    delete[] textures_data;
}
