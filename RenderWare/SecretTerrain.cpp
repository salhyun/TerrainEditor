#include "SecretTerrain.h"
#include "MathOrdinary.h"
#include "Dib.h"
#include "def.h"
#include "GeoLib.h"
#include "ASEData.h"
#include "DataStructLib.h"
#include "SecretToolBrush.h"
#include "SecretShadowMap.h"
#include "SecretMirror.h"
#include "D3DMathLib.h"
#include <algorithm>
#include "time.h"

bool TERRAININDEX::IsEqual(TERRAININDEX index)
{
	if((this->i1 == index.i1) && (this->i2 == index.i2) && (this->i3 == index.i3))
		return true ;
	return false ;
}
//최초 trn파일 로드할때 사용
char *pszTerrainLayerOder[] = 
{
	"rough", "hazard", "tee", "fairway", "bunker", "green", "greenside", "roughside"
} ;

//It took four spaces as QuadTree
//Function Divide a cube into four spaces
void QuadDivide(geo::SAACube *pDivideCube, geo::SAACube *pCube, int nNumSpace)
{
	Vector3 vCenterPos ;

	switch(nNumSpace)
	{
	case 0 : //left-top
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 1 ://right-top
        vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 2 ://left-bottom
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;

	case 3 ://right-bottom
		vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;
	} ;

	pDivideCube->set(vCenterPos, pCube->fWidth/2.0f, pCube->fHeight, pCube->fDepth/2.0f) ;
}

void OctDivide(geo::SAACube *pDivideCube, geo::SAACube *pCube, int nNumSpace)
{
	Vector3 vCenterPos ;

	switch(nNumSpace)
	{
	case 0 : //upper-left-top
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y+pCube->fHeight/4.0f, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 1 ://upper-right-top
        vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y+pCube->fHeight/4.0f, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 2 ://upper-left-bottom
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y+pCube->fHeight/4.0f, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;

	case 3 ://upper-right-bottom
		vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y+pCube->fHeight/4.0f, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;

	case 4 : //lower-left-top
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y-pCube->fHeight/4.0f, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 5 ://lower-right-top
		vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y-pCube->fHeight/4.0f, pCube->vCenter.z+pCube->fDepth/4.0f) ;
		break ;

	case 6 ://lower-left-bottom
		vCenterPos.set(pCube->vCenter.x-pCube->fWidth/4.0f, pCube->vCenter.y-pCube->fHeight/4.0f, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;

	case 7 ://lower-right-bottom
		vCenterPos.set(pCube->vCenter.x+pCube->fWidth/4.0f, pCube->vCenter.y-pCube->fHeight/4.0f, pCube->vCenter.z-pCube->fDepth/4.0f) ;
		break ;

	} ;

	pDivideCube->set(vCenterPos, pCube->fWidth/2.0f, pCube->fHeight/2.0f, pCube->fDepth/2.0f) ;
}

void CSecretTerrainScatteringData::CalculateShaderParameterTerrainOptimized()
{
	//m_shaderParams.vSumBeta1Beta2 *= 0.1f ;
	m_shaderParams.vBetaD1 *= 0.1f ;
	m_shaderParams.vBetaD2 *= 0.1f ;
	//m_shaderParams.vRcpSumBeta1Beta2 *= 15.0f ;
	//m_shaderParams.vTermMultipliers.x *= 1.2f;
	m_shaderParams.vTermMultipliers.y *= 1.2f;

    //20090421 임의수정
	//m_shaderParams.vTermMultipliers.x = 1.484f ;
	//m_shaderParams.vTermMultipliers.x = 1.15f ;

	//20090907 수정
	//m_shaderParams.vTermMultipliers.x = 1.3911f ;
	//20100320 수정
	m_shaderParams.vTermMultipliers.x = 1.6f ;

	TRACE("### Terrain Scattering Parameters ###\r\n") ;
	TRACE("BetaD1(%10.07f %10.07f %10.07f %10.07f)\r\n", m_shaderParams.vBetaD1.x, m_shaderParams.vBetaD1.y, m_shaderParams.vBetaD1.z, m_shaderParams.vBetaD1.w) ;
	TRACE("BetaD2(%10.07f %10.07f %10.07f %10.07f)\r\n", m_shaderParams.vBetaD2.x, m_shaderParams.vBetaD2.y, m_shaderParams.vBetaD2.z, m_shaderParams.vBetaD2.w) ;
	TRACE("TermMultipliers(%10.07f %10.07f %10.07f %10.07f)\r\n", m_shaderParams.vTermMultipliers.x, m_shaderParams.vTermMultipliers.y, m_shaderParams.vTermMultipliers.z, m_shaderParams.vTermMultipliers.w) ;
	TRACE("\r\n") ;
}

STerrainMesh::STerrainMesh()
{
	pVB = NULL ;
	pIB = NULL ;
	ppTexDecal = NULL ;
	psVertices = NULL ;
	ppsIndices = NULL ;
	psPolygons = NULL ;
	plNumIndex = NULL ;
	pIndexSet = NULL ;
	pIndices = NULL ;
	lNumVertex = 0 ;
}

STerrainMesh::~STerrainMesh()
{
	this->Release() ;
}

void STerrainMesh::Release()
{
	SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;

	UINT i ;
	if(ppTexDecal)
	{
		for(i=0 ; i<lNumTexture ; i++)
			SAFE_RELEASE(ppTexDecal[i]) ;
	}

	SAFE_DELETEARRAY(ppTexDecal) ;

	SAFE_DELETEARRAY(psVertices) ;

	if(ppsIndices)
	{
		for(i=0 ; i<lNumTexture ; i++)
			SAFE_DELETEARRAY(ppsIndices[i]) ;
	}

	SAFE_DELETEARRAY(ppsIndices) ;
	SAFE_DELETEARRAY(psPolygons) ;

	SAFE_DELETEARRAY(plNumIndex) ;

	SAFE_DELETEARRAY(pIndexSet) ;
	SAFE_DELETEARRAY(pIndices) ;
}

//#####################################################################################################//
//                                             TERRAINMAP                                              //
//#####################################################################################################//

TERRAINMAP::TERRAINMAP()
{
	bEnable = false ;
	pVB = NULL ;
	pIB = NULL ;
	pVertices = NULL ;
	pIndices = NULL ;
	psOriginIndices = NULL ;
	psFragmentInfo = NULL ;
	psMaterialInfo = NULL ;
	ppfHeightMap = NULL ;
	psTriangles = NULL ;
	pcActionContainer = NULL ;
	pcActionManagement = NULL ;

	pTexBase = NULL ;
	pTexLight = NULL ;
	for(int i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
	{
        apTexLayer[i] = NULL ;
		apTexAlpha[i] = NULL ;
        apSaveKeepingAlpha[i] = NULL ;

		asStrLayerName[i].format("") ;
	}

	pTexNoise00 = NULL ;
	pTexNoise01 = NULL ;

	pcActionContainer = NULL ;
	lWidth = lHeight = nVertexWidth = nVertexHeight = 0 ;
	nBrushDecision = 0 ;
	nAlphaMapSize = 0 ;
	nTileSize = 0 ;
	nNumDrawVLine = 0 ;
	pbDrawVLine = NULL ;
	nIndex = 0 ;
	fBrushStrength = 50.0f ;

	//afRestitution[0] = 0.6f ;	afFriction[0] = 1.45f ;//ob
	//afRestitution[1] = 0.61f ;	afFriction[1] = 1.35f ;//rough
	//afRestitution[2] = 0.625f ;	afFriction[2] = 0.8f ;//hazard
	//afRestitution[3] = 0.625f ;	afFriction[3] = 1.0f ;//tee
	//afRestitution[4] = 0.625f ;	afFriction[4] = 0.8f ;//fairway
	//afRestitution[5] = 0.2f ;	afFriction[5] = 2.5f ;//bunker
	//afRestitution[6] = 0.625f ;	afFriction[6] = 0.13f ;//green
	//afRestitution[7] = 0.625f ;	afFriction[7] = 0.99f ;//greenside
	//afRestitution[8] = 0.61f ;	afFriction[8] = 1.35f ;//roughside

	asContactSurfaceInfo[0].set(0, 0.6f, 1.45f, 1.0f) ;//ob
	asContactSurfaceInfo[1].set(1, 0.61f, 1.35f, 1.0f) ;//rough
	asContactSurfaceInfo[2].set(2, 0.625f, 0.8f, 1.0f) ;//hazard
	asContactSurfaceInfo[3].set(3, 0.625f, 1.0f, 1.0f) ;//tee
	asContactSurfaceInfo[4].set(4, 0.625f, 0.8f, 1.0f) ;//fairway
	asContactSurfaceInfo[5].set(5, 0.2f, 2.5f, 1.0f) ;//bunker
	asContactSurfaceInfo[6].set(6, 0.625f, 0.13f, 1.0f) ;//green
	asContactSurfaceInfo[7].set(7, 0.625f, 0.99f, 1.0f) ;//greenside
	asContactSurfaceInfo[8].set(8, 0.61f, 1.35f, 1.0f) ;//roughside

	restitutionOB = 0.625f ;
	frictionOB = 1.2f ;
	bOverapBrush = false ;

	fMaxElevation = 0.0f ;
	fMinElevation = 0.0f ;

	ImportContactSurfaceInfo("../../Media/collisionData/SurfaceParameters.surfaceinfo") ;

	sExceptionalIndices.Initialize(MAXNUM_EXCEPTIONALINDEX) ;
	psHolecup = NULL ;
}
TERRAINMAP::~TERRAINMAP()
{
	this->Release() ;
}
void TERRAINMAP::Release()
{
	int i ;

	SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;
	SAFE_DELETEARRAY(pVertices) ;
	SAFE_DELETEARRAY(pIndices) ;
	SAFE_DELETEARRAY(psOriginIndices) ;
	SAFE_DELETEARRAY(psTriangles) ;
	SAFE_DELETEARRAY(psMaterialInfo) ;
	SAFE_DELETEARRAY(psFragmentInfo) ;

	if(ppfHeightMap)
	{
		for(i=0 ; i<(int)((lHeight/nTileSize)+1) ; i++)
			SAFE_DELETEARRAY(ppfHeightMap[i]) ;
		SAFE_DELETEARRAY(ppfHeightMap) ;
	}

	//SAFE_RELEASE(pTexBase) ;
	SAFE_RELEASE(pTexLight) ;
	
	//for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
	//	SAFE_RELEASE(apTexLayer[i]) ;

	for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
	{
		SAFE_RELEASE(apSaveKeepingAlpha[i]) ;

		asStrLayerName[i].format("") ;
	}

	SAFE_DELETE(pcActionContainer) ;
    SAFE_DELETEARRAY(pbDrawVLine) ;

	sExceptionalIndices.Reset() ;
}
/*
void DrawHorizonLine(DWORD *pdw, int nX1, int nX2, int nY, DWORD dwColor, geo::SPixelCircle *psCircle, int nBrushDecision)
{
	int dx, dy ;
	int l ;
	int nColor ;
	//int nCoef = (int)((255.0f/psCircle->nRadius*0.25f)+0.5f) ;
	float fCoef = 255.0f/psCircle->nRadius*0.25f ;
	//int dist = (int)((psCircle->nRadius*0.9f)+0.5f) ;

	for(int i=nX1 ; i<nX2 ; i++)
	{
		dx = abs(psCircle->cx-i) ;
		dy = abs(psCircle->cy-nY) ;
		l = (int)(sqrtf((float)(dx*dx + dy*dy))+0.5f) ;
		nColor = (int)(((psCircle->nRadius-l)*fCoef)+0.5f) ;
		nColor = (nColor>=255) ? 255 : nColor ;
		//nColor = (int)((250.0f/100.0f)*l*l) ;
		//nColor = (int)(((-250.0f/(50.0f*50.0f))*l*l)+255.0f) ;
		dwColor = ((nColor<<16)&0xff0000) | ((nColor<<8)&0xff00) | (nColor&0xff) ;

		if(nBrushDecision == 1)//Coloring Brush
		{        
			DWORD dw = (pdw[i]&0xff) + (nColor&0xff) ;
			if(dw >= 0xff)
				pdw[i] = 0xffffffff ;
			else
				pdw[i] += dwColor ;
		}
		else if(nBrushDecision == 2)//Erasing Brush
		{        
			int temp = (pdw[i]&0xff) - (nColor&0xff) ;
			if(temp <= 0)
				pdw[i] = 0 ;
			else
				pdw[i] -= dwColor ;
		}
	}
}

void HorizonLine(DWORD *pdw, int nWidth, int nXStart, int nXEnd, int nYRow, DWORD dwColor, geo::SPixelCircle *psCircle, int nAlphaMapSize, int nBrushDecision)
{
    if( (nYRow>=0) && (nYRow<nAlphaMapSize) )
	{
		if(nXStart < 0)
			nXStart = 0 ;
		if(nXEnd >= nAlphaMapSize)
			nXEnd = nAlphaMapSize-1 ;

		DrawHorizonLine(&pdw[nYRow*nWidth], nXStart, nXEnd, nYRow, dwColor, psCircle, nBrushDecision) ;
	}
}
void TERRAINMAP::DrawCircleBrushing(int nTexNum, int cx, int cy, int R, DWORD dwColor)
{
	int nWidth ;
	D3DLOCKED_RECT d3drect ;
	DWORD *pdw ;
	//apTexAlpha[nTexNum]->LockRect(0, &d3drect, NULL, D3DLOCK_DISCARD) ;
	apTexAlpha[nTexNum]->LockRect(0, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	apTexAlpha[nTexNum]->AddDirtyRect(NULL) ;
	pdw = (DWORD *)d3drect.pBits ;
	nWidth = d3drect.Pitch/4 ;

	geo::SPixelCircle sCircle ;
	sCircle.cx = cx ;
	sCircle.cy = cy ;
	sCircle.nRadius = R ;
    int x=0, y=R, h, nDeltaE, nDeltaNE ;
    h = 1-R ;
	nDeltaE = 3 ;
	nDeltaNE = 5-2*R ;

	static bool bVLine[MAXSIZE_ALPHAMAP] ;
	ZeroMemory(bVLine, sizeof(bool)*MAXSIZE_ALPHAMAP) ;

	int nYRow ;

    while(y >= x)
	{
		nYRow = y+cy ;
		if(!bVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -x+cx, x+cx, nYRow, dwColor, &sCircle, nAlphaMapSize, nBrushDecision) ;
			bVLine[nYRow] = true ;
		}
		nYRow = x+cy ;
		if(!bVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -y+cx, y+cx, nYRow, dwColor, &sCircle, nAlphaMapSize, nBrushDecision) ;
			bVLine[nYRow] = true ;
		}
        nYRow = -x+cy ;
		if(!bVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -y+cx, y+cx, nYRow, dwColor, &sCircle, nAlphaMapSize, nBrushDecision) ;
			bVLine[nYRow] = true ;
		}
		nYRow = -y+cy ;
		if(!bVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -x+cx, x+cx, nYRow, dwColor, &sCircle, nAlphaMapSize, nBrushDecision) ;
			bVLine[nYRow] = true ;
		}

		//nYRow = y+cy ;
		//if(!bVLine[nYRow] && (nYRow>=0) && (nYRow<512))
		//{
		//	DrawHorizonLine(&pdw[nYRow*nWidth], -x+cx, x+cx, nYRow, dwColor, &sCircle) ;
		//	bVLine[nYRow] = true ;
		//}

  //      nYRow = x+cy ;
		//if(!bVLine[nYRow] && (nYRow>=0) && (nYRow<512))
		//{
		//	DrawHorizonLine(&pdw[nYRow*nWidth], -y+cx, y+cx, nYRow, dwColor, &sCircle) ;
		//	bVLine[nYRow] = true ;
		//}

  //      nYRow = -x+cy ;
		//if(!bVLine[nYRow] && (nYRow>=0) && (nYRow<512))
		//{
		//	DrawHorizonLine(&pdw[nYRow*nWidth], -y+cx, y+cx, nYRow, dwColor, &sCircle) ;
		//	bVLine[nYRow] = true ;
		//}

		//nYRow = -y+cy ;
		//if(!bVLine[nYRow] && (nYRow>=0) && (nYRow<512))
		//{
		//	DrawHorizonLine(&pdw[nYRow*nWidth], -x+cx, x+cx, nYRow, dwColor, &sCircle) ;
		//	bVLine[nYRow] = true ;
		//}

		if(h<0)
		{
			h += nDeltaE ;
			nDeltaE += 2 ;
			nDeltaNE += 2 ;
		}
		else
		{
			h += nDeltaNE ;
			nDeltaE += 2 ;
			nDeltaNE += 4 ;
			y-- ;
		}
		x++ ;
	}
	apTexAlpha[nTexNum]->UnlockRect(0) ;
}
*/
void DrawHorizonLine(DWORD *pdw, int nX1, int nX2, int nY, BYTE byColor, bool bOverap, float fStrength, int nShift, geo::SPixelCircle *psCircle, int nBrushDecision)
{
	int dx, dy ;
	int l ;
	int nColor ;
	float d, diff, color ;
	//int nCoef = (int)((255.0f/psCircle->nRadius*0.25f)+0.5f) ;
	//float fCoef = 255.0f/psCircle->nRadius*0.25f ;
	//int dist = (int)((psCircle->nRadius*0.9f)+0.5f) ;

	for(int i=nX1 ; i<nX2 ; i++)
	{
		dx = abs(psCircle->cx-i) ;
		dy = abs(psCircle->cy-nY) ;

		//l = (int)(sqrtf((float)(dx*dx + dy*dy))+0.5f) ;
		//nColor = (int)(((psCircle->nRadius-l)*fCoef)+0.5f) ;

		d = sqrtf((float)(dx*dx + dy*dy)) ;
		if(d > (float)psCircle->nRadius)
			nColor = 0 ;
		else
		{
			diff = (float)psCircle->nRadius - d ;
			color = diff * fStrength/(float)psCircle->nRadius ;
			nColor = (int)(color+0.5f) ;
			nColor = (nColor>=255) ? 255 : nColor ;
		}

		byColor = (BYTE)(nColor&0xff) ;

		if(nBrushDecision == 1)//Coloring Brush
		{
            DWORD dw = (pdw[i]>>nShift)&0xff ;
			dw += byColor ;

			if(!bOverap)
				pdw[i] &= ~(0xff<<nShift) ;

			if(dw >= 0xff)
				pdw[i] |= 0xff<<nShift ;
			else
				pdw[i] |= (DWORD)(dw<<nShift) ;
		}
		else if(nBrushDecision == 2)//Erasing Brush
		{
			int temp = (int)((pdw[i]>>nShift)&0xff) ;
			temp -= (int)byColor ;

			if(temp < 0)
				pdw[i] &= ~(0xff<<nShift) ;
			else
				pdw[i] -= (byColor<<nShift) ;
		}
	}
}
void HorizonLine(DWORD *pdw, int nWidth, int nXStart, int nXEnd, int nYRow, BYTE byColor, bool bOverap, float fStrength, int nShift, geo::SPixelCircle *psCircle, int nAlphaMapSize, int nBrushDecision)
{
    if( (nYRow>=0) && (nYRow<nAlphaMapSize) )
	{
		if(nXStart < 0)
			nXStart = 0 ;
		if(nXEnd >= nAlphaMapSize)
			nXEnd = nAlphaMapSize-1 ;

		DrawHorizonLine(&pdw[nYRow*nWidth], nXStart, nXEnd, nYRow, byColor, bOverap, fStrength, nShift, psCircle, nBrushDecision) ;
	}
}
void TERRAINMAP::DrawCircleBrushing(int nTexNum, int cx, int cy, int R, BYTE byColor)
{
	int nWidth ;
	D3DLOCKED_RECT d3drect ;
	//apTexAlpha[nTexNum]->LockRect(0, &d3drect, NULL, D3DLOCK_DISCARD) ;

	int anShift[] = {24, 16, 8, 0} ;

	int nShift = anShift[nTexNum%4] ;

	TRACE("drawing shift=%d\r\n", nShift) ;

	nTexNum /= 4 ;
	apTexAlpha[nTexNum]->LockRect(0, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	apTexAlpha[nTexNum]->AddDirtyRect(NULL) ;
	DWORD *pdw = (DWORD *)d3drect.pBits ;
	nWidth = d3drect.Pitch/4 ;

	geo::SPixelCircle sCircle ;
	sCircle.cx = cx ;
	sCircle.cy = cy ;
	sCircle.nRadius = R ;
    int x=0, y=R, h, nDeltaE, nDeltaNE ;
    h = 1-R ;
	nDeltaE = 3 ;
	nDeltaNE = 5-2*R ;

	ZeroMemory(pbDrawVLine, sizeof(bool)*nNumDrawVLine) ;

	int nYRow ;

    while(y >= x)
	{
		nYRow = y+cy ;
		if(!pbDrawVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -x+cx, x+cx, nYRow, byColor, bOverapBrush, fBrushStrength, nShift, &sCircle, nAlphaMapSize, nBrushDecision) ;
			pbDrawVLine[nYRow] = true ;
		}
		nYRow = x+cy ;
		if(!pbDrawVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -y+cx, y+cx, nYRow, byColor, bOverapBrush, fBrushStrength, nShift, &sCircle, nAlphaMapSize, nBrushDecision) ;
			pbDrawVLine[nYRow] = true ;
		}
        nYRow = -x+cy ;
		if(!pbDrawVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -y+cx, y+cx, nYRow, byColor, bOverapBrush, fBrushStrength, nShift, &sCircle, nAlphaMapSize, nBrushDecision) ;
			pbDrawVLine[nYRow] = true ;
		}
		nYRow = -y+cy ;
		if(!pbDrawVLine[nYRow])
		{
			HorizonLine(pdw, nWidth, -x+cx, x+cx, nYRow, byColor, bOverapBrush, fBrushStrength, nShift, &sCircle, nAlphaMapSize, nBrushDecision) ;
			pbDrawVLine[nYRow] = true ;
		}
		if(h<0)
		{
			h += nDeltaE ;
			nDeltaE += 2 ;
			nDeltaNE += 2 ;
		}
		else
		{
			h += nDeltaNE ;
			nDeltaE += 2 ;
			nDeltaNE += 4 ;
			y-- ;
		}
		x++ ;
	}
	apTexAlpha[nTexNum]->UnlockRect(0) ;
}
void TERRAINMAP::BrushingAlpha(int nNum, int nBrushCount, Vector2 vStart, Vector2 vEnd, float fRadius)
{
	int i ;
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	apTexAlpha[nNum/4]->GetLevelDesc(0, &desc) ;

	float t, d = fRadius/4.0f ;
    Vector2 s, e, v, p ;

	s.x = vStart.x*desc.Width ;
	s.y = vStart.y*desc.Height ;

	e.x = vEnd.x*desc.Width ;
	e.y = vEnd.y*desc.Height ;

	v = (e-s) ;
	t = v.Magnitude() ;
	if(float_eq(t, 0))
	{
		DrawCircleBrushing(nNum, (int)s.x, (int)s.y, (int)fRadius, 0xff) ;
		return ;
	}

	v = v.Normalize() ;
	//시작점에서 한번 그려주고
	DrawCircleBrushing(nNum, (int)s.x, (int)s.y, (int)fRadius, 0xff) ;

	float count=1.0f, l ;
	while(true)
	{
		l = d*count ;
		if(l >= t)
			break ;

		p = s+(v*l) ;

        for(i=0 ; i<nBrushCount ; i++)
			DrawCircleBrushing(nNum, (int)p.x, (int)p.y, (int)fRadius, 0xff) ;

		count += 1.0f ;
	}
}

