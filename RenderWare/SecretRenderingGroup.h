#pragma once

#include "SecretMeshObject.h"
#include "ArrangedDNode.h"

//�޽����� �����Ұ��ΰ�? ������Ʈ���� �����Ұ��ΰ�?
//�޽����� �����Ѵ�.

//������ ���İ��� ���� �͵��� �������ÿ� ��������� ����� ��->�� ������ ����ְ�... 
//���İ��� �ִ� �͵��� �հͿ��� ����� ������ ������ �ݴ��Դϴ�... 
//(���İ� ���� ���� �����ͺ��� ���� ȿ�����Դϴ�... 
//2���� ���ü����� ���� �ݴ���� ��Ʈ�Լ��� �ѹ��� ȣ���ϴ� ���� �����ϰڳ׿�)

//�޽����� ������ �Ұ��̴�!
//I will sort meshs by the distance from the view camera

//������Ʈ�� ���� �޽����� �����̳ʿ� �����ؼ�
//��Ģ�� �ǰ��Ͽ� �۾��ϵ��� �Ѵ�.

//###############################//
//    How to Management Meshs    //
//###############################//
//Z Ordering NO!
//Classified by Texture NO!  �̹� �����۶����� One Mesh to One Texture���·� ���������.
//Classified by SD3DEffect OK!
//������ AlphaBlending ���ο� ���� �з��Ѵ�.

//One Mesh to some Textures �� ������ Ư���� ȿ���� ����ϱ� ������ ���� ���� �ȵɰ��̴�.
//�׸��� ���� �װ��� ������ ����Ʈ�� ����Ұ��̴�.

class CSecretIdShadow ;
class CSecretRenderingRoll ;
struct STrueRenderingObject ;
class CSecretWaterManager ;

#define MAXNUM_WAITINGFORRENDERINGOBJECT 2048//1024

struct SWaitingforRenderingObject
{
	D3DXMATRIX matWorld ;
	CSecretMeshObject *pcObject ;

	SWaitingforRenderingObject() {D3DXMatrixIdentity(&matWorld); pcObject=NULL;}
	void set(CSecretMeshObject *object, D3DXMATRIX *pmatWorld)
	{
		matWorld = (*pmatWorld) ;
		pcObject = object ;
	}
} ;

#define MAXNUM_RENDERINGROLL 2

class CSecretRenderingGroup
{
public :
	enum ROLLKIND
	{
		ROLLKIND_NOALPHABLENDED=0, ROLLKIND_ALPHABLENDED, ROLLKIND_ALPHATEST
	} ;

//Variable
private :
    LPDIRECT3DDEVICE9 m_pd3dDevice ;
    int m_nObjectCount ;
	SWaitingforRenderingObject *m_psObjects ;
	CSecretShadowMap *m_pcShadowMap ;

    CSecretRenderingRoll *m_psRolls ;

public :
	D3DXMATRIX m_matMirror ;
	CSecretWaterManager *m_pcWaterManager ;

//Function
private :
	void _Sorting() ;

public :
	CSecretRenderingGroup() ;
	~CSecretRenderingGroup() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void Process(float fFrame) ;
	void Render(SD3DEffect *psEffect, bool bWireFrame, int nInstantAttr=0, int nDistNear=0, int nDistFar=0) ;
	void Release() ;

	bool InsertObject(CSecretMeshObject *pcMeshObject, D3DXMATRIX *pmatWorld, Vector3 vCameraPos, int nAttr=0) ;
	void SetShadowMap(CSecretShadowMap *pcShadowMap) ;

	void ResetRolls() ;
	void SetWaterManager(CSecretWaterManager *pcWaterManager) ;
	void SetSpecificRenderingArea(Vector3 vPos, float fRadius) ;
	CSecretRenderingRoll *GetRenderingRolls() ;
} ;

#define MAXNUM_RENDERINGNODE 10000

struct SRenderingNode
{
	enum ATTR { ATTR_SHADOW=0x01, ATTR_ALPHATESTBYLOD=0x02, ATTR_ALPHABLENDINGBYFORCE=0x04 } ;

	int bEnable ;
	int nAttr ;
	int nIndex ;
	int nNumMesh ;
	D3DXMATRIX *pmatWorld ;
	CSecretMesh *pcMesh ;
	CSecretMeshObject *pcParentObject ;
	float fDistfromCamera ;

	SRenderingNode() ;
	SRenderingNode(int index) ;
	void set(CSecretMesh *mesh, CSecretMeshObject *object, int index, int attr=0) ;
} ;

struct SSpecificRenderingArea
{
	Vector3 vPos ;
	float fRadius ;
} ;

class CSecretRenderingRoll
{
public :
	enum RENDERINGROLL_ATTR
	{
		RROLL_ATTR_NEARTOFAR=0x01,//���ĺ��� ���� �ʴ°�.
		RROLL_ATTR_FARTONEAR=0x02,//���ĺ��� �ϴ°�.
		RROLL_ATTR_SHADOWRENDERING=0x08,
		RROLL_ATTR_INSTANTBLENDTOTEST=0x10,
		RROLL_ATTR_MIRRORRENDER=0x20,
		RROLL_ATTR_BEHINDMIRRORRENDER=0x40,
	} ;

//Variable
private :
	int m_nAttr ;

	DWORD m_dwZEnable, m_dwZWriteEnable, m_dwZFunc ;

	int m_nNodeCount ;
	SRenderingNode *m_psNodes ;
    int (*m_pfCompare)(const void *, const void *) ;

public :
	D3DXMATRIX *m_pmatMirror ;
	CSecretWaterManager *m_pcWaterManager ;
	SSpecificRenderingArea m_sSpecificRenderingArea ;

	float m_fAlphaTestDiffuse ;

//Function
private :
	bool _RenderTestbySpecificRenderingArea(SRenderingNode *psNode) ;

public :
	CSecretRenderingRoll() ;
	~CSecretRenderingRoll() ;

	bool Initialize(int nAttr) ;
	void Process(float fFrame) ;
	void Render(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, bool bWireFrame, int nDistNear=0, int nDistFar=0) ;
	void RenderShadow(LPDIRECT3DDEVICE9 pd3dDevice, SD3DEffect *psEffect, CSecretShadowMap *pcShadowMap) ;
	void Release() ;

	void Sorting() ;
	void Reset() ;

	bool InsertMesh(CSecretMesh *pcMesh, CSecretMeshObject *pcParentObject, D3DXMATRIX *pmatWorld, Vector3 *pvCameraPos, int nAttr=0) ;
	float CalculateDistfromCamera(SRenderingNode *psNode, D3DXMATRIX *pmatWorld, Vector3 *pvCamera) ;

	void AddAttr(int nAttr, bool bEnable=true) ;
	bool IsAttr(int nAttr) ;
} ;