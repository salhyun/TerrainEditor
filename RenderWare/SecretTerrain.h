#pragma once

#include "D3Ddef.h"

#include "SecretFrustum.h"
#include "ZQuadTree.h"
#include "SecretQuadTree.h"
#include "SecretOctTree.h"
#include "SecretD3DTerrainEffect.h"
#include "Action.h"
#include "LightScatteringData.h"
#include "SecretTextureContainer.h"

char *pszTerrainLayerOder[] ;

class CD3DEnvironment ;
class CASEData ;
struct STrnPolygon ;
class CActionBrushingContainer ;
class CActionBrushing ;
class CSecretShadowMap ;
class CSecretMirror ;
class CSecretTerrain ;

struct SHolecup ;
struct SContactSurfaceInfo ;

#define MAXSIZE_ALPHAMAP 2048

#define MAXNUM_TERRAINTEXTURES 8
#define MAXNUM_TERRAINALPHA 3
#define MAXNUM_TERRAINROAD 6
#define MAXNUM_TERRAINNOISETEX 2

#define MAXNUM_EXCEPTIONALINDEX 12

#define COLLISION_IN_HOLECUP 0x10000000

#define D3DFVF_TERRAINVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)
#define D3DFVF_TERRAINVERTEX1 (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1))

struct TERRAINVERTEX
{
	D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		t;
} ;
struct TERRAININDEX
{
	DWORD i1, i2, i3 ;

	bool IsEqual(TERRAININDEX index) ;
} ;

struct STerrainVertex1
{
	D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		texDecal ;
	D3DXVECTOR2		texNoise ;
} ;
struct STerrainIndex1
{
	WORD i1, i2, i3 ;
} ;
struct STerrainTilePos
{
	int x, z ;

	STerrainTilePos() {x=z=0;}
	STerrainTilePos(int _x, int _z) {x=_x; z=_z;}
	void set(int _x, int _z) {x=_x; z=_z;}
} ;

struct SCollisionHolecup
{
	int nNumTriangle, nNumCylinder, nNumSphere ;
	geo::STriangle *psTriangles ;
	geo::SCylinder *psCylinders ;
	geo::SSphere *psSpheres ;

	Matrix4 *pmatTransform ;

	SCollisionHolecup() ;
	~SCollisionHolecup() ;
	bool Import(char *pszFileName) ;
	void Release() ;
} ;

class CSecretTerrainScatteringData : public cLightScatteringData
{
public :
	CSecretTerrainScatteringData() {} ;
	~CSecretTerrainScatteringData() {} ;

	void CalculateShaderParameterTerrainOptimized() ;
} ;

//하나의 메쉬에다가 여러장의 서브마타리알로 구성되어 있다.
//지형을 구성하기 위한 메쉬데이타
struct STerrainMesh
{
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 *ppTexDecal ;

    TERRAINVERTEX *psVertices ;//오리지날 버텍스데이타
	TERRAININDEX **ppsIndices ;//오리지날 인덱스데이타
	std::vector<TERRAININDEX *> *pIndexSet ;//렌더링하기위해 트리노드에서 데이타를 받음
	data::SArray<TERRAININDEX> *pIndices ;

	STrnPolygon *psPolygons ;//트리노드들에게 인덱스데이타를 전달해줌

	UINT lNumVertex, *plNumIndex ;
	UINT lNumTexture ;

	STerrainMesh() ;
	~STerrainMesh() ;

	void Release() ;
} ;

//HeightMap struct
struct TERRAINMAP
{
	bool bEnable ;
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;

	LPDIRECT3DTEXTURE9 pTexBase ;
	LPDIRECT3DTEXTURE9 apTexLayer[MAXNUM_TERRAINTEXTURES] ;
	LPDIRECT3DTEXTURE9 apTexAlpha[MAXNUM_TERRAINTEXTURES] ;
	LPDIRECT3DTEXTURE9 pTexLight ;
	LPDIRECT3DTEXTURE9 pTexResult ;
    
	LPDIRECT3DTEXTURE9 pTexNoise00, pTexNoise01 ;

	LPDIRECT3DTEXTURE9 apSaveKeepingAlpha[MAXNUM_TERRAINTEXTURES] ;

