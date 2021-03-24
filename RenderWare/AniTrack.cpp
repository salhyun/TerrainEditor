#include "AniTrack.h"
#include <assert.h>
#include "SecretMath.h"
#include "def.h"
#include "SecretMesh.h"

CAniTrack::CAniTrack()
{
	m_psKeyPos = NULL ;
	m_psKeyRot = NULL ;
	m_psKeyScale = NULL ;

	m_nNumKeyPos = m_nNumKeyRot = m_nNumKeyScale = 0 ;

	//TRACE("CAniTrack Construction\r\n") ;
} ;
CAniTrack::~CAniTrack()
{
	Release() ;
	//TRACE("CAniTrack Distruction\r\n") ;
}

void CAniTrack::InitKeyPos(SMeshKeyPos **ppMeshKeyPos, int nNumKeyPos)
{
	m_nNumKeyPos = nNumKeyPos ;
	D3DXVECTOR4 v ;
    D3DXMATRIX matChildInv ;

	m_psKeyPos = new SKeyPos[m_nNumKeyPos] ;

    for(int i=0 ; i<m_nNumKeyPos ; i++)
	{
		m_psKeyPos[i].nFrame = ppMeshKeyPos[i]->nFrame ;
		m_psKeyPos[i].vPos = D3DXVECTOR3(ppMeshKeyPos[i]->pos.x, ppMeshKeyPos[i]->pos.y, ppMeshKeyPos[i]->pos.z) ;

		//TRACE("KeyPos [%d] frame:%d pos : x=%.3f y=%.3f z=%.3f\r\n",
		//	i, m_psKeyPos[i].nFrame, m_psKeyPos[i].vPos.x, m_psKeyPos[i].vPos.y, m_psKeyPos[i].vPos.z) ;
	}
}

void CAniTrack::InitKeyRot(SMeshKeyRot **ppMeshKeyRot, int nNumKeyRot)
{
	m_nNumKeyRot = nNumKeyRot ;

	D3DXMATRIX matRot, matChildInv ;

	m_psKeyRot = new SKeyRot[m_nNumKeyRot] ;

    for(int i=0 ; i<m_nNumKeyRot ; i++)
	{
		m_psKeyRot[i].nFrame = ppMeshKeyRot[i]->nFrame ;
		m_psKeyRot[i].quat = D3DXQUATERNION(ppMeshKeyRot[i]->q.x, ppMeshKeyRot[i]->q.y, ppMeshKeyRot[i]->q.z, ppMeshKeyRot[i]->q.w) ;

		//TRACE("KeyRot [%d] frame:%d quat : x=%.3f y=%.3f z=%.3f w=%.3f\r\n",
		//	i, m_psKeyRot[i].nFrame, m_psKeyRot[i].quat.x, m_psKeyRot[i].quat.y, m_psKeyRot[i].quat.z, m_psKeyRot[i].quat.w) ;
	}
}

void CAniTrack::InitKeyScale(SMeshKeyScale **ppMeshKeyScale, int nNumKeyScale)
{
	m_nNumKeyScale = nNumKeyScale ;

	m_psKeyScale = new SKeyScale[m_nNumKeyScale] ;

    for(int i=0 ; i<m_nNumKeyScale ; i++)
	{
		m_psKeyScale[i].nFrame = ppMeshKeyScale[i]->nFrame ;
		m_psKeyScale[i].vScale = D3DXVECTOR3(ppMeshKeyScale[i]->scale.x, ppMeshKeyScale[i]->scale.y, ppMeshKeyScale[i]->scale.z) ;

		//TRACE("KeyScale [%d] frame:%d scale : x=%.3f y=%.3f z=%.3f\r\n",
		//	i, m_psKeyScale[i].nFrame, m_psKeyScale[i].vScale.x, m_psKeyScale[i].vScale.y, m_psKeyScale[i].vScale.z) ;
	}
}

