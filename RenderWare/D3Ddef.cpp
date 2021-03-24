#include "def.h"
#include "D3Ddef.h"
#include <assert.h>
#include <iostream>
#include "SecretMath.h"

void SCVertex::set(D3DXVECTOR3 &p, DWORD dwColor)
{
	pos = p ;
	color = dwColor ;
}

D3DXMATRIX GetMatrixLookatLH(D3DXVECTOR3 vLookat, D3DXVECTOR3 vPos, D3DXVECTOR3 vUp)
{
	D3DXMATRIX mat ;
	D3DXMatrixLookAtLH(&mat, &vPos, &vLookat, &vUp) ;
	return mat ;
}

STerrain_File::STerrain_File()
{
	psFileHeader = NULL ;
	psMaterialInfo = NULL ;
	psFragmentInfo = NULL ;
	ppbyAlphaValue = NULL ;

	nNumTerrainLayer = 0 ;
	ppszTerrainLayer = NULL ;

	nNumNoiseMap = 0 ;
	ppszNoiseMap = NULL ;
}
STerrain_File::~STerrain_File()
{
}

void STerrain_File::Release()
{
	int i ;
	for(i=0 ; i<psFileHeader->nNumAlphaMap ; i++)
		SAFE_DELETEARRAY(ppbyAlphaValue[i]) ;
	SAFE_DELETEARRAY(ppbyAlphaValue) ;

    for(i=0 ; i<nNumTerrainLayer ; i++)
		SAFE_DELETEARRAY(ppszTerrainLayer[i]) ;
	SAFE_DELETEARRAY(ppszTerrainLayer) ;

	for(i=0 ; i<nNumNoiseMap ; i++)
		SAFE_DELETEARRAY(ppszNoiseMap[i]) ;
	SAFE_DELETEARRAY(ppszNoiseMap) ;

	SAFE_DELETEARRAY(psFragmentInfo) ;

    SAFE_DELETEARRAY(psMaterialInfo) ;
	SAFE_DELETEARRAY(psFileHeader) ;
}

void STerrain_File::Initialize(int width, int height, int nNumerator, int nDenominator, int numAlphamap, int Alphamapsize, int numMaterial, int numTerrainlayer, int numNoisemap)
{
	int i ;

	psFileHeader = new STerrain_FileHeader ;
	ZeroMemory(psFileHeader, sizeof(STerrain_FileHeader)) ;
	psFileHeader->dwType = 0x154 ;
	psFileHeader->lWidth = width ;
	psFileHeader->lHeight = height ;
	psFileHeader->nNumerator = nNumerator ;
	psFileHeader->nDenominator = nDenominator ;
	psFileHeader->nNumAlphaMap = numAlphamap ;
	psFileHeader->nAlphaMapSize = Alphamapsize ;
	psFileHeader->lNumMaterial = numMaterial ;

    if(psFileHeader->lNumMaterial > 0)
	{
		psMaterialInfo = new STerrain_MaterialInfo[psFileHeader->lNumMaterial] ;
	}

	float fdiv = (float)psFileHeader->nNumerator/(float)psFileHeader->nDenominator ;//1m x 1m 이하로는 할수 없다.
	int nTileSize = (int)fdiv ;

	int nVertexWidth = psFileHeader->lWidth/nTileSize+1 ;
	int nVertexHeight = psFileHeader->lHeight/nTileSize+1 ;

	psFragmentInfo = new STerrain_FragmentInfo[nVertexWidth*nVertexHeight] ;

	if(psFileHeader->nNumAlphaMap > 0)
	{//32bit 알파맵2장을 사용. 그래서 저장을 8bit채널 하나씩 즉 (알파맵2장 * 4채널)

		ppbyAlphaValue = new BYTE*[psFileHeader->nNumAlphaMap] ;
        for(i=0 ; i<psFileHeader->nNumAlphaMap ; i++)
			ppbyAlphaValue[i] = new BYTE[psFileHeader->nAlphaMapSize*psFileHeader->nAlphaMapSize] ;
	}

	nNumTerrainLayer = numTerrainlayer ;
	ppszTerrainLayer = new char*[nNumTerrainLayer] ;
	for(i=0 ; i<nNumTerrainLayer ; i++)
		ppszTerrainLayer[i] = new char[256] ;

	nNumNoiseMap = numNoisemap ;
	ppszNoiseMap = new char*[nNumNoiseMap] ;
	for(i=0 ; i<nNumNoiseMap ; i++)
		ppszNoiseMap[i] = new char[256] ;
}

