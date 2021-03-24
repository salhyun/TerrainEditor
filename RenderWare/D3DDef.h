#pragma once

#if defined(DEBUG) | defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>

#include <stdio.h>

//#include <iostream>
//#include <vector>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) {if(p){p->Release(); p=NULL;}}
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) {if(p){delete p; p=NULL;}}
#endif

#ifndef SAFE_DELETEARRAY
#define SAFE_DELETEARRAY(p) {if(p){delete []p; p=NULL;}}
#endif

#ifndef EPSILON
#define EPSILON 0.0001f
#endif

#ifndef FLOAT_EQ
#define FLOAT_EQ(x, v) ( (((v)-EPSILON) < (x)) && ((x) < ((v)+EPSILON)) )
#endif

#ifndef VECTOR_EQ
#define VECTOR_EQ(v1, v2) ( FLOAT_EQ(v1.x, v2.x) && FLOAT_EQ(v1.y, v2.y) && FLOAT_EQ(v1.z, v2.z) )
#endif

#define ZEROVECTOR4(v) (v.x)=(v.y)=(v.z)=(v.w)=0.0f
#define ZEROVECTOR3(v) (v.x)=(v.y)=(v.z)=0.0f
#define ZEROVECTOR2(v) (v.x)=(v.y)=0.0f

#define COPYVECTOR4(vDest, vSrc) (vDest.x)=(vSrc.x); (vDest.y)=(vSrc.y); (vDest.z)=(vSrc.z); (vDest.w)=(vSrc.w)
#define COPYVECTOR3(vDest, vSrc) (vDest.x)=(vSrc.x); (vDest.y)=(vSrc.y); (vDest.z)=(vSrc.z)
#define COPYVECTOR2(vDest, vSrc) (vDest.x)=(vSrc.x); (vDest.y)=(vSrc.y)

typedef D3DXMATRIX dxMatrix4 ;
typedef D3DXVECTOR3 dxVector3 ;

#ifndef DDSCAPS2
struct DDSCAPS2 {
	DWORD dwCaps1;
	DWORD dwCaps2;
	DWORD Reserved[2];
} ;
#endif

#ifndef DDPIXELFORMAT
struct DDPIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask, dwGBitMask, dwBBitMask;
	DWORD dwRGBAlphaBitMask;
} ;
#endif

#ifndef DDSURFACEDESC2
struct DDSURFACEDESC2 {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	DWORD dwPitchOrLinearSize;
	DWORD dwDepth;
	DWORD dwMipMapCount;
	DWORD dwReserved1[11];
	DDPIXELFORMAT ddpfPixelFormat;
	DDSCAPS2 ddsCaps;
	DWORD dwReserved2;
} ;
#endif

struct SCVertex
{
	D3DXVECTOR3 pos ;
	unsigned long color ;

    void set(D3DXVECTOR3 &p, DWORD dwColor) ;
	//SCVertex() {pos.x=pos.y=pos.z=0; color=0;}
	//SCVertex(float x, float y, float z, DWORD dwColor) {pos.x=x; pos.y=y; pos.z=z; color=dwColor;}
	//SCVertex(D3DXVECTOR3 _pos, DWORD dwColor) {pos=_pos; color=dwColor;}
} ;
struct SCIndex
{
	unsigned short wIndex[3] ;

	SCIndex() {wIndex[0]=wIndex[1]=wIndex[2]=0;}
	SCIndex(unsigned short i0, unsigned short i1, unsigned short i2) {wIndex[0]=i0; wIndex[1]=i1; wIndex[2]=i2;}
} ;

struct STWVertex
{
	D3DXVECTOR4 pos ;
	D3DXVECTOR2 tex ;
} ;


struct SD3DRay// Half Ray
{
	D3DXVECTOR3 vStart ;
	D3DXVECTOR3 vDir ;

    SD3DRay()
	{
		vStart.x = vStart.y = vStart.z = 0.0f ;
		vDir.x = vDir.y = vDir.z = 0.0f ;
	}
    SD3DRay(D3DXVECTOR3 &_vStart, D3DXVECTOR3 &_vDir)
	{
		vStart = _vStart ;
		vDir = _vDir ;
	}
	D3DXVECTOR3 GetPosition(float t)
	{
		return (vStart + vDir*t) ;
	}

	bool IsOnRay(D3DXVECTOR3 &vPos)
	{
		D3DXVECTOR3 vNormal1, vNormal2 ;

		D3DXVec3Normalize(&vNormal1, &(vPos-vStart)) ;
		D3DXVec3Normalize(&vNormal2, &vDir) ;

		float dot = D3DXVec3Dot(&vNormal1, &vNormal2) ;

		if(!FLOAT_EQ(dot, 1.0f))
			return false ;

//		if(!FLOAT_EQ(D3DXVec3Dot(&(vPos-vStart), &vDir), 1.0f))
//			return false ;

		float t = D3DXVec3Length(&(vPos-vStart))/D3DXVec3Length(&vDir) ;

		if(t >= 0.0f && t <= 1.0f)
			return true ;

		return false ;
	}
} ;

