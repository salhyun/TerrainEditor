#include "SecretTextureContainer.h"
#include "assert.h"

bool SD3DTexture::SetTexture(char *pszPath, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	char str[256] ;
	sprintf(str, "%s", pszFileName) ;

	if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex)))
	{
		TRACE("Cannot load Texture path=%s\r\n", pszFileName) ;
		assert(false && "Cannot load TextureFile") ;
		return false ;
	}

	D3DSURFACE_DESC desc ;
	int nLevelCount = pTex->GetLevelCount() ;

	for(int i=0 ; i<nLevelCount ; i++)
	{
		ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
		pTex->GetLevelDesc(i, &desc) ;
		if(desc.Format == D3DFMT_A8R8G8B8)
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
		else if(desc.Format == D3DFMT_X8R8G8B8)
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
		else
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
	}

	sprintf(szName, "%s", pszFileName) ;
	TRACE("set texture file=%s\r\n", szName) ;
	return true ;
}
bool SD3DTexture::SetTexture(LPDIRECT3DDEVICE9 pd3dDevice, STextureHeader *psTexHeader)
{
	sprintf(szName, "%s", psTexHeader->szName) ;

	D3DFORMAT fmt ;
	if(psTexHeader->dwColorDepth == 32)
		fmt = D3DFMT_A8R8G8B8 ;
	else
		fmt = D3DFMT_X8R8G8B8 ;

	pd3dDevice->CreateTexture(psTexHeader->dwWidth, psTexHeader->dwHeight, 0, D3DUSAGE_AUTOGENMIPMAP, fmt, D3DPOOL_MANAGED, &pTex, NULL) ;
	//FillTextureWithColorArray(pTex, (DWORD *)psTexHeader->pbyBuffer) ;
	FillTextureWithBuffer(pTex, psTexHeader->pbyBuffer, psTexHeader->dwColorDepth) ;
	//pTex->SetAutoGenFilterType(D3DTEXF_LINEAR) ;
	//pTex->SetAutoGenFilterType(D3DTEXF_ANISOTROPIC) ;
	pTex->GenerateMipSubLevels() ;
	//pTex->SetAutoGenFilterType(D3DTEXF_LINEAR) ;

	D3DSURFACE_DESC desc ;
	int nLevelCount = pTex->GetLevelCount() ;

	for(int i=0 ; i<nLevelCount ; i++)
	{
		ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
		pTex->GetLevelDesc(i, &desc) ;
		if(desc.Format == D3DFMT_A8R8G8B8)
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
		else if(desc.Format == D3DFMT_X8R8G8B8)
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
		else
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height*4) ;
	}

	return true ;
}
bool SD3DTexture::SetTexture(LPDIRECT3DDEVICE9 pd3dDevice, STexInfo *psTexInfo, BYTE *pbyBuffer, D3DPOOL d3dPool, UINT nMipLevels)
{
	sprintf(szName, "%s", psTexInfo->szTexName) ;

	//if(D3DXCreateTextureFromFileInMemory(pd3dDevice, pbyBuffer, psTexInfo->dwBufferSize, &pTex) != D3D_OK)
	//	return false ;

	if(FAILED(D3DXCreateTextureFromFileInMemoryEx(pd3dDevice, pbyBuffer, psTexInfo->dwBufferSize, D3DX_DEFAULT, D3DX_DEFAULT, nMipLevels, 0, D3DFMT_FROM_FILE, d3dPool, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex)))
		return false ;

	return true ;
}
CSecretTextureContainer::CSecretTextureContainer()
{
	m_nNumTex = 0 ;
	m_psTextures = NULL ;
	m_pd3dDevice = NULL ;
}
CSecretTextureContainer::~CSecretTextureContainer()
{
	Release() ;
}
void CSecretTextureContainer::Release()
{
	SAFE_DELETEARRAY(m_psTextures) ;
}
void CSecretTextureContainer::Initialize(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, D3DPOOL d3dPool, UINT nMipLevels)
{
	//int temp ;
	//FILE *pf=NULL ;
	//pf = fopen("../../Media/map/test.txr", "wb+") ;

	//temp=2 ;
	//fwrite(&temp, sizeof(int), 1, pf) ;

	//LPDIRECT3DTEXTURE9 pTex=NULL ;
	//int nMapLevel=0 ;
 //   STexInfo sInfo ;	
	//LPD3DXBUFFER pd3dBuffer=NULL ;

 //   //first time
	//D3DXCreateTextureFromFileEx(pd3dDevice, "../../Media/map/test02.tga", D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex) ;

	//sprintf(sInfo.szTexName, "test01") ;

	////D3DXSaveTextureToFileInMemory(&pd3dBuffer, D3DXIFF_DDS, pTex, NULL) ;
	//D3DXSaveTextureToFile("../../Media/map/Rettest02.tga", D3DXIFF_TGA, pTex, NULL) ;
	//sInfo.dwBufferSize = pd3dBuffer->GetBufferSize() ;
	//TRACE("tex size = %d\r\n", pd3dBuffer->GetBufferSize()) ;

	//fwrite(&sInfo, sizeof(STexInfo), 1, pf) ;
	//fwrite(pd3dBuffer->GetBufferPointer(), pd3dBuffer->GetBufferSize(), 1, pf) ;

	//SAFE_RELEASE(pTex) ;
	//pd3dBuffer->Release() ;

	////second time
	//D3DXCreateTextureFromFileEx(pd3dDevice, "../../Media/map/test02.tga", D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex) ;

	//sprintf(sInfo.szTexName, "test02") ;
	//ZeroMemory(&sInfo.desc, sizeof(D3DSURFACE_DESC)) ;
	//pTex->GetLevelDesc(nMapLevel, &sInfo.desc) ;

	//D3DXSaveTextureToFileInMemory(&pd3dBuffer, D3DXIFF_DDS, pTex, NULL) ;
	//sInfo.dwBufferSize = pd3dBuffer->GetBufferSize() ;
	//TRACE("tex size = %d\r\n", pd3dBuffer->GetBufferSize()) ;

	//fwrite(&sInfo, sizeof(STexInfo), 1, pf) ;
	//fwrite(pd3dBuffer->GetBufferPointer(), pd3dBuffer->GetBufferSize(), 1, pf) ;

	//SAFE_RELEASE(pTex) ;
	//pd3dBuffer->Release() ;

 //   fclose(pf) ;

	//int temp ;
	//FILE *pf=NULL ;
	//pf = fopen("../../Media/map/test.txr", "rb+") ;
	//fread(&temp, sizeof(int), 1, pf) ;

 //   STexInfo sInfo ;	
	//LPDIRECT3DTEXTURE9 pTex=NULL ;

	//fread(&sInfo, sizeof(STexInfo), 1, pf) ;
	////D3DXCreateTexture(pd3dDevice, desc.Width, desc.Height, 0, D3DUSAGE_AUTOGENMIPMAP, desc.Format, desc.Pool, &pTex) ;

	//BYTE *pbyBuffer = new BYTE[sInfo.dwBufferSize] ;
	//fread(pbyBuffer, sInfo.dwBufferSize, 1, pf) ;	

	//D3DXCreateTextureFromFileInMemory(pd3dDevice, pbyBuffer, sInfo.dwBufferSize, &pTex) ;

	//D3DXSaveTextureToFile("../../Media/map/Rettest01.tga", D3DXIFF_TGA, pTex, NULL) ;

	//fclose(pf) ;

	//SAFE_DELETEARRAY(pbyBuffer) ;

	CTextureResourceFile txr ;
	//txr.Savetxr("../../Media/texture resource/", "test.txr", pd3dDevice) ;
	//txr.Loadtxr("../../Media/texture resource/test.txr", this) ;
	//txr.Savetxr("../../Media/map/", "nature.txr", pd3dDevice) ;
	//txr.Loadtxr("../../Media/map/nature.txr", this) ;

	m_pd3dDevice = pd3dDevice ;
	m_psTextures = new SD3DTexture[MAXNUM_TEXTURE] ;

	txr.Loadtxr(pszFileName, this, d3dPool, nMipLevels) ;
}
LPDIRECT3DTEXTURE9 CSecretTextureContainer::FindTexture(char *pszName)
{
	for(int i=0 ; i<m_nNumTex ; i++)
	{
		if(!strcmp(pszName, m_psTextures[i].szName))
			return m_psTextures[i].pTex ;
	}
	return NULL ;
}
int CSecretTextureContainer::FindTextureIndex(char *pszName)
{
	for(int i=0 ; i<m_nNumTex ; i++)
	{
		if(!strcmp(pszName, m_psTextures[i].szName))
			return i ;
	}
	return -1 ;
}
LPDIRECT3DTEXTURE9 CSecretTextureContainer::GetTexture(int n)
{
	if(n < 0 || n >= m_nNumTex)
		return NULL ;

    return m_psTextures[n].pTex ;
}
char *CSecretTextureContainer::GetTextureName(int n)
{
	if(n < 0 || n >= m_nNumTex)
		return NULL ;

	return m_psTextures[n].szName ;
}

