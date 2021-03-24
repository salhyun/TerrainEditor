#include "def.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>

CSecretDebugMsg g_cDebugMsg ;

#if !defined(TRACE) && !defined(ATLTRACE)
void TRACE(char* lpFormatStr, ...)
{
#if defined(DEBUG) | defined(_DEBUG)

	TCHAR   szBuffer[512];
	va_list args;
	va_start    (args, lpFormatStr);
	_vsntprintf (szBuffer, 512, lpFormatStr, args);
	va_end      (args);
	OutputDebugString (szBuffer);
#endif
}
#endif

void OutputStringforRelease(char* lpFormatStr, ...)
{
#ifdef DEBUGGING_FOR_RELEASE
	TCHAR   szBuffer[512];
	va_list args;
	va_start    (args, lpFormatStr);
	_vsntprintf (szBuffer, 512, lpFormatStr, args);
	va_end      (args);
	OutputDebugString (szBuffer);
#endif
}

bool GePathfromFullPath(char *pszPath, char *pszFullPath)
{
    int nLen = (int)strlen(pszFullPath) ;
	int nCount ;

	nCount=nLen-1 ;
	while(nCount >= 0)
	{
		if(pszFullPath[nCount] == '/' || pszFullPath[nCount] == '\\')
		{
			nCount++ ;
            memcpy(pszPath, pszFullPath, nCount) ;
			pszPath[nCount] = '\0' ;
			return true ;
		}
		nCount-- ;
	}
	return false ;
}
bool GetFileNameFromPath(char *pszFileName, char *pszPath)
{
	int nLen = (int)strlen(pszPath) ;
	int nCount ;

	nCount=nLen-1 ;
	while(nCount >= 0)
	{
		if(pszPath[nCount] == '/' || pszPath[nCount] == '\\')
		{
			char *psz = strrchr(pszPath, '.') ;
			int l = (int)(psz-(&pszPath[nCount+1])) ;
            memcpy(pszFileName, &pszPath[nCount+1], l) ;
			pszFileName[l] = '\0' ;
            return true ;
		}
		nCount-- ;
	}
    return false ;
}
void RemoveExt(char *pszName, char *pszFileName)
{
	char *psz = strrchr(pszFileName, '.') ;
	int l = (int)(psz-pszFileName) ;
	memcpy(pszName, pszFileName, l) ;
	pszName[l] = '\0' ;
}