//가장 상위클래스가 존재하고
//*.fx파일에 따라서 그것에 해당되는 구조체를 생성하도록 할것임.
//각 씬마다 준비해둔 SD3DEffect 구조체들을 초기화.
//오브젝트들은 SD3DEffect와 연관되어 있음. 그러므로 오브젝트와 SD3DEffect구조체와 링크시킬 무언가가 필요함.

struct SD3DEffect
{
	LPDIRECT3DVERTEXDECLARATION9 pDecl ;//정점선언

	D3DXVECTOR4 vLightPos, vLightDir, vLightColor, vLookAt, vCameraPos, vDiffuse ;

	D3DXMATRIX matWorld, matView, matProj ;
	float fElapsedTime ;

    LPD3DXEFFECT pEffect ;//셰이더
	D3DXHANDLE hTechnique ;//테크닉
	D3DXHANDLE hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE hmatWIT ;//월-로칼 변환을 위한 행렬
	D3DXHANDLE hvDiffuse ;
	D3DXHANDLE hvLightPos ;//광원 위치
	D3DXHANDLE hvLightDir ;//광원방향
	D3DXHANDLE hvLightColor ;//광원색상
	D3DXHANDLE htexDecal ;//디컬 텍스쳐
	D3DXHANDLE htexNormal ;//노말 텍스쳐
	D3DXHANDLE hvLookAt ;//시선방향
	D3DXHANDLE hvCameraPos ;//카메라위치
	D3DXHANDLE hmatWLP ;//광원에서 본 {월드-뷰-투영} 행렬
	D3DXHANDLE hmatWLPT ;//광원에서 본 {월드-뷰-투영-텍스쳐} 행렬
	D3DXHANDLE hanObjectID ;//그림자를 위한 ID
	D3DXHANDLE hmatTexCoordAni ;//Texture Coordi Animation
	D3DXHANDLE hAtm ;
	D3DXHANDLE hfEpsilon ;
	D3DXHANDLE hfDistFromCamera ;
	D3DXHANDLE hnEigenIndex ;
	D3DXHANDLE hfAlphaTestDiffuse ;
	D3DXHANDLE hfAlphaBlendingByForce ;
	D3DXHANDLE hfElapsedTime ;

	D3DXHANDLE hfTimeScale ;
	D3DXHANDLE hfFrequency ;
	D3DXHANDLE hfAmplitude ;

	//스키닝을 위한 빠레뜨행렬
	D3DXHANDLE hmatSkin ;

    SD3DEffect() ;	
	~SD3DEffect() ;

	HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	void Release() ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
} ;

D3DXMATRIX GetMatrixLookatLH(D3DXVECTOR3 vLookat, D3DXVECTOR3 vPos, D3DXVECTOR3 vUp) ;
HRESULT RenderVirtualCamera(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj, LPDIRECT3DTEXTURE9 pTex) ;

struct SRenderTarget
{
	UINT lWidth, lHeight ;
	float fZNear, fZFar ;
	LPDIRECT3DTEXTURE9 pTex ;
    LPDIRECT3DSURFACE9 pSurf ;
	LPDIRECT3DSURFACE9 pSurfZBuffer ;
    LPDIRECT3DDEVICE9 pd3dDevice ;

	SRenderTarget()
	{
		lWidth = lHeight = 0 ;
		pd3dDevice = NULL ;
		pTex = NULL ;
		pSurf = NULL ;
		pSurfZBuffer = NULL ;
	}
	~SRenderTarget()
	{
		Release() ;
	}
	HRESULT Initialize(LPDIRECT3DDEVICE9 _pd3dDevice, UINT w, UINT h, D3DFORMAT D3DFMT_rgb, D3DFORMAT D3DFMT_depth, float _fZNear, float _fZFar) ;
	void Release() ;
} ;

struct SBoneWeight
{
	int nIndex ;
	float fWeight ;
} ;

//########################################
// Surface Information
//########################################
struct SContactSurfaceInfo
{
	enum KIND {OB=0, ROUGH, HAZARD, TEE, FAIRWAY, BUNKER, GREEN, GREENSIDE, ROUGHSIDE, HOLECUP, TREE, STONE, STEEL} ;

	int nKind ;//표면의 종류
	float fRestitution ;//물체와 충돌시 저항
	float fFriction ;//물체가 미끄러질때 마찰

	float fVelRange, fVelBase ;
	float fMaxIncidenceBase ;
	float fScaleRestitution ;
	float fRollingCoef ;
	bool bGroundHazard ;

