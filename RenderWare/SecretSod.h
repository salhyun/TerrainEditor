#pragma once

#include "D3DDef.h"
#include "GeoLib.h"
#include "SecretD3DEffect.h"
#include "SecretBoundingBox.h"
#include "SecretPicking.h"
#include "DataStructLib.h"

class CSecretSodEffect ;
class CSecretTerrain ;
class CSecretTerrainScatteringData ;
class CSecretTextureContainer ;
class cLightScatteringData ;
class CSecretTextureContainer ;
class CSecretShadowMap ;

#define D3DFVF_SODVERTEX (D3DFVF_XYZ|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE2(1)|D3DFVF_TEXCOORDSIZE4(2))

struct SSod_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 tex ;
	D3DXVECTOR2 factor ;
	D3DXVECTOR4 reserved ;
} ;
struct SSod_Index
{
	//unsigned short anIndex[3] ;
	DWORD anIndex[3] ;

	SSod_Index() { anIndex[0]=anIndex[1]=anIndex[2]=0;}
	//SSod_Index(unsigned short *pnIndex) {anIndex[0]=pnIndex[0]; anIndex[1]=pnIndex[1]; anIndex[2]=pnIndex[2];}
	SSod_Index(DWORD *pnIndex) {anIndex[0]=pnIndex[0]; anIndex[1]=pnIndex[1]; anIndex[2]=pnIndex[2];}
} ;

struct SClump//잔디 한포기
{
	int nKind ;
    Vector3 vPos ;
	float fDistfromCamera ;
	Vector3 vCrumple ;

	void set(int kind, Vector3 v) {nKind=kind; vPos=v; fDistfromCamera=0.0f;}
} ;

struct SClumpKind
{
	float fWidth, fHeight ;
	Vector2 avTex[4] ;
	char szName[128] ;

	void set(Vector2 vStartPos, float tex_width, float tex_height, float xfactor, float yfactor, char *pszName) ;
} ;
struct SSodNoiseInfo
{
	int nKind ;
	int nResolution ;
	int nNumOctaves ;
    float fPersistence ;
	char szName[256] ;
} ;
struct SSodMark
{
    bool bEnable ;

	SSodMark() {bEnable=false;}
	void set(bool enable) {bEnable=enable;}

	//unsigned char nKind ;
	//SSodMark() {bEnable=false; nKind=0;}
	//void set(bool enable, unsigned char kind) {bEnable=enable; nKind=kind;}
} ;
struct SClumpMark
{
	bool bEnable ;
	unsigned long lWidth, lHeight, lTotalSize ;
    DWORD *pdwMark ;

	SClumpMark() ;
	~SClumpMark() ;
	bool initialize(unsigned long width, unsigned long height) ;
	void SetMark(unsigned long x, unsigned long z, bool enable) ;
	bool GetMark(unsigned long x, unsigned long z) ;
	void operator =(SClumpMark sMark) ;
	void Release() ;
} ;

#define MAXNUM_CLUMPKIND 16

class CSecretSod
{
public :
	enum TEXCOORDORDER { LT=0, RT, LB, RB } ;
	enum BRUSHKIND {IDLE=0, PLAINTING, REMOVE} ;

	const int MAXNUMVERTEX, MAXNUMINDEX ;
	const int MAXNUMCLUMP ;

private :
    LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;
	SSod_Vertex *m_psVertices ;
	SSod_Index *m_psIndices ;
	int m_nVertexCount, m_nIndexCount ;
	int m_nNumVertex, m_nNumIndex ;

	int m_nNumClump ;
	SClump *m_psClumps ;

	bool m_bWireFrame ;
	Vector3 m_vCenter ;
	float m_fRadius ;

	LPDIRECT3DTEXTURE9 m_pTexClump ;
	LPDIRECT3DTEXTURE9 m_pTexNoise ;
	CSecretTerrain *m_pcTerrain ;

	CSecretTextureContainer *m_pcTexContainer ;
	CSecretBoundingBox m_cBBox ;

	void _AddClump(SClump *psClump, Vector3 *pvCameraPos) ;
	void _DistributionClumpPos(Vector2 *pvPos) ;
	bool _CreateNoiseTexture(LPDIRECT3DTEXTURE9 *ppTex) ;
	float _GetWeightfromSodMark(int x1, int x2, int z1, int z2) ;

public :
	int m_nNumClumpKind ;
	SClumpKind *m_apsClumpKind[MAXNUM_CLUMPKIND] ;

