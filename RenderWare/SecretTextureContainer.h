#pragma once

#include "def.h"
#include "D3Ddef.h"

#define MAXNUM_TEXTURE 1024

struct STexInfo ;
struct STextureHeader ;
class CTextureResourceFile ;

struct SD3DTexture
{
	char szName[256] ;
	LPDIRECT3DTEXTURE9 pTex ;

	SD3DTexture() {sprintf(szName, ""); pTex=NULL;}
	~SD3DTexture() {SAFE_RELEASE(pTex);}

    bool SetTexture(char *pszPath, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice) ;
	bool SetTexture(LPDIRECT3DDEVICE9 pd3dDevice, STextureHeader *psTexHeader) ;
	bool SetTexture(LPDIRECT3DDEVICE9 pd3dDevice, STexInfo *psTexInfo, BYTE *pbyBuffer, D3DPOOL d3dPool, UINT nMipLevels) ;
} ;

class CSecretTextureContainer
{
public :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nNumTex ;
	SD3DTexture *m_psTextures ;

public :
	CSecretTextureContainer() ;
	~CSecretTextureContainer() ;

	void Initialize(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, D3DPOOL d3dPool=D3DPOOL_MANAGED, UINT nMipLevels=D3DX_DEFAULT) ;

    LPDIRECT3DTEXTURE9 FindTexture(char *pszName) ;
	int FindTextureIndex(char *pszName) ;
	LPDIRECT3DTEXTURE9 GetTexture(int n) ;
	char *GetTextureName(int n) ;

	void Release() ;
} ;
struct STexInfo
{
	char szTexName[256] ;
	DWORD dwBufferSize ;
} ;
struct STextureHeader
{
	DWORD dwSize ;
	char szName[256] ;
	DWORD dwWidth, dwHeight ;
	DWORD dwColorDepth ;//in bits
	DWORD dwImageSize ;//image size in bytes
	BYTE *pbyBuffer ;

	STextureHeader() {dwSize=0; dwWidth=0; dwHeight=0; dwColorDepth=0; dwImageSize=0; pbyBuffer=NULL;}
	~STextureHeader() {SAFE_DELETEARRAY(pbyBuffer);}
} ;
    
class CTextureResourceFile
{
private :
	int m_nNumTex ;
    STextureHeader *m_psTexHeader ;

    bool _GetFileData(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, STexInfo *psInfo, LPD3DXBUFFER *ppd3dBuffer) ;

public :
	CTextureResourceFile() ;
	~CTextureResourceFile() ;

	bool Savetxr(char *pszPath, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice) ;
	bool Savetxr(char *pszPath, char **ppszFileNames, int nNumFile, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice) ;
    bool Loadtxr(char *pszFileName, CSecretTextureContainer *psTexContainer, D3DPOOL d3dPool=D3DPOOL_MANAGED, UINT nMipLevels=D3DX_DEFAULT) ;
	bool GetImageDatafromFile(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, STextureHeader *m_psTexHeader) ;
} ;

//Texture Resources file format
//file Identity
//file version
//number of textures