	SContactSurfaceInfo()
	{
		nKind = 0 ;
		fRestitution = 0.9f ;
		fFriction = 0.2f ;

		fVelRange = 0.5f ;
		fVelBase = 12.0f ;
		fMaxIncidenceBase = 1.0f ;
		fScaleRestitution = 1.0f ;
		fRollingCoef = 100.0f ;
		bGroundHazard = false ;
	}
	void set(int kind, float restitution, float friction, float scale_restitution)
	{
		nKind = kind ;
		fRestitution = restitution ;
		fFriction = friction ;
		fScaleRestitution = scale_restitution ;
	}
} ;

//########################################
//Terrain Infomation
//########################################
struct STerrain_FragmentInfo
{
	float fElevation ;
	float u, v ;
	int nMaterialID ;
} ;

struct STerrain_MaterialInfo
{
	float afAmbient[3] ;
	float afDiffuse[3] ;
	float afSpecular[3] ;
	char szFileName[256] ;
} ;

struct STerrain_FileHeader
{
	DWORD dwType ;
    DWORD lWidth, lHeight ;
	int nNumerator, nDenominator ;// nNumerator/nDenominator (분자/분모)로 표현한다.
    DWORD lNumMaterial ;
	int nNumAlphaMap ;
	int nAlphaMapSize ;
} ;

struct STerrain_File
{
	STerrain_FileHeader *psFileHeader ;
    STerrain_MaterialInfo *psMaterialInfo ;
	STerrain_FragmentInfo *psFragmentInfo ;
    BYTE **ppbyAlphaValue ;

	int nNumTerrainLayer ;
	char **ppszTerrainLayer ;

	int nNumNoiseMap ;
	char **ppszNoiseMap ;

	STerrain_File() ;
	~STerrain_File() ;

	void Initialize(int width, int height, int nNumerator, int nDenominator, int numAlphamap, int Alphamapsize=512, int numMaterial=0, int numTerrainlayer=9, int numNoisemap=2) ;
	void WriteToFile(FILE *pf) ;
	void ReadFromFile(FILE *pf) ;
	void Release() ;
} ;

bool SaveTerrainFile(STerrain_File *psTerrainFile, char *pszFileName) ;
bool LoadTerrainFile(STerrain_File *psTerrainFile, char *pszFileName) ;

//bool SaveTerrainFile(STerrain_FileHeader *psHeader, STerrain_MaterialInfo *psMaterialInfo, STerrain_FragmentInfo *psFragmentInfo, char *pszFileName) ;
//bool LoadTerrainFile(STerrain_FileHeader *psHeader, STerrain_MaterialInfo *psMaterialInfo, STerrain_FragmentInfo *psFragmentInfo, char *pszFileName) ;

//########################################

bool CreateD3DTexture(LPDIRECT3DDEVICE9 pd3dDevice, LPDIRECT3DTEXTURE9 pTex, char *pszFileName, DWORD dwUsage=0) ;

bool CopyTexture(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DTEXTURE9 pTexDest, LPDIRECT3DDEVICE9 pd3dDevice) ;//밉맵이 없는경우 사용.
void FillTextureWithColor(LPDIRECT3DTEXTURE9 pTex, DWORD dwColor, int nMapLevel=0) ;
void FillTextureWithColorArray(LPDIRECT3DTEXTURE9 pTex, DWORD *pdwSrcColor, int nMapLevel=0) ;
void FillTextureWithBuffer(LPDIRECT3DTEXTURE9 pTex, BYTE *pbyColor, int nColorDepth, int nMapLevel=0) ;
void FillAlphaTexture(LPDIRECT3DTEXTURE9 pTex, BYTE *pbyColor) ;
void FillTexture32WithBuffer(LPDIRECT3DTEXTURE9 pTex, DWORD *pdwSrcColor, int nColorDepth, int nMapLevel=0) ;
void FillSurfaceWithColor(LPDIRECT3DSURFACE9 pSurf, DWORD dwColor) ;

void BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SCVertex *psVertices, SCIndex *psIndices, float fWidth, DWORD color) ;
int BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, SCVertex *psVertices, SCIndex *psIndices, float fWidth, DWORD color) ;
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, D3DXMATRIX *pmatView, D3DXVECTOR3 *pvStripPos) ;//Camera Dependent
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, D3DXVECTOR3 *pvStripPos) ;
int BuildLineStrip(D3DXVECTOR3 *pvPos, int nNumPos, float fWidth, SCVertex *psVertices, DWORD dwColor) ;

bool SetRenderTarget(UINT width, UINT height, D3DFORMAT D3DFMT_rgb, LPDIRECT3DTEXTURE9 *ppTex, LPDIRECT3DSURFACE9 *ppSurf, LPDIRECT3DDEVICE9 pd3dDevice) ;