bool IsExistFile(char *pszFileName, char *pszPath)
{
	HANDLE hfile=NULL ;
	WIN32_FIND_DATA FindfileData ;
	char str[256] ;

	sprintf(str, "%s%s", pszPath, pszFileName) ;

	hfile = FindFirstFile(str, &FindfileData) ;
	if(hfile == INVALID_HANDLE_VALUE)
	{
        TRACE("Invalid file handle.\r\n") ;
		return false ;
	}
	FindClose(hfile) ;
	return true ;
}
bool IsStrStrInPath(char *pszName, char *pszPath, char *pszFileName)
{
	int count=0 ;
	HANDLE hfile=NULL ;
	WIN32_FIND_DATA FindfileData ;
	char str[256], *pstr=NULL ;

	sprintf(str, "%s*.*", pszPath) ;

	hfile = FindFirstFile(str, &FindfileData) ;
	if(hfile == INVALID_HANDLE_VALUE)
	{
        TRACE("Invalid file handle.\r\n") ;
		return false ;
	}
	else
	{
		if(strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
		{
			pstr = strstr(FindfileData.cFileName, pszName) ;
			if(pstr)
			{
				sprintf(pszFileName, "%s", FindfileData.cFileName) ;
				return true ;
			}
		}

		BOOL bExist=true ;
		while(true)
		{
			bExist = FindNextFile(hfile, &FindfileData) ;
			if(bExist)
			{
				if(strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
				{
					pstr = strstr(FindfileData.cFileName, pszName) ;
					if(pstr)
					{
						sprintf(pszFileName, "%s", FindfileData.cFileName) ;
						return true ;
					}
				}
			}
			else
				break ;
		}
	}
	FindClose(hfile) ;
	return false ;
}
int GetAllFileNames(char ***pppszFileNames, char *pszPath, char *pszExt, const bool bFullName)
{
    int count=0 ;
	HANDLE hfile=NULL ;
	WIN32_FIND_DATA FindfileData ;
	char str[256], **ppszFileNames = *pppszFileNames ;

	sprintf(str, "%s*.%s", pszPath, pszExt) ;

	hfile = FindFirstFile(str, &FindfileData) ;
	if(hfile == INVALID_HANDLE_VALUE)
	{
        TRACE("Invalid file handle.\r\n") ;
		return count ;
	}
	else
	{
		if(strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
		{
			if(bFullName)
				sprintf(ppszFileNames[count], "%s%s", pszPath, FindfileData.cFileName) ;
			else
				sprintf(ppszFileNames[count], "%s", FindfileData.cFileName) ;

			//TRACE("\"%s\",\r\n", ppszFileNames[count]) ;
			count++ ;
		}

		BOOL bExist=true ;
		while(true)
		{
			bExist = FindNextFile(hfile, &FindfileData) ;
			if(bExist)
			{
				if(strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
				{
					if(bFullName)
						sprintf(ppszFileNames[count], "%s%s", pszPath, FindfileData.cFileName) ;
					else
						sprintf(ppszFileNames[count], "%s", FindfileData.cFileName) ;

					//TRACE("\"%s\",\r\n", ppszFileNames[count]) ;
					count++ ;
				}
			}
			else
				break ;
		}
	}
	FindClose(hfile) ;
	return count ;
}
int GetAllFolderNames(char ***pppszFileNames, char *pszPath, const bool bFullName)
{
	int count=0 ;
	HANDLE hfile=NULL ;
	WIN32_FIND_DATA FindfileData ;
	char str[256], **ppszFileNames = *pppszFileNames ;

	sprintf(str, "%s*.*", pszPath) ;

	hfile = FindFirstFile(str, &FindfileData) ;
	if(hfile == INVALID_HANDLE_VALUE)
	{
        TRACE("Invalid file handle.\r\n") ;
		return count ;
	}
	else
	{
		if((FindfileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
		{
			if(bFullName)
				sprintf(ppszFileNames[count], "%s%s", pszPath, FindfileData.cFileName) ;
			else
				sprintf(ppszFileNames[count], "%s", FindfileData.cFileName) ;

			TRACE("\"%s\",\r\n", ppszFileNames[count]) ;
			count++ ;
		}

		BOOL bExist=true ;
		while(true)
		{
			bExist = FindNextFile(hfile, &FindfileData) ;
			if(bExist)
			{
				if((FindfileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(FindfileData.cFileName, ".") && strcmp(FindfileData.cFileName, ".."))//상위폴더표시는 제외시킨다.
				{
					if(bFullName)
						sprintf(ppszFileNames[count], "%s%s", pszPath, FindfileData.cFileName) ;
					else
						sprintf(ppszFileNames[count], "%s", FindfileData.cFileName) ;

					TRACE("\"%s\",\r\n", ppszFileNames[count]) ;
					count++ ;
				}
			}
			else
				break ;
		}
	}
	FindClose(hfile) ;
	return count ;
}
bool SaveBitmapFile(char *pszFileName, int nWidth, int nHeight, unsigned long lBitCount, BYTE *pbyData)
{
	BITMAPFILEHEADER bmfh ;
	BITMAPINFOHEADER bmih ;

	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER)) ;
	memset(&bmih, 0, sizeof(BITMAPINFOHEADER)) ;

	bmih.biSize = sizeof(BITMAPINFOHEADER) ;
	bmih.biWidth = nWidth ;
	bmih.biHeight = nHeight ;
	bmih.biPlanes = 1 ;
	bmih.biBitCount = (WORD)lBitCount ;
	bmih.biCompression = BI_RGB ;
	bmih.biSizeImage = 0 ;
	bmih.biXPelsPerMeter = 2834 ;
	bmih.biYPelsPerMeter = 2834 ;

	int i, lExtra=0, lPitch = nWidth*(lBitCount/8) ;
	if(lPitch%4)
	{
		lExtra = 4-(lPitch%4) ;
	}

	BYTE *pbyBuf = new BYTE[lPitch] ;
	DWORD dwBuf=0 ;

	unsigned long lDataSize = bmih.biWidth*bmih.biHeight*(bmih.biBitCount/8) + (lExtra*bmih.biHeight) ;
	bmfh.bfType = ((WORD) ('M' << 8) | 'B') ;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + lDataSize ;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ;

	FILE *pf=NULL ;
	pf = fopen(pszFileName, "wb+") ;
	if(!pf)
		return false ;
	
	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, pf) ;
	fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, pf) ;

	for(i=0 ; i<nHeight ; i++)
	{
		memcpy(pbyBuf, &pbyData[i*lPitch], lPitch) ;
		fwrite(pbyBuf, lPitch, 1, pf) ;

		if(lExtra)
			fwrite(&dwBuf, lExtra, 1, pf) ;
	}

	fclose(pf) ;
	pf=NULL ;

	SAFE_DELETEARRAY(pbyBuf) ;

	return true ;
}

void ConvertRGBtoHSI(float r, float g, float b, float &h, float &s, float &i)
{
	//intensity
	i = (r+g+b)/3.0f ;

	//saturation
	float a = min(r, min(g, b)) ;
	s = 1.0f - 3.0f/(r+g+b)*a ;

	if(r == g && g == b)
	//if(fabs(r-g) <= FLT_EPSILON && fabs(g-b) <= FLT_EPSILON)
		h = 0.0f ;
	else
	{
		h = (r - g*0.5f - b*0.5f) / ( sqrt( (r-g)*(r-g) + (r-b)*(g-b) ) ) ;
		h = acosf(h)*180.0f/3.141592f ;

		if((b/i) > (g/i))
			h = 360.0f - h ;
	}
}
void ConvertHSItoRGB(float h, float s, float i, float &r, float &g, float &b)
{
	float angle1, angle2, scale ;

	if(i == 0.0f)
	{
		r = g = b = 0.0f ;
		return ;
	}
	if(s == 0.0f)
	{
		r = g = b = i ;
		return ;
	}

	if(h < 0.0f)
		h += 360.0f ;

	scale = i * 3.0f ;

	if(h <= 120.0f)
	{
		angle1 = h*3.141592f/180.0f ;
		angle2 = (60.0f-h)*3.141592f/180.0f ;

		b = (1.0f-s)/3.0f ;
		r = (1.0f + (s*cosf(angle1)/cosf(angle2)))/3.0f ;
		g = 1.0f - r - b ;
	}
	else if((h > 120.0f) && (h <= 240.0f))
	{
		h -= 120.0f ;

		angle1 = h*3.141592f/180.0f ;
		angle2 = (60.0f-h)*3.141592f/180.0f ;

		r = (1.0f-s)/3.0f ;
		g = (1.0f + (s*cosf(angle1)/cosf(angle2)))/3.0f ;
		b = 1.0f - r - g ;
	}
	else
	{
		h -= 240.0f ;

		angle1 = h*3.141592f/180.0f ;
		angle2 = (60.0f-h)*3.141592f/180.0f ;

		g = (1.0f-s)/3.0f ;
		b = (1.0f + (s*cosf(angle1)/cosf(angle2)))/3.0f ;
		r = 1.0f - g - b ;
	}

	r *= scale ;
	g *= scale ;
	b *= scale ;
}