	float m_fXInterval, m_fZInterval ;
	//int m_nBoardWidth, m_nBoardHeight ;
	//SSodMark **m_ppsSodMark ;
	SClumpMark m_sClumpMark ;

	CSecretSodEffect *m_pcSodEffect ;
	CSecretBrushTool *m_pcBrushTool ;
	char m_szName[256] ;

public :
	CSecretSod(int nNumTile=262144) ;
	~CSecretSod() ;
	void Release() ;

    bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTerrain *pcTerrain, CSecretTextureContainer *pcTexContainer, CSecretBrushTool *pcBrushTool, CSecretSodEffect *pcSodEffect) ;
	void GenerateClumps(Vector3 *pvCameraPos) ;
	void Render(CSecretShadowMap *pcShadowMap) ;

	void Process(int nPlanting) ;

	void copyVertices() ;
	bool SaveNoiseDataToFile(char *pszFileName, float fPersistence, int nNumOctaves, int nResolution) ;
	void SetSodBoard(float fXInterval, float fZInterval) ;
	void RefreshClumps() ;
	int GetNumClumpKind() {return m_nNumClumpKind;}
	SClumpKind *GetClumpKind(int n) ;
	SClump *getClumps() ;
	int getNumClump() ;
	//SSodMark **GetSodMark() {return m_ppsSodMark;}
	//int GetSodMarkWidth() {return m_nBoardWidth;}
	//int GetSodMarkHeight() {return m_nBoardHeight;}

} ;

class CSecretSodEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉
	D3DXHANDLE m_hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE m_hvAxisX, m_hvAxisY ;
	D3DXHANDLE m_hvCameraPos ;//카메라위치
	D3DXHANDLE m_hmatWLP ;//월드-광원-투영 행렬
	D3DXHANDLE m_hmatWLPT ;//월드-광원-투영-텍스쳐바이어스 행렬

	SEffectEssentialElements *m_psEssentialElements ;

	CSecretSodEffect() ;
	~CSecretSodEffect() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
	void Release() ;
} ;

////////////////////////////////////////////////////////////////////////////
//CSecretSodManager
////////////////////////////////////////////////////////////////////////////

#include "SecretProto.h"

class CSecretSodManager : public CSecretProto
{
public :
	const int MAXNUM_SOD ;

	enum ATTR { ATTR_USINGTHREAD=0x01 } ;
	enum GENERATECLUMP_STATUS { CGSTA_WAITING=0, GCSTA_GENERATING=1, GCSTA_GENERATED=2, GCSTA_TERMINATE=3 } ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	CSecretTextureContainer *m_pcTexContainer ;
	CSecretSodEffect *m_pcSodEffect ;

	int m_nNumClumpKind ;
	SClumpKind *m_psClumpKind ;

	data::SKeepingArray<CSecretSod> m_cSods ;
	CSecretSod *m_pcCurrentSod ;

	void _InitClumpKind() ;
	SClumpKind *_FindClumpKind(char *pszName) ;

public :
	CSecretBrushTool *m_pcBrushTool ;
	CSecretShadowMap *m_pcShadowMap ;

	Vector3 m_vCameraPos ;
	int m_nGenerateClumpThreadStatus ;
	HANDLE m_hGenerateClumpThread ;
	DWORD m_dwGenerateClumpThreadID ;

public :
	CSecretSodManager() ;
	~CSecretSodManager() ;

    bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, CSecretPicking *pcPicking, int nAttr=0) ;
	void Process(int nPlanting, Matrix4 *pmatView, D3DXMATRIX *pmatProj) ;
	void Render() ;
	void Release() ;

	void CreateSod(char *pszName, float fXInterval, float fZInterval, data::SCString *psString, int nNum) ;
	void SelectSod(int nNum) ;
	void DeleteSod(int nNum) ;
	void SetClumpKind(CSecretSod *pcSod, data::SCString *psString, int nNum) ;

	CSecretSod *GetCurrentSod() {return m_pcCurrentSod;}
	CSecretSod *getSod(char *pszName) ;
	int GetNumClumpKind() {return m_nNumClumpKind;}
	SClumpKind *GetClumpKind() {return m_psClumpKind;}
	data::SKeepingArray<CSecretSod> *GetSods() {return &m_cSods;}
	int GetIndexofClumpKind(SClumpKind *psClumpKind) ;
	void SetTerrain(CSecretTerrain *pcTerrain) ;
	void SetShadowMap(CSecretShadowMap *pcShadowMap) ;
	void ResetSods() ;
	void GenerateClumps() ;
} ;