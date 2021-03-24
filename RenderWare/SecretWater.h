#pragma once

#include "SecretMesh.h"
#include "ASEData.h"
#include "DataStructLib.h"

class CSecretMirror ;
class CSecretTerrain ;
class CSecretPicking ;
class CSecretTextureContainer ;
class CSecretD3DTerrainEffect ;
class CSecretSkyEffect ;
class CSecretSky ;
class CSecretCloudEffect ;
class CSecretCloud ;
class CSecretRenderingGroup ;
struct SD3DEffect ;
class CSecretWaterEffect ;
class CSecretGizmoSystem ;
struct SWaterEffectParameter ;
class CSecretWaterManager ;
class CSecretSkyDome ;
class CSecretSkyDomeEffect ;
struct STrueRenderingObject ;

#define D3DFVF_WATERVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEXTUREFORMAT3|D3DFVF_TEX2)
struct SWaterVertex
{
    D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
	D3DXVECTOR3 tangent ;
	D3DXVECTOR2 t ;
} ;

struct SWaterIndex
{
	unsigned short anIndex[3] ;

	SWaterIndex()
	{
		anIndex[0] = anIndex[1] = anIndex[2] = 0 ;
	}

	SWaterIndex(unsigned short *pnIndex)
	{
		anIndex[0] = pnIndex[0] ;
		anIndex[1] = pnIndex[1] ;
		anIndex[2] = pnIndex[2] ;
	}
} ;

struct SWaterKind
{
	char szKindName[64] ;
	int nPass ;

    void set(char *pstr, int pass) ;
} ;

struct SWaterTile
{
    Vector3 vPos ;
	float fEdgeLength ;
} ;

struct SWaterSurface//수면이기때문에 x-y평면이 된다.
{
	bool bEnable ;
	int nWidth, nHeight ;
    int nDelta ;
	Vector3 vCenterPos ;

	SMeshVertex *psVertices ;
	SMeshTriangle *psTriangles ;
	UINT lNumVertex, lNumTriangle ;

	Vector3 vMin, vMax ;

    SWaterSurface() ;
    ~SWaterSurface() ;
	void Initialize(int width, int height, int delta, Vector3 pos) ;
	void BuildVertexData() ;
	void BuildVertexData(SWaterVertex *psVertices, SWaterIndex *psIndices) ;
	void Release() ;
} ;

#define MAXNUM_TEXSURFACE 2
#define MAXNUM_REFLECTEDOBJECT 512

class CSecretTextureContainer ;

class CSecretWater
{
public :
	enum STATUS { PICKING=1, SELECTED, UNSELECTED } ;

//variable
private :
	bool m_bEnable ;
	bool m_bWireFrame ;
	int m_nStatus ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	SWaterVertex *m_psVertices ;
	SWaterIndex *m_psIndices ;

	UINT m_lNumVertex ;//정점의 갯수
	UINT m_lNumTriangle ;//삼각형(폴리곤)의 갯수

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;//렌더링에 사용되는 정점버퍼
	LPDIRECT3DINDEXBUFFER9 m_pIB ;//렌더링에 사용되는 인덱스버퍼

    CSecretMirror *m_pcMirror ;
	CSecretTerrain *m_pcTerrain ;
	CSecretPicking *m_pcPicking ;
	CSecretGizmoSystem *m_pcGizmo ;
	CSecretWaterEffect *m_pcWaterEffect ;

	Vector2 m_avBumpMove[4], m_avBumpMoveScalar[4] ;

	D3DXMATRIX m_matTransform, m_matOriginScale, m_matOriginRot, m_matOriginTrans ;

	int m_nNumReflectedTRObject ;
	STrueRenderingObject *m_apsReflectedTRObjects[MAXNUM_REFLECTEDOBJECT] ;

public :
	SWaterSurface m_sWaterSurface ;
	char m_szName[128] ;
	LPDIRECT3DTEXTURE9 m_pTexReflect, m_pTexRefraction, m_pTexRipple1Normal, m_pTexDetail ;
	LPDIRECT3DVOLUMETEXTURE9 m_pVolTexRipple1Normal ;
	float m_fTimeSinceLastFrame ;
    D3DXVECTOR4 m_vTilingBumpTex ;
	SWaterEffectParameter *m_psParam ;
	SWaterKind *m_psWaterKind ;
	CSecretWaterManager *m_pcWaterManager ;
	int m_nWidth, m_nHeight ;

	D3DXVECTOR4 m_vCamera ;
	D3DXMATRIX m_matLODView ;

	bool m_bRenderingNow, m_bItHasNaverRendered ;