bool CAniTrack::_FindKeyPos(float fFrame, SKeyPos **ppKeyPos1, SKeyPos **ppKeyPos2)
{
	if(m_nNumKeyPos == 0)
		return false ;

	if(fFrame < 0)
	{
		*ppKeyPos1 = *ppKeyPos2 = &m_psKeyPos[0] ;
		return true ;
	}
	if(fFrame > (float)m_psKeyPos[m_nNumKeyPos-1].nFrame)
	{
		*ppKeyPos1 = *ppKeyPos2 = &m_psKeyPos[m_nNumKeyPos-1] ;
		return true ;
	}

	int i ;
	int nNode=-1 ;
	float fFrameGap, ftemp ;

	fFrameGap = (float)m_psKeyPos[m_nNumKeyPos-1].nFrame ;
    for(i=0 ; i<m_nNumKeyPos ; i++)
	{
		if(float_eq((float)m_psKeyPos[i].nFrame, fFrame))
		{
			*ppKeyPos1 = *ppKeyPos2 = &m_psKeyPos[i] ;
			return true ;
		}

		//현재프레임보다 이전프레임중에서 현재프레임과 가장 가까운 프레임을 얻는다.
		ftemp = fFrame-(float)m_psKeyPos[i].nFrame ;
		if(ftemp < 0)
			break ;

		if((ftemp > 0) && (ftemp < fFrameGap))
		{
			fFrameGap = ftemp ;
			nNode = i ;
		}
	}

    if(nNode == -1)
		return false ;

	*ppKeyPos1 = &m_psKeyPos[nNode] ;
	*ppKeyPos2 = &m_psKeyPos[nNode+1] ;
	return true ;
}
bool CAniTrack::_FindKeyRot(float fFrame, SKeyRot **ppKeyRot1, SKeyRot **ppKeyRot2)
{
	if(m_nNumKeyRot == 0)
		return false ;

	if(fFrame < 0)
	{
		*ppKeyRot1 = *ppKeyRot2 = &m_psKeyRot[0] ;
		return true ;
	}
	if(fFrame > (float)m_psKeyRot[m_nNumKeyRot-1].nFrame)
	{
		*ppKeyRot1 = *ppKeyRot2 = &m_psKeyRot[m_nNumKeyRot-1] ;
		return true ;
	}

	int i ;
	int nNode=-1 ;
	float fFrameGap, ftemp ;

	fFrameGap = (float)m_psKeyRot[m_nNumKeyRot-1].nFrame ;
    for(i=0 ; i<m_nNumKeyRot ; i++)
	{
		if(float_eq((float)m_psKeyRot[i].nFrame, fFrame))
		{
			*ppKeyRot1 = *ppKeyRot2 = &m_psKeyRot[i] ;
			return true ;
		}

		//현재프레임보다 이전프레임중에서 현재프레임과 가장 가까운 프레임을 얻는다.
		ftemp = fFrame-(float)m_psKeyRot[i].nFrame ;
		if(ftemp < 0)
			break ;

		if((ftemp > 0) && (ftemp < fFrameGap))
		{
			fFrameGap = ftemp ;
			nNode = i ;
		}
	}

    if(nNode == -1)
		return false ;

	*ppKeyRot1 = &m_psKeyRot[nNode] ;
	*ppKeyRot2 = &m_psKeyRot[nNode+1] ;
	return true ;
}
bool CAniTrack::_FindKeyScale(float fFrame, SKeyScale **ppKeyScale1, SKeyScale **ppKeyScale2)
{
	if(m_nNumKeyScale == 0)
		return false ;

	if(fFrame < 0)
	{
		*ppKeyScale1 = *ppKeyScale2 = &m_psKeyScale[0] ;
		return true ;
	}
	if(fFrame > (float)m_psKeyScale[m_nNumKeyScale-1].nFrame)
	{
		*ppKeyScale1 = *ppKeyScale2 = &m_psKeyScale[m_nNumKeyScale-1] ;
		return true ;
	}

	int i ;
	int nNode=-1 ;
	float fFrameGap, ftemp ;

	fFrameGap = (float)m_psKeyScale[m_nNumKeyScale-1].nFrame ;
    for(i=0 ; i<m_nNumKeyScale ; i++)
	{
		if(float_eq((float)m_psKeyScale[i].nFrame, fFrame))
		{
			*ppKeyScale1 = *ppKeyScale2 = &m_psKeyScale[i] ;
			return true ;
		}

		//현재프레임보다 이전프레임중에서 현재프레임과 가장 가까운 프레임을 얻는다.
		ftemp = fFrame-(float)m_psKeyScale[i].nFrame ;
		if(ftemp < 0)
			break ;

		if((ftemp > 0) && (ftemp < fFrameGap))
		{
			fFrameGap = ftemp ;
			nNode = i ;
		}
	}

    if(nNode == -1)
		return false ;

	*ppKeyScale1 = &m_psKeyScale[nNode] ;
	*ppKeyScale2 = &m_psKeyScale[nNode+1] ;
	return true ;
}

float CAniTrack::_GetFrameAlpha(float fFrame, float fFrame1, float fFrame2)
{
	assert(!float_eq((fFrame2-fFrame1), 0.0f) && "Cannot Division in CAniTrack::_GetFrameAlpha") ;
    return (fFrame-fFrame1)/(fFrame2-fFrame1) ;
}

bool CAniTrack::_GetKeyPos(float fFrame, D3DXVECTOR3 *pv)
{
	SKeyPos *psKeyPos1=NULL, *psKeyPos2=NULL ;

	if(!_FindKeyPos(fFrame, &psKeyPos1, &psKeyPos2))
		return false ;

	if(psKeyPos1->nFrame == psKeyPos2->nFrame)
	{
		*pv = psKeyPos1->vPos ;
		return true ;
	}

	//Linear interpolation
	float fFrameAlpha = _GetFrameAlpha(fFrame, (float)psKeyPos1->nFrame, (float)psKeyPos2->nFrame) ;
	D3DXVec3Lerp(pv, &psKeyPos1->vPos, &psKeyPos2->vPos, fFrameAlpha) ;
	return true ;
}

