#include "SecretTrail.h"
#include "def.h"
#include "SecretTextureContainer.h"

D3DVERTEXELEMENT9 decl_Trail[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//노말
	D3DDECL_END()
} ;

STrailPosition::STrailPosition()
{
	x = y = z = w = CoefFadeout = 0.0f ;
}
STrailPosition::STrailPosition(float _x, float _y, float _z, float _w)
{
	x = _x ;
	y = _y ;
	z = _z ;
	w = _w ;
	CoefFadeout = 0.0f ;
}
STrailPosition::~STrailPosition()
{
}

CSecretTrail::CSecretTrail() :
MAXNUM_POS(2048)
{
	m_nKind = FLYINGTRAIL ;
	m_nAttr = 0 ;
	m_nStatus = IDLING ;
	m_nViewStatus = FREEVIEW ;
	m_psVertices = NULL ;
	m_nNumVertex = 0 ;
	m_pTexAirTrail = m_pTexGroundTrail = NULL ;
	m_bWireFrame = false ;
}
CSecretTrail::~CSecretTrail()
{
	Release() ;
}

void CSecretTrail::Release()
{
	m_Positions.ReleaseAll() ;
	SAFE_DELETEARRAY(m_psVertices) ;
}

void CSecretTrail::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretTrailEffect *pcTrailEffect, float fHalfWidth, float fMaxLength, DWORD dwZEnable)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcTrailEffect = pcTrailEffect ;

	m_Positions.Initialize(MAXNUM_POS) ;
	m_psVertices = new STrailVertex[MAXNUM_POS*2] ;
	m_fHalfWidth = fHalfWidth ;
	m_dwZEnable = dwZEnable ;

	m_nViewStatus = FREEVIEW ;
	//m_nViewStatus = AFTERSHOT ;

	m_fMaxLength = fMaxLength ;
	_InitVariable() ;
}
void CSecretTrail::Process(Vector3 &vCamera, float fElapsedTime)
{
	if(m_nStatus == TRAILING)
	{
	}

	if(m_nAttr & ATTR_FADEOUT)
	{
		if(m_fAlphaValue < 1.0f)
		{
			m_fAlphaValue += (fElapsedTime*0.685f) ;
			if(m_fAlphaValue >= 1.0f)
			{
				m_fAlphaValue = 1.0f ;
				SetStatus(DISAPPEAR) ;
			}
		}
	}
	else if(m_nAttr & ATTR_LIMITEDLENGTH)
	{
		if(m_fCropLength > 0.0f)
		{
			m_fCropingTime += fElapsedTime ;
			float fLength = m_fCropingRate*m_fCropingTime ;

			Vector4 *pvCurPos = (Vector4 *)m_Positions.GetAt(1) ;
			Vector4 *pvPrevPos = (Vector4 *)m_Positions.GetAt(0) ;
			Vector4 v = (*pvCurPos) - (*pvPrevPos) ;
			Vector3 vDist(v.x, v.y, v.z) ;
			float fMagnitude = vDist.Magnitude() ;

			if(fLength >= fMagnitude)
			{
				m_fTrailLength -= fMagnitude ;
				m_Positions.Delete(0) ;

				m_fCropingTime -= fMagnitude/m_fCropingRate ;
				m_fCropLength -= fMagnitude ;
				if(m_fCropLength <= 0.0f)
				{
					m_fCropLength = 0.0f ;
					TRACE("CropingLength is zero %g\r\n", m_fCropLength) ;
				}
			}
		}
	}

	//Vector3 camera(12.635f, 2.0f, -43.773f) ;
	Vector3 camera = vCamera ;

	m_nNumVertex = 0 ;
	//_BuildQuadStrip(FLYINGTRAIL, vCamera, Vector3(0, 1, 0), fElapsedTime) ;
	_BuildQuadStrip(m_nKind, camera, Vector3(0, 1, 0), fElapsedTime) ;
}
void CSecretTrail::Render()
{
	if(m_nNumVertex < 3)
		return ;

	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	DWORD dwAlphaTest, dwZWrite, dwAlphaBlend, dwCull, dwCmp, dwZEnable ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHATESTENABLE, &dwAlphaTest) ;
	m_pd3dDevice->GetRenderState(D3DRS_ZWRITEENABLE, &dwZWrite) ;
	m_pd3dDevice->GetRenderState(D3DRS_ZENABLE, &dwZEnable) ;
	m_pd3dDevice->GetRenderState(D3DRS_ZFUNC, &dwCmp) ;
	m_pd3dDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlphaBlend) ;
	m_pd3dDevice->GetRenderState(D3DRS_CULLMODE, &dwCull) ;

	m_pd3dDevice->SetFVF(D3DFVF_TRAILVERTEX) ;

	m_pcTrailEffect->m_pEffect->SetTechnique(m_pcTrailEffect->m_hTechnique) ;
	m_pcTrailEffect->m_pEffect->Begin(NULL, 0) ;

	m_pcTrailEffect->m_pEffect->BeginPass(0) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, m_dwZEnable) ;

	if(m_nKind == FLYINGTRAIL)
		m_pcTrailEffect->m_pEffect->SetTexture("DecalMap", m_pTexAirTrail) ;
	else if(m_nKind == ROLLINGTRAIL)
		m_pcTrailEffect->m_pEffect->SetTexture("DecalMap", m_pTexGroundTrail) ;

	m_pcTrailEffect->m_pEffect->SetMatrix(m_pcTrailEffect->m_hmatWVP, &m_pcTrailEffect->m_matWVP) ;

	//first rendering
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x000000bf) ;//cut off greater that 0.75
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE) ;
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO) ;

	m_pcTrailEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_nNumVertex-2, m_psVertices, sizeof(STrailVertex)) ;

	//second rendering
	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) ;

	m_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER) ;

	m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) ;
	m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) ;

	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE) ;
	//m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS) ;

	m_pcTrailEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, m_nNumVertex-2, m_psVertices, sizeof(STrailVertex)) ;

	m_pcTrailEffect->m_pEffect->EndPass() ;
	m_pcTrailEffect->m_pEffect->End() ;

	m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, dwCull) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, dwAlphaTest) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, dwZWrite) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, dwZEnable) ;
	m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, dwCmp) ;
	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlphaBlend) ;
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode) ;
}
void CSecretTrail::AddPos(Vector3 &vPos, Vector3 *pvNormal)
{
	if((m_Positions.nCurPos >= m_Positions.nMaxNum) || (m_nStatus != TRAILING))
		return ;

	if(m_Positions.nCurPos > 0)
	{
		//Vector4 *pv = &m_Positions[m_Positions.nCurPos-1] ;
		STrailPosition *pv = &m_Positions[m_Positions.nCurPos-1] ;

		Vector3 vPrevPos(pv->x, pv->y, pv->z) ;

		if((vPrevPos-vPos).Magnitude() < 0.075f)
			return ;
	}

	//TRACE("AddPos(Vector3(%gf, %gf, %gf));\r\n", enumVector(vPos)) ;
	//Vector4 p(vPos.x, vPos.y, vPos.z, 1.0f) ;
	STrailPosition p(vPos.x, vPos.y, vPos.z, 1.0f) ;
	if(pvNormal)
		p.vNormal.set(pvNormal->x, pvNormal->y, pvNormal->z) ;
	else
		p.vNormal.set(0, 1, 0) ;

	m_Positions.Insert(&p) ;

	if(m_Positions.nCurPos > 1)
	{
		Vector4 *pvCurPos, *pvPrevPos ;
		Vector4 v ;
		Vector3 vDist ;

		pvCurPos = (Vector4 *)m_Positions.GetAt(m_Positions.nCurPos-1) ;
		pvPrevPos = (Vector4 *)m_Positions.GetAt(m_Positions.nCurPos-2) ;
		//새로추가된 길이를 합한다.
		v = (*pvCurPos) - (*pvPrevPos) ;
		vDist.set(v.x, v.y, v.z) ;
		m_fTrailLength += vDist.Magnitude() ;

		if(m_nAttr & ATTR_LIMITEDLENGTH)
		{
			float fLength = m_fTrailLength - m_fMaxLength ;
			if(fLength > 0)
			{
				//일단 맨마지막꼬리 하나 제거하고
				pvCurPos = (Vector4 *)m_Positions.GetAt(1) ;
				pvPrevPos = (Vector4 *)m_Positions.GetAt(0) ;
				v = (*pvCurPos) - (*pvPrevPos) ;
				vDist.set(v.x, v.y, v.z) ;
				m_fTrailLength -= vDist.Magnitude() ;
				m_Positions.Delete(0) ;
			}
		}
	}
}
void CSecretTrail::_InitVariable()
{
	m_fTrailLength = 0.0f ;
	m_fFadeoutTime = 1.0f ;
	m_fAlphaValue = 0.0f ;
	m_fCropLength = 0.0f ;
	m_nAttr = 0 ;
	m_dwZEnable = D3DZB_TRUE ;
}
void CSecretTrail::Reset()
{
	m_Positions.Reset() ;
	_InitVariable() ;
}
void CSecretTrail::SetHalfWidth(float fHalfWidth)
{
	m_fHalfWidth = fHalfWidth ;
}
void CSecretTrail::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}
void CSecretTrail::_BuildQuadStrip(int nKind, Vector3 &vCamera, Vector3 &vUp, float fElapsedTime)
{
	if(nKind == FLYINGTRAIL)
	{
		if(m_Positions.nCurPos < 2)
			return ;

		STrailVertex *psVertices = m_psVertices ;
		Vector3 p, pos1, pos2, N, B, T ;
		D3DXVECTOR3 vDist ;
		float fUppersideLength=0.0f, fLowersideLength=0.0f ;
		int i, nPosCount=0 ;

		//first point
		if(m_nViewStatus == FREEVIEW)
		{
			pos1.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			N = (vCamera-pos1).Normalize() ;
			nPosCount++ ;

			pos2.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			B = (pos2-pos1).Normalize() ;

			T = B.cross(N) ;
			if(T.IsZero())
				T = vUp ;
		}
		else if(m_nViewStatus == AFTERSHOT)
		{
			pos1.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			N = (vCamera-pos1).Normalize() ;
			T = N.cross(vUp) ;
			nPosCount++ ;
		}

		p = pos1 - (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;
		psVertices->tex.x = 0.0f ;
		psVertices->tex.y = 0.0f ;
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[0].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		p = pos1 + (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;
		psVertices->tex.x = 1.0f ;
		psVertices->tex.y = 0.0f ;
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[0].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		for(i=nPosCount ; i<(m_Positions.nCurPos-1) ; i++)
		{
			if(m_nViewStatus == FREEVIEW)
			{
				pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;
				N = (vCamera-pos1).Normalize() ;

				pos2.set(m_Positions[i+1].x, m_Positions[i+1].y, m_Positions[i+1].z) ;
				B = (pos2-pos1).Normalize() ;

				T = B.cross(N) ;
				if(T.IsZero())
					T = vUp ;
			}
			else if(m_nViewStatus == AFTERSHOT)
			{
				pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;
				N = (vCamera-pos1).Normalize() ;
				T = N.cross(vUp) ;
			}

			p = pos1 - (T*m_fHalfWidth) ;
			psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

			//이전에 버텍스점과 거리를 구한다.
			vDist = psVertices->pos - (psVertices-2)->pos ;
			fUppersideLength += D3DXVec3Length(&vDist) ;
			psVertices->tex.x = 0.0f ;
			psVertices->tex.y = fUppersideLength ;//현재까지의 길이를 저장한다.
			psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
			psVertices++ ;
			m_nNumVertex++ ;

			p = pos1 + (T*m_fHalfWidth) ;
			psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

			//이전에 버텍스점과 거리를 구한다.
			vDist = psVertices->pos - (psVertices-2)->pos ;
			fLowersideLength += D3DXVec3Length(&vDist) ;
			psVertices->tex.x = 1.0f ;
			psVertices->tex.y = fLowersideLength ;//현재까지의 길이를 저장한다.
			psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
			psVertices++ ;
			m_nNumVertex++ ;
		}

		//last point
		pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;
		N = (vCamera-pos1).Normalize() ;
		T = B.cross(N) ;
		if(T.IsZero())
			T = vUp ;

		p = pos1 - (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

		//이전에 버텍스점과 거리를 구한다.
		vDist = psVertices->pos - (psVertices-2)->pos ;
		fUppersideLength += D3DXVec3Length(&vDist) ;
		psVertices->tex.x = 0.0f ;
		psVertices->tex.y = fUppersideLength ;//현재까지의 길이를 저장한다.
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		p = pos1 + (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

		//이전에 버텍스점과 거리를 구한다.
		vDist = psVertices->pos - (psVertices-2)->pos ;
		fLowersideLength += D3DXVec3Length(&vDist) ;
		psVertices->tex.x = 1.0f ;
		psVertices->tex.y = fLowersideLength ;//현재까지의 길이를 저장한다.
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		assert(m_nNumVertex < (MAXNUM_POS*2)) ;

		float fInvUppersideLength = 1.0f/fUppersideLength ;
		float fInvLowersideLength = 1.0f/fLowersideLength ;

		psVertices = m_psVertices+2 ;
		for(int i=1 ; i<(m_Positions.nCurPos) ; i++)
		{
			psVertices->tex.y *= fInvUppersideLength ;
			psVertices++ ;

			psVertices->tex.y *= fInvLowersideLength ;
			psVertices++ ;
		}
	}
	else if(nKind == ROLLINGTRAIL)
	{
		if(m_Positions.nCurPos < 2)
			return ;

		STrailVertex *psVertices = m_psVertices ;
		Vector3 p, pos1, pos2, N, B, T ;
		D3DXVECTOR3 vDist ;
		float fUppersideLength=0.0f, fLowersideLength=0.0f ;
		int i, nPosCount=0 ;

		//first point
		if(m_nViewStatus == FREEVIEW)
		{
			pos1.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			N = m_Positions[nPosCount].vNormal ;// (vCamera-pos1).Normalize() ;
			nPosCount++ ;

			pos2.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			B = (pos2-pos1).Normalize() ;

			T = B.cross(N) ;
			if(T.IsZero())
				T = vUp ;
		}
		else if(m_nViewStatus == AFTERSHOT)
		{
			pos1.set(m_Positions[nPosCount].x, m_Positions[nPosCount].y, m_Positions[nPosCount].z) ;
			N = m_Positions[nPosCount].vNormal ;//(vCamera-pos1).Normalize() ;
			T = N.cross(vUp) ;
			nPosCount++ ;
		}

		p = pos1 - (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;
		psVertices->tex.x = 0.0f ;
		psVertices->tex.y = 0.0f ;
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[0].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		p = pos1 + (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;
		psVertices->tex.x = 1.0f ;
		psVertices->tex.y = 0.0f ;
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[0].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		for(i=nPosCount ; i<(m_Positions.nCurPos-1) ; i++)
		{
			if(m_nViewStatus == FREEVIEW)
			{
				pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;
				N = m_Positions[i].vNormal ;// (vCamera-pos1).Normalize() ;

				pos2.set(m_Positions[i+1].x, m_Positions[i+1].y, m_Positions[i+1].z) ;
				B = (pos2-pos1).Normalize() ;

				T = B.cross(N) ;
				if(T.IsZero())
					T = vUp ;
			}
			else if(m_nViewStatus == AFTERSHOT)
			{
				pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;
				N = m_Positions[i].vNormal ;// (vCamera-pos1).Normalize() ;
				T = N.cross(vUp) ;
			}

			p = pos1 - (T*m_fHalfWidth) ;
			psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

			//이전에 버텍스점과 거리를 구한다.
			vDist = psVertices->pos - (psVertices-2)->pos ;
			fUppersideLength += D3DXVec3Length(&vDist) ;
			psVertices->tex.x = 0.0f ;
			psVertices->tex.y = fUppersideLength ;//현재까지의 길이를 저장한다.
			psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
			psVertices++ ;
			m_nNumVertex++ ;

			p = pos1 + (T*m_fHalfWidth) ;
			psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

			//이전에 버텍스점과 거리를 구한다.
			vDist = psVertices->pos - (psVertices-2)->pos ;
			fLowersideLength += D3DXVec3Length(&vDist) ;
			psVertices->tex.x = 1.0f ;
			psVertices->tex.y = fLowersideLength ;//현재까지의 길이를 저장한다.
			psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
			psVertices++ ;
			m_nNumVertex++ ;
		}

		//last point
		pos1.set(m_Positions[i].x, m_Positions[i].y, m_Positions[i].z) ;

		p = pos1 - (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

		//이전에 버텍스점과 거리를 구한다.
		vDist = psVertices->pos - (psVertices-2)->pos ;
		fUppersideLength += D3DXVec3Length(&vDist) ;
		psVertices->tex.x = 0.0f ;
		psVertices->tex.y = fUppersideLength ;//현재까지의 길이를 저장한다.
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		p = pos1 + (T*m_fHalfWidth) ;
		psVertices->pos = D3DXVECTOR3(p.x, p.y, p.z) ;

		//이전에 버텍스점과 거리를 구한다.
		vDist = psVertices->pos - (psVertices-2)->pos ;
		fLowersideLength += D3DXVec3Length(&vDist) ;
		psVertices->tex.x = 1.0f ;
		psVertices->tex.y = fLowersideLength ;//현재까지의 길이를 저장한다.
		psVertices->tex.z = m_fAlphaValue ;// m_Positions[i].w ;
		psVertices++ ;
		m_nNumVertex++ ;

		assert(m_nNumVertex < (MAXNUM_POS*2)) ;

		float fInvUppersideLength = 1.0f/fUppersideLength ;
		float fInvLowersideLength = 1.0f/fLowersideLength ;

		psVertices = m_psVertices+2 ;
		for(int i=1 ; i<(m_Positions.nCurPos) ; i++)
		{
			psVertices->tex.y *= fInvUppersideLength ;
			psVertices++ ;

			psVertices->tex.y *= fInvLowersideLength ;
			psVertices++ ;
		}
	}
}
void CSecretTrail::SetTextures(LPDIRECT3DTEXTURE9 pTexAirTrail, LPDIRECT3DTEXTURE9 pTexGroundTrail)
{
	m_pTexAirTrail = pTexAirTrail ;
	m_pTexGroundTrail = pTexGroundTrail ;
}
void CSecretTrail::SetKind(int nKind)
{
	m_nKind = nKind ;
}
int CSecretTrail::GetKind()
{
	return m_nKind ;
}
void CSecretTrail::SetStatus(int nStatus)
{
	if(nStatus == TRAILING)
	{
		_InitVariable() ;
	}

	m_nStatus = nStatus ;
}
bool CSecretTrail::IsStatus(int nStatus)
{
	if(m_nStatus == nStatus)
		return true ;
	return false ;
}
void CSecretTrail::SetViewStatus(int nStatus)
{
	m_nViewStatus = nStatus ;
}
bool CSecretTrail::IsViewStatus(int nStatus)
{
	if(m_nViewStatus == nStatus)
		return true ;
	return false ;
}
void CSecretTrail::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
	{
		m_nAttr |= nAttr ;

		if(nAttr == ATTR_LIMITEDLENGTH)
		{
			float length = m_fTrailLength - m_fMaxLength ;
			if(length > 0.0f)
			{
				m_fCropLength = length ;
				m_fCropingRate = m_fCropLength/2.0f ;
				m_fCropingTime = 0.0f ;
			}
			else
				m_fCropLength = 0.0f ;

			TRACE("addAttr ATTR_LIMITEDLENGTH cropLength=%g\r\n", m_fCropLength) ;
		}
	}
	else
		m_nAttr &= (~nAttr) ;
}
bool CSecretTrail::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}
void CSecretTrail::SetMaxLength(float fLength)
{
	m_fMaxLength = fLength ;
}

//###################################################################################//
//                             CSecretTrailEffect                                    //
//###################################################################################//
CSecretTrailEffect::CSecretTrailEffect()
{
	m_pDecl = NULL ;
	m_pEffect = NULL ;
	m_hTechnique = NULL ;
}
CSecretTrailEffect::~CSecretTrailEffect()
{
	Release() ;
}
void CSecretTrailEffect::Release()
{
	SAFE_RELEASE(m_pEffect) ;
	SAFE_RELEASE(m_pDecl) ;
}
HRESULT CSecretTrailEffect::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName)
{
	HRESULT hr ;
	LPD3DXBUFFER pError=NULL ;
	DWORD dwShaderFlag=NULL ;

#ifdef _SHADERDEBUG_
	dwShaderFlag = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION ;//|D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT|D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT|D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION|D3DXSHADER_SKIPVALIDATION ;
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, dwShaderFlag, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#else
	if(FAILED(hr=D3DXCreateEffectFromFile(pd3dDevice, pszFileName, NULL, NULL, 0, NULL, &m_pEffect, &pError)))
	{
		MessageBox(NULL, (LPCTSTR)pError->GetBufferPointer(), "Error", MB_OK) ;
		return E_FAIL ;
	}
#endif
	else
	{
		if((m_hTechnique = m_pEffect->GetTechniqueByName("TShader")) == NULL)
		{
			assert(false && "hTechnique is NULL (TShader)") ;
			return E_FAIL ;
		}
 	}
	SAFE_RELEASE(pError) ;

	SetD3DXHANDLE(&m_hmatWVP, "matWVP") ;
	SetD3DXHANDLE(&m_hvCameraPos, "vCameraPos") ;
	return S_OK ;
}
bool CSecretTrailEffect::SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName)
{
	if((*pHandle = m_pEffect->GetParameterByName(NULL, pszName)) == NULL)
	{
		char err[128] ;
		sprintf(err, "%s Handle is NULL", pszName) ;
		assert(false && err) ;
		return false ;
	}
	return true ;
}

//###########################################################################################################//
//                                           CSecretTrailManager                                             //
//###########################################################################################################//

CSecretTrailManager::CSecretTrailManager() :
MAXNUM_TRAIL(64)
{
	m_pd3dDevice = NULL ;
	m_pcTexContainer = NULL ;
	m_pcTrail = NULL ;
	m_bWireFrame = false ;

}
CSecretTrailManager::~CSecretTrailManager()
{
	Release() ;
}
void CSecretTrailManager::Release()
{
	SAFE_DELETEARRAY(m_pcTrail) ;
	m_cTrailEffect.Release() ;
}

void CSecretTrailManager::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements)
{
	m_pd3dDevice = pd3dDevice ;

	m_cTrailEffect.Initialize(pd3dDevice, "shader/trail.fx") ;
	m_cTrailEffect.m_psEssentialElements = psEssentialElements ;

	m_pcTrail = new CSecretTrail[MAXNUM_TRAIL] ;
	for(int i=0 ; i<MAXNUM_TRAIL ; i++)
	{
		m_pcTrail[i].Initialize(m_pd3dDevice, &m_cTrailEffect, 0.02f, 25.0f) ;
		m_pcTrail[i].SetTextures(m_pcTexContainer->FindTexture("trail_air04"), m_pcTexContainer->FindTexture("trail_ground")) ;
	}
}
void CSecretTrailManager::Process(Vector3 &vCamera, float fElapsedTime, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj)
{
	m_cTrailEffect.m_matWVP = (*pmatView) * (*pmatProj) ;

	CSecretTrail *pcTrail = m_pcTrail ;
	for(int i=0 ; i<MAXNUM_TRAIL ; i++, pcTrail++)
	{
		pcTrail->Process(vCamera, fElapsedTime) ;
	}
}
void CSecretTrailManager::Render()
{
	CSecretTrail *pcTrail = m_pcTrail ;
	for(int i=0 ; i<MAXNUM_TRAIL ; i++, pcTrail++)
	{
		pcTrail->SetWireFrame(m_bWireFrame) ;
		pcTrail->Render() ;
	}
}
CSecretTrail *CSecretTrailManager::GetEmptyTrail()
{
	CSecretTrail *pcTrail = m_pcTrail ;
	for(int i=0 ; i<MAXNUM_TRAIL ; i++, pcTrail++)
	{
		if(pcTrail->IsStatus(CSecretTrail::IDLING))
		{
			pcTrail->SetStatus(CSecretTrail::TRAILING) ;
			pcTrail->Reset() ;
			return pcTrail ;
		}
	}
	return NULL ;
}
void CSecretTrailManager::SetTexContainer(CSecretTextureContainer *pcTexContainer)
{
	m_pcTexContainer = pcTexContainer ;
}
void CSecretTrailManager::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}
void CSecretTrailManager::ResetAllTrails()
{
	CSecretTrail *pcTrail = m_pcTrail ;
	for(int i=0 ; i<MAXNUM_TRAIL ; i++, pcTrail++)
	{
		pcTrail->Reset() ;
		pcTrail->SetStatus(CSecretTrail::IDLING) ;
	}
}
CSecretTrail *CSecretTrailManager::GetTrail(int nNum)
{
	return &m_pcTrail[nNum] ;
}