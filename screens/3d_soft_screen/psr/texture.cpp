#include <stdlib.h>
#include <stdio.h>
#include "texture.h"
#include "fast_math.h"



Texture::Texture():
size_x(0), size_y(0), size_x_log2(-1), size_y_log2(-1), original_size_x(0), original_size_y(0),
max_lod(-1),
data(NULL),
palette(NULL),
is_palettized_texture(false)
{
	for( int i= 0; i< PSR_MAX_TEXTURE_SIZE_LOG2; i++ )
		lods_data[i]= NULL;
}

void Texture::Create( int width, int height, bool palettized, const unsigned char* pal, const unsigned char* data, ResizeMode resize_mode, bool build_palettized_lods )
{
	original_size_x= width;
	original_size_y= height;
	is_palettized_texture= palettized;

	if( resize_mode != RESIZE_NO )
		ResizeToNearestPOTCeil( data, resize_mode );//resize and allocate memory
	else
	{
		size_x= width;
		size_y= height;
		size_x_log2= Log2Ceil( size_x );
		size_y_log2= Log2Ceil( size_y );
		if( (1<<size_x_log2) > size_x ) size_x_log2--;
		if( (1<<size_y_log2) > size_y ) size_y_log2--;

		if(this->data!=NULL) delete[] this->data;
		int data_size= width * height;
		if(!palettized)
			data_size<<=2;
		this->data= new unsigned char[data_size];
		memcpy(this->data, data, data_size);
	}

	if( is_palettized_texture )
	{
		this->palette= new unsigned char[ 4 * 256 ];
		//this->palette= (unsigned char*) malloc( 1024 + 4 * 256 );
		memcpy( this->palette, pal, 4 * 256 );
	}

	lods_data[0]= this->data;
	int i;
	for( i= 1; i< size_x_log2 && i< size_y_log2; i++ )
	{
		int sx= size_x>>i;
		int sy= size_y>>i;
		if( is_palettized_texture )
		{
			if( lods_data[i] != NULL ) delete[] lods_data[i];
			lods_data[i]= new unsigned char[ sx * sy ];
			if( data != NULL )
			{
				if( build_palettized_lods )
					GenLodPlaettized( sx<<1, sy<<1, lods_data[i-1], lods_data[i] );
			}
		}
		else
		{
			if( lods_data[i] != NULL ) delete[] lods_data[i];
			lods_data[i]= new unsigned char[ sx * sy * 4 ];
			if( data != NULL )
				GenLod( sx<<1, sy<<1, lods_data[i-1], lods_data[i] );
		}
	}
	max_lod= FastIntMin( size_x_log2, size_y_log2 )-1;
}