void TERRAINMAP::EndBrushing(int nNumTex, LPDIRECT3DDEVICE9 pd3dDevice)
{
	if(pcActionManagement)
	{
		pcActionManagement->DisableEndAction() ;
		CActionBrushing *pAction = pcActionContainer->GetEmptyContainer() ;
		assert(pAction != NULL) ;
		pAction->Save(apTexAlpha[nNumTex], pd3dDevice) ;
		pAction->m_psHeightMap = this ;
		pcActionManagement->InsertAction(pAction) ;
	}
}
void TERRAINMAP::ReadAlphaTexture(int nNum, DWORD *pdwBuffer)
{
	D3DSURFACE_DESC desc ;
	ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
	apTexAlpha[nNum]->GetLevelDesc(0, &desc) ;

	D3DLOCKED_RECT d3drect ;
	apTexAlpha[nNum]->LockRect(0, &d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE) ;
	apTexAlpha[nNum]->AddDirtyRect(NULL) ;
	DWORD *pdwSrc = (DWORD *)d3drect.pBits ;

	for(int y=0; y<(int)desc.Height; y++)
	{
		for(int x=0; x<(int)desc.Width; x++, pdwSrc++, pdwBuffer++)
			*pdwBuffer = *pdwSrc ;
	}
	apTexAlpha[nNum]->UnlockRect(0) ;
}
void TERRAINMAP::SetHeightMap(int x, int z, float h)
{
	if((x < 0) || (x >= nVertexWidth))
		return ;
	if((z < 0) || (z >= nVertexHeight))
		return ;    

	ppfHeightMap[z][x] = h ;
}
float TERRAINMAP::GetHeightMap(int x, int z)
{
    if(x < 0)	x = 0 ;
	else if(x >= nVertexWidth)	x = nVertexWidth-1 ;

	if(z < 0)	z = 0 ;
	else if(z >= nVertexHeight)	z = nVertexHeight-1 ;

    return ppfHeightMap[z][x] ;
}
Vector3 TERRAINMAP::GetNormal(int x, int z)
{
	if(x < 0)	x = 0 ;
	if(z < 0)	z = 0 ;
	if(x >= nVertexWidth) x = nVertexWidth-1 ;
	if(z >= nVertexHeight) z = nVertexHeight-1 ;

	int x1, x2, z1, z2 ;
	x1 = ((x-1)<0) ? 0 : x-1 ;
	x2 = ((x+1)>=nVertexWidth) ? nVertexWidth-1 : x+1 ;
	z1 = ((z-1)<0) ? 0 : z-1 ;
    z2 = ((z+1)>=nVertexHeight) ? nVertexHeight-1 : z+1 ;

	Vector3 t, b, n ;

	t.set((float)(x2-x1), GetHeightMap(x2, z)-GetHeightMap(x1, z), 0) ;
	//t = t.Normalize() ;

	b.set(0, GetHeightMap(x, z2)-GetHeightMap(x, z1), (float)(z1-z2)) ;//z의 방향이 이미지기반이기때문에 반대로 해줘야 함.
	//b = b.Normalize() ;

	n = t.cross(b) ;
	return n.Normalize() ;
}
Vector3 TERRAINMAP::GetNormal(float x, float z)
{
    Vector3 vNormal(0, 1, 0) ;
	float ori_x = x, ori_z = z ;

	if(float_less(x, -(float)(lWidth)/2.0f) || float_greater(x, (float)(lWidth)/2.0f))
		return vNormal ;
	if(float_less(z, -(float)(lHeight)/2.0f) || float_greater(z, (float)(lHeight)/2.0f))
		return vNormal ;

	//HeightMap 접근할때는 실제넓이(0-64)로 접근한다.
	x += (float)((lWidth)/2) ;
	z = (-z) + (float)((lHeight)/2) ;

	int x1, x2, z1, z2 ;
	GetVertexPos(x, z, x1, x2, z1, z2, false) ;

	if( (x1 < 0) || (x2 > (int)(lWidth/2)) )
		return vNormal ;
	if( (z1 < 0) || (z2 > (int)(lHeight/2)) )
		return vNormal ;

	x = float_round(x) ;
	z = float_round(z) ;

	geo::STriangle tri ;
	geo::SLine line(Vector3(x, 1000.0f, -z), Vector3(0, -1.0f, 0), 2000.0f) ;

	Vector3 vPos[3], p ;
	vPos[0] = Vector3((float)x1, ppfHeightMap[z1/nTileSize][x1/nTileSize], (float)-z1) ;
	vPos[1] = Vector3((float)x2, ppfHeightMap[z1/nTileSize][x2/nTileSize], (float)-z1) ;
	vPos[2] = Vector3((float)x1, ppfHeightMap[z2/nTileSize][x1/nTileSize], (float)-z2) ;

	tri.set(vPos[0], vPos[1], vPos[2]) ;
	tri.avVertex[0].vNormal = GetNormal(x1/nTileSize, z1/nTileSize) ;
	tri.avVertex[1].vNormal = GetNormal(x2/nTileSize, z1/nTileSize) ;
	tri.avVertex[2].vNormal = GetNormal(x1/nTileSize, z2/nTileSize) ;

	if(IntersectLinetoTriangle(line, tri, p) == geo::INTERSECT_POINT)
	{
		vNormal = tri.avVertex[0].vNormal*p.x + tri.avVertex[1].vNormal*p.y + tri.avVertex[2].vNormal*p.z ;
		return vNormal ;
	}

	vPos[0] = Vector3((float)x1, ppfHeightMap[z2/nTileSize][x1/nTileSize], (float)-z2) ;
	vPos[1] = Vector3((float)x2, ppfHeightMap[z1/nTileSize][x2/nTileSize], (float)-z1) ;
	vPos[2] = Vector3((float)x2, ppfHeightMap[z2/nTileSize][x2/nTileSize], (float)-z2) ;

	tri.set(vPos[0], vPos[1], vPos[2]) ;
	tri.avVertex[0].vNormal = GetNormal(x1/nTileSize, z2/nTileSize) ;
	tri.avVertex[1].vNormal = GetNormal(x2/nTileSize, z1/nTileSize) ;
	tri.avVertex[2].vNormal = GetNormal(x2/nTileSize, z2/nTileSize) ;

	if(IntersectLinetoTriangle(line, tri, p) == geo::INTERSECT_POINT)
	{
		vNormal = tri.avVertex[0].vNormal*p.x + tri.avVertex[1].vNormal*p.y + tri.avVertex[2].vNormal*p.z ;
		return vNormal ;
	}
    return vNormal ;
}
void CalculateBothSides(int &out1, int &out2, float in, int nTileSize)
{
	out1 = (int)floorf(in) ;
	out2 = out1+nTileSize ;
	//out2 = (int)(floorf((float)out1+(float)nTileSize)) ;
	//out2 = (int)floor(in+(float)nTileSize/2.0f) ;
}
void TERRAINMAP::GetVertexPos(float x, float z, int &x1, int &x2, int &z1, int &z2, bool bConvertPos)
{
	int nHalfWidth = lWidth/2 ;
	int nHalfHeight = lHeight/2 ;

	if(bConvertPos)
	{
		x += (float)(nHalfWidth) ;
		z = (-z) + (float)(nHalfHeight) ;

		if(float_less_eq(x, 0.0f))
			x = 0.0f ;
		else if(float_greater_eq(x, (float)lWidth))
			x = (float)lWidth ;

		if(float_less_eq(z, 0.0f))
			z = 0.0f ;
		else if(float_greater_eq(z, (float)lHeight))
			z = (float)lHeight ;
	}

	if(nTileSize == 1)
	{
		CalculateBothSides(x1, x2, x, nTileSize) ;
		CalculateBothSides(z1, z2, z, nTileSize) ;
	}
	else if(nTileSize == 2)
	{
		x1 = (int)x ;
		if(float_eq(x, (float)x1) && (x1 > 0))//x가 소수점 이하가 없을경우
		{
			if(x1%2)
			{
				x1 = (int)x-nTileSize/2 ;
				x2 = (int)x+nTileSize/2 ;
			}
			else
			{
				x1 = (int)x-nTileSize ;
				x2 = (int)x ;
			}		
		}
		else
		{
			if(x1%2)
			{
				x2 = x1+nTileSize/2 ;
				x1 = (int)x-+nTileSize/2 ;
			}
			else
			{
				x2 = x1+nTileSize ;
				x1 = (int)x ;
			}
		}

		z1 = (int)z ;
		if(float_eq(z, (float)z1) && (z1 > 0))//z가 소수점 이하가 없을경우
		{
			if(z1%2)
			{
				z1 = (int)z-nTileSize/2 ;
				z2 = (int)z+nTileSize/2 ;
			}
			else
			{
				z1 = (int)z-nTileSize ;
				z2 = (int)z ;
			}
		}
		else
		{
			if(z1%2)
			{
				z2 = z1+nTileSize/2 ;
				z1 = (int)z-nTileSize/2 ;
			}
			else
			{
				z2 = z1+nTileSize ;
				z1 = (int)z ;
			}
		}
	}

	//if(x1 < 0) x1=0 ;
	//if(x2 > lWidth) x2=lWidth ;

	//if(z1 < 0) z1=0 ;
	//if(z2 > lHeight) z2=lHeight ;
}
float TERRAINMAP::GetHeightMap(float x, float z)
{
	float h=0.0f ;
	float ori_x = x, ori_z = z ;

	if(float_less(x, -(float)(lWidth)/2.0f) || float_greater(x, (float)(lWidth)/2.0f))
		return h ;
	if(float_less(z, -(float)(lHeight)/2.0f) || float_greater(z, (float)(lHeight)/2.0f))
		return h ;

	//HeightMap 접근할때는 실제넓이(0-64)로 접근한다.
	x += (float)((lWidth)/2) ;
	z = (-z) + (float)((lHeight)/2) ;

	int x1, x2, z1, z2 ;
	GetVertexPos(x, z, x1, x2, z1, z2, false) ;

	x = float_round(x) ;
	z = float_round(z) ;

	geo::SLine line(Vector3(x, 1000.0f, -z), Vector3(0, -1.0f, 0), 2000.0f) ;

	//계산해서 변수에 넣고 비교하는거와 바로 비교하는게 결과가 다르다!! 뭐야?!
	float l = fabs(x-x1)+fabs(z-z1) ;
	if(float_less_eq(l, (float)nTileSize))
	//if(float_less_eq(fabs(x-x1)+fabs(z-z1), (float)nTileSize))//intersected left-top triangle ([z1][x1], [z1][x2], [z2][x1])
	{
		Vector3 vPos[3] ;//여기는 x1, x2, z1, z2 실제 위치좌표가 아님. 높이값이 기준이라서 x, z좌표는 상관없음.
		vPos[0] = Vector3((float)x1, ppfHeightMap[z1/nTileSize][x1/nTileSize], (float)-z1) ;
		vPos[1] = Vector3((float)x2, ppfHeightMap[z1/nTileSize][x2/nTileSize], (float)-z1) ;
		vPos[2] = Vector3((float)x1, ppfHeightMap[z2/nTileSize][x1/nTileSize], (float)-z2) ;

		geo::STriangle tri(vPos[0], vPos[1], vPos[2]) ;

		Vector3 p ;
		//IntersectLinetoTriangle(line, tri, p, true) ;
		//h = p.y ;

		IntersectLinetoPlane(line, tri.sPlane, p) ;
		h = p.y ;

        /*
		if(float_eq(h, 0.0f))
		{
			x = float_round(ori_x) ;
			z = float_round(ori_z) ;
			line.set(Vector3(x, 1000.0f, -z), Vector3(0, -1.0f, 0), 2000.0f) ;
			IntersectLinetoTriangle(line, tri, p, true) ;

			float u, v, dist ;
			D3DXVECTOR3 p0, p1, p2, rayPos, rayDir ;

			p0 = D3DXVECTOR3(vPos[0].x, vPos[0].y, vPos[0].z) ;
			p1 = D3DXVECTOR3(vPos[1].x, vPos[1].y, vPos[1].z) ;
			p2 = D3DXVECTOR3(vPos[2].x, vPos[2].y, vPos[2].z) ;
			rayPos = D3DXVECTOR3(line.s.x, line.s.y, line.s.z) ;
			rayDir = D3DXVECTOR3(line.v.x, line.v.y, line.v.z) ;

			D3DXIntersectTri(&p0, &p1, &p2, &rayPos, &rayDir, &u, &v, &dist) ;

			float hx = line.s.y-dist ;
			TRACE("error intersect but D3DXIntersectTri h=%g\r\n", hx) ;
		}
		*/

		//float u, v ;
		//IntesectXZProjTriangleToPoint(tri, Vector3(x, 0, -z), u, v) ;
		//h = (1-u-v)*tri.avVertex[0].vPos.y + u*tri.avVertex[1].vPos.y + v*tri.avVertex[2].vPos.y ;
	}
	else
	{
		Vector3 vPos[3] ;//여기는 x1, x2, z1, z2 실제 위치좌표가 아님. 높이값이 기준이라서 x, z좌표는 상관없음.
		vPos[0] = Vector3((float)x1, ppfHeightMap[z2/nTileSize][x1/nTileSize], (float)-z2) ;
		vPos[1] = Vector3((float)x2, ppfHeightMap[z1/nTileSize][x2/nTileSize], (float)-z1) ;
		vPos[2] = Vector3((float)x2, ppfHeightMap[z2/nTileSize][x2/nTileSize], (float)-z2) ;

		geo::STriangle tri(vPos[0], vPos[1], vPos[2]) ;

		Vector3 p ;
		//IntersectLinetoTriangle(line, tri, p, true) ;
		//h = p.y ;

		IntersectLinetoPlane(line, tri.sPlane, p) ;
		h = p.y ;

        /*
		if(float_eq(h, 0.0f))
		{
			x = float_round(ori_x) ;
			z = float_round(ori_z) ;
			line.set(Vector3(x, 1000.0f, -z), Vector3(0, -1.0f, 0), 2000.0f) ;
			IntersectLinetoTriangle(line, tri, p, true) ;

			float u, v, dist ;
			D3DXVECTOR3 p0, p1, p2, rayPos, rayDir ;

			p0 = D3DXVECTOR3(vPos[0].x, vPos[0].y, vPos[0].z) ;
			p1 = D3DXVECTOR3(vPos[1].x, vPos[1].y, vPos[1].z) ;
			p2 = D3DXVECTOR3(vPos[2].x, vPos[2].y, vPos[2].z) ;
			rayPos = D3DXVECTOR3(line.s.x, line.s.y, line.s.z) ;
			rayDir = D3DXVECTOR3(line.v.x, line.v.y, line.v.z) ;

			D3DXIntersectTri(&p0, &p1, &p2, &rayPos, &rayDir, &u, &v, &dist) ;

			float hx = line.s.y-dist ;
			TRACE("error intersect but D3DXIntersectTri h=%g\r\n", hx) ;
		}
		*/

		//float u, v ;
		//IntesectXZProjTriangleToPoint(tri, Vector3(x, 0, -z), u, v) ;
		//h = (1-u-v)*tri.avVertex[0].vPos.y + u*tri.avVertex[1].vPos.y + v*tri.avVertex[2].vPos.y ;
	}
    return h ;
}

int TERRAINMAP::GetNumTriangle(Vector3 vPos, float width, float height)
{
	int x, z, x1, x2, z1, z2 ;
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nWidth = (int)((width/2.0f)/nTileSize+0.5f) ;
	int nHeight = (int)((height/2.0f)/nTileSize+0.5f) ;

	x1 -= nWidth ;
	x2 += nWidth ;
	z1 -= nHeight ;
	z2 += nHeight ;

	int nTriangleCount=0 ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
			nTriangleCount += 2 ;
	}
	return nTriangleCount ;
}
int TERRAINMAP::GetHolecupTriangles(Vector3 vCenter, geo::STriangle *psRecvTriangles)
{
	Vector3 avExceptionPos[3], avPos[3] ;
	int nTriangleCount=0 ;
	TERRAININDEX *psIndex ;
	Matrix4 matHolecupTransform = MatrixConvert(psHolecup->matTransform) ;

	for(int nNumExceptionIndex=0 ; nNumExceptionIndex < sExceptionalIndices.nCurPos ; nNumExceptionIndex++)
	{
		psIndex = sExceptionalIndices.GetAt(nNumExceptionIndex) ;

		avExceptionPos[0].set(pVertices[psIndex->i1].pos.x, pVertices[psIndex->i1].pos.y, pVertices[psIndex->i1].pos.z) ;
		avExceptionPos[1].set(pVertices[psIndex->i2].pos.x, pVertices[psIndex->i2].pos.y, pVertices[psIndex->i2].pos.z) ;
		avExceptionPos[2].set(pVertices[psIndex->i3].pos.x, pVertices[psIndex->i3].pos.y, pVertices[psIndex->i3].pos.z) ;
		Vector3 vExceptionCenter = (avExceptionPos[0] + avExceptionPos[1] + avExceptionPos[2])/3.0f ;

		//TRACE("exceptionCenter(%g %g %g)\r\n", enumVector(vExceptionCenter)) ;

		//if(vector_eq(vCenter, vExceptionCenter))
		if((vCenter-vExceptionCenter).Magnitude() < 0.01f)
		{
			for(int nNumHolecupTriangle=0 ; nNumHolecupTriangle < psHolecup->sCollisionHolecup.nNumTriangle ; nNumHolecupTriangle++, nTriangleCount++)
			{
				psRecvTriangles[nTriangleCount].set(psHolecup->sCollisionHolecup.psTriangles[nNumHolecupTriangle].avVertex[0].vPos * matHolecupTransform,
					psHolecup->sCollisionHolecup.psTriangles[nNumHolecupTriangle].avVertex[1].vPos * matHolecupTransform,
					psHolecup->sCollisionHolecup.psTriangles[nNumHolecupTriangle].avVertex[2].vPos * matHolecupTransform) ;

				psRecvTriangles[nTriangleCount].MakeBoundingShpere() ;
				psRecvTriangles[nTriangleCount].dwAttr = COLLISION_IN_HOLECUP | nNumHolecupTriangle ;//상위 워드 속성 부여, 하위 워드 삼각형인덱스 부여

				//TRACE("recv HoleTriangles[%d] pos(%g %g %g) r=%g\r\n", nNumHolecupTriangle, enumVector(psRecvTriangles[nTriangleCount].sBoundingSphere.vPos), psRecvTriangles[nTriangleCount].sBoundingSphere.fRadius) ;
			}
		}
	}
	return nTriangleCount ;
}
/*
int TERRAINMAP::GetTriangles(Vector3 vPos, float width, float height, geo::STriangle *psRecvTriangles, bool bHolecup, int nMaxTriangle)
{
	int x, z, x1, x2, z1, z2 ;
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nWidth = (int)((width/2.0f)/nTileSize+0.5f) ;
	int nHeight = (int)((height/2.0f)/nTileSize+0.5f) ;

	x1 -= nWidth ;
	x2 += nWidth ;
	z1 -= nHeight ;
	z2 += nHeight ;

	float h ;
	int nTriangleCount=0, nAddTriangles ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
		{
			h = ppfHeightMap[-z+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)z) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)x+nTileSize, h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[2] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
				return nMaxTriangle ;

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				if((nTriangleCount+psHolecup->sCollisionHolecup.nNumTriangle) > nMaxTriangle)
					return nTriangleCount ;

				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;
				nTriangleCount++ ;
			}

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)(x+nTileSize), h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[2] = Vector3((float)(x+nTileSize), h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
				return nMaxTriangle ;

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				if((nTriangleCount+psHolecup->sCollisionHolecup.nNumTriangle) > nMaxTriangle)
					return nTriangleCount ;

				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;
				nTriangleCount++ ;
			}
		}
	}
	return nTriangleCount ;
}*/
int TERRAINMAP::GetTriangles(Vector3 vPos, float width, float height, geo::STriangle *psRecvTriangles, bool bHolecup, int nMaxTriangle)
{
	int x, z, x1, x2, z1, z2 ;
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nWidth = (int)((width/2.0f)/nTileSize+0.5f) ;
	int nHeight = (int)((height/2.0f)/nTileSize+0.5f) ;

	x1 -= nWidth ;
	x2 += nWidth ;
	z1 -= nHeight ;
	z2 += nHeight ;

	float h ;
	int nTriangleCount=0, nAddTriangles ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
		{
			h = ppfHeightMap[-z+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)z) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)x+nTileSize, h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[2] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
				return 0 ;

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;
				nTriangleCount++ ;
			}

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)(x+nTileSize), h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[2] = Vector3((float)(x+nTileSize), h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
				return 0 ;

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;
				nTriangleCount++ ;
			}
		}
	}
	return nTriangleCount ;
}

bool TERRAINMAP::GetHeightMapAndNormal(Vector3 pos, Vector3 *pvIntersect, Vector3 *pvNormal)
{
	int x, z, x1, x2, z1, z2 ;
	geo::SLine line(pos, Vector3(0, -1, 0), 1000.0f) ;
	geo::STriangle tri ;
	Vector3 vIntersect ;

	GetVertexPos(pos.x, pos.z, x1, x2, z1, z2, false) ;

	int nWidth = (int)(1.0f/nTileSize+0.5f) ;
	int nHeight = (int)(1.0f/nTileSize+0.5f) ;

	x1 -= nWidth ;
	x2 += nWidth ;
	z1 -= nHeight ;
	z2 += nHeight ;

	float h ;
	int nTriangleCount=0, nAddTriangles ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
		{
			h = ppfHeightMap[-z+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)z) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)x+nTileSize, h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[2] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			tri.set(avPos[0], avPos[1], avPos[2]) ;
			tri.avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
			tri.avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
			tri.avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

			if(IntersectLinetoTriangle(line, tri, vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
			{
				(*pvIntersect) = vIntersect ;
				(*pvNormal) = tri.sPlane.GetNormal() ;
				return true ;
			}

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][x+((lWidth)/2)] ;
			avPos[0] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			h = ppfHeightMap[-z+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[1] = Vector3((float)(x+nTileSize), h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+((lHeight)/2)][(x+nTileSize)+((lWidth)/2)] ;
			avPos[2] = Vector3((float)(x+nTileSize), h, (float)(z-nTileSize)) ;

			tri.set(avPos[0], avPos[1], avPos[2]) ;
			tri.avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
			tri.avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
			tri.avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

			if(IntersectLinetoTriangle(line, tri, vIntersect, true, true, FLT_EPSILON) == geo::INTERSECT_POINT)
			{
				(*pvIntersect) = vIntersect ;
				(*pvNormal) = tri.sPlane.GetNormal() ;
				return true ;
			}
		}
	}
	return false ;
}
int TERRAINMAP::GetNumTriangle(Vector3 vPos, float fRadius)
{
	int x, z, x1, x2, z1, z2, r = (int)(fRadius/nTileSize+0.5f) ;//반지름도 타일사이즈만큼 나누어 준다.
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nHalfWidth = lWidth/2 ;
	int nHalfHeight = lHeight/2 ;

	x1 = x1-r ;
	if(x1 < -nHalfWidth)	x1 = -nHalfWidth ;
	x2 = x2+r ;
	if(x2 > nHalfWidth)	x2 = nHalfWidth ;

	z1 = z1-r ;
	if(z1 < -nHalfHeight)	z1 = nHalfHeight ;
	z2 = z2+r ;
	if(z2 > nHalfHeight)	z2 = nHalfHeight ;

	int nTriangleCount=0 ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
			nTriangleCount += 2 ;
	}
	return nTriangleCount ;
}
/*
int TERRAINMAP::GetTriangles(Vector3 vPos, float fRadius, geo::STriangle *psRecvTriangles, bool bMakeBoundingShpere, bool bHolecup, int nMaxTriangle)
{
	int x, z, x1, x2, z1, z2, r = (int)(fRadius/nTileSize+0.5f) ;//반지름도 타일사이즈만큼 나누어 준다.
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nHalfWidth = lWidth/2 ;
	int nHalfHeight = lHeight/2 ;

	if(r == 0)
		r = 1 ;

	x1 = x1-r ;
	if(x1 < -nHalfWidth)	x1 = -nHalfWidth ;
	x2 = x2+r ;
	if(x2 > nHalfWidth)	x2 = nHalfWidth ;

	z1 = z1-r ;
	if(z1 < -nHalfHeight)	z1 = nHalfHeight ;
	z2 = z2+r ;
	if(z2 > nHalfHeight)	z2 = nHalfHeight ;

	float h ;
	int nTriangleCount=0, nAddTriangles ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
		{
			h = ppfHeightMap[-z+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[0] = Vector3((float)x, h, (float)z) ;

			h = ppfHeightMap[-z+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[1] = Vector3((float)x+nTileSize, h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[2] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
			{
				return nMaxTriangle ;
			}

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				if((nTriangleCount+psHolecup->sCollisionHolecup.nNumTriangle) >= nMaxTriangle)
					return nTriangleCount ;

				Vector3 vCTri = (avPos[0]+avPos[1]+avPos[2])/3.0f ;
				nAddTriangles = GetHolecupTriangles(vCTri, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

				if(bMakeBoundingShpere)
					psRecvTriangles[nTriangleCount].MakeBoundingShpere() ;

				nTriangleCount++ ;
			}

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[0] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			h = ppfHeightMap[-z+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[1] = Vector3((float)(x+nTileSize), h, (float)z) ;

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[2] = Vector3((float)(x+nTileSize), h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
				return nMaxTriangle ;

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				if((nTriangleCount+psHolecup->sCollisionHolecup.nNumTriangle) >= nMaxTriangle)
					return nTriangleCount ;

				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

				if(bMakeBoundingShpere)
					psRecvTriangles[nTriangleCount].MakeBoundingShpere() ;

				nTriangleCount++ ;
			}
		}
	}
	return nTriangleCount ;
}*/
int TERRAINMAP::GetTriangles(Vector3 vPos, float fRadius, geo::STriangle *psRecvTriangles, bool bMakeBoundingShpere, bool bHolecup, int nMaxTriangle)
{
	int x, z, x1, x2, z1, z2, r = (int)(fRadius/nTileSize+0.5f) ;//반지름도 타일사이즈만큼 나누어 준다.
	GetVertexPos(vPos.x, vPos.z, x1, x2, z1, z2, false) ;

	int nHalfWidth = lWidth/2 ;
	int nHalfHeight = lHeight/2 ;

	if(r == 0)
		r = 1 ;

	x1 = x1-r ;
	if(x1 < -nHalfWidth)	x1 = -nHalfWidth ;
	x2 = x2+r ;
	if(x2 > nHalfWidth)	x2 = nHalfWidth ;

	z1 = z1-r ;
	if(z1 < -nHalfHeight)	z1 = nHalfHeight ;
	z2 = z2+r ;
	if(z2 > nHalfHeight)	z2 = nHalfHeight ;

	int suck ;
	int xx, zz ;

	float h ;
	int nTriangleCount=0, nAddTriangles ;
	Vector3 avPos[3] ;
	for(z=z2 ; z>z1 ; z--)
	{
		for(x=x1 ; x<x2 ; x++)
		{
						
			xx = x+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -z+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-z+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[0] = Vector3((float)x, h, (float)z) ;


			xx = (x+nTileSize)+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -z+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-z+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[1] = Vector3((float)x+nTileSize, h, (float)z) ;

			xx = x+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -(z-nTileSize)+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[2] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
			{
				OutputStringforRelease("###################### TriangleCount=%d > nMaxTrangle=%d ###################\r\n", nTriangleCount, nMaxTriangle) ;
				return 0 ;
			}

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				Vector3 vCTri = (avPos[0]+avPos[1]+avPos[2])/3.0f ;

				nAddTriangles = GetHolecupTriangles(vCTri, &psRecvTriangles[nTriangleCount]) ;

				nTriangleCount += nAddTriangles ;

				if(nTriangleCount >= nMaxTriangle)
				{
					OutputStringforRelease("###################### TriangleCount=%d > nMaxTrangle=%d ###################\r\n", nTriangleCount, nMaxTriangle) ;
					return 0 ;
				}
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

				if(bMakeBoundingShpere)
					psRecvTriangles[nTriangleCount].MakeBoundingShpere() ;

				nTriangleCount++ ;
			}

			xx = x+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -(z-nTileSize)+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][x+(nHalfWidth)] ;
			avPos[0] = Vector3((float)x, h, (float)(z-nTileSize)) ;

			xx = (x+nTileSize)+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -z+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-z+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[1] = Vector3((float)(x+nTileSize), h, (float)z) ;

			xx = (x+nTileSize)+(nHalfWidth) ;
			if(xx < 0 || xx >= nVertexWidth)
				OutputStringforRelease("suck xx=%d\r\n", xx) ;

			zz = -(z-nTileSize)+(nHalfHeight) ;
			if(zz < 0 || zz >= nVertexHeight)
				OutputStringforRelease("suck zz=%d\r\n", zz) ;

			h = ppfHeightMap[-(z-nTileSize)+(nHalfHeight)][(x+nTileSize)+(nHalfWidth)] ;
			avPos[2] = Vector3((float)(x+nTileSize), h, (float)(z-nTileSize)) ;

			if(nTriangleCount >= nMaxTriangle)
			{
				OutputStringforRelease("###################### TriangleCount=%d > nMaxTrangle=%d ###################\r\n", nTriangleCount, nMaxTriangle) ;
				return 0 ;
			}

			nAddTriangles = 0 ;
			if(sExceptionalIndices.nCurPos && bHolecup)
			{
				nAddTriangles = GetHolecupTriangles((avPos[0]+avPos[1]+avPos[2])/3.0f, &psRecvTriangles[nTriangleCount]) ;
				nTriangleCount += nAddTriangles ;

				if(nTriangleCount >= nMaxTriangle)
				{
					OutputStringforRelease("###################### TriangleCount=%d > nMaxTrangle=%d ###################\r\n", nTriangleCount, nMaxTriangle) ;
					return 0 ;
				}
			}

			if(nAddTriangles == 0)
			{
				psRecvTriangles[nTriangleCount].set(avPos[0], avPos[1], avPos[2]) ;
				psRecvTriangles[nTriangleCount].avVertex[0].vNormal = GetNormal(avPos[0].x, avPos[0].z) ;
				psRecvTriangles[nTriangleCount].avVertex[1].vNormal = GetNormal(avPos[1].x, avPos[1].z) ;
				psRecvTriangles[nTriangleCount].avVertex[2].vNormal = GetNormal(avPos[2].x, avPos[2].z) ;

				if(bMakeBoundingShpere)
					psRecvTriangles[nTriangleCount].MakeBoundingShpere() ;

				nTriangleCount++ ;

				if(nTriangleCount >= nMaxTriangle)
				{
					OutputStringforRelease("###################### TriangleCount=%d > nMaxTrangle=%d ###################\r\n", nTriangleCount, nMaxTriangle) ;
					return 0 ;
				}
			}
		}
	}
	return nTriangleCount ;
}