bool CAniTrack::_GetKeyRot(float fFrame, D3DXQUATERNION *pquat)
{
	SKeyRot *psKeyRot1=NULL, *psKeyRot2=NULL ;

	if(!_FindKeyRot(fFrame, &psKeyRot1, &psKeyRot2))
		return false ;

	if(psKeyRot1->nFrame == psKeyRot2->nFrame)
	{
		*pquat = psKeyRot1->quat ;
		return true ;
	}

	//Quaternion interpolation
	float fFrameAlpha = _GetFrameAlpha(fFrame, (float)psKeyRot1->nFrame, (float)psKeyRot2->nFrame) ;
	D3DXQuaternionSlerp(pquat, &psKeyRot1->quat, &psKeyRot2->quat, fFrameAlpha) ;
    return true ;
}

bool CAniTrack::_GetKeyScale(float fFrame, D3DXVECTOR3 *pv)
{
	SKeyScale *psKeyScale1=NULL, *psKeyScale2=NULL ;

	if(!_FindKeyScale(fFrame, &psKeyScale1, &psKeyScale2))
        return false ;

	if(psKeyScale1->nFrame == psKeyScale2->nFrame)
	{
		*pv = psKeyScale1->vScale ;
		return true ;
	}

	//Linear interpolation
	float fFrameAlpha = _GetFrameAlpha(fFrame, (float)psKeyScale1->nFrame, (float)psKeyScale2->nFrame) ;
	D3DXVec3Lerp(pv, &psKeyScale1->vScale, &psKeyScale2->vScale, fFrameAlpha) ;
	return true ;
}

#define ANIMATION_TRACK_ROT 0x01
#define ANIMATION_TRACK_POS 0x02
#define ANIMATION_TRACK_SCALE 0x04

D3DXMATRIX *CAniTrack::Animation(float fFrame, CSecretMesh *pcMesh)
{
	D3DXMatrixIdentity(&m_matAni) ;

    DWORD dwAnimationFlag=0 ;
	D3DXVECTOR3 vPos, vScale ;
	D3DXVECTOR3 v ;
	D3DXQUATERNION q ;

	//애니메이션 행렬계산
	//matScale * matRotation * matPosition 순서에 입각하여 진행한다.

	if(_GetKeyRot(fFrame, &q))//회전애니가 있으면 그대로 쓰고
	{
		D3DXMatrixRotationQuaternion(&m_matAni, &q) ;
		dwAnimationFlag |= ANIMATION_TRACK_ROT ;
	}
	else//회전애니가 없을경우
	{
		if(pcMesh->m_bParent)//자식행렬*부모의역행렬을 그대로 사용
			m_matAni = (*pcMesh->GetmatChild()) ;
		else//부모가 없을경우 자신의 로칼행렬사용
			m_matAni = (*pcMesh->GetmatLocal()) ;
	}

	if(_GetKeyPos(fFrame, &v))//위치애니가 있으면 그대로 사용
	{
		vPos = v ;
		dwAnimationFlag |= ANIMATION_TRACK_POS ;
	}
	else// 위치애니가 없을경우
	{
		if(pcMesh->m_bParent)//자식행렬*부모의역행렬에서 위치값만 사용
		{
			vPos.x = pcMesh->GetmatChild()->_41 ;
			vPos.y = pcMesh->GetmatChild()->_42 ;
			vPos.z = pcMesh->GetmatChild()->_43 ;
		}
		else//부모가 없을경우 로칼행렬위치값 사용
		{
			vPos.x = pcMesh->GetmatLocal()->_41 ;
			vPos.y = pcMesh->GetmatLocal()->_42 ;
			vPos.z = pcMesh->GetmatLocal()->_43 ;
		}
	}
	if(_GetKeyScale(fFrame, &v))//크기변환행렬이 있을경우 회전행렬에다가 곱한다.
	{
		D3DXMATRIX mat ;
		D3DXMatrixScaling(&mat, v.x, v.y, v.z) ;
		if(dwAnimationFlag & ANIMATION_TRACK_ROT)
			D3DXMatrixMultiply(&m_matAni, &mat, &m_matAni) ;//여기서 변환순서에 주의해서 matScale * matRotation 로 한다.

		dwAnimationFlag |= ANIMATION_TRACK_SCALE ;
	}

	//최종적으로 위치값을 구한다.
	m_matAni._41 = vPos.x ;
	m_matAni._42 = vPos.y ;
	m_matAni._43 = vPos.z ;

//	D3DXMatrixIdentity(&m_matAni) ;

	return &m_matAni ;
}

void CAniTrack::Release()
{
	if(m_psKeyPos)
	{
		delete []m_psKeyPos ;
		m_psKeyPos = NULL ;
	}
	if(m_psKeyRot)
	{
		delete []m_psKeyRot ;
		m_psKeyRot = NULL ;
	}
	if(m_psKeyScale)
	{
		delete []m_psKeyScale ;
		m_psKeyScale = NULL ;
	}
}