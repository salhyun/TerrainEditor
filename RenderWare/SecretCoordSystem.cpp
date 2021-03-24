#include "SecretCoordSystem.h"
#include "MathOrdinary.h"

CSecretCoordSystem::CSecretCoordSystem()
{
	D3DXMatrixIdentity(&m_matWorld) ;
}
CSecretCoordSystem::~CSecretCoordSystem()
{
}

CSecretCoordSystem::CSecretCoordSystem(LPDIRECT3DDEVICE9 pd3dDevice, D3DXMATRIX &mat)
{
	m_pd3dDevice = pd3dDevice ;
	m_matWorld = mat ;
}

void CSecretCoordSystem::Initialize(LPDIRECT3DDEVICE9 pd3dDevice)
{
    m_pd3dDevice = pd3dDevice ;
}

void CSecretCoordSystem::SetMatrix(D3DXMATRIX &mat)
{
	m_matWorld = mat ;
}

void CSecretCoordSystem::SetMatrix(Matrix4 &mat)
{
	m_matWorld = MatrixConvert(mat) ;
}

void CSecretCoordSystem::Render()
{
	//AxisX color is RED
	//AxisY color is GREEN
	//AxisZ color is BLUE
	DWORD colorX=0xffff0000, colorY=0xff00ff00, colorZ=0xff0000ff, black=0xff040404 ;

	SCVertex vertex[] = 
	{
		D3DXVECTOR3(0, 0, 0), colorX,
		D3DXVECTOR3(0, 0, 0), colorY,
		D3DXVECTOR3(0, 0, 0), colorZ,
		D3DXVECTOR3(1, 0, 0), colorX,
		D3DXVECTOR3(0, 1, 0), colorY,
		D3DXVECTOR3(0, 0, 1), colorZ,

        D3DXVECTOR3(1.3f, 0, 0), colorX,//6
		D3DXVECTOR3(1, 0.1f, 0), colorX,
		D3DXVECTOR3(1, 0, -0.1f), colorX,
		D3DXVECTOR3(1, -0.1f, 0), colorX,
		D3DXVECTOR3(1, 0, 0.1f), colorX,

		D3DXVECTOR3(0, 1.3f, 0), colorY,//11
		D3DXVECTOR3(-0.1f, 1, 0), colorY,
		D3DXVECTOR3(0, 1, -0.1f), colorY,
		D3DXVECTOR3(0.1f, 1, 0), colorY,
		D3DXVECTOR3(0, 1, 0.1f), colorY,

		D3DXVECTOR3(0, 0, 1.3f), colorZ,//16
		D3DXVECTOR3(0, 0.1f, 1), colorZ,
		D3DXVECTOR3(0.1f, 0, 1), colorZ,
		D3DXVECTOR3(0, -0.1f, 1), colorZ,
		D3DXVECTOR3(-0.1f, 0, 1), colorZ,

		D3DXVECTOR3(1, 0.1f, 0), black,//21
		D3DXVECTOR3(1, 0, -0.1f), black,
		D3DXVECTOR3(1, -0.1f, 0), black,
		D3DXVECTOR3(1, 0, 0.1f), black,

		D3DXVECTOR3(-0.1f, 1, 0), black,//25
		D3DXVECTOR3(0, 1, -0.1f), black,
		D3DXVECTOR3(0.1f, 1, 0), black,
		D3DXVECTOR3(0, 1, 0.1f), black,

		D3DXVECTOR3(0, 0.1f, 1), black,//29
		D3DXVECTOR3(0.1f, 0, 1), black,
		D3DXVECTOR3(0, -0.1f, 1), black,
		D3DXVECTOR3(-0.1f, 0, 1), black,
	} ;

	unsigned short index[] =
	{
		0, 3,
		1, 4,
		2, 5,

        6, 8, 7,
		6, 7, 10,
		6, 10, 9,
		6, 9, 8,
		23, 24, 21,
		21, 22, 23,

		11, 13, 12,
		11, 12, 15,
		11, 15, 14,
		11, 14, 13,
        27, 28, 25,
		25, 26, 27,

        16, 18, 17,
		16, 17, 20,
		16, 20, 19,
		16, 19, 18,
        31, 32, 29,
		29, 30, 31
	} ;

	D3DXVECTOR4 vpos ;
	for(int i=0 ; i<33 ; i++)
	{
		D3DXVec3Transform(&vpos, &vertex[i].pos, &m_matWorld) ;
		vertex[i].pos = D3DXVECTOR3(vpos.x, vpos.y, vpos.z) ;
	}

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SCVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 6, 3, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;
	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 33, 18, &index[6], D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}