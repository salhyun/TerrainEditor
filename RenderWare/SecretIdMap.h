#pragma once

#include "D3Ddef.h"
#include "SecretFrustum.h"
#include "SecretCoordSystem.h"

class CD3DEnvironment ;
class CSecretRenderingGroup ;
class CSecretPicking ;
class CSecretDragSelection ;
struct STrueRenderingObject ;

class CSecretIdMap
{
public :
	const int MAXNUM_TROBJECT ;

private :
	CD3DEnvironment *m_pEnvironment ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	SRenderTarget m_sRenderTarget ;
	CSecretPicking *m_pcPicking ;
	CSecretDragSelection *m_pcDragSelection ;
	SD3DEffect *m_psEffect ;

	int m_nNumTRObject ;
	STrueRenderingObject **m_ppsTRObject ;
	int m_nNumPickedIndex ;
	float *m_pfPickedEigenIndex ;

	bool _IsPickedIndex(float fIndex) ;

public :
	int m_nNumPickedObject ;
	STrueRenderingObject **m_ppsPickedTRObject ;

public :
	CSecretIdMap() ;
	~CSecretIdMap() ;

	void Initialize(CD3DEnvironment *pEnvironment, CSecretPicking *pcPicking, CSecretDragSelection *pcDragSelection, SD3DEffect *psEffect, int nResolution=256) ;
	void AddObject(STrueRenderingObject *psTRObject) ;
	bool PickOut(int nPickingKind) ;
	STrueRenderingObject *GetPickedObject() ;
	void Release() ;
    SRenderTarget *GetRenderTarget() ;
    LPDIRECT3DTEXTURE9 GetTex() ;
	void Reset() ;
} ;