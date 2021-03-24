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

//�ϳ��� �޽����ٰ� �������� ���긶Ÿ���˷� �����Ǿ� �ִ�.
//������ �����ϱ� ���� �޽�����Ÿ
struct STerrainMesh
{
	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 *ppTexDecal ;

    TERRAINVERTEX *psVertices ;//�������� ���ؽ�����Ÿ
	TERRAININDEX **ppsIndices ;//�������� �ε�������Ÿ
	std::vector<TERRAININDEX *> *pIndexSet ;//�������ϱ����� Ʈ����忡�� ����Ÿ�� ����
	data::SArray<TERRAININDEX> *pIndices ;

	STrnPolygon *psPolygons ;//Ʈ�����鿡�� �ε�������Ÿ�� ��������

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

	//undo redo �� �ҷ��� �̰��� �������� �ʿ��ϴ�.
	TERRAINVERTEX *pVertices ;
	TERRAININDEX *pIndices, *psOriginIndices ;
	geo::STriangle *psTriangles ;

	//�������� ���ܵǴ� �ﰢ��
	data::SKeepingArray<TERRAININDEX> sExceptionalIndices ;

	STerrain_MaterialInfo *psMaterialInfo ;
	STerrain_FragmentInfo *psFragmentInfo ;

	CActionBrushingContainer *pcActionContainer ;
	CActionManagement *pcActionManagement ;

	unsigned long lNumVertices, lPrimitiveCount ;
	unsigned long lNumTriangles, lNumIndex ;
	unsigned long lWidth, lHeight ;//������ ���μ��� ���� ex) 65�� �ƴ϶� 64
	int nVertexWidth, nVertexHeight ;
    int nBrushDecision ;
	int nAlphaMapSize ;
	int nTileSize ;
	int nNumAlphaMap ;
	int nNumDrawVLine ;
	bool *pbDrawVLine ;
	float fBrushStrength ;
	bool bOverapBrush ;
	float fMaxElevation, fMinElevation ;//���� ������ ������� ���� ����
	
	SContactSurfaceInfo asContactSurfaceInfo[9] ;

	float restitutionOB, frictionOB ;

	float afRestitution[9], afFriction[9] ;

	//undo redo �� �ҷ��� �̰��� �������� �ʿ��ϴ�.
	float **ppfHeightMap ;//������ġ�� ���� ���� ��

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
	HRESULT InitHeightMap(char *pszHeightMap, char *pszTexture, LPDIRECT3DDEVICE9 pd3dDevice, CActionManagement *pcActionManagement) ;//���̸��� ������� QuadTree�� �����Ѵ�.
	HRESULT InitOctTree(CASEData *pcASEData, char *pszPath, LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex) ;//����ũ�⿡ ���߾ �������ҵ� OctTree����.

	bool SaveAlphaMapstoTerrainFile(char *pszFileName) ;

	D3DXVECTOR3 D3DXGetPosition(unsigned long index) ;//�ε����� ��ġ��
	Vector3 GetPosition(unsigned long index) ;//�ε����� ��ġ��
	HRESULT ProcessFrustumCulling(const Matrix4 &matWorld) ;
	HRESULT ProcessFrustumCulling(const D3DXMATRIX &matWorld) ;

	void ProcessFrustumCullingInOctTree() ;//��Ʈ���� ����ü�ø��ؼ� �������� �ﰢ��(������)�� �����Ѵ�.
	void ProcessAllInOctTree() ;//Ʈ������ ��� �ﰢ��(������)�� �����Ѵ�.
	void ProcessCullingInOctTree() ;//by Occlusion Culling �̿ϼ�

	void ProcessFrustumCullInQuadTree() ;
	void ProcessFrustumCullInQuadTree(CSecretFrustum *pcFrustum) ;//�ܺ��������һ��

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

	void RenderOctTreeCube() ;//Ʈ�������� ǥ���Ҽ� �ִ� ť�긦 �������Ѵ�.
	
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

class CActionBrushing : public CAction //�����ؽ��� ��������� Action���� ó��
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
	virtual void Restore() ;//Undo ������� ����Ǵ°���.

	void Release() ;
} ;