#pragma once

#include "D3DDef.h"

//#define _SHADERDEBUG_

class CD3DEnvironment
{
private :
    LPDIRECT3D9           m_pd3d ;					//The main D3D object
	LPDIRECT3DDEVICE9     m_pd3dDevice ;			//The D3D Rendering device
	D3DCAPS9              m_d3dCaps ;
	D3DSURFACE_DESC       m_d3dsBackBuffer ;

public :
	HWND                  m_hwnd ;
	UINT m_lScreenWidth, m_lScreenHeight ;
	UINT m_lRefreshRateInHz ;

	float m_fFov ;
	float m_fNear, m_fFar ;

private :
    
public :
	CD3DEnvironment() ;
	~CD3DEnvironment() ;

	HRESULT Initialize(HWND hwnd, UINT lWidth, UINT lHeight, DWORD dwAddBehaviorFlag=0, BOOL bWindowed=TRUE) ;

	HRESULT OutputError(char *pszMessage) ;
	LPDIRECT3DDEVICE9 GetD3DDevice() {return m_pd3dDevice;} ;

	void Cleanup() ;
} ;