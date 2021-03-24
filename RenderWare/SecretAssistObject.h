#pragma once

#include "def.h"
#include "D3Ddef.h"
#include "GeoLib.h"
#include "DataStructLib.h"

#define D3DFVF_ASSISTOBJECT (D3DFVF_XYZ|D3DFVF_TEX1)

class CSecretTextureContainer ;
class CSecretGizmoSystem ;
class CSecretPicking ;
class CSecretToolBrush ;

struct SAssistObjectVertex
{
	D3DXVECTOR3 pos ;
	D3DXVECTOR2 tex ;
} ;
struct SAssistObjectIndex
{
	unsigned short awIndex[3] ;
} ;

struct SAssistObjectKind
{
	char szName[128] ;
	int nNumVertex, nNumIndex ;

	LPDIRECT3DVERTEXBUFFER9 pVB ;
	LPDIRECT3DINDEXBUFFER9 pIB ;
	LPDIRECT3DTEXTURE9 pTex ;

	SAssistObjectKind() ;
	~SAssistObjectKind() ;

    HRESULT Initialize(LPDIRECT3DDEVICE9 pd3dDevice, char *pszName, SAssistObjectVertex *psSrcVertices, int nNumSrcVertex, SAssistObjectIndex *psSrcIndices, int nNumSrcIndex, LPDIRECT3DTEXTURE9 pTexture) ;
	void Release() ;
} ;

class CSecretAssistObject
{
public :
	enum STATUS { PICKING=1, SELECTED, UNSELECTED } ;

private :
	int m_nStatus ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	SAssistObjectKind *m_psObjectKind ;
	CSecretGizmoSystem *m_pcGizmo ;
	D3DXMATRIX m_matTransform, m_matOriginScale, m_matOriginRot, m_matOriginTrans ;
	CSecretToolBrush *m_pcToolBrush ;

public :
	CSecretAssistObject() ;
	~CSecretAssistObject() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, SAssistObjectKind *psObjectKind, CSecretGizmoSystem *pcGizmo, CSecretToolBrush *pcToolBrush) ;
	void Process(Vector3 *pvCameraPos, bool bPressedLButton) ;
	void Render() ;
	void Release() ;

	void SetStatus(int nStatus, Vector3 *pvCameraPos=NULL) ;
	int GetStatus() {return m_nStatus;}
	D3DXMATRIX *GetmatTransform() {return &m_matTransform;}
	void SetmatTransform(D3DXMATRIX *pmat) ;
	char *GetKindName() {return m_psObjectKind->szName;}
	SAssistObjectKind *GetObjectKind() ;
	D3DXMATRIX *GetTransform() ;

} ;

class CSecretAssistObjectManager
{
public :
	const int MAXNUM_OBJECT ;

	enum ATTR { VISIBILITY=0x01 } ;
	enum STATUS {IDLING=0, PROCESSING } ;

private :
	bool m_bEnable ;
	int m_nStatus ;
	int m_nAttr ;
	LPDIRECT3DDEVICE9 m_pd3dDevice ;
	int m_nNumObjectKind ;
	SAssistObjectKind *m_psObjectKind ;
	data::SKeepingArray<CSecretAssistObject> m_cObjects ;
	CSecretAssistObject *m_pcCurrentObject ;
	CSecretTextureContainer *m_pcTexContainer ;

	CSecretGizmoSystem *m_pcGizmo ;

	void _InitObjectKind() ;

public :
	CSecretToolBrush *m_pcToolBrush ;

public :
	CSecretAssistObjectManager() ;
	~CSecretAssistObjectManager() ;

	bool Initialize(LPDIRECT3DDEVICE9 pd3dDevice, CSecretToolBrush *pcToolBrush=NULL) ;
	void Process(Vector3 vCameraPos, bool bPressedLButton) ;
	void Render() ;
	void Release() ;

	void CreateAssistObject(char *pszName) ;
	CSecretAssistObject *SelectAssistObject(int nIndex) ;
	void DeleteAssistObject(int nIndex) ;

	void ImportAssistObject(char *pszName, D3DXMATRIX *pmat) ;

	SAssistObjectKind *FindObjectKind(char *pszName) ;

	void ResetObjects() ;

	CSecretTextureContainer *GetTexContainer() ;

	int GetNumObjectKind() {return m_nNumObjectKind;}
	SAssistObjectKind *GetObjectKind() {return m_psObjectKind;}
	CSecretGizmoSystem *GetGizmoSystem() {return m_pcGizmo;}
	void SetEnable(bool bEnable=true) ;
	bool GetEnable() ;
	void AddAttr(int nAttr, bool bEnable=true) ;
	bool IsAttr(int nAttr) ;
	void SetStatus(int nStatus) ;
	int GetStatus() ;
	CSecretAssistObject *GetCurrentAssistObject() {return m_pcCurrentObject;}
	data::SKeepingArray<CSecretAssistObject> *GetObjects() {return &m_cObjects;}

	D3DXMATRIX *GetObjectMatrix(char *pszName) ;

} ;