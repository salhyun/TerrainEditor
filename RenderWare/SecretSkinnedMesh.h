#pragma once

#include "SecretMesh.h"

#define D3DFVF_SKINNEDMESHVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

struct SSkinnedMesh_Vertex
{
    D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		t;

} ;

struct SSkinnedMesh_Index
{
	unsigned short anIndex[3] ;
} ;


#define MAXNUM_WEIGHTVALUE 12

struct SSkinData
{
	int nNumIndex ;
	std::vector<int> anBoneIndex ;//본인덱스는 본메쉬의 로칼매트릭스를 담은 리스트를 가리킨다
	std::vector<float> afWeight ;//해당되는 본메쉬 로칼매트릭스의 가중치

	D3DXVECTOR3 vPos ;
	D3DXVECTOR3 vNormal ;

	SSkinData()
	{
		vPos.x = vPos.y = vPos.z = 0.0f ;
		vNormal.x = vNormal.y = vNormal.z = 0.0f ;
	}
		
} ;

//소프트웨어로 하는 스키닝에니메이션
class CSecretSkinnedMesh : public CSecretMesh
{
protected :
    SSkinnedMesh_Vertex *m_psVertices ;
    SSkinnedMesh_Index *m_psIndices ;

	SSkinData *m_psSkinData ;//m_dwVertex 만큼 할당된다.

public :
    CSecretSkinnedMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretSkinnedMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;

	//메트릭스 빠레뜨를 사용해서 버텍스에다가 본의 가중치값을 계산해준다.
	HRESULT SetMatrixPaletteSW(D3DXMATRIX *pmatSkin, D3DXMATRIX *pmatDummy, char **ppszBoneName) ;

	SSkinData *GetSkinData() {return m_psSkinData;} ;

	virtual void Render() ;
	virtual void RenderNormalVector(Matrix4 &matWorld) ;
} ;