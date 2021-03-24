#pragma once

#include "SecretMesh.h"

class CSecretIdShadow ;

#define D3DFVF_RIGIDMESHVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEXCOORDSIZE2(0)|D3DFVF_TEXCOORDSIZE1(1)|D3DFVF_TEX2)
struct SRigidMesh_Vertex
{
    D3DXVECTOR3		pos;
	D3DXVECTOR3		normal;
	D3DXVECTOR2		t;
	float    		ambient;
} ;

struct SRigidMesh_Index
{
	unsigned short anIndex[3] ;

	SRigidMesh_Index()
	{
		anIndex[0] = anIndex[1] = anIndex[2] = 0 ;
	}

	SRigidMesh_Index(unsigned short *pnIndex)
	{
		anIndex[0] = pnIndex[0] ;
		anIndex[1] = pnIndex[1] ;
		anIndex[2] = pnIndex[2] ;
	}
} ;
//using SubMaterial in Ascii file
//gathering indexset by same texture
struct SRigidMesh_IndexSubSet
{
	int nMaterialID ;
	int nNumTriangle ;
	SRigidMesh_Index *psIndices ;//Set to same Texture

	SRigidMesh_IndexSubSet()
	{
		nMaterialID = nNumTriangle = 0 ;
		psIndices = NULL ;
	}
	SRigidMesh_IndexSubSet(int nID, int nNum)
	{
		nMaterialID = nID ;
		nNumTriangle = nNum ;
		psIndices = new SRigidMesh_Index[nNum] ;
	}

	~SRigidMesh_IndexSubSet()
	{
		nMaterialID = nNumTriangle = 0 ;
		SAFE_DELETEARRAY(psIndices) ;
	}
} ;

struct SLeafAni_in_RigidMesh
{
	bool bEnable ;
    float fCurTheta, fLimitedTheta, fSpeed ;
	float afNoise[16] ;
	D3DXMATRIX matAni ;

	float xuck, curdelta ;

	SLeafAni_in_RigidMesh() ;
    void Reset(float limited_theta, float speed, float initial_theta=0.0f) ;
	void Process(float fTimeSinceLastFrame) ;
} ;

class CSecretRigidMesh : public CSecretMesh
{
protected :
	SRigidMesh_Vertex *m_psVertices ;//��Ȱ���� ���� ����Ÿ
	SRigidMesh_Index *m_psIndices ;//��Ȱ���� ���� ����Ÿ

	int m_nNumSubSet ;
	SRigidMesh_IndexSubSet *m_psIndexSubSet ;//SubMaterial�� ����ϴ� ����Ÿ
	LPDIRECT3DINDEXBUFFER9 *m_ppSubSetVI ;//D3D���� ����ϴ� ����Ÿ

	void _BuildBoundingSphere() ;
	void _BuildBoundingCube() ;// Oriented-Bounding Box
	void _BuildCollisionModel() ;

	void _ExportHolecupPolygons() ;

public :
	SLeafAni_in_RigidMesh m_sLeafAni ;

public :
	CSecretRigidMesh(unsigned long lEigenIndex, LPDIRECT3DDEVICE9 pd3dDevice) ;
	virtual ~CSecretRigidMesh() ;

	HRESULT Initialize(SMesh *pMesh) ;

	void SetWireFrame(bool bEnable) {m_bWireFrame=bEnable;} ;
	int GetNumSubSet() {return m_nNumSubSet;} ;
	int GetSubMaterialID(int nCount) ;

	SRigidMesh_Vertex *GetVertices() {return m_psVertices;} ;
	SRigidMesh_Index *GetIndices() {return m_psIndices;} ;

	void SetMeshDiffuse(char *pszObjectName) ;
    
	virtual HRESULT RefreashVertexBuffer() ;

	virtual void Render() ;
	virtual void RenderShadow() ;
	virtual void RenderNormalVector(Matrix4 &matWorld) ;//�ﰢ���� ������Ÿ ������
	virtual void RenderVertexDisk(Matrix4 &matWorld) ;//���������� ������ ��ũ ������
	virtual void RenderBoundingVolume(Matrix4 &matWorld) ;

	virtual void PreRenderSubSet() ;
	virtual void RenderSubSet(int nCount) ;
	virtual void PostRenderSubSet() ;
	virtual HRESULT InitShadowData() ;

	void BuildCollisionModel() ;
	void ReleaseCollisionModel() ;

	void GetFragmentInfo(float fMinX, float fMinZ, float fWidth, float fHeight, STerrain_FragmentInfo *psFragmentInfo) ;//Incomplete
	HRESULT RenderPerspectiveShadowMap(dxMatrix4 &dxmatWorld, dxMatrix4 &dxmatView, CSecretIdShadow *pcIdShadow) ;//Incomplete
} ;