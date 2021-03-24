#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "ASEData.h"

#define ANITRACK_KEYPOS 1
#define ANITRACK_KEYROT 2
#define ANITRACK_KEYSCALE 3

class CSecretMesh ;

struct SKey
{
	int nFrame ;
} ;

struct SKeyPos : public SKey
{
	D3DXVECTOR3 vPos ;
} ;
struct SKeyRot : public SKey
{
	D3DXQUATERNION quat ;
} ;
struct SKeyScale : public SKey
{
	D3DXVECTOR3 vScale ;
} ;

class CAniTrack
{
public :
	int m_nNumKeyPos ;
	int m_nNumKeyRot ;
	int m_nNumKeyScale ;

    SKeyPos *m_psKeyPos ;
	SKeyRot *m_psKeyRot ;
	SKeyScale *m_psKeyScale ;

	D3DXMATRIX m_matAni ;

	char m_szNodeName[256], m_szParentNodeName[256] ;

private :
    bool _FindKeyPos(float fFrame, SKeyPos **ppKeyPos1, SKeyPos **ppKeyPos2) ;
	bool _FindKeyRot(float fFrame, SKeyRot **ppKeyRot1, SKeyRot **ppKeyRot2) ;
	bool _FindKeyScale(float fFrame, SKeyScale **ppKeyScale1, SKeyScale **ppKeyScale2) ;

	bool _GetKeyPos(float fFrame, D3DXVECTOR3 *pv) ;
    bool _GetKeyRot(float fFrame, D3DXQUATERNION *pquat) ;
	bool _GetKeyScale(float fFrame, D3DXVECTOR3 *pv) ;

	float _GetFrameAlpha(float fFrame, float fFrame1, float fFrame2) ;

public :
	CAniTrack() ;
	~CAniTrack() ;

	void InitKeyPos(SMeshKeyPos **ppMeshKeyPos, int nNumKeyPos) ;
	void InitKeyRot(SMeshKeyRot **ppMeshKeyRot, int nNumKeyRot) ;
	void InitKeyScale(SMeshKeyScale **ppMeshKeyScale, int nNumKeyScale) ;

	D3DXMATRIX *Animation(float fFrame, CSecretMesh *pcMesh) ;

	void Release() ;
} ;
