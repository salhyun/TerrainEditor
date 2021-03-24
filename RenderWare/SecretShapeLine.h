#pragma once

#include "SecretShape.h"
#include "D3DDef.h"

struct SShapeLine ;
class CSecretTerrain ;

#define D3DFVF_SHAPELINE (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct SShapeLine_Vertex
{
	D3DXVECTOR3 pos ;
	DWORD color ;
} ;

struct SShapeLine_Index
{
	WORD wIndex[3] ;
} ;

class CSecretShapeLine : public CSecretShape
{
protected :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	D3DXVECTOR3 m_vCenter ;
	int m_nNumPosition ;
	D3DXVECTOR3 *m_pvPositions, *m_psOriginPositions ;
	int m_nNumVertex ;
	SShapeLine_Vertex *m_psVertices ;
	int m_nNumIndex ;
	SShapeLine_Index *m_psIndices ;
	D3DXMATRIX m_matLocal ;
	D3DXMATRIX *m_pmatView ;
	float m_fLineWidth ;

	int m_nMaxNumPosition ;
	int m_nMaxNumVertex, m_nMaxNumIndex ;
	DWORD m_dwColor ;

	void _BuildQuadStrip(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SShapeLine_Vertex *psVertices, float fWidth, DWORD color) ;
	void _BuildQuadList(D3DXVECTOR3 *pvPos, int nNumPos, D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatVP, SShapeLine_Vertex *psVertices, SShapeLine_Index *psIndices, float fWidth, DWORD color) ;

public :
	CSecretShapeLine() ;
	CSecretShapeLine(int nKind) ;
	virtual ~CSecretShapeLine() ;

	bool Initialize(SShapeLine *psShapeLine, LPDIRECT3DDEVICE9 pd3dDevice, float fLineWidth=0.05f) ;
	void Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj) ;
	void SetLocalTM(D3DXMATRIX *pmat) ;
	virtual void Render() ;
	virtual void Release() ;

	void SetLineColor(DWORD dwColor) {m_dwColor=dwColor;}
	DWORD GetLineColor() {return m_dwColor;}

	int GetNumPosition() {return m_nNumPosition;}
	void SetNumPosition(int nNum) {m_nNumPosition=nNum;}
	D3DXVECTOR3 *GetPositions() {return m_pvPositions;}
	D3DXVECTOR3 *GetCenterPos() {return &m_vCenter;}
	

	void OnTerrain(CSecretTerrain *pcTerrain) ;

} ;