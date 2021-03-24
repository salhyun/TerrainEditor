#pragma once

#include "DNode.h"
#include "SecretNode.h"
#include "SecretMesh.h"
#include "ASEData.h"
#include "SecretCoordSystem.h"
#include "SecretD3DEffect.h"
#include "LightScatteringData.h"
#include "SecretBoundingBox.h"
#include "SecretTextureContainer.h"

#define SKINNINGUSAGE_SOFTWARE         1
#define SKINNINGUSAGE_FIXEDPILELINE    2
#define SKINNINGUSAGE_VERTEXSHADER     3

#define MAXNUM_MESH MAXNUM_ASEDATAMESH

class CSecretMaterial ;
class CSecretMirror ;
class CSecretTerrain ;
class CSecretShadowMap ;

class CSecretObjectScatteringData : public cLightScatteringData
{
public :
	CSecretObjectScatteringData() {} ;
	~CSecretObjectScatteringData() {} ;

	void CalculateShaderParameterObjectOptimized() ;
} ;

struct STerrainInfo_forExportHeightMap
{
	CASEData *pcASEData ;
	STerrain_FragmentInfo *psFragmentInfo ;
	float fStep ;
	int nWidth, nHeight ;
	float fZStart, fZEnd ;
	bool bEnd ;
	bool bTerminate ;
	HANDLE hThread ;
	DWORD dwThreadID ;
	int nProgressCount ;
} ;

struct SAnimationInfo
{
	float fFrameSpeed, fTicksPerFrame ;
	float fTotalFrame ;
	float fFirstFrame, fLastFrame ;
} ;

struct SD3DMaterial
{
	int nAttr ;
	bool bUseTexDecal ;
	bool bUseOpacity ;
	bool bUseNormalMap ;
	bool bUseAmbientMap ;

	char szDecaleName[256] ;
	char szBumpName[256] ;
	char szAmbientName[256] ;

	int nNumSubMaterial ;
	SD3DMaterial *psSubMaterial ;

	SColor sDiffuse, sAmbient, sSpecular ;

	LPDIRECT3DTEXTURE9 pTexDecal, pTexBump ;

	SD3DMaterial()
	{
		pTexDecal = NULL ;
		pTexBump = NULL ;
		bUseTexDecal = bUseOpacity = bUseNormalMap = bUseAmbientMap = false ;
		sprintf(szDecaleName, "") ;
		sprintf(szBumpName, "") ;
		sprintf(szAmbientName, "") ;
		nNumSubMaterial = 0 ;
		psSubMaterial = NULL ;
	}
	~SD3DMaterial()
	{
		Release() ;
	}

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SMaterial *psMaterial, char *pszPath, CSecretTextureContainer *pcTexContainer) ;
	void Release() ;
} ;

#define MOBJ_RO_RIGID 0x01//RenderOption both CSecretMeshRigid and CSecretMeshSkinned
#define MOBJ_RO_BONE 0x02
#define MOBJ_RO_IDSHADOW 0x04
#define MOBJ_RO_LOCALSYS 0x08

class CSecretMeshObject
{
public :
	enum KIND { KIND_RIGID=0, KIND_BILLBOARD } ;

	enum ATTR
	{
		ATTR_SOLID=0x01,
		ATTR_ANI=0x02,
		ATTR_MIRRORMAPPING=0x04,
		ATTR_IDSHADOW=0x08,
		ATTR_SHADOWRECV=0x10,
		ATTR_BILLBOARD_NOMIPMAPFILTER=0x20,
		ATTR_IDMAP=0x40,
		ATTR_REFLECTED=0x80, //물, 거울등에 비치는 속성
		ATTR_WAVINGLEAF=0x100 //흔들흔들 나뭇잎
	} ;
	enum SUBATTR
	{
		SUBATTR_ANI_KEYFRAME=0x01, SUBATTR_ANI_SKINNED=0x02
	} ;

	enum SHADERPASS
	{
		PASS_USETEX=0, PASS_NOTEXTURE, PASS_SKINNING, PASS_BUMP, PASS_USEAMBIENT, PASS_IDSHADOW, PASS_IDMAP, PASS_SUNLIT, PASS_NOLIGHTINGSUNLIT, PASS_TWOSIDEDSUNLIT, PASS_BILLBOARD, PASS_BILLBOARDNOMIPMAPFILTER, PASS_BILLBOARDSHADOW, PASS_SIZZLING,
	} ;

private :
	unsigned long m_lIndexDistribute ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	UINT m_lSkinningUsage ;
	UINT m_lKind ;
	UINT m_lAttr, m_lSubAttr ;
	UINT m_lEigenIndex ;

	char m_szObjName[256], m_szCatalogName[128] ;

	SAnimationInfo m_sAnimationInfo ;
	UINT m_lRenderOption ;
	bool m_bWireFrame ;
	int m_nNumMesh ;
	CDNode m_cMeshNode ;//Mesh Node

	int m_nNumMaterial ;
	SD3DMaterial *m_psMaterial ;

	CDNode m_cHierarchyMeshList ;//에니메이션을 위한 메쉬계층구조

	char *m_apszBoneName[256] ;
	int m_nNumBone ;
	D3DXMATRIX *m_pmatSkin ;//matBone for Skinning

	D3DXMATRIX m_matWorld ;
	D3DXMATRIX *m_pmatDummy ;

	std::vector<CSecretMaterial *> m_cMaterialNode ;

