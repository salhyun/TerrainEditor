#pragma once

#include "SecretEffect.h"
#include "SecretBillboardEffect.h"
#include "DataStructLib.h"

struct SEffectBillboardAttribute
{
	char szFileName[256] ;
	float fOffsetYinPixel ;
} ;

struct SEffectObject
{
	enum OPERATION { ONCE=1, LOOP, SHUTTLE } ;
	enum ATTR { ATTR_INVISIBLE=0x01, ATTR_REVERSE=0x02 } ;
	int nOperation ;
	int nAttr ;

	CSecretEffect *pcEffect ;
	D3DXMATRIX matWorld ;
	float fElapsedTime, fTimeLimit ;

	SEffectObject() ;
	~SEffectObject() ;

	void setEffect(CSecretEffect *effect, float timelimit=1.0f, int operation=ONCE, int attr=0) ;
	void setPosition(float x, float y, float z) ;
	void process(float time) ;
	void deleteEffect() ;
	void setAttr(int attr, bool enable=true) ;
} ;

#define MAXNUM_EFFECTOBJECT 16

class CSecretEffectManager
{
public :
	enum KIND { KIND_SPLASHWATER=0, KIND_SHOT01, KIND_SHOT02, KIND_APPROACHSHOT01, KIND_APPROACHSHOT02, KIND_BUNKERSHOT01, KIND_BUNKERSHOT02, KIND_BUNKERBOUND, KIND_BALLLIGHT, KIND_END } ;

private :
	LPDIRECT3DDEVICE9 m_pd3dDevice ;

	int m_nNumKind ;
	CSecretEffect **m_ppcEffectKinds ;

	data::SKeepingArray<SEffectObject> m_sEffects ;

	CSecretBillboardEffectEffect *m_pcBillboardEffectEffect ;

public :
	CSecretEffectManager() ;
	~CSecretEffectManager() ;

	void initialize(LPDIRECT3DDEVICE9 pd3dDevice) ;
	void setBillboardEffects(int nNum, char **ppszFileNames) ;

	void process(float time) ;
	void render() ;
	void release() ;

	void setBillboardEffectEffect(CSecretBillboardEffectEffect *pcBillboardEffectEffect) ;
	SEffectObject *addEffect(int nKind, D3DXVECTOR3 pos, float fTimeLimit=1.0f, int nOperation=SEffectObject::ONCE, int nAttr=0) ;

} ;