void STerrain_File::WriteToFile(FILE *pf)
{
	int i ;

	fwrite(psFileHeader, sizeof(STerrain_FileHeader), 1, pf) ;

	if(psFileHeader->lNumMaterial > 0)//don't use this
	{
	}

	float fdiv = (float)psFileHeader->nNumerator/(float)psFileHeader->nDenominator ;//1m x 1m 이하로는 할수 없다.
	int nTileSize = (int)fdiv ;

	int nVertexWidth = psFileHeader->lWidth/nTileSize+1 ;
	int nVertexHeight = psFileHeader->lHeight/nTileSize+1 ;

    fwrite(psFragmentInfo, sizeof(STerrain_FragmentInfo)*nVertexWidth*nVertexHeight, 1, pf) ;

    for(i=0 ; i<psFileHeader->nNumAlphaMap ; i++)
		fwrite(ppbyAlphaValue[i], sizeof(BYTE)*psFileHeader->nAlphaMapSize*psFileHeader->nAlphaMapSize, 1, pf) ;

	fwrite(&nNumTerrainLayer, 4, 1, pf) ;
	for(i=0 ; i<nNumTerrainLayer ; i++)
		fwrite(ppszTerrainLayer[i], 256, 1, pf) ;

	fwrite(&nNumNoiseMap, 4, 1, pf) ;
	for(i=0 ; i<nNumNoiseMap ; i++)
		fwrite(ppszNoiseMap[i], 256, 1, pf) ;
}

void STerrain_File::ReadFromFile(FILE *pf)
{
	int nSizeofRead=0 ;
	psFileHeader = new STerrain_FileHeader ;
	fread(psFileHeader, sizeof(STerrain_FileHeader), 1, pf) ;

	nSizeofRead += sizeof(STerrain_FileHeader) ;

    float fdiv = (float)psFileHeader->nNumerator/(float)psFileHeader->nDenominator ;//1m x 1m 이하로는 할수 없다.
	int nTileSize = (int)fdiv ;

	int nVertexWidth = psFileHeader->lWidth/nTileSize+1 ;
	int nVertexHeight = psFileHeader->lHeight/nTileSize+1 ;

    psFragmentInfo = new STerrain_FragmentInfo[nVertexWidth*nVertexHeight] ;
    fread(psFragmentInfo, sizeof(STerrain_FragmentInfo)*nVertexWidth*nVertexHeight, 1, pf) ;

	nSizeofRead += sizeof(STerrain_FragmentInfo)*nVertexWidth*nVertexHeight ;

	//32bit 알파맵2장을 사용. 그래서 저장을 8bit채널 하나씩 즉 (알파맵2장 * 4채널)

	int i ;
	ppbyAlphaValue = new BYTE*[psFileHeader->nNumAlphaMap] ;
	for(i=0 ; i<psFileHeader->nNumAlphaMap ; i++)
		ppbyAlphaValue[i] = new BYTE[psFileHeader->nAlphaMapSize*psFileHeader->nAlphaMapSize] ;

	for(i=0 ; i<psFileHeader->nNumAlphaMap ; i++)
	{
		fread(ppbyAlphaValue[i], sizeof(BYTE)*psFileHeader->nAlphaMapSize*psFileHeader->nAlphaMapSize, 1, pf) ;
		nSizeofRead += sizeof(BYTE)*psFileHeader->nAlphaMapSize*psFileHeader->nAlphaMapSize ;
	}

	fread(&nNumTerrainLayer, 4, 1, pf) ;
	if(nNumTerrainLayer > 0)
	{
		ppszTerrainLayer = new char*[nNumTerrainLayer] ;
		for(i=0 ; i<nNumTerrainLayer ; i++)
		{
			ppszTerrainLayer[i] = new char[256] ;
			fread(ppszTerrainLayer[i], 256, 1, pf) ;
		}
	}

	fread(&nNumNoiseMap, 4, 1, pf) ;
	if(nNumNoiseMap > 0)
	{
		ppszNoiseMap = new char*[nNumNoiseMap] ;
		for(i=0 ; i<nNumNoiseMap ; i++)
		{
			ppszNoiseMap[i] = new char[256] ;
			fread(ppszNoiseMap[i], 256, 1, pf) ;
		}
	}
}

