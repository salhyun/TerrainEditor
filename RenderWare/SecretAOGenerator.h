#include "def.h"
#include "D3Ddef.h"

class CSecretMeshObject ;
class CASELoader ;
class CASEData ;
class CSecretTextureContainer ;
class CSecretObjectScatteringData ;
struct SD3DEffect ;
struct SMesh ;
class CSecretTextureContainer ;

class CSecretAOGenerator
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	LPDIRECT3DQUERY9 m_pObjectQuery ;
	CASELoader *m_pcASELoader ;
	CASEData *m_pcASEData ;
	int m_nTexSize ;
	LPDIRECT3DTEXTURE9 m_pTexBuffer ;
	LPDIRECT3DSURFACE9 m_pSurfBuffer ;
    LPDIRECT3DSURFACE9 m_pSurfZBuffer ;
	D3DXMATRIX m_matView, m_matProj ;
	SD3DEffect *m_psEffect ;
	CSecretTextureContainer *m_pcTexContainer ;
	char m_szFileName[256] ;

	CSecretMeshObject *m_pcObject ;
	DWORD m_dwColorKey ;

	void _Render() ;
    float _CalculateAmbientDegree(D3DXVECTOR3 *pvPos, D3DXVECTOR3 *pvLookat, D3DXVECTOR3 *pvUp) ;
	SMesh *_AddGroundMesh(float fGroundSize) ;

public :
	CSecretAOGenerator() ;
	~CSecretAOGenerator() ;

    bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTextureContainer *pcTexContainer, SD3DEffect *psEffect) ;
	void LoadASE(char *pszFileName, CSecretObjectScatteringData *pcAtm) ;
	void Process() ;
	void Release() ;

	LPDIRECT3DTEXTURE9 GetTexBuffer() {return m_pTexBuffer;}
	void ExportRMD() ;
} ;