void TERRAINMAP::ExportContactSurfaceInfo(char *pszFileName)
{
	FILE *pfile = fopen(pszFileName, "w+b") ;
	if(pfile)
	{
		fwrite(asContactSurfaceInfo, sizeof(SContactSurfaceInfo)*9, 1, pfile) ;
		fclose(pfile) ;
	}
}

struct SUCKER
{
	int nKind ;//표면의 종류
	float fRestitution ;//물체와 충돌시 저항
	float fFriction ;//물체가 미끄러질때 마찰

	float fVelRange, fVelBase ;
	float fMaxIncidenceBase ;
	float fScaleRestitution ;
	float fRollingCoef ;
} ;

void TERRAINMAP::ImportContactSurfaceInfo(char *pszFileName)
{
	FILE *pfile = fopen(pszFileName, "r+b") ;
	if(pfile)
	{
		fread(asContactSurfaceInfo, sizeof(SContactSurfaceInfo)*9, 1, pfile) ;

		//SUCKER sucker[9] ;
		//fread(sucker, sizeof(SUCKER)*9, 1, pfile) ;

		//for(int i=0 ; i<9 ; i++)
		//{
		//	asContactSurfaceInfo[i].nKind = sucker[i].nKind ;
		//	asContactSurfaceInfo[i].fFriction = sucker[i].fFriction ;
		//	asContactSurfaceInfo[i].fMaxIncidenceBase = sucker[i].fMaxIncidenceBase ;
		//	asContactSurfaceInfo[i].fRestitution = sucker[i].fRestitution ;
		//	asContactSurfaceInfo[i].fRollingCoef = sucker[i].fRollingCoef ;
		//	asContactSurfaceInfo[i].fScaleRestitution = sucker[i].fScaleRestitution ;
		//	asContactSurfaceInfo[i].fVelBase = sucker[i].fVelBase ;
		//	asContactSurfaceInfo[i].fVelRange = sucker[i].fVelRange ;
		//	asContactSurfaceInfo[i].bGroundHazard = false ;
		//}

		fclose(pfile) ;

		//ExportContactSurfaceInfo("../../Media/collisionData/QQSurfaceParameters.surfaceinfo") ;
	}
}
void TERRAINMAP::getTerrainSurfaceProperty(int kind, SContactSurfaceInfo *psContactSurfaceInfo)
{
	*psContactSurfaceInfo = asContactSurfaceInfo[kind] ;
}

int TERRAINMAP::GetSurfaceInfo(float x, float z, SContactSurfaceInfo *psContactSurfaceInfo)
{
	int i, n ;
	int nHalfWidth = lWidth/2 ;
	int nHalfHeight = lHeight/2 ;

	//월드좌표에서 알파텍스쳐좌표 로 변환
	int nx = (int)((x + (float)nHalfWidth)*(float)nAlphaMapSize/(float)lWidth+0.5f) ;
	int ny = (int)(((float)nHalfHeight - z)*(float)nAlphaMapSize/(float)lHeight+0.5f) ;

	if((nx<0) || (nx>(nAlphaMapSize-1)))//rect구조체를 만들때 +1을 하니까 -1을 해준다.
		return -1 ;
	if((ny<0) || (ny>(nAlphaMapSize-1)))//rect구조체를 만들때 +1을 하니까 -1을 해준다.
		return -1 ;

	//ob, rough, hazard, tee, fairway, bunker, green, greenside, roughside

	int anShift[] = {24, 16, 8, 0} ;
	D3DSURFACE_DESC desc ;
	D3DLOCKED_RECT d3drect ;
	DWORD dwHighestColor=1, dwSurfaceKind=0 ;
	DWORD *pdw, dwAlphaValue ;
	DWORD dwHazardValue, dwTeeValue ;
	RECT rt = {nx, ny, nx+1, ny+1} ;
	int nPitch ;
	for(i=0 ; i<2 ; i++)
	{
		apTexAlpha[i]->GetLevelDesc(0, &desc) ;
		apTexAlpha[i]->LockRect(0, &d3drect, &rt, D3DLOCK_READONLY) ;
		pdw = (DWORD *)d3drect.pBits ;
		nPitch = d3drect.Pitch/4 ;

		for(n=0 ; n<4 ; n++)
		{
			if((i==0) && (n==1))//exceptionally hazard
			{
				dwHazardValue = (((*pdw)>>anShift[n])&0xff) ;
				continue ;
			}
			else if((i==0) && (n==2))//tee
			{
				dwTeeValue = (((*pdw)>>anShift[n])&0xff) ;
			}
			else if((i==1) && (n==3))//exceptionally roughside
			{
				continue ;
			}

			dwAlphaValue = (((*pdw)>>anShift[n])&0xff) ;
			if((dwAlphaValue >= dwHighestColor) || (dwAlphaValue >= 0x80))//레이어가 덧칠하는식이라서 뒤에 오는 값이 같거나 크면 그걸로 정한다.
			{
				dwHighestColor = dwAlphaValue ;
				dwSurfaceKind = (i*4)+n+1 ;
			}
		}

		apTexAlpha[i]->UnlockRect(0) ;
	}

	getTerrainSurfaceProperty(dwSurfaceKind, psContactSurfaceInfo) ;

	if((dwHazardValue==0xff) && (dwTeeValue==0xff))
		psContactSurfaceInfo->bGroundHazard = true ;
	else
		psContactSurfaceInfo->bGroundHazard = false ;

	return (int)dwSurfaceKind ;
}
void TERRAINMAP::GetSurfaceInfofromHolecup(int nIndex, SContactSurfaceInfo *psContactSurfaceInfo)
{
	if(nIndex >= 0 && nIndex <= 15)//홀컵의 그린잔디
	{
		//psContactSurfaceInfo->fRestitution = afRestitution[SContactSurfaceInfo::GREEN] ;
		//psContactSurfaceInfo->fFriction = afFriction[SContactSurfaceInfo::GREEN] ;
		getTerrainSurfaceProperty(SContactSurfaceInfo::GREEN, psContactSurfaceInfo) ;
	}
	else if(nIndex >= 16 && nIndex <= 31)//홀컵의 실린더
	{
		psContactSurfaceInfo->fRestitution = 0.65f ;
		psContactSurfaceInfo->fFriction = 0.09f ;
		psContactSurfaceInfo->nKind = SContactSurfaceInfo::HOLECUP ;
	}
	else if(nIndex >= 32 && nIndex <= 33)//홀컵 바닥
	{
		psContactSurfaceInfo->fRestitution = 0.01f ;
		psContactSurfaceInfo->fFriction = 3.0f ;
		psContactSurfaceInfo->nKind = SContactSurfaceInfo::HOLECUP ;
	}
}

int TERRAINMAP::GetIndex(float x, float z, data::SKeepingArray<TERRAININDEX> *psIndices)
{
	int x1, x2, z1, z2 ;
	GetVertexPos(x, z, x1, x2, z1, z2) ;

	//int a = (((nVertexHeight/2)-z1)*nVertexWidth) ;
    //int b = (x1+(nVertexWidth/2)) ;
	//int index = a+b ;

	TERRAININDEX sIndex ;
	int count=0, index = (z1*nVertexWidth)+x1 ;

	//왼쪽상단 삼각형을 얻어낸다.
	sIndex.i1 = index ;
	sIndex.i2 = index+1 ;
	sIndex.i3 = index+nVertexWidth ;
	psIndices->Insert(&sIndex) ;
	count++ ;

	//psIndices[count].i1 = index+nVertexWidth ;
	//psIndices[count].i2 = index+1 ;
	//psIndices[count].i3 = index+nVertexWidth+1 ;
	//count++ ;

	return count ;
}

//####################################################
// STerrainRoad
//####################################################

STerrainRoad::STerrainRoad()
{
	bEnable = false ;
	pVB = NULL ;
	pIB = NULL ;
	pTex = NULL ;
    nNumVertex = nNumIndex = 0 ;
}
STerrainRoad::~STerrainRoad()
{
	Release() ;
}
void STerrainRoad::Release()
{
	SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;
	nNumVertex = nNumIndex = 0 ;
	bEnable = false ;
}
HRESULT STerrainRoad::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, TERRAINVERTEX *pSrcVertices, int nVertexCount, STerrainIndex1 *pSrcIndices, int nIndexCount, LPDIRECT3DTEXTURE9 pTexture)
{
	nNumVertex = nVertexCount ;
	nNumIndex = nIndexCount ;

	int i ;

	if(pVB)
		SAFE_RELEASE(pVB) ;

	//Create VertexBuffer    
	//if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(TERRAINVERTEX), 0, D3DFVF_TERRAINVERTEX, D3DPOOL_MANAGED, &pVB, NULL)))
	if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &pVB, NULL)))
		return E_FAIL ;

    TERRAINVERTEX *pDestVertices ;
	if(FAILED(pVB->Lock(0, nNumVertex*sizeof(TERRAINVERTEX), (void **)&pDestVertices, 0)))
		return E_FAIL ;

    for(i=0 ; i<nNumVertex ; i++, pSrcVertices++, pDestVertices++)
		*pDestVertices = *pSrcVertices ;

	pVB->Unlock() ;

	if(pIB)
		SAFE_RELEASE(pIB) ;

    //Create IndexBuffer
	//if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
	if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB, NULL)))
		return E_FAIL ;

	STerrainIndex1 *pDestIndices ;
	if(FAILED(pIB->Lock(0, nNumIndex*sizeof(STerrainIndex1), (void **)&pDestIndices, 0)))
		return E_FAIL ;

    for(i=0 ; i<nNumIndex ; i++, pSrcIndices++, pDestIndices++)
		*pDestIndices = *pSrcIndices ;

	pIB->Unlock() ;

    pTex = pTexture ;
	bEnable = true ;
	return S_OK ;
}

//####################################################
// STerrainDecal
//####################################################

STerrainDecal::STerrainDecal()
{
	bEnable = false ;
	pVB = NULL ;
	pIB = NULL ;
	apTex[0] = apTex[1] = apTex[2] = NULL ;
    nNumVertex = nNumIndex = 0 ;
}
STerrainDecal::~STerrainDecal()
{
	Release() ;
}
void STerrainDecal::Release()
{
	SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;
	nNumVertex = nNumIndex = 0 ;
	bEnable = false ;
}
HRESULT STerrainDecal::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, STerrainVertex1 *pSrcVertices, int nVertexCount, STerrainIndex1 *pSrcIndices, int nIndexCount)
{
	nNumVertex = nVertexCount ;
	nNumIndex = nIndexCount ;

	int i ;

	if(pVB)
		SAFE_RELEASE(pVB) ;

	//Create VertexBuffer    
	//if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(STerrainVertex1), 0, D3DFVF_TERRAINVERTEX1, D3DPOOL_MANAGED, &pVB, NULL)))
	if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(STerrainVertex1), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX1, D3DPOOL_DEFAULT, &pVB, NULL)))
		return E_FAIL ;

    STerrainVertex1 *pDestVertices ;
	if(FAILED(pVB->Lock(0, nNumVertex*sizeof(STerrainVertex1), (void **)&pDestVertices, 0)))
		return E_FAIL ;

    for(i=0 ; i<nNumVertex ; i++, pSrcVertices++, pDestVertices++)
		*pDestVertices = *pSrcVertices ;

	pVB->Unlock() ;

	if(pIB)
		SAFE_RELEASE(pIB) ;

    //Create IndexBuffer
	//if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
	if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB, NULL)))
		return E_FAIL ;

	STerrainIndex1 *pDestIndices ;
	if(FAILED(pIB->Lock(0, nNumIndex*sizeof(STerrainIndex1), (void **)&pDestIndices, 0)))
		return E_FAIL ;

    for(i=0 ; i<nNumIndex ; i++, pSrcIndices++, pDestIndices++)
		*pDestIndices = *pSrcIndices ;

	pIB->Unlock() ;

	bEnable = true ;
	return S_OK ;
}

//####################################################
// SHolecup
//####################################################
SCollisionHolecup::SCollisionHolecup()
{
	nNumTriangle = nNumCylinder = nNumSphere = 0 ;
	psTriangles = NULL ;
	psCylinders = NULL ;
	psSpheres = NULL ;
}
SCollisionHolecup::~SCollisionHolecup()
{
	Release() ;
}
bool SCollisionHolecup::Import(char *pszFileName)
{
	Release() ;

	FILE *pfile = fopen(pszFileName, "r+b") ;
	if(pfile)
	{
		fread(&nNumTriangle, 4, 1, pfile) ;
		psTriangles = new geo::STriangle[nNumTriangle] ;
		fread(psTriangles, sizeof(geo::STriangle), nNumTriangle, pfile) ;

		fread(&nNumCylinder, 4, 1, pfile) ;
		psCylinders = new geo::SCylinder[nNumCylinder] ;
		fread(psCylinders, sizeof(geo::SCylinder), nNumCylinder, pfile) ;

		fread(&nNumSphere, 4, 1, pfile) ;
		psSpheres = new geo::SSphere[nNumSphere] ;
		fread(psSpheres, sizeof(geo::SSphere), nNumSphere, pfile) ;

		fclose(pfile) ;
		return true ;
	}
	return false ;
}
void SCollisionHolecup::Release()
{
	SAFE_DELETEARRAY(psTriangles) ;
	SAFE_DELETEARRAY(psCylinders) ;
	SAFE_DELETEARRAY(psSpheres) ;
}

//####################################################
// SHolecup
//####################################################

SHolecup::SHolecup()
{
	bEnable = false ;
	pVB = NULL ;
	pIB = NULL ;
	pTex = NULL ;
    nNumVertex = nNumIndex = 0 ;
	nExceptionalIndex = -1 ;
	nLodlevel = 0 ;
	D3DXMatrixIdentity(&matTransform) ;
	fDepthBias=0.0f ;
	fSlopeScaleDepthBias=0.0f ;
}
SHolecup::~SHolecup()
{
	Release() ;
}
void SHolecup::Release()
{
	SAFE_RELEASE(pVB) ;
	SAFE_RELEASE(pIB) ;
	pTex = NULL ;
	nNumVertex = nNumIndex = 0 ;
	nExceptionalIndex = -1 ;
	D3DXMatrixIdentity(&matTransform) ;
	bEnable = false ;

	sDecal.Release() ;
}
HRESULT SHolecup::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszFileName, D3DXMATRIX *pmatTransform, LPDIRECT3DTEXTURE9 ptex, geo::STriangle *psTriangle, int exceptional_index, CSecretTerrain *pcTerrain)
{
	float precision=0.001f ;
	int i, n ;
    CASEData data ;
	if(!data.ImportRMD(pszFileName))
	{
		TRACE("cannot load %s", pszFileName) ;
		assert(false && "Cannot load RMD file") ;
		return E_FAIL ;
	}
	assert(data.m_nNumMesh == 1) ;//메쉬가 하나이어야만 된다.

	nNumVertex = data.m_apMesh[0]->nNumVertex ;
	nNumIndex = data.m_apMesh[0]->nNumTriangle ;

	D3DXMATRIX matScale ;

	D3DXMatrixIdentity(&matScale) ;
    matScale._11 = matScale._22 = matScale._33 = 0.1f ;

	matTransform = matScale * (*pmatTransform) ;
	D3DXMATRIX matRot ;
	D3DXMatrixDecomposeRotation(&matRot, &matTransform) ;
	D3DXVECTOR4 v ;

	float dx = psTriangle->avVertex[1].vPos.x - psTriangle->avVertex[0].vPos.x ;
	float dz = psTriangle->avVertex[2].vPos.z - psTriangle->avVertex[0].vPos.z ;//여기서 음수
	float tex_dx = psTriangle->avVertex[1].vTex.x - psTriangle->avVertex[0].vTex.x ;
	float tex_dy = psTriangle->avVertex[2].vTex.y - psTriangle->avVertex[0].vTex.y ;
	Vector2 tex_s = psTriangle->avVertex[0].vTex ;

	Vector3 p, nor ;
    Matrix4 mat ;
	memcpy(&mat, &data.m_apMesh[0]->smatLocal, sizeof(Matrix4)) ;
	//Create VertexBuffer    
	//if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(STerrainVertex1), 0, D3DFVF_TERRAINVERTEX1, D3DPOOL_MANAGED, &pVB, NULL)))
	if(FAILED(pd3dDevice->CreateVertexBuffer(nNumVertex*sizeof(STerrainVertex1), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX1, D3DPOOL_DEFAULT, &pVB, NULL)))
		return E_FAIL ;

    STerrainVertex1 *pDestVertices ;
	if(FAILED(pVB->Lock(0, nNumVertex*sizeof(STerrainVertex1), (void **)&pDestVertices, 0)))
		return E_FAIL ;

	geo::SVertex *psVertex ;

    for(i=0 ; i<data.m_apMesh[0]->nNumVertex ; i++, pDestVertices++)
	{
		p.set(data.m_apMesh[0]->psVertex[i].pos.x, data.m_apMesh[0]->psVertex[i].pos.y, data.m_apMesh[0]->psVertex[i].pos.z) ;
		p *= mat ;//로칼좌표

		psVertex = NULL ;
		for(n=0 ; n<3 ; n++)//지형에서 제외된 삼각형 버텍스와 가장 가까운 버텍스를 일치시킨다.
		{
			Vector3 a = psTriangle->avVertex[n].vPos-Vector3(matTransform._41, matTransform._42, matTransform._43) ;
            if((float_abs(a.x-(p.x*0.1f))<=precision) && (float_abs(a.z-(p.z*0.1f))<=precision))
			{
				psVertex = &psTriangle->avVertex[n] ;
				break ;
			}
		}

        if(psVertex)
		{
			//TRACE("intersected pos[%02d](%10.07f %10.07f %10.07f)\r\n", i, enumVector(p)) ;
			pDestVertices->pos.x = psVertex->vPos.x ;
			pDestVertices->pos.y = psVertex->vPos.y ;
			pDestVertices->pos.z = psVertex->vPos.z ;

			pDestVertices->normal.x = psVertex->vNormal.x ;
			pDestVertices->normal.y = psVertex->vNormal.y ;
			pDestVertices->normal.z = psVertex->vNormal.z ;

			TRACE("equal vector p(%10.07f %10.07f %10.07f) nor(%10.07f %10.07f %10.07f)\r\n",
				enumVector(pDestVertices->pos), enumVector(pDestVertices->normal)) ;
		}
		else
		{
			COPYVECTOR3(pDestVertices->pos, p) ;
			D3DXVec3Transform(&v, &pDestVertices->pos, &matTransform) ;
			pDestVertices->pos.x = v.x/v.w ;
			pDestVertices->pos.y = v.y/v.w ;
			pDestVertices->pos.z = v.z/v.w ;

			nor.set(data.m_apMesh[0]->psVertex[i].normal.x, data.m_apMesh[0]->psVertex[i].normal.y, data.m_apMesh[0]->psVertex[i].normal.z) ;
			nor.x = nor.x*mat.m11 + nor.y*mat.m21 + nor.z*mat.m31 ;
			nor.y = nor.x*mat.m12 + nor.y*mat.m22 + nor.z*mat.m32 ;
			nor.z = nor.x*mat.m13 + nor.y*mat.m23 + nor.z*mat.m33 ;
			nor = nor.Normalize() ;
			COPYVECTOR3(pDestVertices->normal, nor) ;

			D3DXVec3Transform(&v, &pDestVertices->normal, &matRot) ;
			pDestVertices->normal.x = v.x/v.w ;
			pDestVertices->normal.y = v.y/v.w ;
			pDestVertices->normal.z = v.z/v.w ;
		}

		pDestVertices->texDecal.x = data.m_apMesh[0]->psVertex[i].tex.u ;
		pDestVertices->texDecal.y = data.m_apMesh[0]->psVertex[i].tex.v ;

		//float b1 = (pDestVertices->pos.x-psTriangle->avVertex[0].vPos.x) ;
		//float b2 = (pDestVertices->pos.z-psTriangle->avVertex[0].vPos.z) ;

		//float a1 = (((pDestVertices->pos.x-psTriangle->avVertex[0].vPos.x)/dx)*tex_dx) ;
		//float a2 = (((pDestVertices->pos.z-psTriangle->avVertex[0].vPos.z)/dz)*tex_dy) ;//여기도 음수니까 결국 양수

		pDestVertices->texNoise.x = tex_s.x + (((pDestVertices->pos.x-psTriangle->avVertex[0].vPos.x)/dx)*tex_dx) ;
		pDestVertices->texNoise.y = tex_s.y + (((pDestVertices->pos.z-psTriangle->avVertex[0].vPos.z)/dz)*tex_dy) ;
	}

	pVB->Unlock() ;

	if(pIB)
		SAFE_RELEASE(pIB) ;

    //Create IndexBuffer
	//if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pIB, NULL)))
	if(FAILED(pd3dDevice->CreateIndexBuffer(nNumIndex*sizeof(STerrainIndex1), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pIB, NULL)))
		return E_FAIL ;

	STerrainIndex1 *pDestIndices ;
	if(FAILED(pIB->Lock(0, nNumIndex*sizeof(STerrainIndex1), (void **)&pDestIndices, 0)))
		return E_FAIL ;

	for(i=0 ; i<data.m_apMesh[0]->nNumTriangle ; i++, pDestIndices++)
	{
		pDestIndices->i1 = data.m_apMesh[0]->psTriangle[i].anVertexIndex[0] ;
		pDestIndices->i2 = data.m_apMesh[0]->psTriangle[i].anVertexIndex[1] ;
		pDestIndices->i3 = data.m_apMesh[0]->psTriangle[i].anVertexIndex[2] ;
	}

    //for(i=0 ; i<nNumIndex ; i++, pSrcIndices++, pDestIndices++)
	//	*pDestIndices = *pSrcIndices ;

	pIB->Unlock() ;

	pTex = ptex ;
    nExceptionalIndex = exceptional_index ;
    bEnable = true ;

	if(sDecal.bEnable)
		sDecal.Release() ;

	_InitDecal(pd3dDevice, psTriangle, pcTerrain) ;

	if(!sCollisionHolecup.Import("../../Media/collisionData/Holecup.col"))
		return E_FAIL ;

	pcTerrain->m_sHeightMap.psHolecup = this ;

	return S_OK ;
}
void SHolecup::_InitDecal(LPDIRECT3DDEVICE9 pd3dDevice, geo::STriangle *psTriangle, CSecretTerrain *pcTerrain)
{
	//create decal
	int i, nNumIntersectedTriangle ;
	geo::STriangle asIntersectedTriangle[64] ;

	float dx = psTriangle->avVertex[1].vPos.x - psTriangle->avVertex[0].vPos.x ;
	float dz = psTriangle->avVertex[2].vPos.z - psTriangle->avVertex[0].vPos.z ;//여기서 음수
	float tex_dx = psTriangle->avVertex[1].vTex.x - psTriangle->avVertex[0].vTex.x ;
	float tex_dy = psTriangle->avVertex[2].vTex.y - psTriangle->avVertex[0].vTex.y ;
	Vector2 tex_s = psTriangle->avVertex[0].vTex ;

	Vector3 vCenter(matTransform._41+0.1725f, matTransform._42, matTransform._43-0.1725f) ;
	CSecretDecal decal ;
	decal.Initialize(pd3dDevice, Vector3(0, 1, 0), Vector3(1, 0, 0), 2, 2, 2) ;
	decal.SetNormalOffset(0.0005f) ;
	decal.SetPos(vCenter) ;
	decal.MakingBoundingPlanes() ;
	nNumIntersectedTriangle = pcTerrain->m_sHeightMap.GetTriangles(vCenter, 2.0f, asIntersectedTriangle) ;
	decal.AddInBoundTriangles(asIntersectedTriangle, nNumIntersectedTriangle) ;

	int nNumVertex = decal.GetVertexCount() ;
	int nNumIndex = decal.GetIndexCount() ;

	SDecalVertex *psDecalVertices = decal.GetVertices() ;
	SDecalIndex *psDecalIndices = decal.GetIndices() ;

    STerrainVertex1 *psVertices = new STerrainVertex1[nNumVertex] ;
	STerrainIndex1 *psIndices = new STerrainIndex1[nNumIndex] ;

	for(i=0 ; i<nNumVertex ; i++)
	{
		psVertices[i].pos = psDecalVertices[i].pos ;
		psVertices[i].normal = psDecalVertices[i].normal ;
		psVertices[i].texDecal = psDecalVertices[i].tex ;

		psVertices[i].texNoise.x = tex_s.x + (((psVertices[i].pos.x-psTriangle->avVertex[0].vPos.x)/dx)*tex_dx) ;
		psVertices[i].texNoise.y = tex_s.y + (((psVertices[i].pos.z-psTriangle->avVertex[0].vPos.z)/dz)*tex_dy) ;
	}

	for(i=0 ; i<nNumIndex ; i++)
	{
		psIndices[i].i1 = psDecalIndices[i].awIndex[0] ;
		psIndices[i].i2 = psDecalIndices[i].awIndex[1] ;
		psIndices[i].i3 = psDecalIndices[i].awIndex[2] ;
	}
	sDecal.Initialize(pd3dDevice, psVertices, nNumVertex, psIndices, nNumIndex) ;
	//pcTerrain->m_pcTexContainer->FindTexture("decal_holecup")
	sDecal.apTex[0] = pcTerrain->m_pcTexContainer->FindTexture("decal_holecup0") ;
	sDecal.apTex[1] = pcTerrain->m_pcTexContainer->FindTexture("decal_holecup1") ;
	sDecal.bEnable = true ;

	SAFE_DELETEARRAY(psVertices) ;
	SAFE_DELETEARRAY(psIndices) ;
}
LPDIRECT3DTEXTURE9 SHolecup::GetTextureLOD(Vector3 vCamera)
{
	float d = (vCamera-Vector3(matTransform._41, matTransform._42, matTransform._43)).Magnitude() ;
	if(d < 5.0f)
		return sDecal.apTex[0] ;
	return sDecal.apTex[1] ;
}
void SHolecup::SetLod(Vector3 vCamera)
{
	float d = (vCamera-Vector3(matTransform._41, matTransform._42, matTransform._43)).Magnitude() ;
	if(d < 5.0f)
		nLodlevel = 0 ;//close
	else
		nLodlevel = 1 ;//far
}
Vector3 SHolecup::GetHolecupPos()
{
	return Vector3(matTransform._41-0.0834f, matTransform._42, matTransform._43+0.0834f) ;
}