//###################################################//
//        class CTextureResourceFileFormat           //
//###################################################//
#include "LoadTga.h"

const unsigned long txr_identity = 't'+'x'+'r' ;
const unsigned long txr_version = 20090219 ;// 20081204 ;

CTextureResourceFile::CTextureResourceFile()
{
	m_nNumTex=0 ;
	m_psTexHeader=NULL ;
}
CTextureResourceFile::~CTextureResourceFile()
{
	SAFE_DELETEARRAY(m_psTexHeader) ;
}

bool CTextureResourceFile::_GetFileData(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, STexInfo *psInfo, LPD3DXBUFFER *ppd3dBuffer)
{
	LPDIRECT3DTEXTURE9 pTex = NULL ;

	if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, pszFileName, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex)))
		return false ;

    D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(0, &desc) ;
	
	D3DXIMAGE_FILEFORMAT format ;
	if(desc.Format == D3DFMT_A8R8G8B8)
		format = D3DXIFF_TGA ;
	else if(desc.Format == D3DFMT_X8R8G8B8)
		format = D3DXIFF_PNG ;
	else if(desc.Format == D3DFMT_A8)
		format = D3DXIFF_PNG ;
	else if((desc.Format == D3DFMT_DXT1) || (desc.Format == D3DFMT_DXT5))
		format = D3DXIFF_DDS ;
	else
		format = D3DXIFF_PNG ;

    GetFileNameFromPath(psInfo->szTexName, pszFileName) ;
	D3DXSaveTextureToFileInMemory(ppd3dBuffer, format, pTex, NULL) ;
	SAFE_RELEASE(pTex) ;
	return true ;
}
bool CTextureResourceFile::Savetxr(char *pszPath, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i, nNumTex ;
    char **ppszTextureNames=NULL, **ppStr=NULL ;
	ppszTextureNames = new char*[MAXNUM_TEXTURE] ;
	for(i=0 ; i<MAXNUM_TEXTURE ; i++)
		ppszTextureNames[i] = new char[256] ;

    //m_nNumTex = GetAllFileNames(&ppszTextureNames, pszPath, "tga") ;
	//assert(m_nNumTex > 0) ;

	nNumTex = 0 ;

    ppStr = &ppszTextureNames[nNumTex] ;
	nNumTex += GetAllFileNames(&ppStr, pszPath, "tga") ;

	ppStr = &ppszTextureNames[nNumTex] ;
	nNumTex += GetAllFileNames(&ppStr, pszPath, "dds") ;

	m_nNumTex = nNumTex ;

	CTga tga ;
	STextureHeader sTexHeader ;
	sTexHeader.dwSize = sizeof(STextureHeader) ;

	BYTE *pbyBuffer = new BYTE[16777216] ;//2048*2048*4

	char str[256] ;
	sprintf(str, "%s%s", pszPath, pszFileName) ;
    FILE *pf = fopen(str, "w+b") ;
	if(pf == NULL)
		return false ;

    fwrite(&txr_identity, 4, 1, pf) ;
	fwrite(&txr_version, 4, 1, pf) ;
	fwrite(&m_nNumTex, 4, 1, pf) ;

	for(i=0 ; i<m_nNumTex ; i++)
	{
		TRACE("%s is reading...", ppszTextureNames[i]) ;

		STexInfo sInfo ;
		LPD3DXBUFFER pd3dBuffer=NULL ;
		_GetFileData(ppszTextureNames[i], pd3dDevice, &sInfo, &pd3dBuffer) ;

		sInfo.dwBufferSize = pd3dBuffer->GetBufferSize() ;
		fwrite(&sInfo, sizeof(STexInfo), 1, pf) ;
		fwrite(pd3dBuffer->GetBufferPointer(), sInfo.dwBufferSize, 1, pf) ;
		pd3dBuffer->Release() ;

		//sTexHeader.pbyBuffer = pbyBuffer ;
		//GetImageDatafromFile(ppszTextureNames[i], pd3dDevice, &sTexHeader) ;
		//fwrite(&sTexHeader, sizeof(STextureHeader), 1, pf) ;
		//fwrite(pbyBuffer, sTexHeader.dwImageSize, 1, pf) ;

		TRACE("...finished\r\n", ppszTextureNames[i]) ;
	}

	fclose(pf) ;

	sTexHeader.pbyBuffer = NULL ;
	SAFE_DELETEARRAY(pbyBuffer) ;

	for(i=0 ; i<MAXNUM_TEXTURE ; i++)
		SAFE_DELETEARRAY(ppszTextureNames[i]) ;
	SAFE_DELETEARRAY(ppszTextureNames) ;

	return true ;
}
bool CTextureResourceFile::Savetxr(char *pszPath, char **ppszFileNames, int nNumFile, char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice)
{
	int i ;
	m_nNumTex = nNumFile ;

	CTga tga ;
	STextureHeader sTexHeader ;
	sTexHeader.dwSize = sizeof(STextureHeader) ;

	BYTE *pbyBuffer = new BYTE[16777216] ;//2048*2048*4

	char str[256] ;
	sprintf(str, "%s%s", pszPath, pszFileName) ;
    FILE *pf = fopen(str, "w+b") ;
	if(pf == NULL)
		return false ;

    fwrite(&txr_identity, 4, 1, pf) ;
	fwrite(&txr_version, 4, 1, pf) ;
	fwrite(&m_nNumTex, 4, 1, pf) ;

	for(i=0 ; i<m_nNumTex ; i++)
	{
		TRACE("%s is reading...", ppszFileNames[i]) ;

		STexInfo sInfo ;
		LPD3DXBUFFER pd3dBuffer=NULL ;
		_GetFileData(ppszFileNames[i], pd3dDevice, &sInfo, &pd3dBuffer) ;

		sInfo.dwBufferSize = pd3dBuffer->GetBufferSize() ;
		fwrite(&sInfo, sizeof(STexInfo), 1, pf) ;
		fwrite(pd3dBuffer->GetBufferPointer(), sInfo.dwBufferSize, 1, pf) ;
		pd3dBuffer->Release() ;

		TRACE("...finished\r\n", ppszFileNames[i]) ;
	}

	fclose(pf) ;

	sTexHeader.pbyBuffer = NULL ;
	SAFE_DELETEARRAY(pbyBuffer) ;
	return true ;
}
bool CTextureResourceFile::Loadtxr(char *pszFileName, CSecretTextureContainer *psTexContainer, D3DPOOL d3dPool, UINT nMipLevels)
{
	FILE *pf = fopen(pszFileName, "r+b") ;
	if(pf == NULL)
		return false ;

	int i, temp ;

	fread(&temp, 4, 1, pf) ;
	if(temp != txr_identity)
		return false ;

	fread(&temp, 4, 1, pf) ;
	if(temp != txr_version)
		return false ;

	fread(&m_nNumTex, 4, 1, pf) ;
	if(m_nNumTex <= 0)
		return false ;

	TRACE("TexContainer addr=%x\r\n", psTexContainer) ;

	SD3DTexture *psTextures = &psTexContainer->m_psTextures[psTexContainer->m_nNumTex] ;

	psTexContainer->m_nNumTex += m_nNumTex ;

	STextureHeader sTexHeader ;
	BYTE *pbyBuffer = new BYTE[16777216] ;

	TRACE("Loading txr file...") ;

	for(i=0 ; i<psTexContainer->m_nNumTex ; i++)
	{
		STexInfo sInfo ;
		fread(&sInfo, sizeof(STexInfo), 1, pf) ;
		fread(pbyBuffer, sInfo.dwBufferSize, 1, pf) ;

		psTextures[i].SetTexture(psTexContainer->m_pd3dDevice, &sInfo, pbyBuffer, d3dPool, nMipLevels) ;

		//fread(&sTexHeader, sizeof(STextureHeader), 1, pf) ;
		//sTexHeader.pbyBuffer = pbyBuffer ;
		//fread(sTexHeader.pbyBuffer, sTexHeader.dwImageSize, 1, pf) ;

		//psTextures[i].SetTexture(psTexContainer->m_pd3dDevice, &sTexHeader) ;
	}
	TRACE("...finished txr file reading\r\n") ;

	sTexHeader.pbyBuffer = NULL ;
	SAFE_DELETEARRAY(pbyBuffer) ;

	fclose(pf) ;
	return true ;
}

