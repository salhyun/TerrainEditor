#pragma once

#include "SecretMeshObject.h"
#include "ArrangedDNode.h"

//메쉬별로 정렬할것인가? 오브젝트별로 정렬할것인가?
//메쉬별로 정렬한다.

//순서는 알파값이 없는 것들은 렌더링시엔 상관없더라도 가까운 것->먼 것으로 찍어주고... 
//알파값이 있는 것들이 먼것에서 가까운 것으로 순서가 반대입니다... 
//(알파가 없는 것은 가까운것부터 찍어야 효과적입니다... 
//2개가 소팅순서가 서로 반대더라도 소트함수는 한번만 호출하는 것은 가능하겠네요)

//메쉬별로 정렬을 할것이다!
//I will sort meshs by the distance from the view camera

//오브젝트에 속한 메쉬들은 디자이너와 상의해서
//규칙에 의거하여 작업하도록 한다.

//###############################//
//    How to Management Meshs    //
//###############################//
//Z Ordering NO!
//Classified by Texture NO!  이미 모델제작때부터 One Mesh to One Texture형태로 만들어진다.
//Classified by SD3DEffect OK!
//구분은 AlphaBlending 여부에 따라서 분류한다.

//One Mesh to some Textures 도 있지만 특수한 효과를 사용하기 때문에 많은 양은 안될것이다.
//그리고 또한 그것의 고유의 이펙트를 사용할것이다.

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
		RROLL_ATTR_NEARTOFAR=0x01,//알파블랜딩 하지 않는것.
		RROLL_ATTR_FARTONEAR=0x02,//알파블랜딩 하는것.
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