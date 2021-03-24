#pragma once

#include "D3Ddef.h"

class CSecretMaterial
{
private :
    LPDIRECT3DTEXTURE9 m_pTexture ;

	LPDIRECT3DVERTEXBUFFER9 m_pVB ;
	LPDIRECT3DINDEXBUFFER9 m_pVI ;

	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	UINT m_lNumVertex, m_lVertexZeroStride ;

	UINT m_lNumIndex ;
    D3DFORMAT m_IndexFormat ;
	UINT m_lIndexZeroStride ;

	HRESULT _WriteVertexBuffer(void *pSrcVertices) ;
	HRESULT _WriteIndexBuffer(void *pIndices) ;


public :
	CSecretMaterial() ;
	~CSecretMaterial() ;

	HRESULT SetTexture(char *pszTextureName) ;
	HRESULT InitVertexBuffer(UINT lNumVertex, void *pVertices, DWORD dwFVF, UINT lZeroStride) ;
	HRESULT InitIndexBuffer(UINT lNumIndex, void *pIndices, D3DFORMAT Format) ;

	void Render() ;

	void Release() ;
} ;