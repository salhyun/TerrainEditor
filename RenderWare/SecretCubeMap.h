#pragma once

#include "D3Ddef.h"

class CSecretMeshObject ;

class CSecretCubeMap
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DCUBETEXTURE9 m_pCubeMap ;

public :
	CSecretCubeMap() ;
	~CSecretCubeMap() ;

    HRESULT BuildCubeMap(D3DXVECTOR3 vCameraPos, LPDIRECT3DDEVICE9 pd3dDevice, int nNumObject, CSecretMeshObject *pcSecretMeshObject) ;
	HRESULT LoadCubeMap(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName) ;// using DDS file format

} ;