//bool SaveTerrainFile(STerrain_FileHeader *psHeader, STerrain_MaterialInfo *psMaterialInfo, STerrain_FragmentInfo *psFragmentInfo, char *pszFileName)
bool SaveTerrainFile(STerrain_File *psTerrainFile, char *pszFileName)
{
	int i, n ;
	FILE *pf=NULL ;

	pf = fopen(pszFileName, "w+b") ;
	if(pf == NULL)
		return false ;

	TRACE("### Terrain file writting START ###\r\n") ;

	float fdiv = (float)psTerrainFile->psFileHeader->nNumerator/(float)psTerrainFile->psFileHeader->nDenominator ;
	int nWidth = (int)(psTerrainFile->psFileHeader->lWidth/fdiv)+1 ;
	int nHeight = (int)(psTerrainFile->psFileHeader->lHeight/fdiv)+1 ;

	fwrite(psTerrainFile->psFileHeader, sizeof(STerrain_FileHeader), 1, pf) ;
	TRACE("written %d bytes\r\n", sizeof(STerrain_FileHeader)) ;

	fwrite(psTerrainFile->psMaterialInfo, sizeof(STerrain_MaterialInfo)*psTerrainFile->psFileHeader->lNumMaterial, 1, pf) ;
	TRACE("written %d bytes\r\n", sizeof(STerrain_MaterialInfo)*psTerrainFile->psFileHeader->lNumMaterial) ;
	fwrite(psTerrainFile->psFragmentInfo, sizeof(STerrain_FragmentInfo)*nWidth*nHeight, 1, pf) ;
	TRACE("written %d bytes\r\n", sizeof(STerrain_FragmentInfo)*nWidth*nHeight) ;

	if(psTerrainFile->psFileHeader->nNumAlphaMap > 0)
	{
		int nSize = sizeof(BYTE)*psTerrainFile->psFileHeader->nAlphaMapSize*psTerrainFile->psFileHeader->nAlphaMapSize ;
		for(i=0 ; i<psTerrainFile->psFileHeader->nNumAlphaMap ; i++)
		{
			for(n=0 ; n<nSize ; n++)
				fwrite(&psTerrainFile->ppbyAlphaValue[i][n], 1, 1, pf) ;
		}
	}

	fclose(pf) ;
	TRACE("### Terrain file writting END ###\r\n") ;
	return true ;
}

//###############################################################//
// SD3DEffect
//###############################################################//

