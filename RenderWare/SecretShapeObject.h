#pragma once

#include "SecretShapeLine.h"
#include "ASEData.h"

class CSecretTerrain ;

class CSecretShapeObject
{
private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nNumShape ;
    CSecretShape **m_ppcShape ;
	int m_bRender ;

public :
	CSecretShapeObject() ;
	~CSecretShapeObject() ;

	bool Initialize(CASEData *pcASEData, LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Process(D3DXVECTOR3 *pvCamera, D3DXVECTOR3 *pvUp, D3DXMATRIX *pmatView, D3DXMATRIX *pmatProj, bool bPressed=false, D3DXVECTOR3 *pvCurPos=NULL) ;
	void Render() ;
	void Release() ;

    CSecretShape *GetShape(int nNum) ;
	int GetNumShape() ;

	void SetRender(int bRender) ;
	bool IsRender() ;

	//Dynamic Drawing line
	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, int nMaxNumPosition) ;
	void ProcessDDL(bool bPressed, D3DXVECTOR3 *pvCurPos) ;

	void OnTerrain(CSecretTerrain *pcTerrain) ;
} ;