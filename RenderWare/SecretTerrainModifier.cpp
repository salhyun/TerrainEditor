#include "MathOrdinary.h"
#include "SecretTerrainModifier.h"
#include "SecretTerrain.h"
#include "PerlinNoise.h"

CSecretTerrainModifier::CSecretTerrainModifier() : MAXNUM_DISALLOW(1024)
{
	m_pd3dDevice = NULL ;
	m_pcTerrain = NULL ;
	m_pcPicking = NULL ;
	m_pLine = NULL ;
	m_pvBoundaryPos1 = NULL ;
	m_pvBoundaryPos2 = NULL ;
	m_fRadius = 1.0f ;
	m_nStrengthType = STRENGTHTYPE_DIRECTLY ;
	m_fStrength = 0.05f;// 0.01f ;
	m_fArea = 2.0f ;
	m_bEnable = false ;
	m_bProcessed = false ;
	m_nKindofModify = 0 ;
	m_nFlattenAlign = LOWEST ;
	m_psDisallowPos = NULL ;
	m_nAttr = 0 ;
	m_fRuggedRange = 0.2f ;
	m_fSmoothStrength = 0.6f ;
}
CSecretTerrainModifier::~CSecretTerrainModifier()
{
	Release() ;
}
void CSecretTerrainModifier::Release()
{
	SAFE_RELEASE(m_pLine) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos1) ;
	SAFE_DELETEARRAY(m_pvBoundaryPos2) ;
	SAFE_DELETEARRAY(m_psDisallowPos) ;
}

void CSecretTerrainModifier::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretPicking *pcPicking)
{
	m_pd3dDevice = pd3dDevice ;
	m_pcPicking = pcPicking ;

	//(nBrushType == BRUSHTYPE::CIRCLE)
	m_nNumBoundary = 16 ;

	m_pvBoundaryPos1 = new Vector3[m_nNumBoundary] ;
	m_pvBoundaryPos2 = new D3DXVECTOR3[m_nNumBoundary+1] ;
	D3DXCreateLine(m_pd3dDevice, &m_pLine) ;
	m_pLine->SetWidth(3) ;
	m_pLine->SetAntialias(TRUE) ;

	m_psDisallowPos = new STerrainTilePos[MAXNUM_DISALLOW] ;
}