	data::SCString asStrLayerName[MAXNUM_TERRAINTEXTURES+1] ;
	data::SCString asStrNoiseTex[MAXNUM_TERRAINNOISETEX] ;

	//undo redo 를 할려면 이것이 여러개가 필요하다.
	TERRAINVERTEX *pVertices ;
	TERRAININDEX *pIndices, *psOriginIndices ;
	geo::STriangle *psTriangles ;

	//지형에서 제외되는 삼각형
	data::SKeepingArray<TERRAININDEX> sExceptionalIndices ;

	STerrain_MaterialInfo *psMaterialInfo ;
	STerrain_FragmentInfo *psFragmentInfo ;

	CActionBrushingContainer *pcActionContainer ;
	CActionManagement *pcActionManagement ;

	unsigned long lNumVertices, lPrimitiveCount ;
	unsigned long lNumTriangles, lNumIndex ;
	unsigned long lWidth, lHeight ;//지형의 가로세로 길이 ex) 65가 아니라 64
	int nVertexWidth, nVertexHeight ;
    int nBrushDecision ;
	int nAlphaMapSize ;
	int nTileSize ;
	int nNumAlphaMap ;
	int nNumDrawVLine ;
	bool *pbDrawVLine ;
	float fBrushStrength ;
	bool bOverapBrush ;
	float fMaxElevation, fMinElevation ;//현재 지형의 가장높은 낮은 지점
	
	SContactSurfaceInfo asContactSurfaceInfo[9] ;

	float restitutionOB, frictionOB ;

	float afRestitution[9], afFriction[9] ;

	//undo redo 를 할려면 이것이 여러개가 필요하다.
	float **ppfHeightMap ;//지형위치에 따른 높이 값

	int nIndex ;

	SHolecup *psHolecup ;
    
	TERRAINMAP() ;
    ~TERRAINMAP() ;

	void TextureBlending(int nTexNum, Vector2 &vTex1, Vector2 &vTex2, int nRadius) ;
	void DrawCircleBrushing(int nTexNum, int cx, int cy, int R, BYTE byColor) ;
	void BrushingAlpha(int nNum, int nBrushCount, Vector2 vStart, Vector2 vEnd, float fRadius) ;
	void EndBrushing(int nNumTex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	void GetVertexPos(float x, float z, int &x1, int &x2, int &z1, int &z2, bool bConvertPos=true) ;
	float GetHeightMap(float x, float z) ;
	float GetHeightMap(int x, int z) ;
	Vector3 GetNormal(int x, int z) ;
	Vector3 GetNormal(float x, float z) ;
	void SetHeightMap(int x, int z, float h) ;
	void ReadAlphaTexture(int nNum, DWORD *pdwBuffer) ;

	int GetNumTriangle(Vector3 vPos, float width, float height) ;
	int GetTriangles(Vector3 vPos, float width, float height, geo::STriangle *psTriangles, bool bHolecup=false, int nMaxTriangle=512) ;
	int GetNumTriangle(Vector3 vPos, float fRadius) ;
	int GetTriangles(Vector3 vPos, float fRadius, geo::STriangle *psTriangles, bool bMakeBoundingShpere=false, bool bHolecup=false, int nMaxTriangle=512) ;
	bool GetHeightMapAndNormal(Vector3 pos, Vector3 *pvIntersect, Vector3 *pvNormal) ;
	int GetHolecupTriangles(Vector3 vCenter, geo::STriangle *psTriangle) ;

	void getTerrainSurfaceProperty(int kind, SContactSurfaceInfo *psContactSurfaceInfo) ;
	int GetSurfaceInfo(float x, float z, SContactSurfaceInfo *psContactSurfaceInfo) ;
	void GetSurfaceInfofromHolecup(int nIndex, SContactSurfaceInfo *psContactSurfaceInfo) ;

	void ExportContactSurfaceInfo(char *pszFileName) ;
	void ImportContactSurfaceInfo(char *pszFileName) ;

	int GetIndex(float x, float z, data::SKeepingArray<TERRAININDEX> *psIndices) ;
	void Release() ;
} ;
struct STerrainRoad
{
	bool bEnable ;
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 pTex ;

	int nNumVertex, nNumIndex ;