bool CTextureResourceFile::GetImageDatafromFile(char *pszFileName, LPDIRECT3DDEVICE9 pd3dDevice, STextureHeader *m_psTexHeader)
{
	LPDIRECT3DTEXTURE9 pTex=NULL ;
	if(FAILED(D3DXCreateTextureFromFileEx(pd3dDevice, pszFileName, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTex)))
	{
		TRACE("Cannot load Texture path=%s\r\n", pszFileName) ;
		assert(false && "Cannot load TextureFile") ;
		return false ;
	}

	int nMapLevel=0 ;
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(nMapLevel, &desc) ;

	char str[256] ;
	GetFileNameFromPath(str, pszFileName) ;
	sprintf(m_psTexHeader->szName, "%s", str) ;
	m_psTexHeader->dwWidth = desc.Width ;
	m_psTexHeader->dwHeight = desc.Height ;
	m_psTexHeader->dwColorDepth = 32 ;
	if(desc.Format == D3DFMT_X8R8G8B8)
		m_psTexHeader->dwColorDepth = 24 ;

	m_psTexHeader->dwImageSize = m_psTexHeader->dwWidth*m_psTexHeader->dwHeight*(m_psTexHeader->dwColorDepth/8) ;

	DWORD *pdwColor ;
	D3DLOCKED_RECT d3drect ;
	pTex->LockRect(nMapLevel, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	pTex->AddDirtyRect(NULL) ;
	pdwColor = (DWORD *)d3drect.pBits ;

	int x, y, count=0 ;
	BYTE a, r, g, b ;
	BYTE *pbyBuffer = m_psTexHeader->pbyBuffer ;
	for(y=0 ; y<(int)desc.Height ; y++)
	{
		for(x=0 ; x<(int)desc.Width ; x++, pdwColor++)
		{
			a = (BYTE)(((*pdwColor)>>24)&0xff) ;
			r = (BYTE)(((*pdwColor)>>16)&0xff) ;
			g = (BYTE)(((*pdwColor)>>8)&0xff) ;
			b = (BYTE)((*pdwColor)&0xff) ;

			if(m_psTexHeader->dwColorDepth == 32)
				pbyBuffer[count++] = a ;

			pbyBuffer[count++] = r ;
			pbyBuffer[count++] = g ;
			pbyBuffer[count++] = b ;			
		}
	}

	pTex->UnlockRect(nMapLevel) ;
	assert((DWORD)count == m_psTexHeader->dwImageSize) ;
	SAFE_RELEASE(pTex) ;
	return true ;
}