#include "SecretDragSelection.h"
#include "D3DEnvironment.h"
#include "SecretFrustum.h"
#include "SecretBoundingBox.h"

CSecretDragSelection::CSecretDragSelection()
{
	m_pcEnvironment = NULL ;
	m_pd3dDevice = NULL ;
	m_pcViewFrustum = NULL ;
	m_nStatus = STATUS_IDLE ;
	m_bCurMouseLButton = m_bPrevMouseLButton = false ;
}
CSecretDragSelection::~CSecretDragSelection()
{
}

void CSecretDragSelection::Initialize(CD3DEnvironment *pcEnvironment, LPDIRECT3DDEVICE9 pd3dDevice, CSecretFrustum *pcFrustum)
{
	m_pcEnvironment = pcEnvironment ;
	m_pd3dDevice = pd3dDevice ;
	m_pcViewFrustum = pcFrustum ;
}
void CSecretDragSelection::Process(Vector2 *pvScreenPos, bool bMouseLButton, Matrix4 &matV, Matrix4 &matP)
{
	//decide status
	if(m_nStatus == STATUS_IDLE)
	{
		if(bMouseLButton)
			m_nStatus = STATUS_DRAGSTART ;
	}
	else if(m_nStatus == STATUS_DRAGSTART)
	{
		m_nStatus = STATUS_DRAGGING ;
	}
	else if(m_nStatus == STATUS_DRAGGING)
	{
        if(!bMouseLButton)
			m_nStatus = STATUS_DRAGEND ;
	}
	else if(m_nStatus == STATUS_DRAGEND)
	{
		m_nStatus = STATUS_IDLE ;
		m_bDrawLines = false ;
	}

	//process status
	if(m_nStatus == STATUS_IDLE)
	{
	}
	else if(m_nStatus == STATUS_DRAGSTART)
	{
		m_vPrevScreenPos = m_vCurScreenPos = *pvScreenPos ;
		m_bDrawLines = false ;
	}
	else if(m_nStatus == STATUS_DRAGGING)
	{
		m_vCurScreenPos = *pvScreenPos ;

		float a = m_vCurScreenPos.x-m_vPrevScreenPos.x ;
		float b = m_vCurScreenPos.y-m_vPrevScreenPos.y ;

		//didn't move
		//if(float_eq(m_vCurScreenPos.x, m_vPrevScreenPos.x) || float_eq(m_vCurScreenPos.y, m_vPrevScreenPos.y))
        if(float_less(sqrtf(a*a + b*b), 10.0f) || float_eq(m_vCurScreenPos.x, m_vPrevScreenPos.x) || float_eq(m_vCurScreenPos.y, m_vPrevScreenPos.y))
		{
		}
		else//move
		{
			//finding position in current scene
			float x1, x2, y1, y2, temp ;

			x1 = (m_vPrevScreenPos.x*2/m_pcEnvironment->m_lScreenWidth) - 1 ;
			y1 = ((m_pcEnvironment->m_lScreenHeight-m_vPrevScreenPos.y)*2/m_pcEnvironment->m_lScreenHeight) - 1 ;

			x2 = (m_vCurScreenPos.x*2/m_pcEnvironment->m_lScreenWidth) - 1 ;
			y2 = ((m_pcEnvironment->m_lScreenHeight-m_vCurScreenPos.y)*2/m_pcEnvironment->m_lScreenHeight) - 1 ;

			if(x1 > x2)
			{
				temp = x1 ;
				x1 = x2 ;
				x2 = temp ;
			}
			if(y1 > y2)
			{
				temp = y1 ;
				y1 = y2 ;
				y2 = temp ;
			}

			m_avPos[0].set(x1, y1, 0) ;
			m_avPos[1].set(x2, y1, 0) ;
			m_avPos[2].set(x2, y1, 1) ;
			m_avPos[3].set(x1, y1, 1) ;

			m_avPos[4].set(x1, y2, 0) ;
			m_avPos[5].set(x2, y2, 0) ;
			m_avPos[6].set(x2, y2, 1) ;
			m_avPos[7].set(x1, y2, 1) ;

			Matrix4 matInvVP = (matV*matP).Inverse() ;

			for(int i=0 ; i<8 ; i++)
				m_avPos[i] *= matInvVP ;

            

			if(!m_asPlane[0].MakePlane(m_avPos[4], m_avPos[7], m_avPos[5]))//top
				return ;

			if(!m_asPlane[1].MakePlane(m_avPos[0], m_avPos[1], m_avPos[3]))//bottom
				return ;

			if(!m_asPlane[2].MakePlane(m_avPos[0], m_avPos[4], m_avPos[1]))//near
				return ;
				
			if(!m_asPlane[3].MakePlane(m_avPos[3], m_avPos[2], m_avPos[6]))//far
				return ;

			if(!m_asPlane[4].MakePlane(m_avPos[0], m_avPos[3], m_avPos[4]))//left
				return ;

			if(!m_asPlane[5].MakePlane(m_avPos[1], m_avPos[5], m_avPos[2]))//right
				return ;

			m_bDrawLines = true ;
		}
	}
	else if(m_nStatus == STATUS_DRAGEND)
	{
	}
}
void CSecretDragSelection::Render()
{
	if((m_nStatus != STATUS_DRAGGING) || !m_bDrawLines)
		return ;

	DWORD dwColor = 0xff00ff00 ;

	struct SDragSelectionVertex
	{
		D3DXVECTOR4 pos ;
		DWORD color ;
	} ;

	SDragSelectionVertex vertex[] =
	{
		D3DXVECTOR4(m_vPrevScreenPos.x, m_vPrevScreenPos.y, 0, 1), dwColor,
		D3DXVECTOR4(m_vCurScreenPos.x, m_vPrevScreenPos.y, 0, 1), dwColor,
		D3DXVECTOR4(m_vCurScreenPos.x, m_vCurScreenPos.y, 0, 1), dwColor,
		D3DXVECTOR4(m_vPrevScreenPos.x, m_vCurScreenPos.y, 0, 1), dwColor
	} ;

	unsigned short index[8] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0
	} ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	DWORD dwZEnable ;
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(SDragSelectionVertex)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 4, 4, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}
bool CSecretDragSelection::Intersect(CSecretBoundingBox *pcBBox)
{
	bool bSuccess[4] = {false, false, false, false} ;
	float fDist ;

	fDist = m_asPlane[0].ToPosition(pcBBox->m_vCenter) ;//top
	if(fDist <= 0.0f)
		bSuccess[0] = true ;
	else
		if(fDist <= pcBBox->m_fRadius)
			bSuccess[0] = true ;

	fDist = m_asPlane[1].ToPosition(pcBBox->m_vCenter) ;//bottom
	if(float_less_eq(fDist, 0.0f))
		bSuccess[1] = true ;
	else
		if(fDist <= pcBBox->m_fRadius)
			bSuccess[1] = true ;

	fDist = m_asPlane[4].ToPosition(pcBBox->m_vCenter) ;//left
	if(float_less_eq(fDist, 0.0f))
		bSuccess[2] = true ;
	else
		if(fDist <= pcBBox->m_fRadius)
			bSuccess[2] = true ;

	fDist = m_asPlane[5].ToPosition(pcBBox->m_vCenter) ;//right
	if(float_less_eq(fDist, 0.0f))
		bSuccess[3] = true ;
	else
		if(fDist <= pcBBox->m_fRadius)
			bSuccess[3] = true ;

	if(bSuccess[0] && bSuccess[1] && bSuccess[2] && bSuccess[3])
		return true ;
	return false ;
}