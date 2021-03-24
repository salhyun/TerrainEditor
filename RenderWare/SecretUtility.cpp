#include <assert.h>
#include "SecretUtility.h"
#include "def.h"
#include "LoadTGA.h"

void IsRange(CTga *ptga, unsigned long &x, unsigned long &y)
{
	if(x < 0)								x = 0 ;
	else if(x >= ptga->m_header.d_width)		x = ptga->m_header.d_width-1 ;
	if(y < 0)								y = 0 ;
	else if(y >= ptga->m_header.d_height)	y = ptga->m_header.d_height-1 ;
}

void GetNormalandHeight(CTga *ptga, unsigned long x, unsigned long y, Vector3 &vNormal, float &fHeight)
{
	IsRange(ptga, x, y) ;
	unsigned long rgba = ptga->GetRGBFromTGA(x, y) ;
	vNormal.x = (float)((rgba >> 24) & 0xff) ;
	vNormal.y = (float)((rgba >> 16) & 0xff) ;
	vNormal.z = (float)((rgba >>  8) & 0xff) ;
	fHeight = (float)(rgba & 0xff) ;
}

Vector3 GetNormal(CTga *ptga, unsigned long x, unsigned long y)
{
	IsRange(ptga, x, y) ;
	Vector3 vNormal ;
	unsigned long rgba = ptga->GetRGBFromTGA(x, y) ;
	vNormal.x = (float)((rgba >> 24) & 0xff) ;
	vNormal.y = (float)((rgba >> 16) & 0xff) ;
	vNormal.z = (float)((rgba >>  8) & 0xff) ;
	return vNormal ;
}

float GetHeight(CTga *ptga, unsigned long x, unsigned long y)
{
	IsRange(ptga, x, y) ;
	float fHeight ;

	//     a  b  g  r
	// 0x 00 00 00 00
	unsigned long abgr = ptga->GetRGBFromTGA(x, y) ;

	unsigned char r = (unsigned char)(abgr&0xff) ;
	unsigned char g = (unsigned char)((abgr>>8)&0xff) ;
	unsigned char b = (unsigned char)((abgr>>16)&0xff) ;

	//셋중에 가장 큰값을 사용한다.
	if(r > g)
	{
		if(r > b)
			fHeight = (float)r ;
		else
			fHeight = (float)b ;
	}
	else
	{
		if(g > b)
			fHeight = (float)g ;
		else
			fHeight = (float)b ;
	}

	//fHeight = (float)((rgba&0xff00)>>8) ;//green만 가지고 높이값 뽑아냄.
	return fHeight ;
}

void GetNormalandTangent(CTga *ptga, unsigned long x, unsigned long y, Vector3 &vNormal, Vector3 &vTangent)
{
	float dx, dy ;
	dx = (GetHeight(ptga, x+1, y)-GetHeight(ptga, x-1, y))/2.0f ;
	dy = (GetHeight(ptga, x, y+1)-GetHeight(ptga, x, y-1))/2.0f ;

	vNormal = GetNormal(ptga, x, y) ;
	vNormal = vNormal.Normalize() ;

	vTangent.x = 256.0f ;
	vTangent.z = dx ;
	vTangent.y = 0.0f ;

	vTangent = vTangent.Normalize() ;
}

bool CreateNormalBumpData(char *pszFileName)
{
    CTga tga ;
	tga.LoadTGAFile(pszFileName) ;
	if(!tga.IsEnable())
	{
		assert(false && "cannot found tgafile") ;
		return false ;
	}

	if(tga.m_header.d_pixel_size != 32)
	{
		assert(false && "bitcount is not 32 in CreateNormalBumpData function") ;
		return false ;
	}

	Vector3 vNormal, vTangent ;
	//float fHeight ;
	//unsigned long rgba ;
	unsigned short x, y ;
	for(y=0 ; y<tga.m_header.d_height ; y++)
	{
		for(x=0 ; x<tga.m_header.d_width ; x++)
		{
		}
	}
    
	return true ;
}

float GetDistX(CTga *ptga, int x1, int x2)
{
    if(x1 < 0)
		x1 = 0 ;

	if(x2 >= ptga->m_header.d_width)
		x2 = ptga->m_header.d_width-1 ;

	return (float)abs(x2-x1) ;
}

float GetDistY(CTga *ptga, int y1, int y2)
{
    if(y1 < 0)
		y1 = 0 ;

	if(y2 >= ptga->m_header.d_height)
		y2 = ptga->m_header.d_height-1 ;

	return (float)abs(y2-y1) ;
}