//####################################################
// STerrainLoadMethod
//####################################################
STerrainLoadMethod::STerrainLoadMethod()
{
	nMethod = 0 ;
	sprintf(szFileName, "") ;
	psTerrainFile = NULL ;
}
STerrainLoadMethod::~STerrainLoadMethod()
{
}
void STerrainLoadMethod::SetFileMethod(char *pstr)
{
	sprintf(szFileName, "%s", pstr) ;
	nMethod = STerrainLoadMethod::FROMFILE ;
}
void STerrainLoadMethod::SetMemoryMethod(STerrain_File *pterrainfile)
{
	psTerrainFile = pterrainfile ;
	nMethod = STerrainLoadMethod::FROMMEMORY ;
}

//####################################################
// CSecretTerrain
//####################################################
D3DVERTEXELEMENT9 decl_Terrain[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정 tu, tv 범프맵핑일 경우 사용
	D3DDECL_END()
} ;
D3DVERTEXELEMENT9 decl_Terrain1[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},//정점에 대한 설정
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},//법선에 대한 설정
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},//텍스쳐 좌표에 대한 설정 tu, tv 범프맵핑일 경우 사용
	{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},//텍스쳐 좌표에 대한 설정 tu, tv 범프맵핑일 경우 사용
	D3DDECL_END()
} ;

CSecretTerrain::CSecretTerrain()
{
	m_nAttr = 0 ;
	m_pcFrustum = NULL ;
	m_pcOctTree = NULL ;
	//m_pcZQuadTree = NULL ;
	m_pcQuadTree = NULL ;
	m_bEnable = false ;
	m_pcAtm = NULL ;
	m_pcTexContainer = NULL ;
	sprintf(m_szFileName, "") ;

	int i ;
	for(i=0 ; i<MAXNUM_TERRAINTEXTURES+1 ; i++)
		m_afTilingTex[i] = 0.0f ;

	m_afTilingTex[i] = 0.0f ;
	//ColorSlider(243 243 271)
	//m_vDiffuse.set(243.0f/255.0f, 243.0f/255.0f, 271.0f/255.0f) ;
	//m_vDiffuse.set(243.0f/255.0f, 243.0f/255.0f, 271.0f/255.0f) ;
	Vector3 vDiffuse1(243.0f/255.0f, 243.0f/255.0f, 271.0f/255.0f) ;
	Vector3 vDiffuse2(1.10294f, 1.10294f, 1.21274f) ;
	vDiffuse2 = vDiffuse2 - vDiffuse1 ;
	m_vDiffuse = vDiffuse1 + (vDiffuse2*0.3f) ;
	
	//m_vWaterColorTone.set(1.0f-0.0f, (255.0f-180.0f)/255.0f, (255.0f-242.0f)/255.0f) ;
	m_vWaterColorTone.set(0.0f, 100.0f/255.0f, 200.0f/255.0f) ;

	m_vSpecular.set(1.0f, 1.0f, 1.0f) ;

	m_nRoadCount = 0 ;

	m_fMipmapLodBias = 0.0f ;
}

CSecretTerrain::~CSecretTerrain()
{
}
void CSecretTerrain::Release()
{
	SAFE_DELETE(m_pcFrustum) ;
	m_sHeightMap.Release() ;
	SAFE_DELETE(m_pcOctTree) ;
	//SAFE_DELETE(m_pcZQuadTree) ;
	m_pcQuadTree->DeleteTiles() ;
	SAFE_DELETE(m_pcQuadTree) ;
	m_sTerrainMesh.Release() ;
	SAFE_DELETE(m_pcAtm) ;
	SAFE_DELETE(m_pcTexContainer) ;

    for(int i=0 ; i<MAXNUM_TERRAINROAD ; i++)
	{
		if(m_acRoad[i].bEnable)
			m_acRoad[i].Release() ;
	}
	m_nRoadCount = 0 ;

	m_sHolecup.Release() ;

	sprintf(m_szFileName, "") ;

	m_bEnable = false ;
}

#include "SecretProfile.h"

HRESULT CSecretTerrain::InitOctTree(CASEData *pcASEData, char *pszPath, LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex)//지형은 절대 하나의 메쉬로 해줘야 한다!!
{
	CSecretProfileManager *pcProfileMgr ;

	m_lEigenIndex = lIndex ;
	m_pd3dDevice = pd3dDevice ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;
	SMesh *psMesh = pcASEData->m_apMesh[0] ;

	
	int i, n, ntemp ;
	char str[256] ;

	pcProfileMgr = new CSecretProfileManager() ;
	pcProfileMgr->SetUsage(CSecretProfileManager::TRACETEXT) ;

	//##################################################################################################################//
	//Initialize Texture
	//##################################################################################################################//
	PROFILE_START(pcProfileMgr, InitTexture) ;

	psTerrainMesh->lNumTexture = pcASEData->m_psMaterial->nNumSubMaterial ;

    int nClassifyCount ;
	int *pnTextureID=NULL ;
	data::SClassify *psClassify=NULL ;
	pnTextureID = new int[psMesh->nNumTriangle] ;
	for(i=0 ; i<(int)psMesh->nNumTriangle ; i++)
	{
		pnTextureID[i] = psMesh->psTriangle[i].nMaterialID ;
		//anTextureID[i] = psMesh->psTriangle[i].nMaterialID ;
	}

	nClassifyCount = ClassifyArray(pnTextureID, psMesh->nNumTriangle, &psClassify) ;

	psTerrainMesh->lNumTexture = nClassifyCount ;
	psTerrainMesh->ppTexDecal = new LPDIRECT3DTEXTURE9[nClassifyCount] ;

	for(i=0 ; i<nClassifyCount ; i++)
	{
		ntemp = psClassify[i].nID ;
		sprintf(str, "%s%s", pszPath, pcASEData->m_psMaterial->psSubMaterial[ntemp].szDecaleName) ;
		if(FAILED(D3DXCreateTextureFromFile(m_pd3dDevice, str, &psTerrainMesh->ppTexDecal[i])))
			return E_FAIL ;

		TRACE("[%d] CreateTexture FileName[%s]\r\n", i, str) ;
	}

	SAFE_DELETEARRAY(pnTextureID) ;

	PROFILE_STOP(pcProfileMgr) ;

	//##################################################################################################################//

	//##################################################################################################################//
	//Initialize Vertices
	//##################################################################################################################//
	PROFILE_START(pcProfileMgr, InitVertices) ;

	psTerrainMesh->lNumVertex = psMesh->nNumVertex ;
	psTerrainMesh->psVertices = new TERRAINVERTEX[psTerrainMesh->lNumVertex] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(psTerrainMesh->lNumVertex*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &psTerrainMesh->pVB, NULL)))
		return E_FAIL ;

    //Lock VertexBuffer
    TERRAINVERTEX *pVertices ;
	if(FAILED(psTerrainMesh->pVB->Lock(0, psTerrainMesh->lNumVertex*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
		return E_FAIL ;

	Matrix4 matLocal ;
	Vector3 vPos ;
	int nVertices=0 ;

	memcpy(&matLocal, &psMesh->smatLocal, sizeof(Matrix4)) ;

	matLocal.Identity() ;

	//if cj.rmd
	matLocal.m11 = 0.2f ;
	matLocal.m22 = 0.2f ;
	matLocal.m33 = 0.2f ;
	matLocal.m42 = 40.0f ;//101.0f

	//matLocal.m11 = 4.0f ;
	//matLocal.m22 = 4.0f ;
	//matLocal.m33 = 4.0f ;
	
	bool bPassFirst=false ;
	Vector3 vMinPos, vMaxPos ;

    for(i=0 ; i<(int)psTerrainMesh->lNumVertex ; i++)
	{
		vPos.set(psMesh->psVertex[i].pos.x,	psMesh->psVertex[i].pos.y, psMesh->psVertex[i].pos.z) ;
		vPos *= matLocal ;

		psTerrainMesh->psVertices[nVertices].pos = D3DXVECTOR3(vPos.x, vPos.y, vPos.z) ;
		psTerrainMesh->psVertices[nVertices].normal = D3DXVECTOR3(psMesh->psVertex[i].normal.x, psMesh->psVertex[i].normal.y, psMesh->psVertex[i].normal.z) ;
		psTerrainMesh->psVertices[nVertices].t = D3DXVECTOR2(psMesh->psVertex[i].tex.u, psMesh->psVertex[i].tex.v) ;

		if(!bPassFirst)
		{
			vMinPos = vMaxPos = vPos ;
			bPassFirst=true ;
		}
		else
		{
			if(vMinPos.x > vPos.x)
				vMinPos.x = vPos.x ;
			if(vMinPos.y > vPos.y)
				vMinPos.y = vPos.y ;
			if(vMinPos.z > vPos.z)
				vMinPos.z = vPos.z ;

			if(vMaxPos.x < vPos.x)
				vMaxPos.x = vPos.x ;
			if(vMaxPos.y < vPos.y)
				vMaxPos.y = vPos.y ;
			if(vMaxPos.z < vPos.z)
				vMaxPos.z = vPos.z ;
		}

		pVertices->pos = psTerrainMesh->psVertices[nVertices].pos ;
		pVertices->normal = psTerrainMesh->psVertices[nVertices].normal ;
		pVertices->t = psTerrainMesh->psVertices[nVertices].t ;

		//TRACE("[%02d] P(%02.03f, %02.03f, %02.03f) T(%02.03f, %02.03f)\r\n",
		//	i,
		//	psTerrainMesh->psVertices[nVertices].pos.x,
		//	psTerrainMesh->psVertices[nVertices].pos.y,
		//	psTerrainMesh->psVertices[nVertices].pos.z,
		//	psTerrainMesh->psVertices[nVertices].t.x,
		//	psTerrainMesh->psVertices[nVertices].t.x
		//	) ;


		nVertices++ ;
		pVertices++ ;
	}

	psTerrainMesh->pVB->Unlock() ;

    PROFILE_STOP(pcProfileMgr) ;
	//##################################################################################################################//


    //##################################################################################################################//
	//Initialize Indices
	//##################################################################################################################//
	PROFILE_START(pcProfileMgr, InitIndices) ;

	psTerrainMesh->ppsIndices = new TERRAININDEX*[nClassifyCount] ;
    for(i=0 ; i<nClassifyCount ; i++)
		psTerrainMesh->ppsIndices[i] = new TERRAININDEX[psClassify[i].nCount] ;

	psTerrainMesh->pIndexSet = new std::vector<TERRAININDEX *>[nClassifyCount] ;
	psTerrainMesh->pIndices = new data::SArray<TERRAININDEX>[nClassifyCount] ;

	//STrnPolygon *psPolygons = new STrnPolygon[psMesh->nNumTriangle] ;
	psTerrainMesh->psPolygons = new STrnPolygon[psMesh->nNumTriangle] ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(psMesh->nNumTriangle*sizeof(TERRAININDEX), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &psTerrainMesh->pIB, NULL)))
		return E_FAIL ;

	//Lock IndexBuffer
	TERRAININDEX *psIndex ;
	if(FAILED(psTerrainMesh->pIB->Lock(0, psMesh->nNumTriangle*sizeof(TERRAININDEX), (void **)&psIndex, 0)))
		return E_FAIL ;

	int nTriangles=0 ;
	//int *pnIndexCount = new int[nClassifyCount] ;
	//memset(pnIndexCount, 0, sizeof(int)*nClassifyCount) ;
	psTerrainMesh->plNumIndex = new UINT[nClassifyCount] ;
    memset(psTerrainMesh->plNumIndex, 0, sizeof(UINT)*nClassifyCount) ;

	for(n=0 ; n<psMesh->nNumTriangle ; n++)
	{
		psIndex->i1 = psMesh->psTriangle[n].anVertexIndex[0] ;
		psIndex->i2 = psMesh->psTriangle[n].anVertexIndex[1] ;
		psIndex->i3 = psMesh->psTriangle[n].anVertexIndex[2] ;

        psTerrainMesh->psPolygons[nTriangles].nTexID = -1 ;
		for(i=0 ; i<nClassifyCount ; i++)
		{
			if(psMesh->psTriangle[n].nMaterialID == psClassify[i].nID)
			{
				//psTerrainMesh->ppsIndices[i][pnIndexCount[i]].i1 = psIndex->i1 ;
				//psTerrainMesh->ppsIndices[i][pnIndexCount[i]].i2 = psIndex->i2 ;
				//psTerrainMesh->ppsIndices[i][pnIndexCount[i]].i3 = psIndex->i3 ;
				psTerrainMesh->ppsIndices[i][psTerrainMesh->plNumIndex[i]].i1 = psIndex->i1 ;
				psTerrainMesh->ppsIndices[i][psTerrainMesh->plNumIndex[i]].i2 = psIndex->i2 ;
				psTerrainMesh->ppsIndices[i][psTerrainMesh->plNumIndex[i]].i3 = psIndex->i3 ;

				psTerrainMesh->psPolygons[nTriangles].nTexID = i ;
				psTerrainMesh->psPolygons[nTriangles].pIndex = &psTerrainMesh->ppsIndices[i][psTerrainMesh->plNumIndex[i]] ;
				psTerrainMesh->pIndexSet[i].push_back(&psTerrainMesh->ppsIndices[i][psTerrainMesh->plNumIndex[i]]) ;

				//pnIndexCount[i]++ ;
				psTerrainMesh->plNumIndex[i]++ ;

				//TRACE("[%02d] Index(%02d, %02d, %02d) MaterialID=%02d CurTexID=%02d\r\n",
				//	n,
				//	psTerrainMesh->psPolygons[nTriangles].pIndex->i1,
				//	psTerrainMesh->psPolygons[nTriangles].pIndex->i2,
				//	psTerrainMesh->psPolygons[nTriangles].pIndex->i3,
				//	psMesh->psTriangle[n].nMaterialID, psTerrainMesh->psPolygons[nTriangles].nTexID) ;
				break ;
			}
		}
        assert(psTerrainMesh->psPolygons[nTriangles].nTexID != -1) ;

		nTriangles++ ;
	}

	psTerrainMesh->pIB->Unlock() ;
	SAFE_DELETEARRAY(psClassify) ;

	for(i=0 ; i<nClassifyCount ; i++)
		psTerrainMesh->pIndices[i].Initialize(psTerrainMesh->plNumIndex[i]) ;

    PROFILE_STOP(pcProfileMgr) ;
	//##################################################################################################################//

	//SAFE_DELETEARRAY(pnIndexCount) ;

	PROFILE_START(pcProfileMgr, ReadyOctTree) ;

	float fSideLength ;
	Vector3 vtemp ;

	TRACE("###not yet Calculate exp###\r\n") ;
	TRACE("vMinPos (%03.03f %03.03f %03.03f)\r\n", enumVector(vMinPos)) ;
	TRACE("vMaxPos (%03.03f %03.03f %03.03f)\r\n", enumVector(vMaxPos)) ;

	fSideLength = (float)CloseToExp(-vMinPos.Magnitude(), 2.0f, true) ;
	vMinPos.set(fSideLength, fSideLength, fSideLength) ;

    vMinPos.y = -1.0f ;

	//fSideLength = (float)CloseToExp((vMaxPos-vMinPos).Magnitude(), 2.0f) ;
	//vMaxPos.set(vMinPos.x+fSideLength, vMinPos.y+fSideLength, vMinPos.z+fSideLength) ;
	fSideLength = (float)CloseToExp(vMaxPos.Magnitude(), 2.0f) ;
	vMaxPos.set(fSideLength, fSideLength, fSideLength) ;

	vMaxPos.y = 1.0f ;

	int nBigO = ComputeBigO(vMaxPos.x-vMinPos.x, MINIMUMCUBESIZE, 4) ;

	TRACE("###Calculate exp###\r\n") ;
	TRACE("vMinPos (%03.03f %03.03f %03.03f)\r\n", enumVector(vMinPos)) ;
	TRACE("vMaxPos (%03.03f %03.03f %03.03f)\r\n", enumVector(vMaxPos)) ;
	TRACE("Total Node=%d\r\n", nBigO) ;

	PROFILE_STOP(pcProfileMgr) ;
	PROFILE_START(pcProfileMgr, BuildOctTree) ;

	m_pcOctTree = new CSecretOctTree(vMinPos, vMaxPos) ;
	if(!m_pcOctTree->BuildTree(psTerrainMesh->psVertices, psTerrainMesh->psPolygons, psMesh->nNumTriangle, QuadDivide))// OctDivide))
		assert(false && "BuildTree was Failed") ;

	m_bEnable = true ;

	PROFILE_STOP(pcProfileMgr) ;

	delete pcProfileMgr ;

	return S_OK ;
}



void CSecretTerrain::ProcessFrustumCullingInOctTree()
{
	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;
	int i ;
	for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
		psTerrainMesh->pIndexSet[i].clear() ;

    for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
		psTerrainMesh->pIndices[i].nPushCount = 0 ;


	UINT lTreeCount=0 ;
	//m_pcOctTree->GetTriangleToRender(&m_cFrustum, psTerrainMesh->pIndexSet, &lTreeCount) ;
	m_pcOctTree->GetTriangleToRender(m_pcFrustum, psTerrainMesh->pIndices, &lTreeCount) ;

	//for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
	//	TRACE("psTerrainMesh->pIndices[%d].pushcount=%d\r\n", i, psTerrainMesh->pIndices[i].nPushCount) ;

	g_cDebugMsg.SetDebugMsg(10, "TreeCount=%d", lTreeCount) ;
}

void CSecretTerrain::ProcessAllInOctTree()
{
    STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;
	int i ;
	for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
		psTerrainMesh->pIndexSet[i].clear() ;

	UINT lTreeCount=0 ;
	m_pcOctTree->GetTriangleToRender(psTerrainMesh->pIndexSet, &lTreeCount) ;
}

bool less_AxisZ(CSecretOctTree *pnode1, CSecretOctTree *pnode2)
{
	return pnode1->GetCube()->vCenter.z < pnode1->GetCube()->vCenter.z ;
}

//현재 사용하고 있지 않음.
void CSecretTerrain::ProcessCullingInOctTree()//by Occlusion Culling
{
	std::vector<CSecretOctTree *> node ;
	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;
	int i ;

	for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
		psTerrainMesh->pIndexSet[i].clear() ;

	UINT lTreeCount=0 ;
	m_pcOctTree->GetNodeToRender(m_pcFrustum, &node, &lTreeCount) ;
	std::sort(node.begin(), node.end(), less_AxisZ) ;

	//float DepthLevel=0 ;
	//Vector3 vCenter, vPreCenter = (*node.begin())->m_sCube.vCenter * m_cFrustum.GetmatView() ;
	//std::vector<CSecretOctTree *>::iterator it ;
	//for(it=node.begin() ; it != node.end() ; it++)
	//{
	//	vCenter = (*it)->m_sCube.vCenter * m_cFrustum.GetmatView() ;
	//	if(VECTOR_EQ(vPreCenter, vCenter))
	//	{
	//	}
	//	else
	//	{
	//	}
	//}
}

//################################//
//           개선방향             //
//################################//
//인덱스버퍼를 현재 지형에서 사용하고 있는 텍스쳐 갯수만큼
//만들고 그것을 옥트리에 넣고 트리내에 있는 삼각형들을 그 버퍼로 그대로 받아낸다.
//그리고 바로 렌더링 해버린다.

HRESULT CSecretTerrain::RenderOctTree(SD3DEffect *psd3dEffect)
{
	HRESULT hr ;
	D3DXVECTOR4 vLocalLightPos, vLocalLookAt, vDiffuse ;
	D3DXMATRIX mat ;
	int i ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//Shader Setting
	psd3dEffect->pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &psd3dEffect->pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	D3DXMatrixInverse(&mat, NULL, &psd3dEffect->matWorld) ;
	D3DXVec4Transform(&vLocalLightPos, &psd3dEffect->vLightPos, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLightPos, &vLocalLightPos) ;

	D3DXVec4Transform(&vLocalLookAt, &psd3dEffect->vLookAt, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLookAt, &vLocalLookAt) ;

	mat = psd3dEffect->matWorld * psd3dEffect->matView * psd3dEffect->matProj ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWVP, &mat) ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWIT, &psd3dEffect->matWorld) ;

	m_pd3dDevice->SetStreamSource(0, psTerrainMesh->pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;

	int nTriangles, nTotalTriangles=0 ;
	//std::vector<TERRAININDEX *>::iterator it ;
	//std::vector<TERRAININDEX *> *pIndexSet ;
	//TERRAININDEX *pIndices ;
	data::SArray<TERRAININDEX> *pIndices ;
	for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
	{
		/*
		pIndexSet = &m_sTerrainMesh.pIndexSet[i] ;

		if(pIndexSet->size() == 0)
			continue ;

		nTriangles = (int)pIndexSet->size() ;

		if(FAILED(psTerrainMesh->pIB->Lock(0, nTriangles*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

        nTriangles=0 ;
		for(it=pIndexSet->begin() ; it != pIndexSet->end() ; it++)
		{
			pIndices->i1 = (*it)->i1 ;
			pIndices->i2 = (*it)->i2 ;
			pIndices->i3 = (*it)->i3 ;

			//TRACE("Render [%02d]Triangle Index(%02d, %02d, %02d)\r\n", nTriangles, pIndices->i1, pIndices->i2, pIndices->i3) ;
			nTriangles++ ;
			pIndices++ ;
		}

		psTerrainMesh->pIB->Unlock() ;

		vDiffuse.x = vDiffuse.y = vDiffuse.z = vDiffuse.w = 0.7f ;
		psd3dEffect->pEffect->SetVector(psd3dEffect->hvDiffuse, &vDiffuse) ;
        
		psd3dEffect->pEffect->SetTexture("DecaleMap", psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, nTriangles) ;
		*/

		pIndices = &psTerrainMesh->pIndices[i] ;
		if(pIndices->nPushCount == 0)
			continue ;

		nTriangles = pIndices->nPushCount ;

		m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(D3DFVF_TERRAINVERTEX)) ;
		m_pd3dDevice->SetIndices(0) ;

		vDiffuse.x = vDiffuse.y = vDiffuse.z = vDiffuse.w = 0.7f ;
		psd3dEffect->pEffect->SetVector(psd3dEffect->hvDiffuse, &vDiffuse) ;

		psd3dEffect->pEffect->SetTexture("DecaleMap", psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

		m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, psTerrainMesh->lNumVertex, pIndices->nPushCount, pIndices->ptype, D3DFMT_INDEX16,
			psTerrainMesh->psVertices, sizeof(psTerrainMesh->psVertices[0])) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	psd3dEffect->pEffect->EndPass() ;

	g_cDebugMsg.AddTriangles(nTotalTriangles) ;

	psd3dEffect->pDecl->Release() ;
	psd3dEffect->pDecl = NULL ;

	g_cDebugMsg.SetDebugMsg(8, "Base on OctTree Render Triangles=%03d", nTotalTriangles) ;

	return S_OK ;
}
HRESULT CSecretTerrain::RenderOctTree(CSecretD3DTerrainEffect *pcTerrainEffect)
{
	HRESULT hr ;
	int i ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = pcTerrainEffect->m_psEssentialElements ;
	pcTerrainEffect->m_pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &pcTerrainEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	//D3DXMatrixInverse(&mat, NULL, &psEffectElements->matWorld) ;
	//D3DXVec4Transform(&vLocalLightPos, &psEffectElements->vLightPos, &mat) ;
	//pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvLightPos, &vLocalLightPos) ;

	//D3DXVec4Transform(&vLocalLookAt, &psEffectElements->vLookAt, &mat) ;
	//pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvLookAt, &vLocalLookAt) ;

	//mat = psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	//pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

	D3DXVECTOR4 vLookAt, vSunDir, vCameraPos ;
	D3DXMATRIX mat ;

	vSunDir = D3DXVECTOR4(psEffectElements->vSunDir.x, psEffectElements->vSunDir.y, psEffectElements->vSunDir.z, psEffectElements->vSunDir.w) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSunDir, &vSunDir) ;

	vLookAt = D3DXVECTOR4(psEffectElements->vLookAt.x, psEffectElements->vLookAt.y, psEffectElements->vLookAt.z, psEffectElements->vLookAt.w) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvLookAt, &vLookAt) ;

	vCameraPos = D3DXVECTOR4(psEffectElements->vCameraPos.x, psEffectElements->vCameraPos.y, psEffectElements->vCameraPos.z, psEffectElements->vCameraPos.w) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvCameraPos, &vCameraPos) ;

	D3DXMatrixIdentity(&mat) ;
	mat = mat * psEffectElements->matView * psEffectElements->matProj ;
	pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

	D3DXVECTOR4 vDiffuse(1, 1, 1, 1) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvDiffuse, &vDiffuse) ;

	//m_sHeightMap.DrawCircleBrushing(0, 256, 100, 50, 0xffffffff) ;

	//pcTerrainEffect->m_pEffect->SetTexture("DecaleMap", m_sHeightMap.pTexDecal) ;
	pcTerrainEffect->m_pEffect->SetTexture("BaseMap", m_sHeightMap.pTexBase) ;
	pcTerrainEffect->m_pEffect->SetTexture("LayerMap01", m_sHeightMap.apTexLayer[0]) ;
	pcTerrainEffect->m_pEffect->SetTexture("AlphaMap01", m_sHeightMap.apTexAlpha[0]) ;
	pcTerrainEffect->m_pEffect->SetTexture("LayerMap02", m_sHeightMap.apTexLayer[1]) ;
	pcTerrainEffect->m_pEffect->SetTexture("AlphaMap02", m_sHeightMap.apTexAlpha[1]) ;
	////////////////////////////////////////////////////////////////////////////////////////////////////

	m_pd3dDevice->SetStreamSource(0, psTerrainMesh->pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;

	int nTriangles, nTotalTriangles=0 ;
	//std::vector<TERRAININDEX *>::iterator it ;
	//std::vector<TERRAININDEX *> *pIndexSet ;
	//TERRAININDEX *pIndices ;
	data::SArray<TERRAININDEX> *pIndices ;
	for(i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
	{
		/*
		pIndexSet = &m_sTerrainMesh.pIndexSet[i] ;

		if(pIndexSet->size() == 0)
			continue ;

		nTriangles = (int)pIndexSet->size() ;

		if(FAILED(psTerrainMesh->pIB->Lock(0, nTriangles*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

        nTriangles=0 ;
		for(it=pIndexSet->begin() ; it != pIndexSet->end() ; it++)
		{
			pIndices->i1 = (*it)->i1 ;
			pIndices->i2 = (*it)->i2 ;
			pIndices->i3 = (*it)->i3 ;

			//TRACE("Render [%02d]Triangle Index(%02d, %02d, %02d)\r\n", nTriangles, pIndices->i1, pIndices->i2, pIndices->i3) ;
			nTriangles++ ;
			pIndices++ ;
		}

		psTerrainMesh->pIB->Unlock() ;

		vDiffuse.x = vDiffuse.y = vDiffuse.z = vDiffuse.w = 0.7f ;
		psd3dEffect->pEffect->SetVector(psd3dEffect->hvDiffuse, &vDiffuse) ;
        
		psd3dEffect->pEffect->SetTexture("DecaleMap", psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, nTriangles) ;
		*/

		pIndices = &psTerrainMesh->pIndices[i] ;
		if(pIndices->nPushCount == 0)
			continue ;

		nTriangles = pIndices->nPushCount ;

		m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(D3DFVF_TERRAINVERTEX)) ;
		m_pd3dDevice->SetIndices(0) ;

		//vDiffuse.x = vDiffuse.y = vDiffuse.z = vDiffuse.w = 0.7f ;
		//psd3dEffect->pEffect->SetVector(psd3dEffect->hvDiffuse, &vDiffuse) ;
		//psd3dEffect->pEffect->SetTexture("DecaleMap", psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

		m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, psTerrainMesh->lNumVertex, pIndices->nPushCount, pIndices->ptype, D3DFMT_INDEX16,
			psTerrainMesh->psVertices, sizeof(psTerrainMesh->psVertices[0])) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	pcTerrainEffect->m_pEffect->EndPass() ;

	g_cDebugMsg.AddTriangles(nTotalTriangles) ;

	pcTerrainEffect->m_pDecl->Release() ;
	pcTerrainEffect->m_pDecl= NULL ;

	g_cDebugMsg.SetDebugMsg(8, "Base on OctTree Render Triangles=%03d", nTotalTriangles) ;

	return S_OK ;
}

