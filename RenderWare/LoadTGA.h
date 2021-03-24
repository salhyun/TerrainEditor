#pragma once

#include <stdio.h>

typedef struct
{
   unsigned char  d_iif_size;            // IIF size (after header), usually 0
   unsigned char  d_cmap_type;           // ignored
   unsigned char  d_image_type;          // should be 2
   unsigned char  pad[5];

   unsigned short d_x_origin;
   unsigned short d_y_origin;
   unsigned short d_width;
   unsigned short d_height;

   unsigned char  d_pixel_size;          // 16, 24, or 32
   unsigned char  d_image_descriptor;    // Bits 3-0: size of alpha channel
                                         // Bit 4: must be 0 (reserved)
                                         // Bit 5: should be 0 (origin)
                                         // Bits 6-7: should be 0 (interleaving)
} tTGAHeader_s;

typedef struct
{
	tTGAHeader_s	header;
	unsigned char	*data;
} tTGAFile_s;

#define RGB16(r,g,b)   ( ((r>>3) << 10) + ((g>>3) << 5) + (b >> 3) )
#define RGB24(r,g,b)   ( ((r) << 16) + ((g) << 8) + (b) )
#define GET16R(v)   (v >> 10)
#define GET16G(v)   ((v >> 5) & 0x1f)
#define GET16B(v)   (v & 0x1f)

class CTga
{
public :
	bool m_bEnable ;
	tTGAFile_s m_file ;
	tTGAHeader_s m_header ;

public :
	CTga()
	{
		this->m_bEnable = false ;
		this->m_file.data = NULL ;
	}

	~CTga()
	{
		if(m_file.data)
		{
			delete []m_file.data ;
			m_file.data = NULL ;
		}
	}

	void LoadTGAFile( char * strFilename)
	{
		/// Local Variables ///////////////////////////////////////////////////////////
		short			BitsPerPixel;
		unsigned char	*buffer=NULL;
		int bitsize;		/* Total size of bitmap */
		unsigned char	*newbits=NULL;		/* New RGB bits */
		unsigned char	*from, *to;		/* RGB looping vars */
		int		i, 			/* Looping vars */
			width;			/* Aligned width of bitmap */
		FILE	*file;
		long dwWidth, dwHeight;
		tTGAHeader_s	*header;
		tTGAFile_s *tgafile = &this->m_file ;
		///////////////////////////////////////////////////////////////////////////////
		header = &tgafile->header;

		// Open the file and read the header
		file = fopen( strFilename, "rb");
		if( NULL == file )
			return;

		if ( fread( &this->m_header, sizeof( tTGAHeader_s ), 1, file ) != 1 )
		{
			fclose( file );
			return;
		}

		// Parse the TGA header
		dwWidth = (long)this->m_header.d_width;
		dwHeight = (long)this->m_header.d_height;
		BitsPerPixel = (short)this->m_header.d_pixel_size;          // 16, 24, or 32

		// Create a bitmap to load the data into

		bitsize = dwWidth * dwHeight * (BitsPerPixel/8);
		//if ((newbits = (unsigned char *)calloc(bitsize, 1)) == NULL)
		if((newbits = new BYTE[bitsize]) == NULL)
		{
			fclose( file );
			return;
		}
		//buffer = (unsigned char *)malloc(dwWidth*dwHeight*(BitsPerPixel / 8));
		buffer = new BYTE[dwWidth*dwHeight*(BitsPerPixel / 8)] ;

		if ( fread( buffer, dwWidth*dwHeight*(BitsPerPixel / 8), 1, file ) != 1 )
		{
			fclose( file );
			//free(buffer);
			//free(newbits);
			delete []buffer ;
			delete []newbits ;
			return;
		}

		width   = (BitsPerPixel / 8) * dwWidth;

		for(i=0; i<dwHeight; i++)
		{
			to = newbits+(i*(dwWidth*(BitsPerPixel/8))) ;
			from = buffer+(((dwHeight-1)-i)*(dwWidth*(BitsPerPixel/8))) ;

			memcpy(to, from, dwWidth*(BitsPerPixel/8)) ;
		}

		fclose( file );
		delete []buffer ;

		tgafile->data = newbits;

		m_bEnable = true ;
	}

	long GetRGBFromTGA(int x, int y)
	{
		unsigned char *pixelpnt;

		pixelpnt = &m_file.data[((y * m_header.d_width) + x) * (m_header.d_pixel_size / 8)];
		if (m_header.d_pixel_size == 8)
			return *pixelpnt ;
		else if (m_header.d_pixel_size == 24)
			return (0xff000000 + (pixelpnt[0] << 16) + (pixelpnt[1] << 8) + pixelpnt[2]);
		else
			return ((pixelpnt[0] << 24) + (pixelpnt[1] << 16) + (pixelpnt[2] << 8) + pixelpnt[3]);
	}

	bool IsEnable()
	{
		return m_bEnable ;
	}

	void Release()
	{
		if(m_file.data)
		{
			delete []m_file.data ;
			m_file.data = NULL ;
		}
		m_bEnable = false ;

	}

} ;