#include "SecretMaterial.h"

CSecretMaterial::CSecretMaterial()
{
	m_pTexture = NULL ;
	m_pVB = NULL ;
	m_pVI = NULL ;
}

CSecretMaterial::~CSecretMaterial()
{
	Release() ;
}

//void CSecretMaterial::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszTextureName, int nNumVertex, void *pvVertices, int nNumIndex, void *pvIndices)
//{
//	m_pd3dDevice = pd3dDevice ;
//
//	if(FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, str, &m_pTexture)))
//	{
//		TRACE("Cannot load Texture path=%s\r\n", pszTextureName) ;
//		assert(false && "Cannot load TextureFile") ;
//		return false ;
//	}
//}

HRESULT CSecretMaterial::InitVertexBuffer(UINT lNumVertex, void *pVertices, DWORD dwFVF, UINT lZeroStride)
{
    ///////////////////////////////////////////////////
	//Create Vertices
	///////////////////////////////////////////////////
	m_lNumVertex = lNumVertex ;
	m_lVertexZeroStride = lZeroStride ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_lNumVertex*m_lVertexZeroStride, D3DUSAGE_WRITEONLY, dwFVF, D3DPOOL_DEFAULT, &m_pVB, NULL)))
		return E_FAIL ;

	_WriteVertexBuffer(pVertices) ;

	return S_OK ;
}

HRESULT CSecretMaterial::_WriteVertexBuffer(void *pSrcVertices)
{
	void *pDestVertices ;
	UINT lSize = m_lNumVertex*m_lVertexZeroStride ;

	if(FAILED(m_pVB->Lock(0, lSize, &pDestVertices, 0)))
		return E_FAIL ;

	memcpy(pDestVertices, pSrcVertices, lSize) ;

	m_pVB->Unlock() ;

	return S_OK ;
}

HRESULT CSecretMaterial::InitIndexBuffer(UINT lNumIndex, void *pIndices, D3DFORMAT Format)
{
	///////////////////////////////////////////////////
	//Create Indices
	///////////////////////////////////////////////////
	m_lNumIndex = lNumIndex ;
	m_IndexFormat = Format ;

	if(m_IndexFormat == D3DFMT_INDEX16)
		m_lIndexZeroStride = sizeof(WORD)*3 ;
	else if(m_IndexFormat == D3DFMT_INDEX32)
		m_lIndexZeroStride = sizeof(DWORD)*3 ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_lNumIndex*m_lIndexZeroStride, D3DUSAGE_WRITEONLY, m_IndexFormat, D3DPOOL_DEFAULT, &m_pVI, NULL)))
		return E_FAIL ;

	_WriteIndexBuffer(pIndices) ;

	return S_OK ;
}

HRESULT CSecretMaterial::_WriteIndexBuffer(void *pSrcIndices)
{
	void *pDestIndices ;
	UINT lSize = m_lNumIndex*m_lIndexZeroStride ;

	if(FAILED(m_pVI->Lock(0, lSize, (void **)&pDestIndices, 0)))
		return E_FAIL ;

    memcpy(pDestIndices, pSrcIndices, lSize) ;

	m_pVI->Unlock() ;

	return S_OK ;
}

void CSecretMaterial::Render()
{

}

void CSecretMaterial::Release()
{
	SAFE_RELEASE(m_pTexture) ;
	SAFE_RELEASE(m_pVB) ;
	SAFE_RELEASE(m_pVI) ;
}