HRESULT CSecretTerrain::RenderOctTree()
{
	D3DMATERIAL9 mtrl ;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9)) ;
	mtrl.Ambient.a = mtrl.Ambient.r = mtrl.Ambient.g = mtrl.Ambient.b = 0.7f ;
	mtrl.Diffuse.a = mtrl.Diffuse.r = mtrl.Diffuse.g = mtrl.Diffuse.b = 0.7f ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;

	m_pd3dDevice->SetStreamSource(0, psTerrainMesh->pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT) ;

	int nTriangles, nTotalTriangles=0 ;
	std::vector<TERRAININDEX *>::iterator it ;
	std::vector<TERRAININDEX *> *pIndexSet ;
	TERRAININDEX *pIndices ;
	for(int i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
	{
		pIndexSet = &m_sTerrainMesh.pIndexSet[i] ;

		if(pIndexSet->size() == 0)
			continue ;

		nTriangles = (int)pIndexSet->size() ;

		if(FAILED(psTerrainMesh->pIB->Lock(0, nTriangles*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

        nTriangles=0 ;
		for(it=pIndexSet->begin() ; it != pIndexSet->end() ; it++)
		{
			pIndices->i1 = (*it)->i1 ;
			pIndices->i2 = (*it)->i2 ;
			pIndices->i3 = (*it)->i3 ;

			//TRACE("Render [%02d]Triangle Index(%02d, %02d, %02d)\r\n", nTriangles, pIndices->i1, pIndices->i2, pIndices->i3) ;
			nTriangles++ ;
			pIndices++ ;
		}

		psTerrainMesh->pIB->Unlock() ;

		m_pd3dDevice->SetMaterial(&mtrl) ;
		m_pd3dDevice->SetTexture(0, psTerrainMesh->ppTexDecal[i]) ;

		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

        if(nTriangles)
			m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, nTriangles) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	g_cDebugMsg.AddTriangles(nTotalTriangles) ;

	return S_OK ;
}

HRESULT CSecretTerrain::RenderAllOctTree(SD3DEffect *psd3dEffect)
{
	HRESULT hr ;
	D3DXVECTOR4 vLocalLightPos, vLocalLookAt, vDiffuse ;
	D3DXMATRIX mat ;
	UINT i, n ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//Shader Setting

	psd3dEffect->pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &psd3dEffect->pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	D3DXMatrixInverse(&mat, NULL, &psd3dEffect->matWorld) ;
	D3DXVec4Transform(&vLocalLightPos, &psd3dEffect->vLightPos, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLightPos, &vLocalLightPos) ;

	D3DXVec4Transform(&vLocalLookAt, &psd3dEffect->vLookAt, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLookAt, &vLocalLookAt) ;

	mat = psd3dEffect->matWorld * psd3dEffect->matView * psd3dEffect->matProj ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWVP, &mat) ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWIT, &psd3dEffect->matWorld) ;

	m_pd3dDevice->SetStreamSource(0, psTerrainMesh->pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;
	
	TERRAININDEX *pIndices=NULL ;

	for(i=0 ; i<m_sTerrainMesh.lNumTexture ; i++)
	{
		if(FAILED(psTerrainMesh->pIB->Lock(0, m_sTerrainMesh.plNumIndex[i]*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

		for(n=0 ; n<m_sTerrainMesh.plNumIndex[i] ; n++)
			pIndices[n] = m_sTerrainMesh.ppsIndices[i][n] ;

		psTerrainMesh->pIB->Unlock() ;

		m_pd3dDevice->SetTexture(0, psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, m_sTerrainMesh.plNumIndex[i]) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	psd3dEffect->pEffect->EndPass() ;

	psd3dEffect->pDecl->Release() ;
	psd3dEffect->pDecl = NULL ;

	return S_OK ;

}

HRESULT CSecretTerrain::RenderAllOctTree()
{
	D3DMATERIAL9 mtrl ;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9)) ;
	mtrl.Ambient.a = mtrl.Ambient.r = mtrl.Ambient.g = mtrl.Ambient.b = 0.7f ;
	mtrl.Diffuse.a = mtrl.Diffuse.r = mtrl.Diffuse.g = mtrl.Diffuse.b = 0.7f ;

	STerrainMesh *psTerrainMesh = &m_sTerrainMesh ;

	m_pd3dDevice->SetStreamSource(0, psTerrainMesh->pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT) ;

	UINT i, n ;
	TERRAININDEX *pIndices=NULL ;

	for(i=0 ; i<m_sTerrainMesh.lNumTexture ; i++)
	{
		if(FAILED(psTerrainMesh->pIB->Lock(0, m_sTerrainMesh.plNumIndex[i]*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

		for(n=0 ; n<m_sTerrainMesh.plNumIndex[i] ; n++)
			pIndices[n] = m_sTerrainMesh.ppsIndices[i][n] ;

		psTerrainMesh->pIB->Unlock() ;

		m_pd3dDevice->SetMaterial(&mtrl) ;
		m_pd3dDevice->SetTexture(0, psTerrainMesh->ppTexDecal[i]) ;
		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, m_sTerrainMesh.plNumIndex[i]) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

    /*
	int nTriangles, nTotalTriangles=0 ;
	std::vector<TERRAININDEX *>::iterator it ;
	std::vector<TERRAININDEX *> *pIndexSet ;
	TERRAININDEX *pIndices ;
	for(int i=0 ; i<(int)m_sTerrainMesh.lNumTexture ; i++)
	{
		pIndexSet = &m_sTerrainMesh.pIndexSet[i] ;

		if(pIndexSet->size() == 0)
			continue ;

		nTriangles = pIndexSet->size() ;

		if(FAILED(psTerrainMesh->pIB->Lock(0, nTriangles*sizeof(TERRAININDEX), (void **)&pIndices, 0)))
			return E_FAIL ;

        nTriangles=0 ;
		for(it=pIndexSet->begin() ; it != pIndexSet->end() ; it++)
		{
			pIndices->i1 = (*it)->i1 ;
			pIndices->i2 = (*it)->i2 ;
			pIndices->i3 = (*it)->i3 ;

			//TRACE("Render [%02d]Triangle Index(%02d, %02d, %02d)\r\n", nTriangles, pIndices->i1, pIndices->i2, pIndices->i3) ;
			nTriangles++ ;
			pIndices++ ;
		}

		psTerrainMesh->pIB->Unlock() ;

		m_pd3dDevice->SetMaterial(&mtrl) ;
		m_pd3dDevice->SetTexture(0, psTerrainMesh->ppTexDecal[i]) ;

		m_pd3dDevice->SetIndices(psTerrainMesh->pIB) ;

		nTotalTriangles += nTriangles ;

        if(nTriangles)
			m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, psTerrainMesh->lNumVertex, 0, nTriangles) ;
	}

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	g_cDebugMsg.AddTriangles(nTotalTriangles) ;
	*/

	return S_OK ;
}

float FragmentHeight(STerrain_FragmentInfo *psFragmentInfo, float x, float z, int nWidth, int nHeight, int nPos)
{
	int x1, x2, z1, z2 ;

	x1 = (int)x ;
	if(float_eq(x, (float)x1) && (x1 > 0))//x가 소수점 이하가 없을경우
	{
		x1 = (int)x-1 ;
		x2 = (int)x ;
	}
	else
	{
		x1 = (int)x ;
		x2 = x1+1 ;
	}

	z1 = (int)z ;
	if(float_eq(z, (float)z1) && (z1 > 0))//z가 소수점 이하가 없을경우
	{
		z1 = (int)z-1 ;
		z2 = (int)z ;
	}
	else
	{
		z1 = (int)z ;
		z2 = z1+1 ;
	}

	if(x1 < 0)
		x1 = 0 ;
	if(x2 >= nWidth)
		x2 = nWidth-1 ;

	if(z1 < 0)
		z1 = 0 ;
	if(z2 >= nHeight)
		z2 = nHeight-1 ;

	float h ;
    
	if(nPos == 0)
		h = psFragmentInfo[(z1*nWidth)+x1].fElevation ;
	else if(nPos == 1)
		h = psFragmentInfo[(z1*nWidth)+x2].fElevation ;
	else if(nPos == 2)
		h = psFragmentInfo[(z2*nWidth)+x1].fElevation ;
	else if(nPos == 3)
		h = psFragmentInfo[(z2*nWidth)+x2].fElevation ;

	//TRACE("height=%g\r\n", h) ;

	return h ;
}

float CSecretTerrain::GetHeightfromTerrainFile(STerrain_FragmentInfo *psFragmentInfo, float x, float z, int nWidth, int nHeight)
{
	float h=0.0f ;
	float xx = x, zz = z ;

	if(float_less(x, -(float)(nWidth)/2.0f) || float_greater(x, (float)(nWidth)/2.0f))
		return h ;
	if(float_less(z, -(float)(nHeight)/2.0f) || float_greater(z, (float)(nHeight)/2.0f))
		return h ;

	//HeightMap 접근할때는 실제넓이(0-64)로 접근한다.
	x += (float)((nWidth)/2) ;
	z = (-z) + (float)((nHeight)/2) ;

	int x1, x2, z1, z2 ;

	x1 = (int)x ;
	if(float_eq(x, (float)x1) && (x1 > 0))//x가 소수점 이하가 없을경우
	{
		x1 = (int)x-2 ;
		x2 = (int)x ;
	}
	else
	{
		x1 = (int)x ;
		x2 = x1+2 ;
	}

	z1 = (int)z ;
	if(float_eq(z, (float)z1) && (z1 > 0))//z가 소수점 이하가 없을경우
	{
		z1 = (int)z-2 ;
		z2 = (int)z ;
	}
	else
	{
		z1 = (int)z ;
		z2 = z1+2 ;
	}

	geo::SLine line(Vector3(x, 1000.0f, -z), Vector3(0, -1.0f, 0), 2000.0f) ;

	if(float_less_eq(fabs(x-x1)+fabs(z-z1), 1.0f))//intersected left-top triangle ([z1][x1], [z1][x2], [z2][x1])
	{
		Vector3 vPos[3] ;

        //vPos[0] = Vector3((float)x1, psFragmentInfo[(z1*nWidth)+x1].fElevation, (float)-z1) ;
		//vPos[1] = Vector3((float)x2, psFragmentInfo[(z1*nWidth)+x2].fElevation, (float)-z1) ;
		//vPos[2] = Vector3((float)x1, psFragmentInfo[(z2*nWidth)+x1].fElevation, (float)-z2) ;
		vPos[0] = Vector3((float)x1, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 0), (float)-z1) ;
		vPos[1] = Vector3((float)x2, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 1), (float)-z1) ;
		vPos[2] = Vector3((float)x1, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 2), (float)-z2) ;

		geo::STriangle tri(vPos[0], vPos[1], vPos[2]) ;

		Vector3 p ;
		IntersectLinetoTriangle(line, tri, p, true) ;
		h = p.y ;

		//float u, v ;
		//IntesectXZProjTriangleToPoint(tri, Vector3(x, 0, -z), u, v) ;
		//h = (1-u-v)*tri.avVertex[0].vPos.y + u*tri.avVertex[1].vPos.y + v*tri.avVertex[2].vPos.y ;
	}
	else
	{
		Vector3 vPos[3] ;
		//vPos[0] = Vector3((float)x1, psFragmentInfo[(z2*nWidth)+x1].fElevation, (float)-z2) ;
		//vPos[1] = Vector3((float)x2, psFragmentInfo[(z1*nWidth)+x2].fElevation, (float)-z1) ;
		//vPos[2] = Vector3((float)x2, psFragmentInfo[(z2*nWidth)+x2].fElevation, (float)-z2) ;
		vPos[0] = Vector3((float)x1, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 2), (float)-z2) ;
		vPos[1] = Vector3((float)x2, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 1), (float)-z1) ;
		vPos[2] = Vector3((float)x2, FragmentHeight(psFragmentInfo, xx, zz, nWidth, nHeight, 3), (float)-z2) ;

		geo::STriangle tri(vPos[0], vPos[1], vPos[2]) ;

		Vector3 p ;
		IntersectLinetoTriangle(line, tri, p, true) ;
		h = p.y ;

		//float u, v ;
		//IntesectXZProjTriangleToPoint(tri, Vector3(x, 0, -z), u, v) ;
		//h = (1-u-v)*tri.avVertex[0].vPos.y + u*tri.avVertex[1].vPos.y + v*tri.avVertex[2].vPos.y ;
	}
    return h ;
}
Vector3 CSecretTerrain::ComputeNormalfromTerrainFile(STerrain_FragmentInfo *psFragmentInfo, int nCount, int nWidth, int nHeight)
{
	int nNeighbor[4] ;//0:up, 1:down, 2:left, 3:right

	nNeighbor[0] = nNeighbor[1] = nNeighbor[2] = nNeighbor[3] = -1 ;

	if(nCount < nWidth)//위쪽끝에 있는경우
	{
		nNeighbor[0] = nCount ;
	}
	else if(nCount >= (nWidth*(nHeight-1)))//아래쪽끝에 있는경우
	{
		nNeighbor[1] = nCount ;
	}
	else if((nCount%nWidth) == 0)//왼쪽끝에 있는경우
	{
		nNeighbor[2] = nCount ;
	}
	else if(((nCount+1)%nWidth) == 0)//오른쪽끝에 있는경우
	{
		nNeighbor[3] = nCount ;
	}

	if(nNeighbor[0] == -1)
	{
		nNeighbor[0] = nCount-nWidth ;
	}
	if(nNeighbor[1] == -1)
	{
		nNeighbor[1] = nCount+nWidth ;
	}
	if(nNeighbor[2] == -1)
	{
		nNeighbor[2] = nCount-1 ;
	}
	if(nNeighbor[3] == -1)
	{
		nNeighbor[3] = nCount+1 ;
	}

	//if(nCount >= 585)
	//{
	//	TRACE("Height up=%07.03f down=%07.03f left=%07.03f right=%07.03f\r\n",
	//		psFragmentInfo[nNeighbor[0]].fElevation,
	//		psFragmentInfo[nNeighbor[1]].fElevation,
	//		psFragmentInfo[nNeighbor[2]].fElevation,
	//		psFragmentInfo[nNeighbor[3]].fElevation) ;

	//	TRACE("du=%07.03f dv=%07.03f\r\n",
	//		(psFragmentInfo[nNeighbor[3]].fElevation-psFragmentInfo[nNeighbor[2]].fElevation),
	//		(psFragmentInfo[nNeighbor[1]].fElevation-psFragmentInfo[nNeighbor[0]].fElevation)) ;
	//}

	//Vector3 du, dv ;
	//du.set(2, 0, (psFragmentInfo[nNeighbor[3]].fElevation-psFragmentInfo[nNeighbor[2]].fElevation)*0.5f) ;
	//dv.set(0, 2, (psFragmentInfo[nNeighbor[1]].fElevation-psFragmentInfo[nNeighbor[0]].fElevation)*0.5f) ;

	Vector3 N(-psFragmentInfo[nNeighbor[3]].fElevation+psFragmentInfo[nNeighbor[2]].fElevation, 2,
		psFragmentInfo[nNeighbor[1]].fElevation-psFragmentInfo[nNeighbor[0]].fElevation) ;

	return N.Normalize() ;
}

void CSecretTerrain::ExportTerrainFile(STerrain_File *psTerrainFile)
{
	int i, n, c ;
	int nNumerator, nDenominator ;

	if(m_sHeightMap.nTileSize == 1)
        nNumerator = nDenominator = 1 ;
	else if(m_sHeightMap.nTileSize == 2)
	{
		nNumerator = 2 ;
		nDenominator = 1 ;
	}

	psTerrainFile->Initialize(m_sHeightMap.lWidth, m_sHeightMap.lHeight, nNumerator, nDenominator, (m_sHeightMap.nNumAlphaMap*4), m_sHeightMap.nAlphaMapSize, 0) ;

	for(i=0 ; i<(m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight) ; i++)
	{
		psTerrainFile->psFragmentInfo[i] = m_sHeightMap.psFragmentInfo[i] ;
		psTerrainFile->psFragmentInfo[i].fElevation = m_sHeightMap.pVertices[i].pos.y ;
	}

	int nNumLeftAlpha, nShift, anShift[] = {24, 16, 8, 0} ;
	BYTE *pbyBuffer = new BYTE[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;
	DWORD *pdwBuffer = new DWORD[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;

	int nAlphaMapCount=0 ;
	nNumLeftAlpha = psTerrainFile->psFileHeader->nNumAlphaMap ;//헤더의 알파레이어 장수
	for(i=0 ; i<m_sHeightMap.nNumAlphaMap ; i++)
	{
		m_sHeightMap.ReadAlphaTexture(i, pdwBuffer) ;

		int nColorPos ;
        nNumLeftAlpha -= 4 ;
		if(nNumLeftAlpha < 0)
			nColorPos = 4+nNumLeftAlpha ;
		else
			nColorPos = 4 ;

        for(n=0 ; n<nColorPos; n++)
		{
			nShift = anShift[n%4] ;
            TRACE("[%02d] shift=%d\r\n", n, nShift) ;
			for(c=0; c<(psTerrainFile->psFileHeader->nAlphaMapSize*psTerrainFile->psFileHeader->nAlphaMapSize); c++)
				pbyBuffer[c] = (BYTE)((pdwBuffer[c]>>nShift)&0xff) ;

			assert(nAlphaMapCount < psTerrainFile->psFileHeader->nNumAlphaMap) ;

			memcpy(psTerrainFile->ppbyAlphaValue[nAlphaMapCount++], pbyBuffer, sizeof(BYTE)*(psTerrainFile->psFileHeader->nAlphaMapSize*psTerrainFile->psFileHeader->nAlphaMapSize)) ;
		}
	}
	SAFE_DELETEARRAY(pbyBuffer) ;
	SAFE_DELETEARRAY(pdwBuffer) ;

	for(i=0 ; i<psTerrainFile->nNumTerrainLayer ; i++)
	{
		sprintf(psTerrainFile->ppszTerrainLayer[i], "%s", m_sHeightMap.asStrLayerName[i].GetString()) ;
		TRACE("Terrainlayer[%d]=%s\r\n", i, psTerrainFile->ppszTerrainLayer[i]) ;
	}

	for(i=0 ; i<psTerrainFile->nNumNoiseMap ; i++)
	{
		sprintf(psTerrainFile->ppszNoiseMap[i], "%s", m_sHeightMap.asStrNoiseTex[i].GetString()) ;
		TRACE("TerrainNoisemap[%d]=%s\r\n", i, psTerrainFile->ppszNoiseMap[i]) ;
	}
}

//trn File
//1. STerrain_FileHeader
//2. STerrain_MaterialInfo 갯수만큼
//3. STerrain_FragmentInfo 갯수만큼
//4. (AlphaMapSize * AlphaMapSize in Bytes) * nNumAlphaMap

bool CSecretTerrain::LoadTerrainFromMemory(STerrain_File *psTerrainFile, int nEigenIndex)
{
	char szMsg[256] ;
	int i ;

	m_lEigenIndex = nEigenIndex ;

	m_pcFrustum = new CSecretFrustum() ;

    m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/GroundMap/ground.txr", m_pd3dDevice) ;

	//sprintf(m_szFileName, "%s", pszFileName) ;

	if(psTerrainFile->psFileHeader->dwType != 0x154)
		return false ;

	//1x1 사이즈를 기준으로 얼마나 분할된것인지를 계산한다.
	float fdiv = (float)psTerrainFile->psFileHeader->nNumerator/(float)psTerrainFile->psFileHeader->nDenominator ;//1m x 1m 이하로는 할수 없다.
	//int nWidth = (int)((sHeader.lWidth/fdiv)+1) ;
	//int nHeight = (int)((sHeader.lHeight/fdiv)+1) ;
	m_sHeightMap.nTileSize = (int)fdiv ;

	m_sHeightMap.lWidth = psTerrainFile->psFileHeader->lWidth ;
	m_sHeightMap.lHeight = psTerrainFile->psFileHeader->lHeight ;
	m_sHeightMap.nVertexWidth = m_sHeightMap.lWidth/m_sHeightMap.nTileSize+1 ;
	m_sHeightMap.nVertexHeight = m_sHeightMap.lHeight/m_sHeightMap.nTileSize+1 ;
	m_sHeightMap.lNumVertices = m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight ;
	m_sHeightMap.lPrimitiveCount = (m_sHeightMap.nVertexWidth-1)*(m_sHeightMap.nVertexHeight-1)*2 ;

	TRACE("terrain width=%d height=%d TileSize=%d\r\n", psTerrainFile->psFileHeader->lWidth, psTerrainFile->psFileHeader->lHeight, m_sHeightMap.nTileSize) ;
	TRACE("terrain alphamap size=%d\r\n", psTerrainFile->psFileHeader->nAlphaMapSize) ;

	//분할된 갯수를 psFragmentInfo에 할당한다.
	//psFragmentInfo = new STerrain_FragmentInfo[sHeader.lWidth * sHeader.lHeight] ;
	m_sHeightMap.psFragmentInfo = new STerrain_FragmentInfo[m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight] ;
	STerrain_FragmentInfo *psFragmentInfo = m_sHeightMap.psFragmentInfo ;

	memcpy(psFragmentInfo, psTerrainFile->psFragmentInfo, sizeof(STerrain_FragmentInfo)*(m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight)) ;

	//지형위치에 따른 높이값 할당
	m_sHeightMap.ppfHeightMap = new float*[m_sHeightMap.nVertexHeight] ;
	for(i=0 ; i<m_sHeightMap.nVertexHeight ; i++)
		m_sHeightMap.ppfHeightMap[i] = new float[m_sHeightMap.nVertexWidth] ;

    assert(m_sHeightMap.lNumVertices > 0) ;
    m_sHeightMap.pVertices = new TERRAINVERTEX[m_sHeightMap.lNumVertices] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &m_sHeightMap.pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), 0, D3DFVF_TERRAINVERTEX, D3DPOOL_MANAGED, &m_sHeightMap.pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &m_sHeightMap.pVB, NULL)))
		return false ;

	g_cDebugMsg.AddVBSize(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX)) ;

    //Lock VertexBuffer
	void *pVertices ;
	if(FAILED(m_sHeightMap.pVB->Lock(0, m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
	{
		return false ;
	}

	TERRAINVERTEX v ;
	TERRAINVERTEX *pv=(TERRAINVERTEX *)pVertices, *pv1=m_sHeightMap.pVertices ;

	Vector3 n ;
	int nCount=0 ;//, nWidth=sHeader.lWidth, nHeight=sHeader.lHeight ;
    
	//여기에서는 실제 위치값을 계산하기 때문에 실제 Width, Height를 사용한다.
	float x, z, fMaxHeight, fMinHeight ;//int x, z ;
	fMaxHeight = fMinHeight = psFragmentInfo->fElevation ;
	for(z=0 ; z<=(float)psTerrainFile->psFileHeader->lHeight ; z += fdiv)
	{
		for(x=0 ; x<=(float)psTerrainFile->psFileHeader->lWidth ; x += fdiv, nCount++)
		{
			//Position

			float fLength ;

			fLength = (float)psTerrainFile->psFileHeader->lWidth ;
			v.pos.x = x-fLength/2.0f ;// (float)(x-(int)nWidth/2) ;

			fLength = (float)psTerrainFile->psFileHeader->lHeight ;
			v.pos.z = -(z-fLength/2.0f) ;//-(float)(z-(int)nHeight/2) ;
			v.pos.y = psFragmentInfo[nCount].fElevation ;

			if(fMaxHeight < v.pos.y)
				fMaxHeight = v.pos.y ;
			if(fMinHeight > v.pos.y)
				fMinHeight = v.pos.y ;

			v.t.x = psFragmentInfo[nCount].u ;
			v.t.y = psFragmentInfo[nCount].v ;

			//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n", nCount, enumVector(v.pos), v.t.x, v.t.y) ;

			//must compute NormalVector
			v.normal = D3DXVECTOR3(0, 1, 0) ;
			n = ComputeNormalfromTerrainFile(psFragmentInfo, nCount, m_sHeightMap.nVertexWidth, m_sHeightMap.nVertexHeight) ;//타일의 갯수가 64라면 버텍스는 65이기 때문에 +1을 해준다.
			//TRACE("[%04d] normal(%+07.03f %+07.03f %+07.03f)\r\n", nCount, enumVector(n)) ;
			v.normal = D3DXVECTOR3(n.x, n.y, n.z) ;

			*pv++ = v ;
			*pv1++ = v ;
		}
	}
	m_sHeightMap.pVB->Unlock() ;

	m_sHeightMap.fMaxElevation = fMaxHeight ;
	m_sHeightMap.fMinElevation = fMinHeight ;

	nCount = 0 ;
	for(z=0 ; z<(float)m_sHeightMap.nVertexHeight ; z += 1.0f)
	{
		for(x=0 ; x<(float)m_sHeightMap.nVertexWidth ; x += 1.0f, nCount++)
		{
			m_sHeightMap.ppfHeightMap[(int)z][(int)x] = psFragmentInfo[nCount].fElevation ;
		}
	}

	//###########################################################################################
	//Create Alpha Texture
	//###########################################################################################
	if(psTerrainFile->psFileHeader->nNumAlphaMap > 0)
	{
		m_sHeightMap.nAlphaMapSize = psTerrainFile->psFileHeader->nAlphaMapSize ;
		BYTE *pbyValue = new BYTE[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;
		DWORD *pdwValue = new DWORD[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;

		m_sHeightMap.nNumDrawVLine = m_sHeightMap.nAlphaMapSize ;
		m_sHeightMap.pbDrawVLine = new bool[m_sHeightMap.nNumDrawVLine] ;

		m_sHeightMap.nNumAlphaMap = psTerrainFile->psFileHeader->nNumAlphaMap/4 ;
		if(psTerrainFile->psFileHeader->nNumAlphaMap%4)
			m_sHeightMap.nNumAlphaMap++ ;

		for(i=0 ; i<m_sHeightMap.nNumAlphaMap ; i++)
		{
			if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_sHeightMap.apTexAlpha[i])))
			{
				sprintf(szMsg, "Cannot Create AlphaTexture") ;
				assert(false && szMsg) ;
				return false ;
			}
		}

		ZeroMemory(pdwValue, sizeof(DWORD)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize) ;
		nCount=0 ;
		bool bClear=false ;
		int anShift[] = {24, 16, 8, 0} ;
        for(i=0 ; i<psTerrainFile->psFileHeader->nNumAlphaMap ; i++)
		{
			//fread(pbyValue, sizeof(BYTE)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize, 1, pf) ;
			memcpy(pbyValue, psTerrainFile->ppbyAlphaValue[i], sizeof(BYTE)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize) ;

			int nFormatPos = (i+1)%4 ;
			int nShift = anShift[i%4] ;
			TRACE("[%02d] shift=%d\r\n", i, nShift) ;
            for(int n=0; n<(m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize); n++)
			{
				pdwValue[n] |= (DWORD)(pbyValue[n]<<nShift) ;
			}
			bClear = false ;

			if((i>0) && !nFormatPos)//fill texture every four
			{
				FillTextureWithBuffer(m_sHeightMap.apTexAlpha[nCount++], (BYTE *)pdwValue, 32) ;
				ZeroMemory(pdwValue, sizeof(DWORD)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize) ;
				bClear = true ;
			}
		}
		if(!bClear)
		{
			FillTextureWithBuffer(m_sHeightMap.apTexAlpha[nCount++], (BYTE *)pdwValue, 32) ;
		}

		SAFE_DELETEARRAY(pdwValue) ;
		SAFE_DELETEARRAY(pbyValue) ;
	}

	m_sHeightMap.pcActionContainer = new CActionBrushingContainer() ;
	m_sHeightMap.pcActionContainer->Initialize(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                 Create Texture                                                     //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	assert(psTerrainFile->nNumTerrainLayer == (MAXNUM_TERRAINTEXTURES+1)) ;
	for(i=0 ; i<(MAXNUM_TERRAINTEXTURES+1) ; i++)
	{
		if(!strcmp(psTerrainFile->ppszTerrainLayer[i], "suri"))
			m_sHeightMap.asStrLayerName[i].format("roughside") ;
		else
			m_sHeightMap.asStrLayerName[i].format("%s", psTerrainFile->ppszTerrainLayer[i]) ;
	}

	m_sHeightMap.pTexBase = m_pcTexContainer->FindTexture(m_sHeightMap.asStrLayerName[0].GetString()) ;
	for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
		m_sHeightMap.apTexLayer[i] = m_pcTexContainer->FindTexture(m_sHeightMap.asStrLayerName[i+1].GetString()) ;


	assert(psTerrainFile->nNumNoiseMap == MAXNUM_TERRAINNOISETEX) ;
	for(i=0 ; i<MAXNUM_TERRAINNOISETEX ; i++)
		m_sHeightMap.asStrNoiseTex[i].format("%s", psTerrainFile->ppszNoiseMap[i]) ;

	m_sHeightMap.pTexNoise00 = m_pcTexContainer->FindTexture(m_sHeightMap.asStrNoiseTex[0].GetString()) ;
	m_sHeightMap.pTexNoise01 = m_pcTexContainer->FindTexture(m_sHeightMap.asStrNoiseTex[1].GetString()) ;

	for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
		m_sHeightMap.apSaveKeepingAlpha[i] = m_sHeightMap.apTexAlpha[i] ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Create QuadTree 
	//m_pcZQuadTree = new ZQuadTree(nWidth, nHeight) ;
	//m_pcZQuadTree->Build(m_sHeightMap.pVertices) ;

	assert(m_sHeightMap.lPrimitiveCount > 0) ;

    //Initialize IndexBuffer
	//(nWidth-1) means Number of rectangle in grid
	//horizon rectangle * vertical rectangle * 2 is Number of triangle in grid
	m_sHeightMap.pIndices = new TERRAININDEX[m_sHeightMap.lPrimitiveCount] ;
	m_sHeightMap.psOriginIndices = new TERRAININDEX[m_sHeightMap.lPrimitiveCount] ;
	//m_sHeightMap.psTriangles = new geo::STriangle[m_sHeightMap.lPrimitiveCount/3] ;
	//int s = sizeof(geo::STriangle) ;
	//m_sHeightMap.psTriangles = new geo::STriangle[10] ;

	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_sHeightMap.pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_sHeightMap.pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_sHeightMap.pIB, NULL)))
	{
		return false ;
	}

	g_cDebugMsg.AddIBSize(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX)) ;

	//Lock IndexBuffer
	TERRAININDEX sIndex, *psIndex, *psIndex1=m_sHeightMap.pIndices, *psIndex2=m_sHeightMap.psOriginIndices ;
	//geo::STriangle *pTriangles = m_sHeightMap.psTriangles ;
	if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&psIndex, 0)))
		return false ;

	m_sHeightMap.lNumTriangles=0 ;

	for(z=0 ; z<m_sHeightMap.nVertexHeight-1 ; z++)
	{
		for(x=0 ; x<m_sHeightMap.nVertexWidth-1 ; x++)
		{
			sIndex.i1 = (DWORD)(z*m_sHeightMap.nVertexHeight+x) ;
			sIndex.i2 = (DWORD)(z*m_sHeightMap.nVertexHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+x) ;

			//TRACE("index(%03d %03d %03d)\r\n", sIndex.i1, sIndex.i2, sIndex.i3) ;

			*psIndex++ = sIndex ;
            *psIndex1++ = sIndex ;
			*psIndex2++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;

			sIndex.i1 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+x) ;
			sIndex.i2 = (DWORD)(z*m_sHeightMap.nVertexHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+(x+1)) ;

			//TRACE("index(%03d %03d %03d)\r\n", sIndex.i1, sIndex.i2, sIndex.i3) ;

			*psIndex++ = sIndex ;
			*psIndex1++ = sIndex ;
			*psIndex2++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;
		}
	}
	m_sHeightMap.pIB->Unlock() ;

	//"rough", "hazard", "tee", "fairway", "bunker", "green", "greenside", "suri"

	for(i=0 ; i<MAXNUM_TERRAINTEXTURES+1 ; i++)
		//m_afTilingTex[i] = (float)m_sHeightMap.nVertexWidth*6.0f ;
		m_afTilingTex[i] = (float)m_sHeightMap.nVertexWidth*0.5f ;

	//m_afTilingTex[1] = (float)m_sHeightMap.nVertexWidth*1.0f ;//rough
	//m_afTilingTex[3] = (float)m_sHeightMap.nVertexWidth*0.55f ;//tee
	//m_afTilingTex[4] = (float)m_sHeightMap.nVertexWidth*0.55f ;//fairway
	m_afTilingTex[0] = 292.5f ;//ob
	m_afTilingTex[1] = (float)m_sHeightMap.nVertexWidth*2.0f ;//rough

	//m_afTilingTex[2] = 656.5f ;//hazard
	m_afTilingTex[2] = 292.5f ;//hazard

	m_afTilingTex[3] = (float)m_sHeightMap.nVertexWidth*0.55f*2.0f ;//tee
	m_afTilingTex[4] = (float)m_sHeightMap.nVertexWidth*0.55f*2.0f ;//fairway
	m_afTilingTex[6] = (float)m_sHeightMap.nVertexWidth ;//green
	m_afTilingTex[7] = (float)m_sHeightMap.nVertexWidth ;//greenside
	m_afTilingTex[8] = (float)m_sHeightMap.nVertexWidth ;//roughside

	//texture resize
	m_afTilingTex[6] *= 0.5f ;
	m_afTilingTex[4] *= 0.95f ;//fairway
	m_afTilingTex[1] *= 0.5f ;//rough
	m_afTilingTex[3] *= 0.95f ;//tee

	//Create QuadTree
	m_pcQuadTree = new CSecretQuadTree(m_sHeightMap.nVertexWidth, m_sHeightMap.nVertexHeight) ;
	m_pcQuadTree->Build(m_sHeightMap.pVertices, fMaxHeight, fMinHeight, m_sHeightMap.nTileSize) ;

	m_sHeightMap.bEnable=true ;
	m_bWireFrame = false ;

	m_pcAtm = new CSecretTerrainScatteringData ;
	m_pcAtm->CalculateShaderParameterTerrainOptimized() ;

	m_nRoadCount = 0 ;

    m_bEnable = true ;
	return true ;
}