SD3DEffect::SD3DEffect()
{
	pEffect = NULL ;
	hTechnique = NULL ;
	hmatWVP = NULL ;
	hmatWIT = NULL ;
	hvDiffuse = NULL ;
	hvLightPos = NULL ;
	hvLightDir = NULL ;
	hvLightColor = NULL ;
	hvLookAt = NULL ;
	hmatSkin = NULL ;
	htexDecal = NULL ;
	htexNormal = NULL ;
	hvCameraPos = NULL ;
	hmatWLP = NULL ;
	hmatWLPT = NULL ;
	hanObjectID = NULL ;
	hmatTexCoordAni = NULL ;
	hAtm = NULL ;
	pDecl = NULL ;
	hfEpsilon = NULL ;

	vLightPos = D3DXVECTOR4(0, 0, 0, 1) ;
	vLightDir = D3DXVECTOR4(0, 0, 0, 0) ;
	vLightColor = D3DXVECTOR4(1, 1, 1, 1) ;
	vLookAt = D3DXVECTOR4(0, 0, 0, 0) ;
	vCameraPos = D3DXVECTOR4(0, 0, 0, 1) ;
	vDiffuse = D3DXVECTOR4(0.9f, 1, 1, 1) ;
	fElapsedTime = 0.0f ;

	D3DXMatrixIdentity(&matWorld) ;
	D3DXMatrixIdentity(&matView) ;
	D3DXMatrixIdentity(&matProj) ;
}
SD3DEffect::~SD3DEffect()
{
	if(pEffect)
	{
		delete pEffect ;
		pEffect = NULL ;
	}
}

HRESULT SD3DEffect::Initialze(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
{
	HRESULT hr ;
	LPD3DXBUFFER pError=NULL ;

#ifdef _SHADERDEBUG_
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, D3DXSHADER_DEBUG, NULL, &pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#else
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, 0, NULL, &pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#endif
	else
	{
		if((hTechnique = pEffect->GetTechniqueByName("TShader")) == NULL)
		{
			assert(false && "hTechnique is NULL (TShader)") ;
			return E_FAIL ;
		}

		SetD3DXHANDLE(&hvDiffuse, "vDiffuse") ;
		SetD3DXHANDLE(&hvLightPos, "vLightPos") ;
		SetD3DXHANDLE(&hvLightDir, "vLightDir") ;
		SetD3DXHANDLE(&hvLightColor, "vLightColor") ;
		SetD3DXHANDLE(&hvLookAt, "vLookAt") ;
		SetD3DXHANDLE(&hvCameraPos, "vCameraPos") ;
		SetD3DXHANDLE(&hanObjectID, "anObjectID") ;

		SetD3DXHANDLE(&hmatWVP, "matWVP") ;
		SetD3DXHANDLE(&hmatWIT, "matWIT") ;
		SetD3DXHANDLE(&hmatWLP, "matWLP") ;
		SetD3DXHANDLE(&hmatWLPT, "matWLPT") ;
		SetD3DXHANDLE(&hmatSkin, "amatSkin") ;
		SetD3DXHANDLE(&hmatTexCoordAni, "matTexCoordAni") ;
		SetD3DXHANDLE(&hAtm, "atm") ;
		SetD3DXHANDLE(&hfEpsilon, "epsilon") ;
		SetD3DXHANDLE(&hfDistFromCamera, "fDistFromCamera") ;
		SetD3DXHANDLE(&hnEigenIndex, "nEigenIndex") ;
		SetD3DXHANDLE(&hfAlphaTestDiffuse, "fAlphaTestDiffuse") ;
		SetD3DXHANDLE(&hfAlphaBlendingByForce, "fAlphaBlendingByForce") ;
		SetD3DXHANDLE(&hfElapsedTime, "fElapsedTime") ;

		SetD3DXHANDLE(&hfTimeScale, "fTimeScale") ;
		SetD3DXHANDLE(&hfFrequency, "fFrequency") ;
		SetD3DXHANDLE(&hfAmplitude, "fAmplitude") ;
	}
	SAFE_RELEASE(pError) ;

	return S_OK ;
}

void SD3DEffect::Release()
{
	SAFE_RELEASE(pEffect) ;
	SAFE_RELEASE(pDecl) ;
}

bool SD3DEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
{
	if((*pHandle = pEffect->GetParameterByName(NULL, pszName)) == NULL)
	{
		char err[128] ;
		sprintf(err, "%s Handle is NULL", pszName) ;
		assert(false && err) ;
		return false ;
	}
	return true ;
}

HRESULT SRenderTarget::Initialize(LPDIRECT3DDEVICE9 _pd3dDevice, UINT w, UINT h, D3DFORMAT D3DFMT_rgb, D3DFORMAT D3DFMT_depth, float _fZNear, float _fZFar)
{
	lWidth = w ;
	lHeight = h ;
	fZNear = _fZNear ;
	fZFar = _fZFar ;
	pd3dDevice = _pd3dDevice ;
	if(FAILED(pd3dDevice->CreateTexture(lWidth, lHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_rgb, D3DPOOL_DEFAULT, &pTex, NULL)))
		return E_FAIL ;
	if (FAILED(pTex->GetSurfaceLevel(0, &pSurf)))
		return E_FAIL;
	if(FAILED(pd3dDevice->CreateDepthStencilSurface(lWidth, lHeight, D3DFMT_depth, D3DMULTISAMPLE_NONE, 0, TRUE, &pSurfZBuffer, NULL)))
		return E_FAIL ;

	return S_OK ;
}

void SRenderTarget::Release()
{
	SAFE_RELEASE(pTex) ;
	SAFE_RELEASE(pSurf) ;
	SAFE_RELEASE(pSurfZBuffer) ;
}

bool CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDest, LPDIRECT3DDEVICE9 pd3dDevice)
{
    LPDIRECT3DSURFACE9 pSrcSurf=NULL ;
    D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTexSrc->GetSurfaceLevel(0, &pSrcSurf) ;
	pSrcSurf->GetDesc(&desc) ;

	//if(FAILED(D3DXCreateTexture(pd3dDevice, desc.Width, desc.Height, 0, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED, &pTexDest)))
	//{
	//	char szMsg[256] ;
	//	sprintf(szMsg, "Error CreateTexture") ;
	//	assert(false && szMsg) ;
	//	return false ;
	//}

    LPDIRECT3DSURFACE9 pDestSurf=NULL ;
	pTexDest->GetSurfaceLevel(0, &pDestSurf) ;

	D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSrcSurf, NULL, NULL, D3DX_FILTER_BOX, 0) ;

	pSrcSurf->Release() ;
	pDestSurf->Release() ;

	return true ;
}