SNormalRGB GetNormalRGB(CTga *ptga, unsigned long x, unsigned long y)
{
	float dx, dy, denominater ;

	//float height = GetHeight(ptga, x, y) ;
	//height = GetHeight(ptga, 0, 4) ;
	//height = GetHeight(ptga, 7, 35) ;

    denominater = GetDistX(ptga, x-1, x+1) ;
	dx = (GetHeight(ptga, x+1, y)-GetHeight(ptga, x-1, y))/denominater ;

	denominater = GetDistY(ptga, y-1, y+1) ;
	dy = (GetHeight(ptga, x, y+1)-GetHeight(ptga, x, y-1))/denominater ;

//	if(x == 66 && y ==22)
//		x = 66 ;

	float ratio=8.0f ;
	Vector3 u, v ;
	u = Vector3(1, 0, dx/ratio).Normalize() ;
	v = Vector3(0, 1, dy/ratio).Normalize() ;
	Vector3 vNormal = u.cross(v) ;

	SNormalRGB sNormalRGB ;
	//값을 저장할때
	//[-1, 1] 범위를 [0, 255] 사이에 값으로 저장한다
	sNormalRGB.r = (BYTE)((vNormal.x+1)*127.5f) ;
	sNormalRGB.g = (BYTE)((vNormal.y+1)*127.5f) ;
	sNormalRGB.b = (BYTE)((vNormal.z+1)*127.5f) ;

	if(sNormalRGB.r > 255)
		sNormalRGB.r = 255 ;
	if(sNormalRGB.g > 255)
		sNormalRGB.g = 255 ;
	if(sNormalRGB.b > 255)
		sNormalRGB.b = 255 ;

	return sNormalRGB ;
}

bool CreateNormalMap(char *pszFileName)
{
	CTga tga ;
	tga.LoadTGAFile(pszFileName) ;
	if(!tga.IsEnable())
	{
		assert(false && "cannot found tgafile") ;
		return false ;
	}

	unsigned short x, y ;
	SNormalRGB *psNormalRGB = new SNormalRGB[tga.m_header.d_height * tga.m_header.d_width] ;
	for(y=0 ; y<tga.m_header.d_height ; y++)
	{
		for(x=0 ; x<tga.m_header.d_width ; x++)
		{
			psNormalRGB[(-((y+1)-tga.m_header.d_height)*tga.m_header.d_width)+x] = GetNormalRGB(&tga, x, y) ;
			//TRACE("psNormalRGB[%04d] = (%d %d %d)\r\n", (-((y+1)-tga.m_header.d_height)*tga.m_header.d_width)+x,
			//	psNormalRGB[(-((y+1)-tga.m_header.d_height)*tga.m_header.d_width)+x].r,
			//	psNormalRGB[(-((y+1)-tga.m_header.d_height)*tga.m_header.d_width)+x].g,
			//	psNormalRGB[(-((y+1)-tga.m_header.d_height)*tga.m_header.d_width)+x].b) ;
		}
	}

	tTGAHeader_s sTGAHeader ;
	ZeroMemory(&sTGAHeader, sizeof(tTGAHeader_s)) ;
	sTGAHeader.d_iif_size = 0 ;
	sTGAHeader.d_cmap_type = 0 ;
	sTGAHeader.d_image_type = 2 ;
	//sTGAHeader.pad[0] - pad[4] 모두 0
	sTGAHeader.d_x_origin = 0 ;
	sTGAHeader.d_y_origin = 0 ;
	sTGAHeader.d_width = tga.m_header.d_width ;
	sTGAHeader.d_height = tga.m_header.d_height ;
	sTGAHeader.d_pixel_size = 24 ;
	sTGAHeader.d_image_descriptor = 0 ;

	char szNMFileName[256], *psz ;
    psz = strrchr(pszFileName, '.') ;
	memcpy(szNMFileName, pszFileName, psz-pszFileName) ;
	szNMFileName[psz-pszFileName] = '\0' ;
	strcat(szNMFileName, "_NM.tga") ;

	int lExtra=0, lPitch = sTGAHeader.d_width*(sTGAHeader.d_pixel_size/8) ;
	if(lPitch%4)
		lExtra = 4-(lPitch%4) ;

	BYTE *pbyBuf= new BYTE[lPitch], *pbyData=(BYTE *)psNormalRGB ;
	DWORD dwBuf=0 ;

	FILE *pf=NULL ;
	pf = fopen(szNMFileName, "wb") ;
	fwrite(&sTGAHeader, sizeof(tTGAHeader_s), 1, pf) ;

	for(y=0 ; y<sTGAHeader.d_height ; y++)
	{
		memcpy(pbyBuf, &pbyData[y*lPitch], lPitch) ;
		fwrite(pbyBuf, lPitch, 1, pf) ;

		if(lExtra)
			fwrite(&dwBuf, lExtra, 1, pf) ;
	}

	//fwrite(psNormalRGB, sizeof(SNormalRGB), tga.m_header.d_height*tga.m_header.d_width, pf) ;
	//fwrite(psNormalRGB, sizeof(SNormalRGB)*tga.m_header.d_height*tga.m_header.d_width, 1, pf) ;
	fclose(pf) ;

	delete []psNormalRGB ;
	delete []pbyBuf ;

	return true ;
}