bool CSecretTerrain::LoadTerrainFile(char *pszFileName, int nEigenIndex)
{
	FILE *pf=NULL ;
	char szMsg[256] ;
	int i ;

	m_lEigenIndex = nEigenIndex ;

	m_pcFrustum = new CSecretFrustum() ;

	//CTextureResourceFile txr ;
	//txr.Savetxr("../../Media/GroundMap/", "ground.txr", m_pd3dDevice) ;

    m_pcTexContainer = new CSecretTextureContainer() ;
	m_pcTexContainer->Initialize("../../Media/GroundMap/ground.txr", m_pd3dDevice) ;

	STerrain_FileHeader sHeader ;
	STerrain_MaterialInfo *psMaterialInfo=NULL ;
	STerrain_FragmentInfo *psFragmentInfo=NULL ;

	pf = fopen(pszFileName, "rb+") ;
	if(pf == NULL)
	{
		fclose(pf) ;
		return false ;
	}

	sprintf(m_szFileName, "%s", pszFileName) ;

    fread(&sHeader, sizeof(STerrain_FileHeader), 1, pf) ;
	if(sHeader.dwType != 0x154)
	{
		fclose(pf) ;
		return false ;
	}

	if(sHeader.lNumMaterial > 0)
	{
		m_sHeightMap.psMaterialInfo = new STerrain_MaterialInfo[sHeader.lNumMaterial] ;
		psMaterialInfo = m_sHeightMap.psMaterialInfo ;
		fread(psMaterialInfo, sizeof(STerrain_MaterialInfo)*sHeader.lNumMaterial, 1, pf) ;
	}

	//if((sHeader.nNumerator/sHeader.nDenominator) != 1)
	//{
	//	fclose(pf) ;
	//	assert(false && "div isn't one") ;
	//	return false ;
	//}

	//1x1 사이즈를 기준으로 얼마나 분할된것인지를 계산한다.
	float fdiv = (float)sHeader.nNumerator/(float)sHeader.nDenominator ;//1m x 1m 이하로는 할수 없다.
	//int nWidth = (int)((sHeader.lWidth/fdiv)+1) ;
	//int nHeight = (int)((sHeader.lHeight/fdiv)+1) ;
	m_sHeightMap.nTileSize = (int)fdiv ;

	m_sHeightMap.lWidth = sHeader.lWidth ;
	m_sHeightMap.lHeight = sHeader.lHeight ;
	m_sHeightMap.nVertexWidth = m_sHeightMap.lWidth/m_sHeightMap.nTileSize+1 ;
	m_sHeightMap.nVertexHeight = m_sHeightMap.lHeight/m_sHeightMap.nTileSize+1 ;
	m_sHeightMap.lNumVertices = m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight ;
	m_sHeightMap.lPrimitiveCount = (m_sHeightMap.nVertexWidth-1)*(m_sHeightMap.nVertexHeight-1)*2 ;

	TRACE("terrain width=%d height=%d TileSize=%d\r\n", sHeader.lWidth, sHeader.lHeight, m_sHeightMap.nTileSize) ;
	TRACE("terrain alphamap size=%d\r\n", sHeader.nAlphaMapSize) ;

	//분할된 갯수를 psFragmentInfo에 할당한다.
	//psFragmentInfo = new STerrain_FragmentInfo[sHeader.lWidth * sHeader.lHeight] ;
	m_sHeightMap.psFragmentInfo = new STerrain_FragmentInfo[m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight] ;
	psFragmentInfo = m_sHeightMap.psFragmentInfo ;

	fread(psFragmentInfo, sizeof(STerrain_FragmentInfo)*(m_sHeightMap.nVertexWidth*m_sHeightMap.nVertexHeight), 1, pf) ;

	//지형위치에 따른 높이값 할당
	m_sHeightMap.ppfHeightMap = new float*[m_sHeightMap.nVertexHeight] ;
	for(i=0 ; i<m_sHeightMap.nVertexHeight ; i++)
		m_sHeightMap.ppfHeightMap[i] = new float[m_sHeightMap.nVertexWidth] ;

    assert(m_sHeightMap.lNumVertices > 0) ;
    m_sHeightMap.pVertices = new TERRAINVERTEX[m_sHeightMap.lNumVertices] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &m_sHeightMap.pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), 0, D3DFVF_TERRAINVERTEX, D3DPOOL_MANAGED, &m_sHeightMap.pVB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), 0, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &m_sHeightMap.pVB, NULL)))
		return false ;

	g_cDebugMsg.AddVBSize(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX)) ;

    //Lock VertexBuffer
	void *pVertices ;
	if(FAILED(m_sHeightMap.pVB->Lock(0, m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
	{
		return false ;
	}

	TERRAINVERTEX v ;
	TERRAINVERTEX *pv=(TERRAINVERTEX *)pVertices, *pv1=m_sHeightMap.pVertices ;

	Vector3 n ;
	int nCount=0 ;//, nWidth=sHeader.lWidth, nHeight=sHeader.lHeight ;
    
	//여기에서는 실제 위치값을 계산하기 때문에 실제 Width, Height를 사용한다.
	float x, z, fMaxHeight, fMinHeight ;//int x, z ;
	fMaxHeight = fMinHeight = psFragmentInfo->fElevation ;
	for(z=0 ; z<=(float)sHeader.lHeight ; z += fdiv)
	{
		for(x=0 ; x<=(float)sHeader.lWidth ; x += fdiv, nCount++)
		{
			//Position

			float fLength ;

			fLength = (float)sHeader.lWidth ;
			v.pos.x = x-fLength/2.0f ;// (float)(x-(int)nWidth/2) ;

			fLength = (float)sHeader.lHeight ;
			v.pos.z = -(z-fLength/2.0f) ;//-(float)(z-(int)nHeight/2) ;
			v.pos.y = psFragmentInfo[nCount].fElevation ;

			if(fMaxHeight < v.pos.y)
				fMaxHeight = v.pos.y ;
			if(fMinHeight > v.pos.y)
				fMinHeight = v.pos.y ;

			v.t.x = psFragmentInfo[nCount].u ;
			v.t.y = psFragmentInfo[nCount].v ;

			//TRACE("[%02d] pos(%+07.03f %+07.03f %+07.03f) tex(%+07.03f %+07.03f)\r\n", nCount, enumVector(v.pos), v.t.x, v.t.y) ;

			//must compute NormalVector
			v.normal = D3DXVECTOR3(0, 1, 0) ;
			n = ComputeNormalfromTerrainFile(psFragmentInfo, nCount, m_sHeightMap.nVertexWidth, m_sHeightMap.nVertexHeight) ;//타일의 갯수가 64라면 버텍스는 65이기 때문에 +1을 해준다.
			//TRACE("[%04d] normal(%+07.03f %+07.03f %+07.03f)\r\n", nCount, enumVector(n)) ;
			v.normal = D3DXVECTOR3(n.x, n.y, n.z) ;

			*pv++ = v ;
			*pv1++ = v ;
		}
	}
	m_sHeightMap.pVB->Unlock() ;

	m_sHeightMap.fMaxElevation = fMaxHeight ;
	m_sHeightMap.fMinElevation = fMinHeight ;

	nCount = 0 ;
	for(z=0 ; z<(float)m_sHeightMap.nVertexHeight ; z += 1.0f)
	{
		for(x=0 ; x<(float)m_sHeightMap.nVertexWidth ; x += 1.0f, nCount++)
		{
			m_sHeightMap.ppfHeightMap[(int)z][(int)x] = psFragmentInfo[nCount].fElevation ;
		}
	}

	if(sHeader.nNumAlphaMap > 0)
	{
		m_sHeightMap.nAlphaMapSize = sHeader.nAlphaMapSize ;
		BYTE *pbyValue = new BYTE[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;
		DWORD *pdwValue = new DWORD[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;

		m_sHeightMap.nNumDrawVLine = m_sHeightMap.nAlphaMapSize ;
		m_sHeightMap.pbDrawVLine = new bool[m_sHeightMap.nNumDrawVLine] ;

		m_sHeightMap.nNumAlphaMap = sHeader.nNumAlphaMap/4 ;
		if(sHeader.nNumAlphaMap%4)
			m_sHeightMap.nNumAlphaMap++ ;

		for(i=0 ; i<m_sHeightMap.nNumAlphaMap ; i++)
		{
			if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_sHeightMap.apTexAlpha[i])))
			{
				sprintf(szMsg, "Cannot Create AlphaTexture") ;
				assert(false && szMsg) ;
				return false ;
			}
		}

		ZeroMemory(pdwValue, sizeof(DWORD)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize) ;
		nCount=0 ;
		bool bClear=false ;
		int anShift[] = {24, 16, 8, 0} ;
        for(i=0 ; i<sHeader.nNumAlphaMap ; i++)
		{
			fread(pbyValue, sizeof(BYTE)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize, 1, pf) ;

			int nFormatPos = (i+1)%4 ;
			int nShift = anShift[i%4] ;
			TRACE("[%02d] shift=%d\r\n", i, nShift) ;
            for(int n=0; n<(m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize); n++)
			{
				pdwValue[n] |= (DWORD)(pbyValue[n]<<nShift) ;
			}
			bClear = false ;

			if((i>0) && !nFormatPos)//fill texture every four
			{
				FillTextureWithBuffer(m_sHeightMap.apTexAlpha[nCount++], (BYTE *)pdwValue, 32) ;
				ZeroMemory(pdwValue, sizeof(DWORD)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize) ;
				bClear = true ;
			}
		}
		if(!bClear)
		{
			FillTextureWithBuffer(m_sHeightMap.apTexAlpha[nCount++], (BYTE *)pdwValue, 32) ;
		}

		/*
		//for(i=0 ; i<sHeader.nNumAlphaMap ; i++)
		for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
		{
			fread(pbyValue, sizeof(BYTE)*m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize, 1, pf) ;

			//if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_sHeightMap.apTexAlpha[i])))
			if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_A8, D3DPOOL_MANAGED, &m_sHeightMap.apTexAlpha[i])))
			//if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &m_sHeightMap.apTexAlpha[i])))
			{
				sprintf(szMsg, "Cannot Create AlphaTexture") ;
				assert(false && szMsg) ;
				return false ;
			}
			FillAlphaTexture(m_sHeightMap.apTexAlpha[i], pbyValue) ;
			m_anEnableTex[i] = TRUE ;

			D3DSURFACE_DESC desc ;
			ZeroMemory(&desc, sizeof(D3DSURFACE_DESC)) ;
			m_sHeightMap.apTexAlpha[i]->GetLevelDesc(0, &desc) ;
			g_cDebugMsg.AddTexSize(desc.Width*desc.Height) ;
		}
		*/

		SAFE_DELETEARRAY(pdwValue) ;
		SAFE_DELETEARRAY(pbyValue) ;		
	}

	SAFE_DELETEARRAY(psMaterialInfo) ;

	fclose(pf) ;

	m_sHeightMap.pcActionContainer = new CActionBrushingContainer() ;
	m_sHeightMap.pcActionContainer->Initialize(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                 Create Texture                                                     //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//m_sHeightMap.pTexBase = m_pcTexContainer->FindTexture("rough") ;
	m_sHeightMap.pTexBase = m_pcTexContainer->FindTexture("ob") ;
	m_sHeightMap.asStrLayerName[0].format("ob") ;

	for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
	{
		m_sHeightMap.apTexLayer[i] = m_pcTexContainer->FindTexture(pszTerrainLayerOder[i]) ;
		m_sHeightMap.asStrLayerName[i+1].format("%s", pszTerrainLayerOder[i]) ;
	}

	m_sHeightMap.pTexNoise00 = m_pcTexContainer->FindTexture("ground_noise01") ;
	m_sHeightMap.asStrNoiseTex[0].format("ground_noise01") ;

	m_sHeightMap.pTexNoise01 = m_pcTexContainer->FindTexture("ground_noise02") ;
	m_sHeightMap.asStrNoiseTex[1].format("ground_noise02") ;

    for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
		m_sHeightMap.apSaveKeepingAlpha[i] = m_sHeightMap.apTexAlpha[i] ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Create QuadTree 
	//m_pcZQuadTree = new ZQuadTree(nWidth, nHeight) ;
	//m_pcZQuadTree->Build(m_sHeightMap.pVertices) ;

	assert(m_sHeightMap.lPrimitiveCount > 0) ;

    //Initialize IndexBuffer
	//(nWidth-1) means Number of rectangle in grid
	//horizon rectangle * vertical rectangle * 2 is Number of triangle in grid
	m_sHeightMap.pIndices = new TERRAININDEX[m_sHeightMap.lPrimitiveCount] ;
	m_sHeightMap.psOriginIndices = new TERRAININDEX[m_sHeightMap.lPrimitiveCount] ;
	//m_sHeightMap.psTriangles = new geo::STriangle[m_sHeightMap.lPrimitiveCount/3] ;
	//int s = sizeof(geo::STriangle) ;
	//m_sHeightMap.psTriangles = new geo::STriangle[10] ;

	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_sHeightMap.pIB, NULL)))
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_sHeightMap.pIB, NULL)))
	//if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_sHeightMap.pIB, NULL)))
	{
		return false ;
	}

	g_cDebugMsg.AddIBSize(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX)) ;

	//Lock IndexBuffer
	TERRAININDEX sIndex, *psIndex, *psIndex1=m_sHeightMap.pIndices, *psIndex2=m_sHeightMap.psOriginIndices ;
	//geo::STriangle *pTriangles = m_sHeightMap.psTriangles ;
	if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&psIndex, 0)))
		return false ;

	m_sHeightMap.lNumTriangles=0 ;

	for(z=0 ; z<m_sHeightMap.nVertexHeight-1 ; z++)
	{
		for(x=0 ; x<m_sHeightMap.nVertexWidth-1 ; x++)
		{
			sIndex.i1 = (DWORD)(z*m_sHeightMap.nVertexHeight+x) ;
			sIndex.i2 = (DWORD)(z*m_sHeightMap.nVertexHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+x) ;

			//TRACE("index(%03d %03d %03d)\r\n", sIndex.i1, sIndex.i2, sIndex.i3) ;

			*psIndex++ = sIndex ;
            *psIndex1++ = sIndex ;
			*psIndex2++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;

			sIndex.i1 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+x) ;
			sIndex.i2 = (DWORD)(z*m_sHeightMap.nVertexHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*m_sHeightMap.nVertexHeight+(x+1)) ;

			//TRACE("index(%03d %03d %03d)\r\n", sIndex.i1, sIndex.i2, sIndex.i3) ;

			*psIndex++ = sIndex ;
			*psIndex1++ = sIndex ;
			*psIndex2++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;
		}
	}
	m_sHeightMap.pIB->Unlock() ;

	//"rough", "hazard", "tee", "fairway", "bunker", "green", "greenside", "suri"

	for(i=0 ; i<MAXNUM_TERRAINTEXTURES+1 ; i++)
		//m_afTilingTex[i] = (float)m_sHeightMap.nVertexWidth*6.0f ;
		m_afTilingTex[i] = (float)m_sHeightMap.nVertexWidth*0.5f ;

	//m_afTilingTex[1] = (float)m_sHeightMap.nVertexWidth*1.0f ;//rough
	//m_afTilingTex[3] = (float)m_sHeightMap.nVertexWidth*0.55f ;//tee
	//m_afTilingTex[4] = (float)m_sHeightMap.nVertexWidth*0.55f ;//fairway
	m_afTilingTex[0] = 292.5f ;//ob
	m_afTilingTex[1] = (float)m_sHeightMap.nVertexWidth*2.0f ;//rough
	
	//m_afTilingTex[2] = 656.5f ;//hazard
	m_afTilingTex[2] = 292.5f ;//hazard

	m_afTilingTex[3] = (float)m_sHeightMap.nVertexWidth*0.55f*2.0f ;//tee
	m_afTilingTex[4] = (float)m_sHeightMap.nVertexWidth*0.55f*2.0f ;//fairway

	m_afTilingTex[6] = (float)m_sHeightMap.nVertexWidth ;//green
	m_afTilingTex[7] = (float)m_sHeightMap.nVertexWidth ;//greenside
	m_afTilingTex[8] = (float)m_sHeightMap.nVertexWidth ;//roughside

	//texture resize
	m_afTilingTex[6] *= 0.5f ;
	m_afTilingTex[4] *= 0.95f ;//fairway
	m_afTilingTex[1] *= 0.5f ;//rough
	m_afTilingTex[3] *= 0.95f ;//tee

	TRACE("ready quadtree\r\n") ;
	CSecretTimer quadtree_timer ;
	quadtree_timer.Initialize_real() ;

	//Create QuadTree
	m_pcQuadTree = new CSecretQuadTree(m_sHeightMap.nVertexWidth, m_sHeightMap.nVertexHeight) ;
	m_pcQuadTree->Build(m_sHeightMap.pVertices, fMaxHeight, fMinHeight, m_sHeightMap.nTileSize) ;

	float dur = quadtree_timer.Process_real() ;
	TRACE("quadtree elapsed time=%g\r\n", dur) ;
    
	m_sHeightMap.bEnable=true ;
	m_bWireFrame = false ;

	m_pcAtm = new CSecretTerrainScatteringData ;
	m_pcAtm->CalculateShaderParameterTerrainOptimized() ;

	m_nRoadCount = 0 ;

    m_bEnable = true ;
	return true ;
}
#include <vector>