	float m_fOldTime, m_fCurTime, m_fTimeGap, m_fAniTime ;
	float m_fElapsedTime, m_fOffset ;

	CSecretCoordSystem m_cLocalSystem ;
	CSecretMirror *m_pcMirror ;
	CSecretShadowMap *m_pcShadowMap ;

	void _TimerAnimation(float fCurTime) ;
	void _SetEigenIndexforIdShadow(CSecretNode *pcMeshNode, D3DXVECTOR4 &vId) ;

public :
	CSecretObjectScatteringData *m_pcAtm ;
	Vector3 m_vMin, m_vMax ;
	float m_fObjectHeight ;
	DWORD m_dwAddAlpha ;
	float m_fAlphaTestDiffuse ;
	int m_anTemporary[16] ;
	float m_fWaveLeafTimeScale, m_fWaveLeafFrequency, m_fWaveLeafAmplitude ;

public :
	CSecretMeshObject() ;
	CSecretMeshObject(LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex) ;
	CSecretMeshObject(LPDIRECT3DDEVICE9 pd3dDevice, CASEData *pcASEData, char *pszPath, CSecretTextureContainer *pcTexContainer, UINT lIndex) ;
	~CSecretMeshObject() ;

	void InitMemberVariable(LPDIRECT3DDEVICE9 pd3dDevice, UINT lIndex) ;
	bool InitObject(CASEData *pcASEData, char *pszPath, CSecretTextureContainer *pcTexContainer, UINT lAddNodeAttr=0) ;

	void Animation(float fFrame) ;//Animation 이 되는 Mesh 에서 사용.

	void Render(D3DXMATRIX *pmatWorld) ;//use FixedPipeLine
	void Render(SD3DEffect *psEffect, D3DXMATRIX *pmatWorld) ;
	void Render(SD3DEffect *psEffect, CSecretMesh *pcMeshNode, D3DXMATRIX *pmatWorld) ;//use Shader
	//void RenderIdShadow(SD3DEffect *psEffect, CSecretIdShadow *pcIdShadow) ;//ID Shadow 라이트뷰공간에서 렌더링한다. 오브젝트의 Id, 메쉬의 깊이값을 저장

	void SetObjName(char *pszObjName) ;
	char *GetObjName() ;
	char *GetCatalogName() ;

	void AddAttr(int nAttr, bool bEnable=true) ;
	void SetShadowMap(CSecretShadowMap *pcShadowMap) ;

	void SetKind(UINT lKind) {m_lKind=lKind;}
	UINT GetKind() {return m_lKind;}
	UINT GetAttr() {return m_lAttr;}
	UINT GetSubAttr() {return m_lSubAttr;}
	int GetNumMesh() {return m_nNumMesh;}
	int GetNumBoneMesh() {return m_nNumBone;}
	int GetNumMaterial() {return m_nNumMaterial;}
	SD3DMaterial *GetMaterial(int nIndex) {return &m_psMaterial[nIndex];}
	CSecretMesh *GetMesh(int nIndex) ;
	
	void SetRenderOption(UINT lRenderOption) ;

	void SetmatWorld(D3DXMATRIX &matWorld) ;//위부행렬을 가지고 셋팅(변환들이 누적되지 않고 일회성으로 끝남.)
	D3DXMATRIX *GetmatWorld() {return &m_matWorld;} ;

	SAnimationInfo *GetAnimationInfo() {return &m_sAnimationInfo;} ;
	void SetWireFrame(bool bEnable) ;
	D3DXMATRIX *GetmatSkin() {return m_pmatSkin;} ;

	bool ExportHeightMap(CASEData *pcASEData, char *pszFileName) ;//only use for RigidMesh

	void SetElapsedTime(float ftime) ;
	float GetElapsedTime() ;

	//Function for MirrorPlane
	void AttachMirror(D3DXVECTOR3 &vMirrorPos, D3DXVECTOR3 &vNormal, float fMirrorSize) ;
	void ProcessMirror(D3DXVECTOR3 &vCameraPos, SD3DEffect *psEffect, CSecretTerrain *pcTerrain) ;
	void MirrorMapping() ;
	void RenderMirrorPlane() ;

	//삼각형에 대한 노말벡터를 렌더링한다.
	//It's Rendering NormalVector of Triangle(Polygon) for Debugging
	void RenderNormalVector(D3DXMATRIX *pmatWorld) ;

	//Render CollisionVolume
	void RenderCollisionVolume(D3DXMATRIX *pmatWorld) ;
	
	//VertexDisk, AmbientOcclusion
	void RenderVertexDisk() ;
	void GetVertexDisks(std::vector<SVertexDisk *> *pVertexDisks) ;
	void PreComputedAmbientOcclusionPerVertex() ;
	void SetVertexDisks(std::vector<SVertexDisk *> *pVertexDisks) ;
	void SetVertexDisks(CASEData *pcASEData, std::vector<SVertexDisk *> *pVertexDisks) ;

	void SetAddAlpha(char *pszObjectName) ;
	void SetBillboardAttr(char *pszObjectName) ;

	//void ApplyOctTree(CASEData *pcASEData) ;

	int GetBoundingPosfromAllMesh(Vector3 *pvRecvPos) ;

	void buildCollisionVolume() ;
	void releaseCollisionVolume() ;

	void setAlphaBlendingbyForce(float alpha) ;
	void setWavingLeaf() ;
	SContactSurfaceInfo getSurfaceInfo(char *pszFileName) ;

	void Release() ;

} ;