void CSecretTerrainModifier::Process(bool bLButtonPressed, bool bRButtonPressed, Matrix4 *pmatView, D3DXMATRIX *pmatProj)
{
	if(!m_bEnable)
		return ;

	m_pmatView = pmatView ;
    m_pmatProj = pmatProj ;

	static bool bFirst=false ;
	if(m_pcPicking->m_bIntersect)
	{
		m_vtPrevPicking = m_vtCurPicking ;
		memcpy(&m_vtCurPicking, m_pcPicking->GetCurVertex(), sizeof(SPickingVertex)) ;

		if(!bFirst)
		{
			m_vtPrevPicking = m_vtCurPicking ;
			bFirst = true ;
		}

		if(IsAttr(CSecretTerrainModifier::SNAPGRID))
		{
			m_vCenter.x = floorf(m_vtCurPicking.pos.x)+0.5f ;
			m_vCenter.y = m_vtCurPicking.pos.y ;
			m_vCenter.z = floorf(m_vtCurPicking.pos.z)+0.5f ;
		}
		else
			m_vCenter = m_vtCurPicking.pos ;
	}

    if(bLButtonPressed)
		_ModifyTerrain_LButton(m_nKindofModify) ;
	else if(bRButtonPressed)
		_ModifyTerrain_RButton(m_nKindofModify) ;

	int i ;
	float add, theta ;
	add = deg_rad(360.0f/m_nNumBoundary) ;
	theta = 0 ;
	Vector3 v ;

	for(i=0 ; i<m_nNumBoundary ; i++, theta += add)
	{
		v.x = m_vCenter.x + cosf(theta)*m_fRadius ;
		v.z = m_vCenter.z + sinf(theta)*m_fRadius ;
		v.y = m_pcTerrain->m_sHeightMap.GetHeightMap(v.x, v.z) ;

		v  *= (*m_pmatView) ;
		m_pvBoundaryPos2[i] = D3DXVECTOR3(v.x, v.y, v.z) ;
	}
	m_bProcessed = true ;
}
void CSecretTerrainModifier::Render()
{
	if(!m_bProcessed)
		return ;

	int i ;
	static D3DXVECTOR3 avPos[33] ;

	for(i=0 ; i<m_nNumBoundary ; i++)
		avPos[i] = m_pvBoundaryPos2[i] ;

	avPos[i] = avPos[0] ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE) ;

	m_pLine->Begin() ;
	m_pLine->DrawTransform(avPos, m_nNumBoundary+1, m_pmatProj, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)) ;
	m_pLine->End() ;

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE) ;
	m_bProcessed = false ;
}
void CSecretTerrainModifier::SetStrength(float strength)
{
	if(m_nKindofModify == KIND_RAISE)
		m_fStrength = strength*0.05f ;
	else if(m_nKindofModify == KIND_LOWER)
		m_fStrength = -strength*0.05f ;
}
void CSecretTerrainModifier::SetSize(float fRadius)
{
	m_fRadius = fRadius ;
	m_fArea = m_fRadius-2.0f ;
	if(float_less_eq(m_fArea, 0.0f))
		m_fArea = 1.0f ;
}
void CSecretTerrainModifier::SetStrengthType(int nType)
{
    m_nStrengthType = nType ;
}
void CSecretTerrainModifier::SetModifyKind(int nKind)
{
	if(nKind == KIND_RAISE)
	{
		if(m_fStrength < 0.0f)
			m_fStrength = -m_fStrength ;
	}
	else if(nKind == KIND_LOWER)
	{
		if(m_fStrength > 0.0f)
			m_fStrength = -m_fStrength ;
	}
	else if(nKind == KIND_SMOOTH)
	{
	}
	else if(nKind == KIND_FLATTEN)
	{
	}
	m_nKindofModify = nKind ;
}
void CSecretTerrainModifier::_SmoothAround(STerrainTilePos *psStartPos, STerrainTilePos *psEndPos, STerrainTilePos *psDisallowPos, int nDisallowCount)
{
	float h ;
    int x, z, i ;
	bool bDisallow ;
	int nVertexWidth = m_pcTerrain->m_sHeightMap.nVertexWidth ;
	int nVertexHeight = m_pcTerrain->m_sHeightMap.nVertexHeight ;
	int nHeight = psEndPos->z-psStartPos->z ;
	int nWidth = psEndPos->x-psStartPos->x ;
	psStartPos->x -= nWidth/2 ;
	psEndPos->x += nWidth/2 ;
	psStartPos->z -= nHeight/2 ;
	psEndPos->z += nHeight/2 ;

	for(z=psStartPos->z ; z<psEndPos->z ; z++)
	{
		for(x=psStartPos->x ; x<psEndPos->x ; x++)
		{
			bDisallow = false ;
            for(i=0 ; i<nDisallowCount ; i++)
			{
				if((psDisallowPos[i].x == x) && (psDisallowPos[i].z == z))
				{
					bDisallow = true ;
					break ;
				}
			}

			if(!bDisallow)
			{
				h=0.0f ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x-1, z) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x+1, z) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z-1) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z+1) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
				h *= 0.2f ;
				h = float_round(h) ;
				//TRACE("prev height=%07.03f", m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
				//TRACE("will change height=%07.03f\r\n", h) ;
				m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y = h ;
			}
		}
	}
	for(z=psStartPos->z ; z<psEndPos->z ; z++)
	{
		for(x=psStartPos->x ; x<psEndPos->x ; x++)
			m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
	}

	int nStart, nCount ;
	nHeight = psEndPos->z-psStartPos->z ;
	nWidth = psEndPos->x-psStartPos->x ;

	psStartPos->z -= nHeight/2 ;
	psEndPos->z += nHeight/2 ;
	psStartPos->x -= nWidth/2 ;
	psEndPos->x += nWidth/2 ;
	Vector3 vNormal ;
	TERRAINVERTEX *pVertices=NULL ;
	for(z=psStartPos->z ; z<psEndPos->z ; z++)
	{
		pVertices=NULL ;
		nStart = z*nVertexWidth+psStartPos->x ;
		nCount = psEndPos->x-psStartPos->x ;
		if(!FAILED(m_pcTerrain->m_sHeightMap.pVB->Lock(nStart*sizeof(TERRAINVERTEX), nCount*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
		{
			for(x=psStartPos->x ; x<psEndPos->x ; x++)
			{
				vNormal = m_pcTerrain->m_sHeightMap.GetNormal(x, z) ;
				//TRACE("prev normal(%07.03f %07.03f %07.03f) ", enumVector(m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal)) ;
				//TRACE("will change normal(%07.03f %07.03f %07.03f)\r\n", enumVector(vNormal)) ;
				m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
			}
			memcpy(pVertices, &m_pcTerrain->m_sHeightMap.pVertices[nStart], sizeof(TERRAINVERTEX)*nCount) ;
			m_pcTerrain->m_sHeightMap.pVB->Unlock() ;
		}
	}
}
void CSecretTerrainModifier::_ModifyTerrain_LButton(int nKind)
{
	float cx, cz ;
	cx = (m_vCenter.x+(float)m_pcTerrain->m_sHeightMap.lWidth/2.0f)/m_pcTerrain->m_sHeightMap.nTileSize ;
	cz = (-m_vCenter.z+(float)m_pcTerrain->m_sHeightMap.lHeight/2.0f)/m_pcTerrain->m_sHeightMap.nTileSize ;

	int nVertexWidth, nVertexHeight ;
	nVertexWidth = m_pcTerrain->m_sHeightMap.nVertexWidth ;
	nVertexHeight = m_pcTerrain->m_sHeightMap.nVertexHeight ;

	int x1, x2, z1, z2, r = (int)(m_fRadius/m_pcTerrain->m_sHeightMap.nTileSize+0.5f) ;//반지름도 타일사이즈만큼 나누어 준다.
	m_pcTerrain->m_sHeightMap.GetVertexPos(m_vCenter.x, m_vCenter.z, x1, x2, z1, z2) ;//이 함수 이후로는 x1, x2, z1, z2 가 실제 좌표가 아니라 타일배열의 번호다.
	x1 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	x2 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	z1 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	z2 /= m_pcTerrain->m_sHeightMap.nTileSize ;

	x1 = x1-r ;
	if(x1 < 0)	x1 = 0 ;
	x2 = x2+r ;
	if(x2 > nVertexWidth)	x2 = nVertexWidth ;

	z1 = z1-r ;
	if(z1 < 0)	z1 = 0 ;
	z2 = z2+r ;
	if(z2 > nVertexHeight)	z2 = nVertexHeight ;

	int x, z, nCount=0, nStart ;
	float h ;

	if(nKind == KIND_RAISE || nKind == KIND_LOWER)
	{
		for(z=z1 ; z<z2 ; z++)
		{
			nStart = z*nVertexWidth+x1 ;
			nCount = 0 ;
			for(x=x1 ; x<x2 ; x++, nCount++)
			{
				float a, b, l ;
				a = x*(float)m_pcTerrain->m_sHeightMap.nTileSize-(float)m_pcTerrain->m_sHeightMap.lWidth*0.5f ;
				b = -z*(float)m_pcTerrain->m_sHeightMap.nTileSize+(float)m_pcTerrain->m_sHeightMap.lHeight*0.5f ;
				a = fabs(a-m_vCenter.x) ;
				b = fabs(b-m_vCenter.z) ;
				l = sqrtf(a*a + b*b) ;
				if(float_less_eq(l, m_fRadius))
				{
					h = m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;

					if(m_nStrengthType == STRENGTHTYPE_DIRECTLY)
						l = m_fStrength ;
					else if(m_nStrengthType == STRENGTHTYPE_GAUSSIAN)
						l = GaussianFunc((float)x-cx, (float)z-cz, m_fArea, m_fStrength) ;

					m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, h+l) ;

					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y = h+l ;
				}
			}
		}

		z1 -= r ;
		if(z1 < 0) z1=0 ;
		z2 += r ;
		if(z2 >= nVertexHeight) z2 = nVertexHeight-1 ;
		x1 -= r ;
		if(x1 < 0) x1 = 0 ;
		x2 += r ;
		if(x2 >= nVertexWidth) x2 = nVertexWidth-1 ;
		Vector3 vNormal ;
		TERRAINVERTEX *pVertices=NULL ;
		for(z=z1 ; z<z2 ; z++)
		{
			pVertices=NULL ;
			nStart = z*nVertexWidth+x1 ;
			nCount = x2-x1 ;
			if(!FAILED(m_pcTerrain->m_sHeightMap.pVB->Lock(nStart*sizeof(TERRAINVERTEX), nCount*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
			{
				for(x=x1 ; x<x2 ; x++)
				{
					vNormal = m_pcTerrain->m_sHeightMap.GetNormal(x, z) ;
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
				}
				memcpy(pVertices, &m_pcTerrain->m_sHeightMap.pVertices[nStart], sizeof(TERRAINVERTEX)*nCount) ;
				m_pcTerrain->m_sHeightMap.pVB->Unlock() ;
			}
		}
	}
	else if(nKind == KIND_SMOOTH)
	{
		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++)
			{
				//h=0.0f ;
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x-1, z) ;
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x+1, z) ;
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z-1) ;
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z+1) ;
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
				//h *= 0.2f ;
				//h = float_round(h) ;
				//TRACE("prev height=%07.03f", m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
				//TRACE("will change height=%07.03f\r\n", h) ;
				h = _AvgHeight(x, z, 3) ;
				float l = h - m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y ;
				l *= m_fSmoothStrength ;
				m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y += l ;
			}
		}
		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++)
				m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
		}

        z1 -= r ;
		if(z1 < 0) z1=0 ;
		z2 += r ;
		if(z2 >= nVertexHeight) z2 = nVertexHeight-1 ;
		x1 -= r ;
		if(x1 < 0) x1 = 0 ;
		x2 += r ;
		if(x2 >= nVertexWidth) x2 = nVertexWidth-1 ;
		Vector3 vNormal ;
		TERRAINVERTEX *pVertices=NULL ;
		for(z=z1 ; z<z2 ; z++)
		{
			pVertices=NULL ;
			nStart = z*nVertexWidth+x1 ;
			nCount = x2-x1 ;
			if(!FAILED(m_pcTerrain->m_sHeightMap.pVB->Lock(nStart*sizeof(TERRAINVERTEX), nCount*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
			{
				for(x=x1 ; x<x2 ; x++)
				{
					vNormal = m_pcTerrain->m_sHeightMap.GetNormal(x, z) ;
					//TRACE("prev normal(%07.03f %07.03f %07.03f) ", enumVector(m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal)) ;
					//TRACE("will change normal(%07.03f %07.03f %07.03f)\r\n", enumVector(vNormal)) ;
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
				}
				memcpy(pVertices, &m_pcTerrain->m_sHeightMap.pVertices[nStart], sizeof(TERRAINVERTEX)*nCount) ;
				m_pcTerrain->m_sHeightMap.pVB->Unlock() ;
			}
		}
	}
	else if(nKind == KIND_FLATTEN)
	{
		h=0.0f ;
		nCount=0 ;
		m_nDisallowCount=0 ;

		for(z=z1+2 ; z<z2-2 ; z++)
		{
			for(x=x1+2 ; x<x2-2 ; x++)
			{
				float a, b, l ;
				a = x*(float)m_pcTerrain->m_sHeightMap.nTileSize-(float)m_pcTerrain->m_sHeightMap.lWidth*0.5f ;
				b = -z*(float)m_pcTerrain->m_sHeightMap.nTileSize+(float)m_pcTerrain->m_sHeightMap.lHeight*0.5f ;
				a = fabs(a-m_vCenter.x) ;
				b = fabs(b-m_vCenter.z) ;
				l = sqrtf(a*a + b*b) ;
				l = float_round(l) ;
				if(float_less_eq(l, m_fRadius))
					m_psDisallowPos[m_nDisallowCount++].set(x, z) ;
			}
		}

		float fHeightAlignment ;
		float fLowest = m_pcTerrain->m_sHeightMap.GetHeightMap(x1+((x2-x1)/2), z1+((z2-z1)/2)) ;
		float fHighest = fLowest ;

		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++, nCount++)
			{
				//h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
				float a, b, l ;
				a = x*(float)m_pcTerrain->m_sHeightMap.nTileSize-(float)m_pcTerrain->m_sHeightMap.lWidth*0.5f ;
				b = -z*(float)m_pcTerrain->m_sHeightMap.nTileSize+(float)m_pcTerrain->m_sHeightMap.lHeight*0.5f ;
				a = fabs(a-m_vCenter.x) ;
				b = fabs(b-m_vCenter.z) ;
				l = sqrtf(a*a + b*b) ;
				l = float_round(l) ;

				if(float_less_eq(l, m_fRadius))
				{
					h = m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
					if(fLowest > h)
						fLowest = h ;
					if(fHighest < h)
						fHighest = h ;
				}
			}
		}
		if(!nCount)
			return ;

		//h = float_round(h/(float)nCount) ;

		if(m_nFlattenAlign == LOWEST)
			fHeightAlignment = fLowest ;
		else if(m_nFlattenAlign == MIDDLE)
			fHeightAlignment = (fHighest+fLowest)*0.5f ;
		else if(m_nFlattenAlign == HIGHEST)
			fHeightAlignment = fHighest ;

		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++)
			{
				float a, b, l ;
				a = x*(float)m_pcTerrain->m_sHeightMap.nTileSize-(float)m_pcTerrain->m_sHeightMap.lWidth*0.5f ;
				b = -z*(float)m_pcTerrain->m_sHeightMap.nTileSize+(float)m_pcTerrain->m_sHeightMap.lHeight*0.5f ;
				a = fabs(a-m_vCenter.x) ;
				b = fabs(b-m_vCenter.z) ;
				l = sqrtf(a*a + b*b) ;
				l = float_round(l) ;
				if(float_less_eq(l, m_fRadius))
				{
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y = fHeightAlignment ;
					m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
				}
			}
		}

		z1 -= r ;
		if(z1 < 0) z1=0 ;
		z2 += r ;
		if(z2 >= nVertexHeight) z2 = nVertexHeight-1 ;
		x1 -= r ;
		if(x1 < 0) x1 = 0 ;
		x2 += r ;
		if(x2 >= nVertexWidth) x2 = nVertexWidth-1 ;

		Vector3 vNormal ;
		TERRAINVERTEX *pVertices=NULL ;
		for(z=z1 ; z<z2 ; z++)
		{
			pVertices=NULL ;
			nStart = z*nVertexWidth+x1 ;
			nCount = x2-x1 ;
			if(!FAILED(m_pcTerrain->m_sHeightMap.pVB->Lock(nStart*sizeof(TERRAINVERTEX), nCount*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
			{
				for(x=x1 ; x<x2 ; x++)
				{
					vNormal = m_pcTerrain->m_sHeightMap.GetNormal(x, z) ;
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
				}
				memcpy(pVertices, &m_pcTerrain->m_sHeightMap.pVertices[nStart], sizeof(TERRAINVERTEX)*nCount) ;
				m_pcTerrain->m_sHeightMap.pVB->Unlock() ;
			}
		}

		//STerrainTilePos sStartPos(x1, z1), sEndPos(x2, z2) ;
  //      _SmoothAround(&sStartPos, &sEndPos, m_psDisallowPos, m_nDisallowCount) ;
		//sStartPos.set(x1, z1), sEndPos.set(x2, z2) ;
		//_SmoothAround(&sStartPos, &sEndPos, m_psDisallowPos, m_nDisallowCount) ;
		//sStartPos.set(x1, z1), sEndPos.set(x2, z2) ;
		//_SmoothAround(&sStartPos, &sEndPos, m_psDisallowPos, m_nDisallowCount) ;
	}
	else if(nKind == KIND_RUGGED)
	{
        CPerlinNoise cPerlin ;
		cPerlin.Initialize(1.0f/2.0f, 8, (int)(m_fRadius+0.5f)) ;
		float fRuggedRange = m_fRuggedRange/2.0f ;
		cPerlin.SetRange(-fRuggedRange, fRuggedRange) ;

		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++, nCount++)
			{
				float a, b, l ;
				a = x*(float)m_pcTerrain->m_sHeightMap.nTileSize-(float)m_pcTerrain->m_sHeightMap.lWidth*0.5f ;
				b = -z*(float)m_pcTerrain->m_sHeightMap.nTileSize+(float)m_pcTerrain->m_sHeightMap.lHeight*0.5f ;
				a = fabs(a-m_vCenter.x) ;
				b = fabs(b-m_vCenter.z) ;
				l = sqrtf(a*a + b*b) ;
				l = float_round(l) ;

				if(float_less_eq(l, m_fRadius))
				{
					float rugged = cPerlin.PerlinNoise2D((float)(x-x1), (float)(z2-z)) ;
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y += rugged ;
					m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
				}
			}
		}

        //smooth
		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++)
			{
				h=0.0f ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x-1, z) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x+1, z) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z-1) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z+1) ;
				h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
				h *= 0.2f ;
				h = float_round(h) ;
				//TRACE("prev height=%07.03f", m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
				//TRACE("will change height=%07.03f\r\n", h) ;
				m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y = h ;
			}
		}
		for(z=z1 ; z<z2 ; z++)
		{
			for(x=x1 ; x<x2 ; x++)
				m_pcTerrain->m_sHeightMap.SetHeightMap(x, z, m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].pos.y) ;
		}

		Vector3 vNormal ;
		TERRAINVERTEX *pVertices=NULL ;
		for(z=z1 ; z<z2 ; z++)
		{
			pVertices=NULL ;
			nStart = z*nVertexWidth+x1 ;
			nCount = x2-x1 ;
			if(!FAILED(m_pcTerrain->m_sHeightMap.pVB->Lock(nStart*sizeof(TERRAINVERTEX), nCount*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
			{
				for(x=x1 ; x<x2 ; x++)
				{
					vNormal = m_pcTerrain->m_sHeightMap.GetNormal(x, z) ;
					//TRACE("prev normal(%07.03f %07.03f %07.03f) ", enumVector(m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal)) ;
					//TRACE("will change normal(%07.03f %07.03f %07.03f)\r\n", enumVector(vNormal)) ;
					m_pcTerrain->m_sHeightMap.pVertices[z*nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
				}
				memcpy(pVertices, &m_pcTerrain->m_sHeightMap.pVertices[nStart], sizeof(TERRAINVERTEX)*nCount) ;
				m_pcTerrain->m_sHeightMap.pVB->Unlock() ;
			}
		}
	}
	else if(nKind == KIND_HOLECUP)
	{
		if(!m_pcTerrain->m_sHolecup.bEnable)
		{
			if(m_pcTerrain->m_sHeightMap.GetIndex(m_vCenter.x, m_vCenter.z, &m_pcTerrain->m_sHeightMap.sExceptionalIndices) == 0)
				return ;

			D3DXMATRIX mat ;
			int nExceptionalIndex = m_pcTerrain->m_sHeightMap.sExceptionalIndices.nCurPos-1 ;
			TERRAININDEX *psIndex = m_pcTerrain->m_sHeightMap.sExceptionalIndices.GetAt(nExceptionalIndex) ;

			Vector3 v1, v2, v3, vMid ;
			v1.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.z) ;
			v2.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.z) ;
			v3.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.z) ;
			
			vMid.x = (v1.x+v2.x+v3.x)/3.0f ;
			vMid.y = (v1.y+v2.y+v3.y)/3.0f ;
			vMid.z = (v1.z+v2.z+v3.z)/3.0f ;

			//v1 -= vMid ;
			//v2 -= vMid ;
			//v3 -= vMid ;

			TRACE("tri[0](%10.07f %10.07f %10.07f)\r\n", enumVector(v1)) ;
			TRACE("tri[1](%10.07f %10.07f %10.07f)\r\n", enumVector(v2)) ;
			TRACE("tri[2](%10.07f %10.07f %10.07f)\r\n", enumVector(v3)) ;

			geo::STriangle tri ;

			tri.set(v1, v2, v3) ;
			tri.avVertex[0].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.z) ;
			tri.avVertex[1].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.z) ;
			tri.avVertex[2].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.z) ;

			tri.avVertex[0].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].t.y) ;
			tri.avVertex[1].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].t.y) ;
			tri.avVertex[2].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].t.y) ;

            Vector3 vAxis(0, 1, 0), vPlaneNormal(tri.sPlane.a, tri.sPlane.b, tri.sPlane.c) ;            
			if(!vector_eq(vAxis, vPlaneNormal))
			{
				Matrix4 matRot ;
				Quaternion q(vAxis, vPlaneNormal) ;
				q.GetMatrix(matRot) ;
				//vMid = tri.GetMidPos() ;
				matRot.m41 = vMid.x ;
				matRot.m42 = vMid.y ;
				matRot.m43 = vMid.z ;
				mat = MatrixConvert(matRot) ;
			}
			else
			{
				D3DXMatrixIdentity(&mat) ;
				mat._41 = vMid.x ;
				mat._42 = vMid.y ;
				mat._43 = vMid.z ;
			}

			m_pcTerrain->m_sHolecup.Initialize(m_pd3dDevice, "../../Media/GroundMap/holecup.RMD", &mat, m_pcTerrain->m_pcTexContainer->FindTexture("holecup_1d"), &tri, nExceptionalIndex, m_pcTerrain) ;
			//m_pcTerrain->m_sHolecup.Initialize(m_pd3dDevice, "../../Media/objectlist/3_parts/holecup_test.RMD", &mat, m_pcTerrain->m_pcTexContainer->FindTexture("green"), &tri, nExceptionalIndex) ;
		}
	}
}
void CSecretTerrainModifier::_ModifyTerrain_RButton(int nKind)
{
	float cx, cz ;
	cx = (m_vCenter.x+(float)m_pcTerrain->m_sHeightMap.lWidth/2.0f)/m_pcTerrain->m_sHeightMap.nTileSize ;
	cz = (-m_vCenter.z+(float)m_pcTerrain->m_sHeightMap.lHeight/2.0f)/m_pcTerrain->m_sHeightMap.nTileSize ;

	int nVertexWidth, nVertexHeight ;
	nVertexWidth = m_pcTerrain->m_sHeightMap.nVertexWidth ;
	nVertexHeight = m_pcTerrain->m_sHeightMap.nVertexHeight ;

	int x1, x2, z1, z2, r = (int)(m_fRadius/m_pcTerrain->m_sHeightMap.nTileSize+0.5f) ;//반지름도 타일사이즈만큼 나누어 준다.
	m_pcTerrain->m_sHeightMap.GetVertexPos(m_vCenter.x, m_vCenter.z, x1, x2, z1, z2) ;//이 함수 이후로는 x1, x2, z1, z2 가 실제 좌표가 아니라 타일배열의 번호다.
	x1 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	x2 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	z1 /= m_pcTerrain->m_sHeightMap.nTileSize ;
	z2 /= m_pcTerrain->m_sHeightMap.nTileSize ;

	x1 = x1-r ;
	if(x1 < 0)	x1 = 0 ;
	x2 = x2+r ;
	if(x2 > nVertexWidth)	x2 = nVertexWidth ;

	z1 = z1-r ;
	if(z1 < 0)	z1 = 0 ;
	z2 = z2+r ;
	if(z2 > nVertexHeight)	z2 = nVertexHeight ;

	//int x, z, nCount=0, nStart ;
	//float h ;

	if(nKind == KIND_HOLECUP)
	{
		data::SKeepingArray<TERRAININDEX> sIndices ;
		sIndices.Initialize(MAXNUM_EXCEPTIONALINDEX) ;
		m_pcTerrain->m_sHeightMap.GetIndex(m_vCenter.x, m_vCenter.z, &sIndices) ;

		int i, n ;
		for(i=0 ; i<sIndices.nCurPos ; i++)
		{
			for(n=0 ; n<m_pcTerrain->m_sHeightMap.sExceptionalIndices.nCurPos ; n++)
			{
				//if(sIndices.GetAt(i)->IsEqual(*m_pcTerrain->m_sHeightMap.sExceptionalIndices.GetAt(n)))
				{
					m_pcTerrain->m_sHeightMap.sExceptionalIndices.Delete(n) ;
					if(n == m_pcTerrain->m_sHolecup.nExceptionalIndex)
						m_pcTerrain->m_sHolecup.Release() ;
					break ;
				}
			}
		}
	}
}
void CSecretTerrainModifier::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
int CSecretTerrainModifier::GetAttr()
{
	return m_nAttr ;
}
bool CSecretTerrainModifier::IsAttr(int nAttr)
{
	if(m_nAttr & nAttr)
		return true ;
	return false ;
}
void CSecretTerrainModifier::CreateHolecup(Vector3 vPos)
{
	if(m_pcTerrain->m_sHolecup.bEnable)
		return ;

	m_pcTerrain->m_sHeightMap.GetIndex(vPos.x, vPos.z, &m_pcTerrain->m_sHeightMap.sExceptionalIndices) ;

	D3DXMATRIX mat ;
	int nExceptionalIndex = m_pcTerrain->m_sHeightMap.sExceptionalIndices.nCurPos-1 ;
	TERRAININDEX *psIndex = m_pcTerrain->m_sHeightMap.sExceptionalIndices.GetAt(nExceptionalIndex) ;

	Vector3 v1, v2, v3, vMid ;
	v1.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].pos.z) ;
	v2.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].pos.z) ;
	v3.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].pos.z) ;

	vMid.x = (v1.x+v2.x+v3.x)/3.0f ;
	vMid.y = (v1.y+v2.y+v3.y)/3.0f ;
	vMid.z = (v1.z+v2.z+v3.z)/3.0f ;

	//v1 -= vMid ;
	//v2 -= vMid ;
	//v3 -= vMid ;

	TRACE("tri[0](%10.07f %10.07f %10.07f)\r\n", enumVector(v1)) ;
	TRACE("tri[1](%10.07f %10.07f %10.07f)\r\n", enumVector(v2)) ;
	TRACE("tri[2](%10.07f %10.07f %10.07f)\r\n", enumVector(v3)) ;

	geo::STriangle tri ;

	tri.set(v1, v2, v3) ;
	tri.avVertex[0].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].normal.z) ;
	tri.avVertex[1].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].normal.z) ;
	tri.avVertex[2].vNormal.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.y, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].normal.z) ;

	tri.avVertex[0].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i1].t.y) ;
	tri.avVertex[1].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i2].t.y) ;
	tri.avVertex[2].vTex.set(m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].t.x, m_pcTerrain->m_sHeightMap.pVertices[psIndex->i3].t.y) ;

	Vector3 vAxis(0, 1, 0), vPlaneNormal(tri.sPlane.a, tri.sPlane.b, tri.sPlane.c) ;            
	if(!vector_eq(vAxis, vPlaneNormal))
	{
		Matrix4 matRot ;
		Quaternion q(vAxis, vPlaneNormal) ;
		q.GetMatrix(matRot) ;
		//vMid = tri.GetMidPos() ;
		matRot.m41 = vMid.x ;
		matRot.m42 = vMid.y ;
		matRot.m43 = vMid.z ;
		mat = MatrixConvert(matRot) ;
	}
	else
	{
		D3DXMatrixIdentity(&mat) ;
		mat._41 = vMid.x ;
		mat._42 = vMid.y ;
		mat._43 = vMid.z ;
	}

	m_pcTerrain->m_sHolecup.Initialize(m_pd3dDevice, "../../Media/GroundMap/holecup.RMD", &mat, m_pcTerrain->m_pcTexContainer->FindTexture("holecup_1d"), &tri, nExceptionalIndex, m_pcTerrain) ;
	//m_pcTerrain->m_sHolecup.Initialize(m_pd3dDevice, "../../Media/objectlist/3_parts/holecup_test.RMD", &mat, m_pcTerrain->m_pcTexContainer->FindTexture("green"), &tri, nExceptionalIndex) ;
}

float CSecretTerrainModifier::_AvgHeight(int x, int z, int nSize)
{
	int x1, x2, z1, z2, nHalfSize ;

	nHalfSize = nSize/2 ;
	if(nHalfSize <= 0)
		nHalfSize = 1 ;

	x1 = x-nHalfSize ;
	x2 = x+nHalfSize ;
	z1 = z-nHalfSize ;
	z2 = z+nHalfSize ;

	int nCount=0 ;
	float h=0.0f ;
	for(z=z1 ; z<=z2 ; z++)
	{
		for(x=x1 ; x<=x2 ; x++, nCount++)
			h += m_pcTerrain->m_sHeightMap.GetHeightMap(x, z) ;
	}

	h *= (1.0f/(float)nCount) ;
	return float_round(h) ;
}

void CSecretTerrainModifier::SetSmoothStr(int nStr)
{
	m_fSmoothStrength = (float)nStr*0.01f ;
}
float CSecretTerrainModifier::GetSmoothStr()
{
	return m_fSmoothStrength ;
}