HRESULT CSecretTerrain::InitHeightMap(char *pszHeightMap, char *pszTexture, LPDIRECT3DDEVICE9 pd3dDevice, CActionManagement *pcActionManagement)
{
	m_pd3dDevice = pd3dDevice ;

	int nWidth, nHeight ;
	int z, x ;

	//높이맵 텍스쳐
//	if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice, pszHeightMap,
//		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED,
//		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.pTexHeight)))
//		return E_FAIL ;

    BYTE *pbyDib = DibLoadHandle(pszHeightMap) ;
	if(!pbyDib)
	{
		char sz[256] ;
		sprintf(sz, "Cannot Load %s", pszHeightMap) ;
		assert(false && sz) ;
		return E_FAIL ;
	}

	nWidth = DIB_CX(pbyDib) ;
	nHeight = DIB_CY(pbyDib) ;

	m_sHeightMap.pcActionContainer = new CActionBrushingContainer() ;
	m_sHeightMap.pcActionContainer->Initialize(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                 Create Texture                                                     //
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	char szMsg[256] ;

	if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice,
		"../../Media/tex01.bmp",
		512, 512, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.pTexBase)))
	{
		sprintf(szMsg, "Cannot Load ../../Media/tex01.bmp") ;
		assert(false && szMsg) ;
		return false ;
	}

	if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice,
		"../../Media/tile2.tga",
		512, 512, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.apTexLayer[0])))
	{
		sprintf(szMsg, "Cannot Load ../../Media/tile2.tga") ;
		assert(false && szMsg) ;
		return false ;
	}
	//if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice,
	//	"../../Media/alpha01.bmp",
	//	512, 512, 1, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED,
	//	D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.apTexAlpha[0])))
	//{
	//	sprintf(szMsg, "Cannot Load ../../Media/alpha01.bmp") ;
	//	assert(false && szMsg) ;
	//	return false ;
	//}
	if(FAILED(D3DXCreateTexture(m_pd3dDevice, m_sHeightMap.nAlphaMapSize, m_sHeightMap.nAlphaMapSize, 1, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED, &m_sHeightMap.apTexAlpha[0])))
	{
		sprintf(szMsg, "Cannot Create AlphaTexture") ;
		assert(false && szMsg) ;
		return false ;
	}
	FillTextureWithColor(m_sHeightMap.apTexAlpha[0], 0x00000000) ;


	if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice,
		"../../Media/rockwall.tga",
		512, 512, D3DX_DEFAULT, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.apTexLayer[1])))
	{
		sprintf(szMsg, "Cannot Load ../../Media/rockwall.tga") ;
		assert(false && szMsg) ;
		return false ;
	}
	if(FAILED(D3DXCreateTextureFromFileEx(m_pd3dDevice,
		"../../Media/alpha03.bmp",
		512, 512, 1, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &m_sHeightMap.apTexAlpha[1])))
	{
		sprintf(szMsg, "Cannot Load ../../Media/alpha02.bmp") ;
		assert(false && szMsg) ;
		return false ;
	}

	int i ;
    for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
		m_sHeightMap.apSaveKeepingAlpha[i] = m_sHeightMap.apTexAlpha[i] ;
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
	m_sHeightMap.lWidth = nWidth-1 ;
	m_sHeightMap.lHeight = nHeight-1 ;
	m_sHeightMap.lNumVertices = nWidth*nHeight ;
	m_sHeightMap.lPrimitiveCount = (nWidth-1)*(nHeight-1)*2 ;

	m_sHeightMap.pVertices = new TERRAINVERTEX[m_sHeightMap.lNumVertices] ;

	if(FAILED(m_pd3dDevice->CreateVertexBuffer(m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_TERRAINVERTEX, D3DPOOL_DEFAULT, &m_sHeightMap.pVB, NULL)))
		return E_FAIL ;

    //Lock VertexBuffer
	void *pVertices ;
	if(FAILED(m_sHeightMap.pVB->Lock(0, m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
		return E_FAIL ;

	TERRAINVERTEX v ;
	TERRAINVERTEX *pv=(TERRAINVERTEX *)pVertices, *pv1=m_sHeightMap.pVertices ;
	float fMaxHeight, fMinHeight ;

	//하나의 픽셀단위로 진행하니까 DWORD단위로 진행하는 것임
	//좌측하단부터 시작됨
	DWORD dwValue ;
    dwValue = *((LPDWORD)DIB_DATAXY_INV(pbyDib, 0, 0)) ;
	fMaxHeight = fMinHeight = ((float)(dwValue&0xff))/5.0f ;

	for(z=0 ; z<nHeight ; z++)
	{
		for(x=0 ; x<nWidth ; x++)
		{
			//Position
			v.pos.x = (float)(x-(int)nWidth/2) ;
			v.pos.z = -(float)(z-(int)nHeight/2) ;

			dwValue = *((LPDWORD)DIB_DATAXY_INV(pbyDib, x, z)) ;

			//TRACE("heightmap Index(%03d %03d) Value=%x\r\n", x, z, dwValue) ;

			//v.pos.y = ((float)(dwValue&0xff)) * (1.7f/255.f) ;
			v.pos.y = ((float)(dwValue&0xff))/5.0f ;
			if(fMaxHeight < v.pos.y)
				fMaxHeight = v.pos.y ;
			if(fMinHeight > v.pos.y)
				fMinHeight = v.pos.y ;
            
			//TRACE("pos(%03.07f %03.07f %03.07f)\r\n", v.pos.x, v.pos.y, v.pos.z) ;
			//TRACE("\r\n") ;

			//Normal
			//D3DXVec3Normalize(&v.normal, &v.pos) ;
			v.normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f) ;

			//Texture
			v.t.x = (float)x/(nWidth-1) ;
			v.t.y = (float)z/(nHeight-1) ;
			//v.t.x = (float)x*6.0f/(float)nWidth ;
			//v.t.y = (float)z*6.0f/(float)nHeight ;

			*pv++ = v ;
			*pv1++ = v ;
		}
	}

	m_sHeightMap.pVB->Unlock() ;

	m_sHeightMap.pIndices = new TERRAININDEX[m_sHeightMap.lPrimitiveCount] ;
	//m_sHeightMap.psTriangles = new geo::STriangle[m_sHeightMap.lPrimitiveCount] ;

    //Initialize IndexBuffer
	//(nWidth-1) means Number of rectangle in grid
	//horizon rectangle * vertical rectangle * 2 is Number of triangle in grid
	if(FAILED(m_pd3dDevice->CreateIndexBuffer(m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_sHeightMap.pIB, NULL)))
		return E_FAIL ;

	//Lock IndexBuffer
	TERRAININDEX sIndex, *psIndex, *psIndex1=m_sHeightMap.pIndices ;
	geo::STriangle *pTriangles = m_sHeightMap.psTriangles ;
	if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&psIndex, 0)))
		return E_FAIL ;

	m_sHeightMap.lNumTriangles=0 ;

	for(z=0 ; z<nHeight-1 ; z++)
	{
		for(x=0 ; x<nWidth-1 ; x++)
		{
			sIndex.i1 = (DWORD)(z*nHeight+x) ;
			sIndex.i2 = (DWORD)(z*nHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*nHeight+x) ;
			*psIndex++ = sIndex ;
			*psIndex1++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;

			sIndex.i1 = (DWORD)((z+1)*nHeight+x) ;
			sIndex.i2 = (DWORD)(z*nHeight+(x+1)) ;
			sIndex.i3 = (DWORD)((z+1)*nHeight+(x+1)) ;
			*psIndex++ = sIndex ;
			*psIndex1++ = sIndex ;
			//pTriangles->set( Vector3(m_sHeightMap.pVertices[sIndex.i1].pos.x, m_sHeightMap.pVertices[sIndex.i1].pos.y, m_sHeightMap.pVertices[sIndex.i1].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i2].pos.x, m_sHeightMap.pVertices[sIndex.i2].pos.y, m_sHeightMap.pVertices[sIndex.i2].pos.z),
			//	Vector3(m_sHeightMap.pVertices[sIndex.i3].pos.x, m_sHeightMap.pVertices[sIndex.i3].pos.y, m_sHeightMap.pVertices[sIndex.i3].pos.z)) ;
			m_sHeightMap.lNumTriangles++ ;
		}
	}
	m_sHeightMap.pIB->Unlock() ;

	//Create QuadTree 
	m_pcQuadTree = new CSecretQuadTree(nWidth, nHeight) ;
	m_pcQuadTree->Build(m_sHeightMap.pVertices, fMaxHeight, fMinHeight, m_sHeightMap.nTileSize) ;

	DibDeleteHandle(pbyDib);
    
	m_sHeightMap.bEnable=true ;
	m_bWireFrame = false ;
    m_bEnable = true ;

	return S_OK ;
}

D3DXVECTOR3 CSecretTerrain::D3DXGetPosition(unsigned long index)
{
	return m_sHeightMap.pVertices[index].pos ;
}

HRESULT CSecretTerrain::ProcessFrustumCulling(const D3DXMATRIX &matWorld)
{
	unsigned long z, x ;
	unsigned long index[4] ;
	bool bEnable[3] ;
	TERRAININDEX *pMyIndex ;

	if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&pMyIndex, 0)))
		return E_FAIL ;

	m_sHeightMap.lNumTriangles=0 ;
	D3DXVECTOR3 vPosition ;
	Vector3 v ;

	for(z=0 ; z<m_sHeightMap.lHeight-1 ; z++)
	{
		for(x=0 ; x<m_sHeightMap.lWidth-1 ; x++)
		{
			index[0] = z*m_sHeightMap.lHeight+x ;
			index[1] = z*m_sHeightMap.lHeight+(x+1) ;
			index[2] = (z+1)*m_sHeightMap.lHeight+x ;
			index[3] = (z+1)*m_sHeightMap.lHeight+(x+1) ;

			vPosition = D3DXGetPosition(index[0]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[0] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			vPosition = D3DXGetPosition(index[1]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[1] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			vPosition = D3DXGetPosition(index[2]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[2] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			if(bEnable[0] || bEnable[1] || bEnable[2])
			{
				pMyIndex->i1 = (WORD)index[0] ;
				pMyIndex->i2 = (WORD)index[1] ;
				pMyIndex->i3 = (WORD)index[2] ;
				pMyIndex++ ;
				m_sHeightMap.lNumTriangles++ ;
			}

			vPosition = D3DXGetPosition(index[2]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[0] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			vPosition = D3DXGetPosition(index[1]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[1] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			vPosition = D3DXGetPosition(index[3]) ;
			D3DXVec3TransformCoord(&vPosition, &vPosition, &matWorld) ;
			bEnable[2] = m_pcFrustum->IsIn(v = VectorConvert(vPosition)) ;

			if(bEnable[0] || bEnable[1] || bEnable[2])
			{
				pMyIndex->i1 = (WORD)index[2] ;
				pMyIndex->i2 = (WORD)index[1] ;
				pMyIndex->i3 = (WORD)index[3] ;
				pMyIndex++ ;
				m_sHeightMap.lNumTriangles++ ;
			}
		}
	}
	m_sHeightMap.pIB->Unlock() ;

	return S_OK ;
}

Vector3 CSecretTerrain::GetPosition(unsigned long index)
{
	return Vector3(m_sHeightMap.pVertices[index].pos.x, m_sHeightMap.pVertices[index].pos.y, m_sHeightMap.pVertices[index].pos.z) ;
}

void CSecretTerrain::SetTerrainMatrix(Matrix4 &matWorld)
{
	m_matWorld=matWorld ;
}

HRESULT CSecretTerrain::ProcessFrustumCulling(const Matrix4 &matWorld)
{
	unsigned long z, x ;
	unsigned long index[4] ;
	bool bEnable[3] ;
	TERRAININDEX *pMyIndex ;

	if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&pMyIndex, 0)))
		return E_FAIL ;

	m_sHeightMap.lNumTriangles=0 ;
	D3DXVECTOR3 vtemp ;
	Vector3 vPosition ;

	for(z=0 ; z<m_sHeightMap.lHeight-1 ; z++)
	{
		for(x=0 ; x<m_sHeightMap.lWidth-1 ; x++)
		{
			index[0] = z*m_sHeightMap.lHeight+x ;
			index[1] = z*m_sHeightMap.lHeight+(x+1) ;
			index[2] = (z+1)*m_sHeightMap.lHeight+x ;
			index[3] = (z+1)*m_sHeightMap.lHeight+(x+1) ;

			vPosition = GetPosition(index[0]) ;
			vPosition *= matWorld ;
			bEnable[0] = m_pcFrustum->IsIn(vPosition) ;

			vPosition = GetPosition(index[1]) ;
			vPosition *= matWorld ;
			bEnable[1] = m_pcFrustum->IsIn(vPosition) ;

			vPosition = GetPosition(index[2]) ;
			vPosition *= matWorld ;
			bEnable[2] = m_pcFrustum->IsIn(vPosition) ;

			if(bEnable[0] && bEnable[1] && bEnable[2])
			{
				pMyIndex->i1 = (WORD)index[0] ;
				pMyIndex->i2 = (WORD)index[1] ;
				pMyIndex->i3 = (WORD)index[2] ;
				pMyIndex++ ;
				m_sHeightMap.lNumTriangles++ ;
			}

			vPosition = GetPosition(index[2]) ;
			vPosition *= matWorld ;
			bEnable[0] = m_pcFrustum->IsIn(vPosition) ;

			vPosition = GetPosition(index[1]) ;
			vPosition *= matWorld ;
			bEnable[1] = m_pcFrustum->IsIn(vPosition) ;

			vPosition = GetPosition(index[3]) ;
			vPosition *= matWorld ;
			bEnable[2] = m_pcFrustum->IsIn(vPosition) ;

			if(bEnable[0] && bEnable[1] && bEnable[2])
			{
				pMyIndex->i1 = (WORD)index[2] ;
				pMyIndex->i2 = (WORD)index[1] ;
				pMyIndex->i3 = (WORD)index[3] ;
				pMyIndex++ ;
				m_sHeightMap.lNumTriangles++ ;
			}
		}
	}
	m_sHeightMap.pIB->Unlock() ;

	return S_OK ;
}

void CSecretTerrain::SetWireFrame(bool bEnable)
{
	m_bWireFrame = bEnable ;
}

void CSecretTerrain::ProcessFrustumCullInQuadTree()
{
	//m_pcZQuadTree->_FrustumCull(m_sHeightMap.pVertices, &m_cFrustum) ;
	//m_sHeightMap.lNumTriangles = m_pcZQuadTree->GenerateTriangles(&m_sHeightMap, &m_cFrustum.GetCameraPosition()) ;
	
	//m_cFrustum.m_pcToolBrush->m_cPicking.ResetTilesMustbeTested() ;

	m_pcFrustum->m_nNumBoundingPosition = 0 ;

	m_sHeightMap.nIndex = m_pcQuadTree->FrustumCull(m_sHeightMap.pVertices, m_pcFrustum, &m_pcFrustum->GetCameraPosition(), m_sHeightMap.pIndices, 0, &m_sHeightMap.sExceptionalIndices) ;

	//m_cFrustum.m_nIntersectCount = m_cFrustum.m_pcToolBrush->m_cPicking.m_nNumTiles ;
	//m_cFrustum.m_pcToolBrush->m_cPicking.IntersectTiles(m_sHeightMap.pVertices) ;
}
void CSecretTerrain::ProcessFrustumCullInQuadTree(CSecretFrustum *pcFrustum)
{
	pcFrustum->m_nNumBoundingPosition = 0 ;

	m_sHeightMap.nIndex = m_pcQuadTree->FrustumCull(m_sHeightMap.pVertices, pcFrustum, &pcFrustum->GetCameraPosition(), m_sHeightMap.pIndices, 0, &m_sHeightMap.sExceptionalIndices) ;

	//m_cFrustum.m_nIntersectCount = m_cFrustum.m_pcToolBrush->m_cPicking.m_nNumTiles ;
	//m_cFrustum.m_pcToolBrush->m_cPicking.IntersectTiles(m_sHeightMap.pVertices) ;
}

void CSecretTerrain::BrushingTerrain()
{
	/*
	D3DLOCKED_RECT d3drect ;
    LPDIRECT3DSURFACE9 pSurf=NULL ;
	int x, y ;
	DWORD *pdw=NULL ;

	m_sHeightMap.pTexDecal->LockRect(0, &d3drect, NULL, D3DLOCK_DISCARD) ;
	pdw = (DWORD *)d3drect.pBits ;

    for(y=0 ; y<256 ; y++)
	{
		for(x=0 ; x<512 ; x++)
		{
            *pdw++ = 0xffff0000 ;
		}
	}
    
	m_sHeightMap.pTexDecal->UnlockRect(0) ;
	*/

	//브러쉬 작업이 모두 끝난다음에 액션을 저장한다.
	//CActionBrushing *pcActionBrushing = m_sHeightMap.pcActionContainer->GetEmptyContainer() ;
	//assert(pcActionBrushing != NULL) ;
	//pcActionBrushing->Save(m_sHeightMap.apTexAlpha[nNum], m_pd3dDevice) ;
	//m_pcActionManagement->InsertAction((CAction *)pcActionBrushing) ;
}

HRESULT CSecretTerrain::RenderQuadTree(SD3DEffect *psd3dEffect)
{
	if(m_sHeightMap.lNumTriangles == 0)
		return E_FAIL ;

	HRESULT hr ;
	DWORD dwTriangles ;
	WORD *pwIndex ;

	D3DXVECTOR4 vLocalLightPos, vLocalLookAt ;
	D3DXMATRIX mat ;

	//m_pcZQuadTree->_FrustumCull(m_sHeightMap.pVertices, &m_cFrustum) ;

 //   if(FAILED(m_sHeightMap.pIB->Lock(0, m_sHeightMap.lPrimitiveCount*sizeof(TERRAININDEX), (void **)&pwIndex, 0)))
	//	return E_FAIL ;

	//dwTriangles = m_pcZQuadTree->GenerateIndex(pwIndex, m_sHeightMap.pVertices, &m_cFrustum) ;

	//m_sHeightMap.pIB->Unlock() ;

	dwTriangles = m_sHeightMap.lNumTriangles ;

	if(FAILED(m_sHeightMap.pIB->Lock(0, sizeof(TERRAININDEX)*dwTriangles, (void **)&pwIndex, 0)))
		return E_FAIL ;

	memcpy(pwIndex, m_sHeightMap.pIndices, sizeof(TERRAININDEX)*dwTriangles) ;

	m_sHeightMap.pIB->Unlock() ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	//Shader Setting

	psd3dEffect->pEffect->BeginPass(0) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &psd3dEffect->pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	D3DXMatrixInverse(&mat, NULL, &psd3dEffect->matWorld) ;
	D3DXVec4Transform(&vLocalLightPos, &psd3dEffect->vLightPos, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLightPos, &vLocalLightPos) ;

	D3DXVec4Transform(&vLocalLookAt, &psd3dEffect->vLookAt, &mat) ;
	psd3dEffect->pEffect->SetVector(psd3dEffect->hvLookAt, &vLocalLookAt) ;

	mat = psd3dEffect->matWorld * psd3dEffect->matView * psd3dEffect->matProj ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWVP, &mat) ;
	psd3dEffect->pEffect->SetMatrix(psd3dEffect->hmatWIT, &psd3dEffect->matWorld) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;

	m_pd3dDevice->SetStreamSource(0, m_sHeightMap.pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetFVF(D3DFVF_TERRAINVERTEX) ;
	m_pd3dDevice->SetIndices(m_sHeightMap.pIB) ;

	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_sHeightMap.lNumVertices, 0, dwTriangles) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	psd3dEffect->pEffect->EndPass() ;

	psd3dEffect->pDecl->Release() ;
	psd3dEffect->pDecl = NULL ;

	return S_OK ;
}

