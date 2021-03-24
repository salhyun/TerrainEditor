#include "D3DEnvironment.h"
#include "def.h"
#include <stdio.h>

CD3DEnvironment::CD3DEnvironment()
{
	m_pd3d = NULL ;
	m_pd3dDevice = NULL ;
	m_hwnd = NULL ;
	m_fFov = 45.0f*3.141592f/180.0f ;
	m_fNear = 0.1f ;
	m_fFar = 2048.0f ;
}

CD3DEnvironment::~CD3DEnvironment()
{
}

HRESULT CD3DEnvironment::Initialize(HWND hwnd, UINT lWidth, UINT lHeight, DWORD dwAddBehaviorFlag, BOOL bWindowed)
{
	m_hwnd = hwnd ;

	m_pd3d = Direct3DCreate9(D3D_SDK_VERSION) ;
	if(m_pd3d == NULL)
		return OutputError("Direct3DCreate9 is failure") ;

	D3DCAPS9 caps ;
	m_pd3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps) ;
	DWORD dwBehavior ;
	if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING ;
	else
		dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING ;

	_D3DDEVTYPE devType = D3DDEVTYPE_HAL ;

#ifdef _SHADERDEBUG_
	dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING ;
	devType = D3DDEVTYPE_REF ;
#endif

    m_lScreenWidth = lWidth ;
	m_lScreenHeight = lHeight ;
	m_lRefreshRateInHz = D3DPRESENT_RATE_DEFAULT ;

	if(bWindowed == FALSE)
	{
		D3DDISPLAYMODE d3dDisplayMode ;
		ZeroMemory(&d3dDisplayMode, sizeof(D3DDISPLAYMODE)) ;
		m_pd3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3dDisplayMode) ;

		m_lScreenWidth = d3dDisplayMode.Width ;
		m_lScreenHeight = d3dDisplayMode.Height ;
		m_lRefreshRateInHz = d3dDisplayMode.RefreshRate ;

		TRACE("full-screen mode width=%d height=%d refreshrate=%dHz\r\n", m_lScreenWidth, m_lScreenHeight, m_lRefreshRateInHz) ;
	}
    
	dwBehavior |= dwAddBehaviorFlag ;

    D3DPRESENT_PARAMETERS d3dpp ;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS)) ;
	d3dpp.Windowed = bWindowed ;
	d3dpp.BackBufferWidth = m_lScreenWidth ;
	d3dpp.BackBufferHeight = m_lScreenHeight ;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8 ;
	d3dpp.BackBufferCount = 1 ;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE ;
	d3dpp.MultiSampleQuality = 0 ;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD ;
	d3dpp.hDeviceWindow = m_hwnd ;
	d3dpp.EnableAutoDepthStencil = true ;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8 ;
	d3dpp.Flags = 0 ;
	d3dpp.FullScreen_RefreshRateInHz = m_lRefreshRateInHz ;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE ;

	if(FAILED(m_pd3d->CreateDevice(D3DADAPTER_DEFAULT, devType, m_hwnd, dwBehavior, &d3dpp, &m_pd3dDevice)))
		return OutputError("CreateDevice is failure") ;

//	D3DVIEWPORT9 Viewport={0, 0, m_lScreenWidth, m_lScreenHeight, 0.1f, 1.0f} ;
//	if(FAILED(m_pd3dDevice->SetViewport(&Viewport)))
//		return OutputError("SetViewport is failure") ;

	return S_OK ;
}

HRESULT CD3DEnvironment::OutputError(char *pszMessage)
{
	char szError[256], szCaption[] = "D3DEnviroment" ;
	DWORD dwError = GetLastError() ;

	if(dwError == D3DERR_INVALIDCALL)
		sprintf(szError, "%s, D3DERR_INVALIDCALL : The method call is invalid. For example, a method's parameter may have an invalid value", pszMessage) ;
	else if(dwError == D3DERR_NOTAVAILABLE)
		sprintf(szError, "%s, D3DERR_NOTAVAILABLE : This device does not support the queried technique", pszMessage) ;
	else if(dwError == D3DERR_OUTOFVIDEOMEMORY)
		sprintf(szError, "%s, D3DERR_OUTOFVIDEOMEMORY : Direct3D does not have enough display memory to perform the operation", pszMessage) ;

	//sprintf(szError, "%s, ERRORCODE:%d", pszMessage, dwError) ;
    MessageBox(m_hwnd, szError, szCaption, MB_ICONERROR) ;
	return E_FAIL ;
}

void CD3DEnvironment::Cleanup()
{
    SAFE_RELEASE(m_pd3dDevice) ;
	SAFE_RELEASE(m_pd3d) ;
}