#pragma once

#include "D3Ddef.h"

struct STrueRenderingObject ;
struct SEffectEssentialElements ;
class CSecretMassiveBillboardEffect ;
class CSecretObjectScatteringData ;
class CSecretShadowMap ;

#define D3DFVF_MASSIVEBILLBOARDVERTEX (D3DFVF_XYZ|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE3(1)|D3DFVF_TEXCOORDSIZE3(2))

struct SMassiveBillboardVertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 tex ;
	D3DXVECTOR3 factor ;//x:right length, y:up length, z:texture index
	D3DXVECTOR3 diffuse ;
} ;
struct SMassiveBillboardIndex
{
	WORD wIndex[3] ;
} ;

struct SBillboardIndex
{
	char szName[32] ;
	int index ;
	int x, y ;
	int width, height ;

	void set(char *pszName, int _index, int _x, int _y, int _width, int _height)
	{
		sprintf_s(szName, 32, "%s", pszName) ;
		index = _index ;
		x = _x ;
		y = _y ;
		width = _width ;
		height = _height ;
	}
} ;

struct SUnitBillboard
{
	enum ATTR { ATTR_FLIPHORIZONTAL=0x01 } ;

	int nAttr ;
	SBillboardIndex *psBillboardIndex ;
	STrueRenderingObject *psTRObject ;
	float fDistFromCameraInViewMatrix ;
	D3DXVECTOR3 vDiffuse ;

	SUnitBillboard()
	{
		nAttr = 0 ;
		psBillboardIndex = NULL ;
		psTRObject = NULL ;
		vDiffuse.x = vDiffuse.y = vDiffuse.z = 0.0f ;
		fDistFromCameraInViewMatrix = 0.0f ;
	}
} ;

class CSecretMassiveBillboard
{
public :
	const int MAXNUMVERTEX, MAXNUMINDEX ;
	const int MAXNUMBILLBOARD ;
	const int TEXTURESIZE ;

	enum ATTR {ATTR_IDSHADOW=0x01, ATTR_FIRSTPASS=0x02, ATTR_SECONDPASS=0x04} ;

private :
	int m_nAttr ;

	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pIB ;

	CSecretMassiveBillboardEffect *m_pcEffect ;

	int m_nNumVertex ;
	SMassiveBillboardVertex *m_psVertices ;
	int m_nNumIndex ;
	SMassiveBillboardIndex *m_psIndices ;

	int m_nNumBillboardIndex ;
	SBillboardIndex *m_psBillboardIndex ;

	SUnitBillboard *m_psUnitBillboards ;
	int m_nUnitBillboardCount ;

	bool m_bWireFrame ;

	LPDIRECT3DTEXTURE9 m_apTex[4] ;

	void _ImportBillboardIndex() ;
	void _BuildBillboardIndex() ;
	bool _FindTextureCoord(SUnitBillboard *psUnitBillboard, STrueRenderingObject *psTRObject) ;
	void _SetBillboardDiffuse(SUnitBillboard *psUnitBillboard, STrueRenderingObject *psTRObject) ;

public :
	CSecretObjectScatteringData *m_pcAtm ;
	CSecretShadowMap *m_pcShadowMap ;

public :
	CSecretMassiveBillboard() ;
	~CSecretMassiveBillboard() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, CSecretObjectScatteringData *pcAtm) ;
	bool AddBillboard(STrueRenderingObject *psTRObject) ;
	void Process() ;
	void Render() ;
	void Release() ;

	void SetTexture(int nIndex, LPDIRECT3DTEXTURE9 pTex) ;
	void Reset() ;

	void AddAttr(int nAttr, bool bEnable=true) ;
	bool IsAttr(int nAttr) ;
	void SetShadowMap(CSecretShadowMap *pcShadowMap) ;
	void SetWireFrame(bool bEnable) ;
} ;

class CSecretMassiveBillboardEffect
{
public :
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//정점선언
	LPD3DXEFFECT m_pEffect ;//셰이더
	D3DXHANDLE m_hTechnique ;//테크닉
	D3DXHANDLE m_hmatWVP ;//월드-뷰-투영 변환을 위한 행렬
	D3DXHANDLE m_hvCameraPos ;//카메라위치
	D3DXHANDLE m_hmatWLP ;//월드-광원-투영 행렬
	D3DXHANDLE m_hmatWLPT ;//월드-광원-투영-텍스쳐바이어스 행렬
	D3DXHANDLE m_hvLightDir ;
	D3DXHANDLE m_hvLightColor ;
	D3DXHANDLE m_hAtm ;
	D3DXHANDLE m_hvLookat ;

	SEffectEssentialElements *m_psEssentialElements ;

	CSecretMassiveBillboardEffect() ;
	~CSecretMassiveBillboardEffect() ;

	HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
	void Release() ;
} ;