HRESULT CSecretTerrain::RenderQuadTree(CSecretD3DTerrainEffect *pcTerrainEffect, D3DXMATRIX *pmatWorld, CSecretMirror *pcMirror)
{
	if(m_sHeightMap.lNumTriangles == 0)
		return E_FAIL ;

	HRESULT hr ;
	DWORD dwTriangles ;
	TERRAININDEX *psSrcIndex ;
	DWORD *pwIndex ;


	if(m_nAttr & CSecretTerrain::ATTR_RENDERSHADOW)
	{
		psSrcIndex = m_sHeightMap.psOriginIndices ;
		dwTriangles = m_sHeightMap.lPrimitiveCount ;
	}
	else
	{
		psSrcIndex = m_sHeightMap.pIndices ;
		dwTriangles = m_sHeightMap.nIndex ;
	}
	if(dwTriangles == 0)
		return E_FAIL ;

	if(FAILED(m_sHeightMap.pIB->Lock(0, sizeof(TERRAININDEX)*dwTriangles, (void **)&pwIndex, 0)))
		return E_FAIL ;
	memcpy(pwIndex, psSrcIndex, sizeof(TERRAININDEX)*dwTriangles) ;
	m_sHeightMap.pIB->Unlock() ;

	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	DWORD dwMipMapLodBias ;
	m_pd3dDevice->GetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, &dwMipMapLodBias) ;
	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, (DWORD)m_fMipmapLodBias) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = pcTerrainEffect->m_psEssentialElements ;
	D3DXMATRIX matWorld ;

	if(m_nAttr & CSecretTerrain::ATTR_RENDERSHADOW)
		pcTerrainEffect->m_pEffect->BeginPass(PASS_SHADOW) ;
	else if(m_nAttr & CSecretTerrain::ATTR_DEPTHWATER)
		pcTerrainEffect->m_pEffect->BeginPass(PASS_DEPTHWATER) ;
	else
		pcTerrainEffect->m_pEffect->BeginPass(PASS_SUNLIT) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &pcTerrainEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	if(m_nAttr & CSecretTerrain::ATTR_RENDERSHADOW)
	{
		SRenderTarget *psRenderTarget = m_pcShadowMap->GetRenderTarget() ;
		D3DXMATRIX matWLP ;
		matWLP = (*m_pcShadowMap->GetmatLVP()) ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWLP, &matWLP) ;
	}
	else if(m_nAttr & CSecretTerrain::ATTR_DEPTHWATER)
	{
		D3DXMATRIX mat ;
		if(pmatWorld)
			matWorld = psEffectElements->matWorld * (*pmatWorld) ;
		else
			matWorld = psEffectElements->matWorld ;

		mat = matWorld * psEffectElements->matView * psEffectElements->matProj ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

		Vector3 vCenterPos = pcMirror->GetCenterPos() ;
		D3DXVECTOR4 v(vCenterPos.x, vCenterPos.y, vCenterPos.z, 0.5f) ;
		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvWaterLevel, &v) ;

		//v = D3DXVECTOR4(1-0, (255.0f-180.0f)/255.0f, (255.0f-242.0f)/255.0f, 1) ;
		v = D3DXVECTOR4(m_vWaterColorTone.x, m_vWaterColorTone.y, m_vWaterColorTone.z, 1) ;
		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvWaterColorTone, &v) ;
	}
	else
	{
		//D3DXVECTOR4 vLookAt, vSunDir, vSunColor, vCameraPos ;
		D3DXVECTOR4 vSunColor ;
		D3DXMATRIX mat ;

		if(pmatWorld)
			matWorld = psEffectElements->matWorld * (*pmatWorld) ;
		else
			matWorld = psEffectElements->matWorld ;

		vSunColor = D3DXVECTOR4(psEffectElements->vSunColor.x, psEffectElements->vSunColor.y, psEffectElements->vSunColor.z, psEffectElements->vSunColor.w) ;
		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSunColor, &vSunColor) ;

		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSunDir, &psEffectElements->vSunDir) ;

		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvLookAt, &psEffectElements->vLookAt) ;

		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

		mat = matWorld * psEffectElements->matView * psEffectElements->matProj * psEffectElements->matClipProj ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWIT, &matWorld) ;

		pcTerrainEffect->m_pEffect->SetFloatArray(pcTerrainEffect->m_hfTilingTex, m_afTilingTex, MAXNUM_TERRAINTEXTURES+1) ;
		//pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWIT, &psEffectElements->matWorld) ;

		//D3DXVECTOR4 vDiffuse(0.95f, 0.95f, 1.05f, 1) ;
		//D3DXVECTOR4 vDiffuse(0.8f, 0.8f, 0.8f, 1) ;
		D3DXVECTOR4 vDiffuse(m_vDiffuse.x, m_vDiffuse.y, m_vDiffuse.z, 1) ;
		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvDiffuse, &vDiffuse) ;

		D3DXVECTOR4 vSpecular(m_vSpecular.x, m_vSpecular.y, m_vSpecular.z, 1.0f) ;
		pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSpecular, &vSpecular) ;

		pcTerrainEffect->m_pEffect->SetValue(pcTerrainEffect->m_hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;

		float fOffsetX = 0.5f+(0.5f/(float)m_pcShadowMap->GetRenderTarget()->lWidth) ;
		float fOffsetY = 0.5f+(0.5f/(float)m_pcShadowMap->GetRenderTarget()->lHeight) ;

		//투영변환후에 텍스쳐공간으로 변환하기 위한 행렬, Offset값으로 오차를 조정한다.
		D3DXMATRIX matTexBias(0.5f,	    0.0f,	   0.0f,	   0.0f,
								0.0f,    -0.5f,      0.0f,       0.0f,
								0.0f,     0.0f,      0.0f,       0.0f,
								fOffsetX, fOffsetY,  0.0f,       1.0f) ;

		SRenderTarget *psRenderTarget = m_pcShadowMap->GetRenderTarget() ;
		D3DXMATRIX matWLP, matWLPT ;
		matWLP = matWorld * (*m_pcShadowMap->GetmatLVP()) ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWLP, &matWLP) ;
		matWLPT = matWLP * matTexBias ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWLPT, &matWLPT) ;

		D3DXMATRIX matTexRot, matTexScale ;
		D3DXMatrixIdentity(&matTexScale) ;
		matTexScale._11 = matTexScale._22 = matTexScale._33 = 128.0f ;//256/128=2m
		D3DXMatrixRotationZ(&matTexRot, deg_rad(45.0f)) ;
		matTexRot = matTexScale * matTexRot ;
		pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatRotTex, &matTexRot) ;

		pcTerrainEffect->m_pEffect->SetTexture("ShadowMap", m_pcShadowMap->GetRenderTarget()->pTex) ;
		pcTerrainEffect->m_pEffect->SetTexture("SrcShadowMap", m_pcShadowMap->GetTexSoft()) ;
	}

	if(m_nAttr & CSecretTerrain::ATTR_RENDERSHADOW)
	{
	}
	else
	{
		pcTerrainEffect->m_pEffect->SetTexture("BaseMap", m_sHeightMap.pTexBase) ;

		pcTerrainEffect->m_pEffect->SetTexture("LayerMap00", m_sHeightMap.apTexLayer[0]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap01", m_sHeightMap.apTexLayer[1]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap02", m_sHeightMap.apTexLayer[2]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap03", m_sHeightMap.apTexLayer[3]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap04", m_sHeightMap.apTexLayer[4]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap05", m_sHeightMap.apTexLayer[5]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap06", m_sHeightMap.apTexLayer[6]) ;
		pcTerrainEffect->m_pEffect->SetTexture("LayerMap07", m_sHeightMap.apTexLayer[7]) ;

		pcTerrainEffect->m_pEffect->SetTexture("AlphaMap00", m_sHeightMap.apTexAlpha[0]) ;
		pcTerrainEffect->m_pEffect->SetTexture("AlphaMap01", m_sHeightMap.apTexAlpha[1]) ;

		pcTerrainEffect->m_pEffect->SetTexture("NoiseMap00", m_sHeightMap.pTexNoise00) ;
		pcTerrainEffect->m_pEffect->SetTexture("NoiseMap01", m_sHeightMap.pTexNoise01) ;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////

	m_pd3dDevice->SetVertexDeclaration(pcTerrainEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_sHeightMap.pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetIndices(m_sHeightMap.pIB) ;

	pcTerrainEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_sHeightMap.lNumVertices, 0, dwTriangles) ;

	pcTerrainEffect->m_pEffect->EndPass() ;

	for(int i=0 ; i<MAXNUM_TERRAINROAD ; i++)
	{
		if(m_acRoad[i].bEnable)
		{
			pcTerrainEffect->m_pEffect->BeginPass(PASS_ROADSUNLIT) ;

            if(m_acRoad[i].pTex)
				//pcTerrainEffect->m_pEffect->SetTexture("LayerMap01", m_acRoad[i].pTex) ;
				pcTerrainEffect->m_pEffect->SetTexture("RoadMap", m_acRoad[i].pTex) ;

			m_pd3dDevice->SetVertexDeclaration(pcTerrainEffect->m_pDecl) ;
			m_pd3dDevice->SetStreamSource(0, m_acRoad[i].pVB, 0, sizeof(TERRAINVERTEX)) ;
			m_pd3dDevice->SetIndices(m_acRoad[i].pIB) ;

			pcTerrainEffect->m_pEffect->CommitChanges() ;
			m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_acRoad[i].nNumVertex, 0, m_acRoad[i].nNumIndex) ;

			pcTerrainEffect->m_pEffect->EndPass() ;
		}
	}

	m_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, dwMipMapLodBias) ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode) ;
	pcTerrainEffect->m_pDecl->Release() ;
	pcTerrainEffect->m_pDecl= NULL ;

	return S_OK ;
}
void CSecretTerrain::RenderHolecup(CSecretD3DTerrainEffect *pcTerrainEffect)
{
	if(!m_sHolecup.bEnable)
		return ;

	HRESULT hr ;
	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;
	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	DWORD dwDepthBias, dwSlopeScaleDepthBias ;
	m_pd3dDevice->GetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, &dwSlopeScaleDepthBias) ;
	m_pd3dDevice->GetRenderState(D3DRS_DEPTHBIAS, &dwDepthBias) ;

	//m_pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, *((DWORD*)&m_sHolecup.fSlopeScaleDepthBias)) ;
	//m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *((DWORD*)&m_sHolecup.fDepthBias)) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = pcTerrainEffect->m_psEssentialElements ;
	D3DXMATRIX matWorld ;

	//Render holecup
	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain1, &pcTerrainEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	pcTerrainEffect->m_pEffect->BeginPass(PASS_HOLECUP) ;

	//D3DXVECTOR4 vLookAt, vSunDir, vSunColor, vCameraPos ;
	D3DXVECTOR4 vSunColor ;
	D3DXMATRIX mat ;
	matWorld = psEffectElements->matWorld ;

	vSunColor = D3DXVECTOR4(psEffectElements->vSunColor.x, psEffectElements->vSunColor.y, psEffectElements->vSunColor.z, psEffectElements->vSunColor.w) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSunColor, &vSunColor) ;

	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSunDir, &psEffectElements->vSunDir) ;

	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvLookAt, &psEffectElements->vLookAt) ;

	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvCameraPos, &psEffectElements->vCameraPos) ;

	mat = matWorld * psEffectElements->matView * psEffectElements->matProj * psEffectElements->matClipProj ;
	pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

	pcTerrainEffect->m_pEffect->SetFloatArray(pcTerrainEffect->m_hfTilingTex, m_afTilingTex, MAXNUM_TERRAINTEXTURES+1) ;
	//pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWIT, &psEffectElements->matWorld) ;

	//D3DXVECTOR4 vDiffuse(0.95f, 0.95f, 1.05f, 1) ;
	//D3DXVECTOR4 vDiffuse(0.8f, 0.8f, 0.8f, 1) ;
	D3DXVECTOR4 vDiffuse(m_vDiffuse.x, m_vDiffuse.y, m_vDiffuse.z, 1) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvDiffuse, &vDiffuse) ;

	D3DXVECTOR4 vSpecular(m_vSpecular.x, m_vSpecular.y, m_vSpecular.z, 1.0f) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvSpecular, &vSpecular) ;

	pcTerrainEffect->m_pEffect->SetValue(pcTerrainEffect->m_hAtm, &m_pcAtm->m_shaderParams, sizeof(sLightScatteringShaderParams)) ;

	//rendering holecup geometry
	pcTerrainEffect->m_pEffect->SetTexture("NoiseMap00", m_sHeightMap.pTexNoise00) ;
	pcTerrainEffect->m_pEffect->SetTexture("NoiseMap01", m_sHeightMap.pTexNoise01) ;

	pcTerrainEffect->m_pEffect->SetTexture("ShadowMap", m_pcShadowMap->GetRenderTarget()->pTex) ;
	pcTerrainEffect->m_pEffect->SetTexture("SrcShadowMap", m_pcShadowMap->GetTexSoft()) ;

	pcTerrainEffect->m_pEffect->SetTexture("DecalMap", m_sHolecup.pTex) ;

	m_pd3dDevice->SetVertexDeclaration(pcTerrainEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_sHolecup.pVB, 0, sizeof(STerrainVertex1)) ;
	m_pd3dDevice->SetIndices(m_sHolecup.pIB) ;

	pcTerrainEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_sHolecup.nNumVertex, 0, m_sHolecup.nNumIndex) ;


	//rendering green decal
	D3DXVECTOR3 vHolecupPos ;
	vHolecupPos.x = m_sHolecup.matTransform._41 ;
	vHolecupPos.y = m_sHolecup.matTransform._42 ;
	vHolecupPos.z = m_sHolecup.matTransform._43 ;

	D3DXVECTOR4 vResult ;
	D3DXVec3Transform(&vResult, &vHolecupPos, &psEffectElements->matView) ;

	D3DXMATRIX matNewProj = psEffectElements->matProj ;
	float f=m_pcd3dEnvironment->m_fNear, n=m_pcd3dEnvironment->m_fFar ;
	float pz = vResult.z ;

	float k = (f-n)/(2.0f*f*n*(powf(2.0f, 16.0f)-1.0f)) ;
	float delta = (k*pz*pz)/(1.0f-k*pz) ;
	float eps = (2.0f*f*n*delta)/((f+n)*(pz*(pz+delta))) ;
	matNewProj._33 *= (1.0f+eps) ;

	mat = matWorld * psEffectElements->matView * matNewProj * psEffectElements->matClipProj ;
	pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

	//pcTerrainEffect->m_pEffect->SetTexture("DecalMap", m_sHolecup.sDecal.apTex[0]) ;
	//pcTerrainEffect->m_pEffect->SetTexture("DecalMap", m_sHolecup.sDecal.apTex[1]) ;
	pcTerrainEffect->m_pEffect->SetTexture("DecalMap", m_sHolecup.GetTextureLOD(Vector3(pcTerrainEffect->m_psEssentialElements->vCameraPos.x, pcTerrainEffect->m_psEssentialElements->vCameraPos.y, pcTerrainEffect->m_psEssentialElements->vCameraPos.z))) ;

	m_pd3dDevice->SetVertexDeclaration(pcTerrainEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_sHolecup.sDecal.pVB, 0, sizeof(STerrainVertex1)) ;
	m_pd3dDevice->SetIndices(m_sHolecup.sDecal.pIB) ;

	pcTerrainEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_sHolecup.sDecal.nNumVertex, 0, m_sHolecup.sDecal.nNumIndex) ;

	pcTerrainEffect->m_pEffect->EndPass() ;

	pcTerrainEffect->m_pDecl->Release() ;
	pcTerrainEffect->m_pDecl= NULL ;

	m_pd3dDevice->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, dwSlopeScaleDepthBias) ;
	m_pd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, dwDepthBias) ;
}
HRESULT CSecretTerrain::RenderAltitude(CSecretD3DTerrainEffect *pcTerrainEffect)
{
	HRESULT hr ;
	DWORD *pwIndex ;

	if(FAILED(m_sHeightMap.pIB->Lock(0, sizeof(TERRAININDEX)*m_sHeightMap.lPrimitiveCount, (void **)&pwIndex, 0)))
		return E_FAIL ;
	memcpy(pwIndex, m_sHeightMap.psOriginIndices, sizeof(TERRAININDEX)*m_sHeightMap.lPrimitiveCount) ;
	m_sHeightMap.pIB->Unlock() ;

	DWORD dwFillMode ;
	m_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &dwFillMode) ;

	m_pd3dDevice->SetRenderState( D3DRS_FILLMODE, m_bWireFrame ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                       Shader Setting                                           //
	////////////////////////////////////////////////////////////////////////////////////////////////////
	SEffectEssentialElements *psEffectElements = pcTerrainEffect->m_psEssentialElements ;

	pcTerrainEffect->m_pEffect->BeginPass(PASS_ALTITUDE) ;

	if(FAILED(hr=m_pd3dDevice->CreateVertexDeclaration( decl_Terrain, &pcTerrainEffect->m_pDecl )))
		assert(false && "Failure m_pd3dDevice->CreateVertexDeclaration in CSecretTerrain::Render()") ;

	D3DXMATRIX mat ;
	mat = psEffectElements->matWorld * psEffectElements->matView * psEffectElements->matProj ;
	pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWVP, &mat) ;

	pcTerrainEffect->m_pEffect->SetMatrix(pcTerrainEffect->m_hmatWIT, &psEffectElements->matWorld) ;

	D3DXVECTOR4 vHolecup(m_sHolecup.matTransform._41, m_sHolecup.matTransform._42, m_sHolecup.matTransform._43, 1.0f) ;
	vHolecup.y = m_sHeightMap.GetHeightMap(vHolecup.x, vHolecup.z) ;
	pcTerrainEffect->m_pEffect->SetVector(pcTerrainEffect->m_hvHolecup, &vHolecup) ;

	pcTerrainEffect->m_pEffect->SetTexture("AlphaMap00", m_sHeightMap.apTexAlpha[0]) ;
	pcTerrainEffect->m_pEffect->SetTexture("AlphaMap01", m_sHeightMap.apTexAlpha[1]) ;

	////////////////////////////////////////////////////////////////////////////////////////////////////

	m_pd3dDevice->SetVertexDeclaration(pcTerrainEffect->m_pDecl) ;
	m_pd3dDevice->SetStreamSource(0, m_sHeightMap.pVB, 0, sizeof(TERRAINVERTEX)) ;
	m_pd3dDevice->SetIndices(m_sHeightMap.pIB) ;

	pcTerrainEffect->m_pEffect->CommitChanges() ;
	m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_sHeightMap.lNumVertices, 0, m_sHeightMap.lPrimitiveCount) ;

	pcTerrainEffect->m_pEffect->EndPass() ;

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, dwFillMode) ;
	pcTerrainEffect->m_pDecl->Release() ;
	pcTerrainEffect->m_pDecl= NULL ;

	return S_OK ;
}

void CSecretTerrain::RenderOctTreeCube()
{
	typedef struct
	{
		D3DXVECTOR3 pos ;
		unsigned long color ;
	} VERTEX ;

	unsigned short index[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0,

		4, 5,
		5, 6,
		6, 7,
		7, 4,

		0, 4,
		1, 5,
		2, 6,
		3, 7,

//		0, 6,
//		3, 5
	} ;

	std::vector<geo::SAACube *> CubeSet ;

	CubeSet.clear() ;
	//this->m_pcOctTree->GetAllTheCubes(&CubeSet) ;
	int nNumCube = m_pcQuadTree->GetTreeCubes(&CubeSet, 0) ;

	int i ;
	VERTEX vertex[9] ;

	DWORD dwTssColorOp01, dwTssColorOp02 ;
	m_pd3dDevice->GetTextureStageState(0, D3DTSS_COLOROP, &dwTssColorOp01) ;
	m_pd3dDevice->GetTextureStageState(1, D3DTSS_COLOROP, &dwTssColorOp02) ;

	m_pd3dDevice->SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE) ;
	m_pd3dDevice->SetStreamSource(0, NULL, 0, sizeof(VERTEX)) ;
	m_pd3dDevice->SetTexture(0, NULL) ;
	m_pd3dDevice->SetIndices(0) ;
	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE) ;

	std::vector<geo::SAACube *>::iterator it ;
	for(it=CubeSet.begin() ; it != CubeSet.end() ; it++)
	{
        for(i=0 ; i<8 ; i++)
		{
			vertex[i].pos = VectorConvert((*it)->GetPos(i)) ;
		}
		vertex[8].pos = VectorConvert((*it)->vCenter) ;

		m_pd3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, 9, 12, index, D3DFMT_INDEX16, vertex, sizeof(vertex[0])) ;
	}

	m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE) ;

	m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, dwTssColorOp01) ;
	m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, dwTssColorOp02) ;
}

bool CSecretTerrain::SaveAlphaMapstoTerrainFile(char *pszFileName)
{
    FILE *pf = fopen(pszFileName, "r+b") ;
	if(pf == NULL)
	{
		fclose(pf) ;
		return false ;
	}

	int i, n ;
	unsigned long seek ;
	STerrain_FileHeader sHeader ;

	fread(&sHeader, sizeof(STerrain_FileHeader), 1, pf) ;
	if(sHeader.dwType != 0x154)
		return false ;

	float fdiv = (float)sHeader.nNumerator/(float)sHeader.nDenominator ;//1m x 1m 이하로는 할수 없다.
	int nWidth = (int)((sHeader.lWidth/fdiv)+1) ;
	int nHeight = (int)((sHeader.lHeight/fdiv)+1) ;

	//알파맵 2048로 다시 저장하는 경우 사용
	//fseek(pf, 0, SEEK_SET) ;
	//sHeader.nAlphaMapSize = 2048 ;//알파맵크기를 2048로 다시저장
	//fwrite(&sHeader, sizeof(STerrain_FileHeader), 1, pf) ;

    seek = (sHeader.lNumMaterial*sizeof(STerrain_MaterialInfo)) ;
	fseek(pf, seek, SEEK_CUR) ;

    for(i=0 ; i<(nWidth*nHeight) ; i++)
		m_sHeightMap.psFragmentInfo[i].fElevation = m_sHeightMap.pVertices[i].pos.y ;

	fwrite(m_sHeightMap.psFragmentInfo, (nWidth*nHeight)*sizeof(STerrain_FragmentInfo), 1, pf) ;

	//알파맵 2048로 다시 저장하는 경우 사용
	//BYTE *pbyBuffer = new BYTE[sHeader.nAlphaMapSize*sHeader.nAlphaMapSize] ;
	//ZeroMemory(pbyBuffer, sHeader.nAlphaMapSize*sHeader.nAlphaMapSize) ;
	//for(i=0 ; i<MAXNUM_TERRAINTEXTURES ; i++)
	//	fwrite(pbyBuffer, sizeof(BYTE)*sHeader.nAlphaMapSize*sHeader.nAlphaMapSize, 1, pf) ;

	int nNumLeftAlpha, nShift, anShift[] = {24, 16, 8, 0} ;
	BYTE *pbyBuffer = new BYTE[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;
	DWORD *pdwBuffer = new DWORD[m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize] ;

	nNumLeftAlpha = sHeader.nNumAlphaMap ;//헤더의 알파레이어 장수
	for(i=0 ; i<m_sHeightMap.nNumAlphaMap ; i++)
	{
		m_sHeightMap.ReadAlphaTexture(i, pdwBuffer) ;

		int nColorPos ;
        nNumLeftAlpha -= 4 ;
		if(nNumLeftAlpha < 0)
			nColorPos = 4+nNumLeftAlpha ;
		else
			nColorPos = 4 ;

        for(n=0 ; n<nColorPos; n++)
		{
			nShift = anShift[n%4] ;
            TRACE("[%02d] shift=%d\r\n", n, nShift) ;
			for(int c=0; c<(m_sHeightMap.nAlphaMapSize*m_sHeightMap.nAlphaMapSize); c++)
				pbyBuffer[c] = (BYTE)((pdwBuffer[c]>>nShift)&0xff) ;

			fwrite(pbyBuffer, sizeof(BYTE)*sHeader.nAlphaMapSize*sHeader.nAlphaMapSize, 1, pf) ;
		}
	}

	fclose(pf) ;
	SAFE_DELETEARRAY(pbyBuffer) ;
    SAFE_DELETEARRAY(pdwBuffer) ;
	return true ;
}
void CSecretTerrain::AddAttr(int nAttr, bool bEnable)
{
	if(bEnable)
		m_nAttr |= nAttr ;
	else
		m_nAttr &= (~nAttr) ;
}
int CSecretTerrain::GetAttr()
{
	return m_nAttr ;
}
bool CSecretTerrain::ReCalculateAllTerrainNormal()
{
	TERRAINVERTEX *pVertices=NULL ;
	if(FAILED(m_sHeightMap.pVB->Lock(0, m_sHeightMap.lNumVertices*sizeof(TERRAINVERTEX), (void **)&pVertices, 0)))
		return false ;

	int x, z ;	
	Vector3 vNormal ;
	for(z=0 ; z<m_sHeightMap.nVertexHeight ; z++)
	{
		for(x=0 ; x<m_sHeightMap.nVertexWidth ; x++, pVertices++)
		{
			vNormal = m_sHeightMap.GetNormal(x, z) ;
			m_sHeightMap.pVertices[z*m_sHeightMap.nVertexWidth+x].normal = D3DXVECTOR3(vNormal.x, vNormal.y, vNormal.z) ;
			pVertices->normal.x = vNormal.x ;
			pVertices->normal.y = vNormal.y ;
			pVertices->normal.z = vNormal.z ;
		}
	}

	m_sHeightMap.pVB->Unlock() ;
	return true ;
}
int CSecretTerrain::GetEmptyRoad()
{
	for(int i=0 ; i<MAXNUM_TERRAINROAD ; i++)
	{
		if(!m_acRoad[i].bEnable)
			return i ;
	}
	return -1 ;
}
void CSecretTerrain::CreateHolecup(Vector3 vPos)
{
	if(m_sHolecup.bEnable)
		return ;

	m_sHeightMap.GetIndex(vPos.x, vPos.z, &m_sHeightMap.sExceptionalIndices) ;

	D3DXMATRIX mat ;
	int nExceptionalIndex = m_sHeightMap.sExceptionalIndices.nCurPos-1 ;
	TERRAININDEX *psIndex = m_sHeightMap.sExceptionalIndices.GetAt(nExceptionalIndex) ;

	Vector3 v1, v2, v3, vMid ;
	v1.set(m_sHeightMap.pVertices[psIndex->i1].pos.x, m_sHeightMap.pVertices[psIndex->i1].pos.y, m_sHeightMap.pVertices[psIndex->i1].pos.z) ;
	v2.set(m_sHeightMap.pVertices[psIndex->i2].pos.x, m_sHeightMap.pVertices[psIndex->i2].pos.y, m_sHeightMap.pVertices[psIndex->i2].pos.z) ;
	v3.set(m_sHeightMap.pVertices[psIndex->i3].pos.x, m_sHeightMap.pVertices[psIndex->i3].pos.y, m_sHeightMap.pVertices[psIndex->i3].pos.z) ;

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
	tri.avVertex[0].vNormal.set(m_sHeightMap.pVertices[psIndex->i1].normal.x, m_sHeightMap.pVertices[psIndex->i1].normal.y, m_sHeightMap.pVertices[psIndex->i1].normal.z) ;
	tri.avVertex[1].vNormal.set(m_sHeightMap.pVertices[psIndex->i2].normal.x, m_sHeightMap.pVertices[psIndex->i2].normal.y, m_sHeightMap.pVertices[psIndex->i2].normal.z) ;
	tri.avVertex[2].vNormal.set(m_sHeightMap.pVertices[psIndex->i3].normal.x, m_sHeightMap.pVertices[psIndex->i3].normal.y, m_sHeightMap.pVertices[psIndex->i3].normal.z) ;

	tri.avVertex[0].vTex.set(m_sHeightMap.pVertices[psIndex->i1].t.x, m_sHeightMap.pVertices[psIndex->i1].t.y) ;
	tri.avVertex[1].vTex.set(m_sHeightMap.pVertices[psIndex->i2].t.x, m_sHeightMap.pVertices[psIndex->i2].t.y) ;
	tri.avVertex[2].vTex.set(m_sHeightMap.pVertices[psIndex->i3].t.x, m_sHeightMap.pVertices[psIndex->i3].t.y) ;

	Matrix4 matRot ;
	Quaternion q(Vector3(0, 1, 0), Vector3(tri.sPlane.a, tri.sPlane.b, tri.sPlane.c)) ;
	q.GetMatrix(matRot) ;
	//vMid = tri.GetMidPos() ;
	matRot.m41 = vMid.x ;
	matRot.m42 = vMid.y ;
	matRot.m43 = vMid.z ;
	mat = MatrixConvert(matRot) ;

	m_sHolecup.Initialize(m_pd3dDevice, "../../Media/GroundMap/holecup.RMD", &mat, m_pcTexContainer->FindTexture("holecup_1d"), &tri, nExceptionalIndex, this) ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                     CActionBrushingContainer                                                             //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CActionBrushingContainer::CActionBrushingContainer()
{
    for(int i=0 ; i<MAXNUM_ACTION ; i++)
		m_apContainer[i] = NULL ;
}

CActionBrushingContainer::~CActionBrushingContainer()
{
	Release() ;
}

void CActionBrushingContainer::Release()
{
	for(int i=0 ; i<MAXNUM_ACTION ; i++)
		SAFE_DELETE(m_apContainer[i]) ;
}

void CActionBrushingContainer::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nWidth, int nHeight)
{
	int i ;
	for(i=0 ; i<MAXNUM_ACTION ; i++)
	{
		m_apContainer[i] = new CActionBrushing() ;
		m_apContainer[i]->Initialize(pd3dDevice, nWidth, nHeight) ;
	}
}

CActionBrushing *CActionBrushingContainer::GetEmptyContainer()
{
	int i ;
	for(i=0 ; i<MAXNUM_ACTION ; i++)
	{
		if(!m_apContainer[i]->m_bEnable)
			return m_apContainer[i] ;
	}
	return NULL ;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                         CActionBrushing                                                                  //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CActionBrushing::CActionBrushing()
{
	m_pTex = NULL ;
	m_nNumTex = 0 ;
}

//CActionBrushing::CActionBrushing(LPDIRECT3DDEVICE9 pd3dDevice, int nNumTex, LPDIRECT3DTEXTURE9 pTex, TERRAINMAP *psHeightMap)
//{
//	m_nKind = CAction::ACTION_BRUSHING ;
//	Save(pTex, pd3dDevice) ;
//	m_nNumTex = nNumTex ;
//}

CActionBrushing::~CActionBrushing()
{
	Release() ;
}

void CActionBrushing::Release()
{
    SAFE_RELEASE(m_pTex) ;
}

bool CActionBrushing::Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nWidth, int nHeight)
{
	m_nKind = CAction::ACTION_BRUSHING ;
	m_bEnable = false ;

	if(FAILED(D3DXCreateTexture(pd3dDevice, nWidth, nHeight, 1, 0, D3DFMT_R8G8B8, D3DPOOL_MANAGED, &m_pTex)))
	{
		char szMsg[256] ;
		sprintf(szMsg, "Error CreateTexture") ;
		assert(false && szMsg) ;
		return false ;
	}
	FillTextureWithColor(m_pTex, 0x00000000) ;
	TRACE("m_pTex = %x\r\n", m_pTex) ;

	return true ;	
}

void CActionBrushing::Save(LPDIRECT3DTEXTURE9 pTexSrc, LPDIRECT3DDEVICE9 pd3dDevice)
{
	CopyTexture(pTexSrc, m_pTex, pd3dDevice) ;
	m_bEnable = true ;
}

void CActionBrushing::Disable()
{
	m_bEnable = false ;
}

void CActionBrushing::Restore()
{
	//CActionBrushing *psAction = (CActionBrushing *)m_psHeightMap->pcActionManagement->GetLastAction() ;
	m_psHeightMap->apTexAlpha[m_nNumTex] = m_pTex ;
}
/*
void CActionBrushing::Undo()
{
	LPDIRECT3DTEXTURE9 pTex = m_psHeightMap->apTexAlpha[m_nNumTex] ;
	m_psHeightMap->apTexAlpha[m_nNumTex] = m_pTex ;
}

void CActionBrushing::Redo()
{
}
*/