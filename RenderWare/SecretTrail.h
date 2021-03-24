#pragma once

#include "D3DDef.h"
#include "Vector3.h"
#include "Vector4.h"
#include "DataStructLib.h"

struct SEffectEssentialElements ;
class CSecretTrailEffect ;
class CSecretTextureContainer ;

#define D3DFVF_TRAILVERTEX (D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0))

struct STrailPosition : public Vector4
{
	float CoefFadeout ;
	Vector3 vNormal ;

	STrailPosition() ;
	STrailPosition(float _x, float _y, float _z, float _w) ;
	~STrailPosition() ;
} ;

struct STrailVertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR3 tex ;
} ;

class CSecretTrail
{
public :
	const int MAXNUM_POS ;
	enum KIND {FLYINGTRAIL=0, ROLLINGTRAIL} ;
	enum STATUS {IDLING=0, TRAILING, DISAPPEAR} ;
	enum VIEW_STATUS {FREEVIEW=0, AFTERSHOT} ;
	enum ATTR { ATTR_LIMITEDLENGTH=0x01, ATTR_FADEOUT=0x02 } ;

	bool m_bWireFrame ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nKind, m_nAttr ;
	int m_nStatus ;
	int m_nViewStatus ;//카메라보기상태
	int m_nNumVertex ;
	STrailVertex *m_psVertices ;
	float m_fHalfWidth ;
	CSecretTrailEffect *m_pcTrailEffect ;
	
	LPDIRECT3DTEXTURE9 m_pTexAirTrail, m_pTexGroundTrail ;

	void _BuildQuadStrip(int nKind, Vector3 &vCamera, Vector3 &vUp, float fElapsedTime) ;
	void _BuildCrossTrail(int nKind, Vector3 &vCamera, Vector3 &vUp, float fElapsedTime) ;
	void _InitVariable() ;

public :
	float m_fTrailLength, m_fMaxLength ;
	float m_fFadeoutTime ;
	float m_fAlphaValue ;
	float m_fCropLength, m_fCropingRate, m_fCropingTime ;
	DWORD m_dwZEnable ;

	data::SKeepingArray<STrailPosition> m_Positions ;

public :
	CSecretTrail() ;
	~CSecretTrail() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTrailEffect *pcTrailEffect, float fHalfWidth=0.02f, float fMaxLength=7.0f, DWORD dwZEnable=D3DZB_TRUE) ;
	void Process(Vector3 &vCamera, float fElapsedTime) ;
	void Render() ;
	void AddPos(Vector3 &vPos, Vector3 *pvNormal=NULL) ;
	void Reset() ;
	void SetHalfWidth(float fHalfWidth) ;

	void SetTextures(LPDIRECT3DTEXTURE9 pTexAirTrail, LPDIRECT3DTEXTURE9 pTexGroundTrail) ;

	void SetKind(int nKind) ;
	int GetKind() ;
	void SetStatus(int nStatus) ;
	bool IsStatus(int nStatus) ;
	void SetViewStatus(int nStatus) ;
	bool IsViewStatus(int nStatus) ;
	void AddAttr(int nAttr, bool bEnable=true) ;
	bool IsAttr(int nAttr) ;
	void Release() ;
	void SetWireFrame(bool bEnable) ;
	void SetMaxLength(float fLength) ;
} ;

class CSecretTrailEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉
	D3DXHANDLE m_hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE m_hvCameraPos ;//카메라위치

	SEffectEssentialElements *m_psEssentialElements ;

	D3DXMATRIX m_matWVP ;

	CSecretTrailEffect() ;
	~CSecretTrailEffect() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
	void Release() ;
} ;

class CSecretTrailManager
{
public :
	const int MAXNUM_TRAIL ;


private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	CSecretTrailEffect m_cTrailEffect ;
	CSecretTextureContainer *m_pcTexContainer ;

	CSecretTrail *m_pcTrail ;

public :
	bool m_bWireFrame ;

public :
	CSecretTrailManager() ;
	~CSecretTrailManager() ;

	void Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements) ;
	void Process(Vector3 &vCamera, float fElapsedTime, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj) ;
	void Render() ;
	CSecretTrail *GetEmptyTrail() ;

	void SetTexContainer(CSecretTextureContainer *pcTexContainer) ;
	void SetWireFrame(bool bEnable) ;
	void ResetAllTrails() ;

	CSecretTrail *GetTrail(int nNum) ;
	
	void Release() ;
} ;