void FillTextureWithColor(LPDIRECT3DTEXTURE9 pTex, DWORD dwColor, int nMapLevel)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(nMapLevel, &desc) ;

	DWORD *pdwColor ;
	D3DLOCKED_RECT d3drect ;
	pTex->LockRect(nMapLevel, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	pTex->AddDirtyRect(NULL) ;
	pdwColor = (DWORD *)d3drect.pBits ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pdwColor++)
		{
			*pdwColor = dwColor ;
		}
	}
	pTex->UnlockRect(nMapLevel) ;
}
void FillTexture32WithBuffer(LPDIRECT3DTEXTURE9 pTex, DWORD *pdwSrcColor, int nColorDepth, int nMapLevel)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(nMapLevel, &desc) ;

	DWORD *pdwColor ;
	D3DLOCKED_RECT d3drect ;
	pTex->LockRect(nMapLevel, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	pTex->AddDirtyRect(NULL) ;
	pdwColor = (DWORD *)d3drect.pBits ;
	int nDepthSize = nColorDepth/8 ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pdwColor++, pdwSrcColor++)
		{
			*pdwColor = *pdwSrcColor ;
		}
	}
	pTex->UnlockRect(nMapLevel) ;
}
DWORD GetColorfromBuffer(BYTE *pbyBuffer, int nDepthSize, D3DFORMAT fmt)
{
	DWORD value=0 ;

    if(nDepthSize == 1)//8bit color
	{
		BYTE byColor = *pbyBuffer ;

		if(fmt == D3DFMT_X8R8G8B8)
			value = 0x00000000|(byColor<<16)|(byColor<<8)|(byColor) ;
		else if(fmt == D3DFMT_A8R8G8B8)
			value = 0xff000000|(byColor<<16)|(byColor<<8)|(byColor) ;
	}
	else if(nDepthSize == 3)//24bit color
	{
		//TRACE("r=%x, g=%x b=%x\r\n", *pbyBuffer, *(pbyBuffer+1), *(pbyBuffer+2)) ;

			//       alpha              red                green                blue
			value = 0xff000000 | ((*(pbyBuffer+2))<<16) | ((*(pbyBuffer+1))<<8) | (*(pbyBuffer)) ;
	}
	else if(nDepthSize == 4)//32bit color
	{//DWORD를 BYTE로 바꿔서 0xffffffff 이 원본이 바이트형태로 되어버려서 가장끝부분부터 액세스가 되고 앞쪽으로 가게 된다. 뒤에서부터 읽어드린다고 생각하면 된다.
		value = (*(pbyBuffer+3))<<24 | (*(pbyBuffer+2))<<16 | (*(pbyBuffer+1))<<8 | (*(pbyBuffer)) ;
	}

	return value ;
}
void FillTextureWithBuffer(LPDIRECT3DTEXTURE9 pTex, BYTE *pbyColor, int nColorDepth, int nMapLevel)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(nMapLevel, &desc) ;

	DWORD *pdwColor ;
	D3DLOCKED_RECT d3drect ;
	pTex->LockRect(nMapLevel, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	pTex->AddDirtyRect(NULL) ;
	pdwColor = (DWORD *)d3drect.pBits ;
	int nDepthSize = nColorDepth/8 ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pbyColor += nDepthSize, pdwColor++)
		{
			*pdwColor = GetColorfromBuffer(pbyColor, nDepthSize, desc.Format) ;
		}
	}
	pTex->UnlockRect(nMapLevel) ;
}
void FillAlphaTexture(LPDIRECT3DTEXTURE9 pTex, BYTE *pbyColor)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pTex->GetLevelDesc(0, &desc) ;

	BYTE *pbyDest ;
	D3DLOCKED_RECT d3drect ;
	pTex->LockRect(0, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	pTex->AddDirtyRect(NULL) ;
	pbyDest = (BYTE *)d3drect.pBits ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pbyColor++, pbyDest++)
			(*pbyDest) = (*pbyColor) ;
	}
	pTex->UnlockRect(0) ;
}
void FillSurfaceWithColor(LPDIRECT3DSURFACE9 pSurf, DWORD dwColor)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	pSurf->GetDesc(&desc) ;

	D3DLOCKED_RECT d3drect ;
	pSurf->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	DWORD *pdw = (DWORD *)d3drect.pBits ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pdw++)
			*pdw = dwColor ;
	}
	pSurf->UnlockRect() ;
}