void Texture::GenLod( int sx, int sy, const unsigned char* in_data, unsigned char* out_data )
{
	unsigned char* dst= out_data;
	for( int y= 0; y< sy; y+=2 )
		for( int x= 0; x< sx; x+=2, dst+=4 )
		{
			int d_src= ( x + y * sx )<<2;
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
			d_src+= sx<<2;
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

void Texture::GenLodPlaettized( int sx, int sy, unsigned char* in_data, unsigned char* out_data )
{
	unsigned char* dst= out_data;
	for( int y= 0; y< sy; y+=2 )
		for( int x= 0; x< sx; x+=2, dst++ )
		{
			int d_src= x + y * sx;
			int index= in_data[ d_src ]<<2;
			unsigned int color[4];
			color[0]= palette[ index+0 ];
			color[1]= palette[ index+1 ];
			color[2]= palette[ index+2 ];
			d_src++;
			index= in_data[ d_src ]<<2;
			color[0]+= palette[ index+0 ];
			color[1]+= palette[ index+1 ];
			color[2]+= palette[ index+2 ];
			d_src+=sx;
			index= in_data[ d_src ]<<2;
			color[0]+= palette[ index+0 ];
			color[1]+= palette[ index+1 ];
			color[2]+= palette[ index+2 ];
			d_src--;
			index= in_data[ d_src ]<<2;
			color[0]+= palette[ index+0 ];
			color[1]+= palette[ index+1 ];
			color[2]+= palette[ index+2 ];

			unsigned char c[]= { color[0]>>2, color[1]>>2, color[2]>>2 };
			*dst= FindNearestColorInPalette( c );
		}
}
void Texture::RGB2RGBS( int sx, int sy, unsigned char* in_out_data )
{
	unsigned char* data_end= in_out_data + sx*sy*4;
	for( unsigned char* d= in_out_data; d< data_end; d+=4 )
	{
		unsigned char s= FastIntMax( FastIntMax( d[0], d[1] ), FastIntMax( d[2], 1 ) );
		d[0]= ( 255u * d[0] ) / s;
		d[1]= ( 255u * d[1] ) / s;
		d[2]= ( 255u * d[2] ) / s;
		d[3]= s;
	}
}

unsigned char Texture::FindNearestColorInPalette( unsigned char* color )
{
	unsigned char nearest_color= 0;
	int min_delta=
		FastIntAbs( color[0] - palette[0] ) + FastIntAbs( color[1] - palette[1] ) + FastIntAbs( color[2] - palette[2] );
	for( int i= 1, j=4; i< 256; i++, j+=4 )//scan all palette
	{
		int delta= FastIntAbs( color[0] - palette[j] ) + FastIntAbs( color[1] - palette[j+1] ) + FastIntAbs( color[2] - palette[j+2] );
		if( delta < min_delta )
		{
			min_delta= delta;
			nearest_color= i;
		}
	}
	return nearest_color;
}


void Texture::ResizeToNearestPOTCeil( const unsigned char* in_data, ResizeMode resize_mode )
{
	size_x_log2= Log2Ceil( original_size_x );
	size_y_log2= Log2Ceil( original_size_y );
	size_x= 1<<size_x_log2;
	size_y= 1<<size_y_log2;

	int tex_data_size= is_palettized_texture ? (size_x * size_y) : (size_x * size_y * 4);
	//data= (unsigned char*) malloc( tex_data_size);
	if( data != NULL ) delete[] data;
	data= new unsigned char[ tex_data_size ];

	if( in_data == NULL )
		return;//nothing to do


	if( size_x == original_size_x && size_y == original_size_y )
	{//simple copying of src texture data
		memcpy( data, in_data, tex_data_size );
		return;
	}

	//int x2x_new= 65536 * original_size_x / size_x;
	//int y2y_new= 65536 * original_size_y / size_y;
	int x2x_new= ( 65536 * original_size_x )>> size_x_log2;
	int y2y_new= ( 65536 * original_size_y )>> size_y_log2;

	if( ! is_palettized_texture )
	{//copy per int

		if( resize_mode == RESIZE_STRETCH )
		{
			for( int y= 0; y< size_y; y++ )
				for( int x= 0; x< size_x; x++ )
				{
					//make linear color interpolation
					int old_x= (x*x2x_new);
					int old_y= (y*y2y_new);
					int dx= (old_x>>8)&0xFF;
					int dy= (old_y>>8)&0xFF;
					old_x>>=16; old_y>>= 16;
					int old_x1= old_x+1, old_y1= old_y+1;

					int dx1= 255 - dx, dy1= 255 - dy;

					int colors[16];
					int mixed_colors[8];
					int addr= (old_x + old_y *original_size_x)<<2;
					colors[0 ]= in_data[addr  ];
					colors[1 ]= in_data[addr+1];
					colors[2 ]= in_data[addr+2];
					colors[3 ]= in_data[addr+3];
					addr= (old_x1 + old_y *original_size_x)<<2;
					colors[4 ]= in_data[addr  ];
					colors[5 ]= in_data[addr+1];
					colors[6 ]= in_data[addr+2];
					colors[7 ]= in_data[addr+3];
					addr= (old_x + old_y1 *original_size_x)<<2;
					colors[8 ]= in_data[addr  ];
					colors[9 ]= in_data[addr+1];
					colors[10]= in_data[addr+2];
					colors[11]= in_data[addr+3];
					addr= (old_x1 + old_y1 *original_size_x)<<2;
					colors[12]= in_data[addr  ];
					colors[13]= in_data[addr+1];
					colors[14]= in_data[addr+2];
					colors[15]= in_data[addr+3];

					mixed_colors[0]= ( colors[ 0] * dx1 + colors[ 4] * dx );
					mixed_colors[1]= ( colors[ 1] * dx1 + colors[ 5] * dx );
					mixed_colors[2]= ( colors[ 2] * dx1 + colors[ 6] * dx );
					mixed_colors[3]= ( colors[ 3] * dx1 + colors[ 7] * dx );
					mixed_colors[4]= ( colors[ 8] * dx1 + colors[12] * dx );
					mixed_colors[5]= ( colors[ 9] * dx1 + colors[13] * dx );
					mixed_colors[6]= ( colors[10] * dx1 + colors[14] * dx );
					mixed_colors[7]= ( colors[11] * dx1 + colors[15] * dx );
					unsigned char* out_color= data + (x + (y<<size_x_log2));
					out_color[0]= ( mixed_colors[0] * dy1 + mixed_colors[4] * dy ) >> 16;
					out_color[1]= ( mixed_colors[1] * dy1 + mixed_colors[5] * dy ) >> 16;
					out_color[2]= ( mixed_colors[2] * dy1 + mixed_colors[6] * dy ) >> 16;
					out_color[3]= ( mixed_colors[3] * dy1 + mixed_colors[7] * dy ) >> 16;
					//nearest filter
					/*((int*)data)[ x + (y<<size_x_log2) ]=
					((int*)in_data)[ ((x*x2x_new)>>16) + ((y*y2y_new)>>16) * original_size_x ];*/
				}
		}//if stretch resize
		else
		{
			for( int y= 0; y< original_size_y; y++ )
				for( int x= 0; x< original_size_x; x++ )
				{
					((int*)data)[ x + (y<<size_x_log2) ]=
						((int*)in_data)[ x + y * original_size_x ];
				}

			int border_color;
			for( int x= 0; x< original_size_x; x++ )
			{
				border_color= ((int*)in_data)[ x + (original_size_y-1) * original_size_x ];
				for( int y= original_size_y; y< size_y; y++ )
					((int*)data)[ x + (y<<size_x_log2) ]= border_color;
			}

			for( int y= 0; y< original_size_y; y++ )
			{
				border_color= ((int*)in_data)[ (original_size_x-1) + y * original_size_x ];
				for( int x= original_size_x; x< size_x; x++ )
					((int*)data)[ x + (y<<size_x_log2) ]= border_color;
			}

			border_color= ((int*)in_data)[ (original_size_x-1) + (original_size_y-1) * original_size_x ];
			for( int y= original_size_y; y< size_y; y++ )
				for( int x= original_size_x; x< size_x; x++ )
					((int*)data)[ x + (y<<size_x_log2) ]= border_color;
		}
	}
	else
	{//copy per byte
		if( resize_mode == RESIZE_STRETCH )
		{
			for( int y= 0; y< size_y; y++ )
				for( int x= 0; x< size_x; x++ )
						data[ x + (y<<size_x_log2) ]=
						in_data[ ((x*x2x_new)>>16) + ((y*y2y_new)>>16) * original_size_x ];
		}
		else
		{
			for( int y= 0; y< original_size_y; y++ )
				for( int x= 0; x< original_size_x; x++ )
					data[ x + (y<<size_x_log2) ]= in_data[ x + y * original_size_x ];

			unsigned char border_color;
			for( int x= 0; x< original_size_x; x++ )
			{
				border_color= in_data[ x + (original_size_y-1) * original_size_x ];
				for( int y= original_size_y; y< size_y; y++ )
					data[ x + (y<<size_x_log2) ]= border_color;
			}

			for( int y= 0; y< original_size_y; y++ )
			{
				border_color= in_data[ (original_size_x-1) + y * original_size_x ];
				for( int x= original_size_x; x< size_x; x++ )
					data[ x + (y<<size_x_log2) ]= border_color;
			}

			border_color= in_data[ (original_size_x-1) + (original_size_y-1) * original_size_x ];
			for( int y= original_size_y; y< size_y; y++ )
				for( int x= original_size_x; x< size_x; x++ )
					data[ x + (y<<size_x_log2) ]= border_color;

		}
	}//if palettized

}

struct pcx_t
{
    char	manufacturer;
    char	version;
    char	encoding;
    char	bits_per_pixel;
    unsigned short	xmin,ymin,xmax,ymax;
    unsigned short	hres,vres;
    unsigned char	palette[48];
    char	reserved;
    char	color_planes;
    unsigned short	bytes_per_line;
    unsigned short	palette_type;
    char	filler[58];
    unsigned char	data;			// unbounded
};



void Texture::Convert2GRBS()
{
	for( int i= 0; i<= max_lod; i++ )
		RGB2RGBS( size_x>>i, size_y>>i, lods_data[i] );
}

void Texture::Convert2RGBAFromPlaettized()
{
	unsigned char* new_data= new unsigned char[ size_x * size_y * 4 ];
	for( int i= 0; i< size_x * size_y; i++ )
	{
		((int*)new_data)[i]= ((int*)palette)[ data[i] ];
	}
	delete[] data;
	data= new_data;

	lods_data[0]= data;
	for( int i= 1; i< size_x_log2 && i< size_y_log2; i++ )
	{
		int sx= size_x>>i;
		int sy= size_y>>i;
		delete[] lods_data[i];
		lods_data[i]= new unsigned char[ sx * sy * 4 ];
		GenLod( sx<<1, sy<<1, lods_data[i-1], lods_data[i] );
	}
	max_lod= FastIntMin( size_x_log2, size_y_log2 )-1;

	delete[] palette;
	palette= NULL;
	is_palettized_texture= false;

}

void Texture::SetColorKeyToAlpha( const unsigned char* color_key, unsigned char alpha )
{
	int c= *((int*)color_key);
	unsigned char* pix= data, *pix_end= data + size_x * size_y * 4;
	for( ; pix!= pix_end; pix+=4 )
	{
		if( c == *((int*)pix) )
		{
			pix[3]= alpha;
		}
	}
}

void Texture::FindAndReplaceColor( const unsigned char* color_key, const unsigned char* new_color )
{
	int c0= *((int*)color_key);
	int c1= *((int*)new_color);
	unsigned char* pix= data, *pix_end= data + size_x * size_y * 4;
	for( ; pix!= pix_end; pix+=4 )
	{
		if( *((int*)pix) == c0 )
		*((int*)pix)= c1;
	}
}

void Texture::SwapRedBlueChannles()
{
	if(!is_palettized_texture)
	{
		for( int i= 0; i<= max_lod; i++ )
		{
			unsigned char* d= lods_data[i];
			unsigned char* d_end= d + ((size_x * size_y*4)>>i);
			for( ; d!=d_end; d+=4 )
			{
				unsigned char tmp= d[0];
				d[0]= d[2];
				d[2]= tmp;
			}
		}
	}
	else
	{
		for( int i= 0; i< 256*4; i+= 4 )
		{
			unsigned char tmp= palette[i];
			palette[i]= palette[i+2];
			palette[i+2]= tmp;
		}
	}
}