	STerrainRoad() ;
	~STerrainRoad() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, TERRAINVERTEX *pSrcVertices, int nVertexCount, STerrainIndex1 *pSrcIndices, int nIndexCount, LPDIRECT3DTEXTURE9 pTexture) ;
	void Release() ;
} ;

struct STerrainDecal
{
	bool bEnable ;
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 apTex[2] ;

	int nNumVertex, nNumIndex ;

	STerrainDecal() ;
	~STerrainDecal() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, STerrainVertex1 *pSrcVertices, int nVertexCount, STerrainIndex1 *pSrcIndices, int nIndexCount) ;
	void Release() ;
} ;

struct SHolecup
{
	bool bEnable ;
	int nExceptionalIndex ;
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 pTex ;

    STerrainDecal sDecal ;

	int nNumVertex, nNumIndex ;
	D3DXMATRIX matTransform ;

	int nLodlevel ;

	SCollisionHolecup sCollisionHolecup ;
	float fDepthBias, fSlopeScaleDepthBias ;

	SHolecup() ;
	~SHolecup() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName, D3DXMATRIX *pmatTransform, LPDIRECT3DTEXTURE9 ptex, geo::STriangle *psTriangle, int exceptional_index, CSecretTerrain *pcTerrain) ;
	void _InitDecal(LPDIRECT3DDEVICE9 pd3dDevice, geo::STriangle *psTriangle, CSecretTerrain *pcTerrain) ;
	LPDIRECT3DTEXTURE9 GetTextureLOD(Vector3 vCamera) ;
	void SetLod(Vector3 vCamera) ;
	Vector3 GetHolecupPos() ;

	void Release() ;
} ;

struct STerrainLoadMethod
{
	enum TERRAINLOAD_METHOD {FROMFILE=1, FROMMEMORY} ;

	int nMethod ;
    char szFileName[256] ;
	STerrain_File *psTerrainFile ;

	STerrainLoadMethod() ;
	~STerrainLoadMethod() ;

    void SetFileMethod(char *pstr) ;
	void SetMemoryMethod(STerrain_File *pterrainfile) ;
	int GetMethod() {return nMethod;}
} ;

class CSecretTerrain
{
public :
	enum SHADERPASS { PASS_SUNLIT=0, PASS_SHADOW=1, PASS_ROADSUNLIT=2, PASS_DEPTHWATER=3, PASS_HOLECUP=4, PASS_ALTITUDE=5 } ;
	enum ATTR {  ATTR_RENDERSHADOW=0x01, ATTR_DEPTHWATER=0x02 } ;

private :
	UINT m_lEigenIndex ;

public :
	bool m_bEnable ;
	int m_nAttr ;
	CSecretFrustum *m_pcFrustum ;
	TERRAINMAP m_sHeightMap ;
	CD3DEnvironment *m_pcd3dEnvironment ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	//ZQuadTree *m_pcZQuadTree ;
	Matrix4 m_matWorld ;
	bool m_bWireFrame ;

	STerrainMesh m_sTerrainMesh ;
	CSecretOctTree *m_pcOctTree ;
	CSecretQuadTree *m_pcQuadTree ;

	CSecretTerrainScatteringData *m_pcAtm ;
	CSecretTextureContainer *m_pcTexContainer ;

	float m_afTilingTex[MAXNUM_TERRAINTEXTURES+1] ;
    
	CSecretShadowMap *m_pcShadowMap ;
	int m_nRoadCount ;
	STerrainRoad m_acRoad[MAXNUM_TERRAINROAD] ;
	SHolecup m_sHolecup ;

	char m_szFileName[256] ;

	Vector3 m_vDiffuse, m_vSpecular ;
    Vector3 m_vWaterColorTone ;
	float m_fMipmapLodBias ;

	CSecretTerrain() ;
	~CSecretTerrain() ;

	bool IsEnable() {return m_bEnable;} ;

