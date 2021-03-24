#pragma once

#include "D3Ddef.h"
#include "def.h"
#include "Vector3.h"
#include "Matrix.h"
#include "SecretFrustum.h"
#include "SecretCoordSystem.h"
#include <vector>

struct SMirror_Vertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR3 normal ;
	D3DXVECTOR2 tex ;
} ;

struct SMirror_Index
{
	WORD index[3] ;
} ;

//반사매핑에 사용하는 평면의 데이타
struct SMirrorPlane
{
	Vector3 vNormal ;
	Matrix4 matTransform ;
	SMirror_Vertex sVertex[4] ;
	SMirror_Index sIndex[2] ;
	float fWidth, fHeight ;

	void BuildMirror(Matrix4 &_matTransform, Vector3 &_vNormal, float width, float height) ;
	void BuildBehindMirror(Matrix4 &_matTransform, Vector3 &_vNormal, float width, float height) ;
	Vector3 GetPos() ;
} ;
//반사표면을 구성하는 데이타
struct SMirrorSurface
{
	UINT lNumVertex, lNumIndex ;
	SMirror_Vertex *psVertex ;
	SMirror_Index *psIndex ;
} ;

class CSecretTerrain ;
class CSecretMeshObject ;
class CSecretD3DTerrainEffect ;
class CSecretTerrain ;
class CSecretSkyEffect ;
class CSecretSky ;
class CSecretCloudEffect ;
class CSecretCloud ;
class CSecretRenderingGroup ;
class CSecretSkyDomeEffect ;
class CSecretSkyDome ;
//평면을 이루는 데이타에다가 주변환경을 반사시켜주는 클래스. 거울에 사용
class CSecretMirror
{
public :
	enum {LEFTTOP=0, RIGHTTOP=1, RIGHTBOTTOM=2, LEFTBOTTOM=3} ;
	enum {TEXTURESIZE=1024} ;
	enum RENDERMETHOD {RENDER_MIRROR=0, RENDER_BEHINDMIRROR} ;
private :
    bool m_bDrawFrustum, m_bDrawCoordsys, m_bRender ;
	int m_nRenderMethod ;
	bool m_bDepthWater ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	SMirrorPlane m_sMirrorPlane ;
	SMirrorSurface m_sMirrorSurface ;
	D3DXMATRIX m_matReflectView, m_matReflectProj, m_matReflect, m_matMirrorTransform, m_matBehindTransform ;
	LPDIRECT3DTEXTURE9 m_pTexReflect ;
	LPDIRECT3DSURFACE9 m_pSurfReflect ;
	LPDIRECT3DSURFACE9 m_pSurfZBuffer ;
	LPDIRECT3DTEXTURE9 m_pTexBehind ;
	LPDIRECT3DSURFACE9 m_pSurfBehind ;
	CSecretFrustum m_cFrustum ;
	CSecretCoordSystem m_cCoordsys ;
    SMirror_Vertex m_sVertex[4] ;
	SMirror_Index m_sIndex[2] ;
	float m_fRadius ;

public :
	float m_fClipingMoveZ ;

private :
	void _SetMirrorPlane(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation) ;
	void _BuildReflectProjection(Matrix4 matTransform, const Vector3 &vMirrorPos, const Matrix4 &matReflectView) ;
	void _BuildFrustum() ;
	void _ProcessMirror(Vector3 &vCameraPos) ;
	void _ProcessBehindMirror(Vector3 &vCameraPos) ;

public :
	CSecretMirror() ;
	CSecretMirror(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation, LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawFrustum=false, bool bDrawCoordsys=false) ;
	~CSecretMirror() ;
	HRESULT Initialize(Vector3 vMirrorPos, Vector3 vNormal, float fWidth, float fHeight, Matrix4 &matRotation, LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawFrustum=false, bool bDrawCoordsys=false) ;
	
	void SetMirrorPlaneMatrix(Matrix4 &matRotation) ;
	void Process(Vector3 &vCameraPos) ;
	
	void Render() ;
	void Release() ;

	HRESULT RenderVirtualCamera(D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj, LPDIRECT3DTEXTURE9 pTex, SD3DEffect *psEffect, CSecretTerrain *pcTerrain) ;
    HRESULT RenderVirtualCamera(
		D3DXMATRIX *pmatView,
		D3DXMATRIX *pmatProj,
		LPDIRECT3DTEXTURE9 pTex,
		SD3DEffect *psEffect,
		CSecretD3DTerrainEffect *pcTerrainEffect,
		CSecretTerrain *pcTerrain,
		//CSecretSkyEffect *pcSkyEffect,
		//CSecretSky *pcSky,
		//CSecretCloudEffect *pcCloudEffect,
		//CSecretCloud *pcCloud,
		CSecretSkyDomeEffect *pcSkyDomeEffect,
		CSecretSkyDome *pcSkyTop,
		CSecretSkyDome *pcSkyLower,
		CSecretRenderingGroup *pcRenderingGroup,
		STrueRenderingObject ***pppsTRObjects=NULL,
		int nNumTRObject=0) ;
	

	void SetMirrorSurface(UINT lNumVertex, SMirror_Vertex *psVertex, UINT lNumIndex, SMirror_Index *psIndex) ;
	void CalculateTexCoordOnMirrorSurface(Matrix4 &matTransform) ;

	D3DXMATRIX *GetmatReflectView() { return &m_matReflectView;} ;
	D3DXMATRIX *GetmatReflectProj() { return &m_matReflectProj;} ;
	D3DXMATRIX *GetmatMirrorTransform() { return &m_matMirrorTransform;} ;
	D3DXMATRIX *GetmatBehindTransform() { return &m_matBehindTransform;} ;
	Matrix4 *GetmatTransform() ;
	LPDIRECT3DTEXTURE9 GetTexReflect() { return m_pTexReflect;} ;
	LPDIRECT3DTEXTURE9 GetTexBehind() { return m_pTexBehind;} ;
	Vector3 ProjectionNormal(Vector3 &vLookat, Vector3 &vNormal) ;
    Vector3 GetPos(UINT lPos) ;
	void SetRenderMethod(int nMethod, bool bDepthWater=false) ;
	int GetRenderMethod() {return m_nRenderMethod;}
	Vector3 GetCenterPos() ;
	void SetMirrorPos(Vector3 vPos) ;
	void SetMirrorSize(int nWidth, int nHeight) ;
	float GetRadius() {return m_fRadius;}
	CSecretFrustum* getFrustum() {
		return &m_cFrustum;
	}

} ;