	float m_fSphereRadius ;
	Vector3 m_vBoundingPos[4] ;
	geo::SSphere m_sCollisionSphere ;
	geo::SPlane m_sCollisionPlane ;
	geo::STriangle m_asCollisionTriangles[2] ;


//function
private :
    
public :
	CSecretWater() ;
	~CSecretWater() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszName, int nWidth, int nHeight, CSecretTerrain *pcTerrain, CSecretPicking *pcPicking, CSecretGizmoSystem *pcGizmo, CSecretWaterEffect *pcWaterEffect, SWaterKind *psWaterKind) ;
	void Process(bool bMouseLButtonEvent, bool bMouseLButtonDirect) ;
	void ProcessBump() ;
	void Render() ;//'psd3dEffect' is loaded 'Water.fx'
	void Release() ;

	//void SetTextures(UINT lNumTexDecale, char **ppszTexDecale, UINT lNumTexNormal, char **ppszTexNormal) ;
	void SetWaterSurface(SWaterSurface *psWaterSurface) {memcpy(&m_sWaterSurface, psWaterSurface, sizeof(SWaterSurface)) ; } ;
	SWaterSurface *GetWaterSurface() { return &m_sWaterSurface; } ;
	CSecretMirror *GetMirror() {return m_pcMirror;}
	void SetStatus(int nStatus) ;
	int GetStatus() ;
	D3DXMATRIX *GetTransform() ;
	void SetTransform(D3DXMATRIX *pmatTransform) ;

	void ResetReflectedTRObject() ;
	void SelectReflectedTRObject(STrueRenderingObject *psTRObject) ;
	int GetNumReflectedTRObject() ;
	STrueRenderingObject **GetReflectedTRObject() ;
	bool IsInFrustum() ;
	
	bool Intersect(geo::SLine *psLine, Vector3 &vIntersect, bool bLineTest) ;
	bool IsUnderWater(D3DXVECTOR3 *p) ;
	bool isInWaterRect(Vector3 p) ;
	void getWaterPlane(geo::SPlane *psPlane) ;
} ;

struct SWaterEffectParameter
{
	float fElapsedTime ;
	float fScreenPosOffset ;
	float fSunStrength ;
	float fSunShininess ;

	SWaterEffectParameter() ;
} ;

class CSecretWaterManager
{
public :
	enum WATERKIND { RIVER=0, LAKE } ;
	enum STATUS { IDLING=0, PROCESSING } ;
	const int MAXNUM_WATER ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	data::SKeepingArray<CSecretWater> m_cWater ;
	CSecretTextureContainer *m_pcTexContainer ;
	CSecretWater *m_pcCurrentWater ;
	CSecretWaterEffect *m_pcWaterEffect ;
	CSecretGizmoSystem *m_pcGizmo ;

	int m_nStatus ;
	int m_nNumWaterKind ;
	SWaterKind *m_psWaterKind ;

public :
    CSecretTerrain *m_pcTerrain ;
	CSecretPicking *m_pcPicking ;
	SD3DEffect *m_psEffect ;
	CSecretD3DTerrainEffect *m_pcTerrainEffect ;
	CSecretSkyEffect *m_pcSkyEffect ;
	CSecretSky *m_pcSky ;
	CSecretCloudEffect *m_pcCloudEffect ;
	CSecretCloud *m_pcCloud ;
	CSecretRenderingGroup *m_pcRenderingGroup ;

	CSecretSkyDomeEffect *m_pcSkyDomeEffect ;
	CSecretSkyDome *m_pcSkyTop, *m_pcSkyLower ;

	float m_fTimeSinceLastFrame ;

	//shader parameter
	SWaterEffectParameter m_sParam ;

	LPDIRECT3DVOLUMETEXTURE9 m_pVolTexRipple1Normal ;

private :
	void _InitKind() ;


public :
	CSecretWaterManager() ;
	~CSecretWaterManager() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretWaterEffect *pcWaterEffect) ;
	void Process(bool bMouseLButtonEvent, bool bMouseLButtonDirect) ;
	void Render() ;
	void Release() ;

	void CreateWater(char *pszName, int nWidth, int nHeight, char *pszWaterKind) ;
	CSecretWater *SelectWater(int nIndex) ;
	void DeleteWater(int nIndex) ;
	CSecretWater *GetCurrentWater() {return m_pcCurrentWater;}
	void ImportWater(char *pszName, D3DXMATRIX matTransform, int nWidth, int nHeight, char *pszWaterKind) ;

	void SetTerrain(CSecretTerrain *pcTerrain) {m_pcTerrain=pcTerrain;}
	int GetNumWaterKind() {return m_nNumWaterKind;}
	SWaterKind *GetWaterKind() {return m_psWaterKind;}
	int FindWaterKind(char *psz) ;
	data::SKeepingArray<CSecretWater> *GetWater() {return &m_cWater;}
	CSecretWater *GetWater(int nIndex) ;
	int GetNumWater() ;
	CSecretGizmoSystem *GetGizmo() {return m_pcGizmo;}
	void ResetWater() ;
	void SetStatus(int nStatus) ;
	int GetStatus() ;

	void ResetReflectedTRObject() ;
	void SelectReflectedTRObject(STrueRenderingObject *psTRObject) ;
	int Intersect(geo::SLine *psLine, Vector3 &vIntersect, bool bLineTest=false) ;
	int UnderWater(Vector3 &vPos) ;
	int isInWaterRect(Vector3 &vPos) ;
} ;

#include "SecretD3DEffect.h"

class CSecretWaterEffect : public CSecretD3DEffect
{
public :
	D3DXHANDLE m_hmatWorld ;
	D3DXHANDLE m_havBumpMove ;
	D3DXHANDLE m_hvTilingBumpTex ;
	D3DXHANDLE m_hfElapsedTime ;
	D3DXHANDLE m_hfScreenPosOffset ;
	D3DXHANDLE m_hfProjOffset ;
	D3DXHANDLE m_hfSunStrength ;
	D3DXHANDLE m_hfSunShininess ;
	D3DXHANDLE m_hmatLODWVP ;

	float m_fProjOffset ;

	CSecretWaterEffect() ;
	virtual ~CSecretWaterEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
} ;