	float GetHeightfromTerrainFile(STerrain_FragmentInfo *psFragmentInfo, float x, float z, int nWidth, int nHeight) ;
	Vector3 ComputeNormalfromTerrainFile(STerrain_FragmentInfo *psFragmentInfo, int nCount, int nWidth, int nHeight) ;
	bool LoadTerrainFile(char *pszFileName, int nEigenIndex) ;
	bool LoadTerrainFromMemory(STerrain_File *psTerrainFile, int nEigenIndex) ;
	void ExportTerrainFile(STerrain_File *psTerrainFile) ;
	HRESULT InitHeightMap(char *pszHeightMap, char *pszTexture, LPDIRECT3DDEVICE9 pd3dDevice, CActionManagement *pcActionManagement) ;//높이맵을 기반으로 QuadTree를 생성한다.
	HRESULT InitOctTree(CASEData *pcASEData, char *pszPath, LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex) ;//지형크기에 맞추어서 공간분할된 OctTree생성.

	bool SaveAlphaMapstoTerrainFile(char *pszFileName) ;

	D3DXVECTOR3 D3DXGetPosition(unsigned long index) ;//인덱스의 위치값
	Vector3 GetPosition(unsigned long index) ;//인덱스의 위치값
	HRESULT ProcessFrustumCulling(const Matrix4 &matWorld) ;
	HRESULT ProcessFrustumCulling(const D3DXMATRIX &matWorld) ;

	void ProcessFrustumCullingInOctTree() ;//옥트리를 절두체컬링해서 렌더링할 삼각형(폴리곤)을 선별한다.
	void ProcessAllInOctTree() ;//트리내에 모든 삼각형(폴리곤)을 선별한다.
	void ProcessCullingInOctTree() ;//by Occlusion Culling 미완성

	void ProcessFrustumCullInQuadTree() ;
	void ProcessFrustumCullInQuadTree(CSecretFrustum *pcFrustum) ;//외부프러스텀사용

	void SetTerrainMatrix(Matrix4 &matWorld) ;
	HRESULT RenderQuadTree(SD3DEffect *psd3dEffect) ;//use shader
	HRESULT RenderQuadTree(CSecretD3DTerrainEffect *pcTerrainEffect, D3DXMATRIX *pmatWorld=NULL, CSecretMirror *pcMirror=NULL) ;//use shader

	void RenderHolecup(CSecretD3DTerrainEffect *pcTerrainEffect) ;
	HRESULT RenderAltitude(CSecretD3DTerrainEffect *pcTerrainEffect) ;

	HRESULT RenderOctTree(SD3DEffect *psd3dEffect) ;//use shader
	HRESULT RenderOctTree(CSecretD3DTerrainEffect *pcTerrainEffect) ;//use shader

	HRESULT RenderOctTree() ;//use fixed pipe-line
	HRESULT RenderAllOctTree(SD3DEffect *psd3dEffect) ;//use shader
	HRESULT RenderAllOctTree() ;//use fixed pipe-line

	void RenderOctTreeCube() ;//트리공간을 표시할수 있는 큐브를 렌더링한다.
	
	void SetWireFrame(bool bEnable) ;

	void BrushingTerrain() ;

	void Release() ;

	void AddAttr(int nAttr, bool bEnable=true) ;
	int GetAttr() ;

	bool ReCalculateAllTerrainNormal() ;
	int GetEmptyRoad() ;

	void CreateHolecup(Vector3 vPos) ;
} ;

class CActionBrushingContainer
{
public :
	CActionBrushing *m_apContainer[MAXNUM_ACTION] ;

    CActionBrushingContainer() ;
	~CActionBrushingContainer() ;

    void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nWidth, int nHeight) ;
	CActionBrushing *GetEmptyContainer() ;
    void Release() ;
} ;

class CActionBrushing : public CAction //알파텍스쳐 변경사항을 Action으로 처리
{
public :
    LPDIRECT3DTEXTURE9 m_pTex ;
	int m_nNumTex ;
	TERRAINMAP *m_psHeightMap ;

public :
	CActionBrushing() ;
	//CActionBrushing(LPDIRECT3DDEVICE9 pd3dDevice, int nNumTex, LPDIRECT3DTEXTURE9 pTex, TERRAINMAP *psHeightMap) ;
	virtual ~CActionBrushing() ;

	//void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nNumTex, LPDIRECT3DTEXTURE9 pTex, TERRAINMAP *psHeightMap) ;
	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nWidth, int nHeight) ;

	void Save(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual void Disable() ;
	virtual void Restore() ;//Undo 했을경우 실행되는것임.

	void Release() ;
} ;