#include "LoadTGA.h"

bool CreateD3DTexture(LPDIRECT3DDEVICE9 pd3dDevice, LPDIRECT3DTEXTURE9 pTex, char *pszFileName, DWORD dwUsage)
{
	char *pszExt = strrchr(pszFileName, '.')+1 ;
	int width, height, bAlpha=false ;

	FILE *pf=NULL ;
	pf = fopen(pszFileName, "rb+") ;
	if(pf == NULL)
		return false ;

	if(!stricmp(pszExt, "bmp"))//비트맵파일 일경우
	{
		BITMAPFILEHEADER bmpfh ;
		BITMAPINFOHEADER bmpih ;

		fread(&bmpfh, sizeof(BITMAPFILEHEADER), 1, pf) ;
		assert(bmpfh.bfType == 0x4d42) ;
        
		fread(&bmpih, sizeof(BITMAPINFOHEADER), 1, pf) ;
		width = bmpih.biWidth ;
		height = bmpih.biHeight ;
	}
	else if(!stricmp(pszExt, "tga"))//타가파일 일경우
	{
		tTGAHeader_s tgah ;
		fread(&tgah, sizeof(tTGAHeader_s), 1, pf) ;
		width = tgah.d_width ;
		height = tgah.d_height ;
		if(tgah.d_pixel_size == 32)
			bAlpha=true ;
	}
	fclose(pf) ;
	pf=NULL ;

	D3DFORMAT fmt=D3DFMT_R8G8B8 ;

	if(bAlpha)
		fmt = D3DFMT_A8R8G8B8 ;

	if(FAILED(D3DXCreateTexture(pd3dDevice, width, height, dwUsage, 0, fmt, D3DPOOL_MANAGED, &pTex)))
		return false ;

	return true ;
}
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, D3DXVECTOR3 *pvStripPos)
{
	int i, nStrip=0 ;
	D3DXVECTOR3 N, B, T, prevT ;

	//first point
	N.x = N.z = 0 ;
	N.y = 1.0f ;

	B = (*(pvPos+1))-(*pvPos) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;


    pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
	pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;
	pvPos++ ;

	for(i=1 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		B = (*(pvPos+1))-(*(pvPos-1)) ;
		D3DXVec3Normalize(&B, &B) ;

		D3DXVec3Cross(&T, &B, &N) ;

		pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
		pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;
	}

	//end point
	B = (*pvPos)-(*(pvPos-1)) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;

    pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
	pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;

	return nStrip ;
}
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, SCVertex *psVertices, DWORD dwColor)
{
	int i, nStrip=0 ;
	D3DXVECTOR3 N, B, T, prevT ;

	//first point
	N.x = N.z = 0 ;
	N.y = 1.0f ;

	B = (*(pvPos+1))-(*pvPos) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;
	//N|  B/
	// |  /
	// | /
	// |/_______T

    //psVertices[nStrip++].pos = (*pvPos) + fWidth*T ;
	//psVertices[nStrip++].pos = (*pvPos) - fWidth*T ;

	psVertices[nStrip++].set((*pvPos) - fWidth*T, dwColor) ;
	psVertices[nStrip++].set((*pvPos) + fWidth*T, dwColor) ;
    pvPos++ ;

	for(i=1 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		B = (*(pvPos+1))-(*(pvPos-1)) ;
		D3DXVec3Normalize(&B, &B) ;

		D3DXVec3Cross(&T, &B, &N) ;

		//psVertices[nStrip++].pos = (*pvPos) + fWidth*T ;
		//psVertices[nStrip++].pos = (*pvPos) - fWidth*T ;
		psVertices[nStrip++].set((*pvPos) - fWidth*T, dwColor) ;
		psVertices[nStrip++].set((*pvPos) + fWidth*T, dwColor) ;
	}

	//end point
	B = (*pvPos)-(*(pvPos-1)) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;

    //psVertices[nStrip++].pos = (*pvPos) + fWidth*T ;
	//psVertices[nStrip++].pos = (*pvPos) - fWidth*T ;
	psVertices[nStrip++].set((*pvPos) - fWidth*T, dwColor) ;
	psVertices[nStrip++].set((*pvPos) + fWidth*T, dwColor) ;

	return nStrip ;
}
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, D3DXMATRIX *pmatView, D3DXVECTOR3 *pvStripPos)
{
	int i, nStrip=0 ;
	D3DXVECTOR3 N, B, T, prevT, vCamera(pmatView->_41, pmatView->_42, pmatView->_43), vUp(pmatView->_12, pmatView->_22, pmatView->_32) ;

	//first point
	N = vCamera-(*pvPos) ;
	D3DXVec3Normalize(&N, &N) ;

	B = (*(pvPos+1))-(*pvPos) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;
	if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
        T = vUp ;

    pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
	pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;

	for(i=1 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		N = vCamera-(*pvPos) ;
		D3DXVec3Normalize(&N, &N) ;

		B = (*(pvPos+1))-(*(pvPos-1)) ;
		D3DXVec3Normalize(&B, &B) ;

		D3DXVec3Cross(&T, &B, &N) ;
		if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
			T = vUp ;

		pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
		pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;
	}

	//end point
	N = vCamera-(*pvPos) ;
	D3DXVec3Normalize(&N, &N) ;

	B = (*pvPos)-(*(pvPos-1)) ;
	D3DXVec3Normalize(&B, &B) ;

	D3DXVec3Cross(&T, &B, &N) ;
	if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
        T = vUp ;

    pvStripPos[nStrip++] = (*pvPos) + fWidth*T ;
	pvStripPos[nStrip++] = (*pvPos) - fWidth*T ;

	return nStrip ;
}
void BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SCVertex *psVertices, SCIndex *psIndices, float fWidth, DWORD color)
{
	int i, nVertexCount=0, nIndexCount=0 ;
	float ftemp ;
	D3DXVECTOR3 N, B, T, v ;
	D3DXVECTOR4 v4 ;

	for(i=0 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		B = (*(pvPos+1))-(*pvPos) ;
		ftemp = D3DXVec3Length(&B)/2.0f ;
		D3DXVec3Normalize(&B, &B) ;

		v = (*pvPos) + ftemp*B ;
		N = (*pvCamera)-v ;
		D3DXVec3Normalize(&N, &N) ;

		D3DXVec3Cross(&T, &B, &N) ;
		D3DXVec3Normalize(&T, &T) ;
		if(float_eq(T.x, 0.0f) && float_eq(T.y, 0.0f) && float_eq(T.z, 0.0f))// opposite direction between camera dir and tangent dir
			T = *pvUp ;

		//D3DXVec3Transform(&v4, &v, pmatVP) ;
		//fWidth = v4.w*m_fLineWidth/1000.0f ;//  v4.w/m_fLineWidth ;

		psVertices->pos = (*pvPos) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*pvPos) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psIndices->wIndex[0] = nVertexCount-4 ;
		psIndices->wIndex[1] = nVertexCount-3 ;
		psIndices->wIndex[2] = nVertexCount-1 ;
		psIndices++ ;
		nIndexCount++ ;

        psIndices->wIndex[0] = nVertexCount-1 ;
		psIndices->wIndex[1] = nVertexCount-2 ;
		psIndices->wIndex[2] = nVertexCount-4 ;
		psIndices++ ;
		nIndexCount++ ;
	}
}
int BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, SCVertex *psVertices, SCIndex *psIndices, float fWidth, DWORD color)
{
	int i, nVertexCount=0, nIndexCount=0 ;
	float ftemp ;
	D3DXVECTOR3 N(0, 1, 0), B, T, v ;
	D3DXVECTOR4 v4 ;

	for(i=0 ; i<(nNumPos-1) ; i++, pvPos++)
	{
		B = (*(pvPos+1))-(*pvPos) ;
		ftemp = D3DXVec3Length(&B)/2.0f ;
		D3DXVec3Normalize(&B, &B) ;

		v = (*pvPos) + ftemp*B ;
		
		D3DXVec3Cross(&T, &B, &N) ;
		D3DXVec3Normalize(&T, &T) ;

		psVertices->pos = (*pvPos) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*pvPos) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) - fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psVertices->pos = (*(pvPos+1)) + fWidth*T ;
		psVertices->color = color ;
		psVertices++ ;
		nVertexCount++ ;

		psIndices->wIndex[0] = nVertexCount-4 ;
		psIndices->wIndex[1] = nVertexCount-3 ;
		psIndices->wIndex[2] = nVertexCount-1 ;
		psIndices++ ;
		nIndexCount++ ;

        psIndices->wIndex[0] = nVertexCount-1 ;
		psIndices->wIndex[1] = nVertexCount-2 ;
		psIndices->wIndex[2] = nVertexCount-4 ;
		psIndices++ ;
		nIndexCount++ ;
	}
	return nVertexCount ;
}

bool SetRenderTarget(UINT width, UINT height, D3DFORMAT D3DFMT_rgb, LPDIRECT3DTEXTURE9 *ppTex, LPDIRECT3DSURFACE9 *ppSurf, LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(FAILED(pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_rgb, D3DPOOL_DEFAULT, ppTex, NULL)))
		return false ;
	if (FAILED((*ppTex)->GetSurfaceLevel(0, ppSurf)))
		return false ;
	return true ;
}