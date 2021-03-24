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
	std::vector<int> anBoneIndex ;//���ε����� ���޽��� ��Į��Ʈ������ ���� ����Ʈ�� ����Ų��
	std::vector<float> afWeight ;//�ش�Ǵ� ���޽� ��Į��Ʈ������ ����ġ

	D3DXVECTOR3 vPos ;
	D3DXVECTOR3 vNormal ;

	SSkinData()
	{
		vPos.x = vPos.y = vPos.z = 0.0f ;
		vNormal.x = vNormal.y = vNormal.z = 0.0f ;
	}
		
} ;

//����Ʈ����� �ϴ� ��Ű�׿��ϸ��̼�
class CSecretSkinnedMesh : public CSecretMesh
{
protected :
    SSkinnedMesh_Vertex *m_psVertices ;
    SSkinnedMesh_Index *m_psIndices ;

	SSkinData *m_psSkinData ;//m_dwVertex ��ŭ �Ҵ�ȴ�.

public :
    CSecretSkinnedMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretSkinnedMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;

	//��Ʈ���� �����߸� ����ؼ� ���ؽ����ٰ� ���� ����ġ���� ������ش�.
	HRESULT SetMatrixPaletteSW(D3DXMATRIX *pmatSkin, D3DXMATRIX *pmatDummy, char **ppszBoneName) ;

	SSkinData *GetSkinData() {return m_psSkinData;} ;

	virtual void Render() ;
	virtual void RenderNormalVector(Matrix4 &matWorld) ;
} ;