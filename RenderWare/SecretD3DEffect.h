#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#include "../RenderWare/D3DEnvironment.h"

enum D3DEFFECTKIND
{
    EFFECTKIND_TERRAIN=1,
	EFFECTKIND_ATOMSPHERE,
	EFFECTKIND_CLOUD,
	EFFECTKIND_SUNLIT_OBJECT,
} ;


struct SEffectEssentialElements//���̴� ���� �ʼ� ���
{
	D3DXMATRIX matWorld, matView, matProj, matClipProj ;//����, ��, ���� ���
	D3DXVECTOR4 vDiffuse ;
	D3DXVECTOR4 vSunDir ;//�������⺤��
	D3DXVECTOR4 vSunColor ;//��������
	D3DXVECTOR4 vCameraPos ;//ī�޶� ��ġ����
	D3DXVECTOR4 vLookAt ;//ī�޶� ���� ���⺤��
	
	SEffectEssentialElements() ;
} ;

class CSecretD3DEffect
{
public :
    int m_nEffectKind ;
	LPDIRECT3DVERTEXDECLARATION9 m_pDecl ;//��������
	SEffectEssentialElements *m_psEssentialElements ;

    LPD3DXEFFECT m_pEffect ;//���̴�
	D3DXHANDLE m_hTechnique ;//��ũ��
	D3DXHANDLE m_hmatWVP ;//����-��-���� ��ȯ�� ���� ���
	D3DXHANDLE m_hvDiffuse ;
	D3DXHANDLE m_hvSunDir ;//�¾��� ����
	D3DXHANDLE m_hvSunColor ;
	D3DXHANDLE m_hvLookAt ;//�ü�����
	D3DXHANDLE m_hvCameraPos ;//ī�޶���ġ

public :
	CSecretD3DEffect() ;
	virtual ~CSecretD3DEffect() ;

	virtual HRESULT Initialze(LPDIRECT3DDEVICE9 pd3dDevice, SEffectEssentialElements *psEssentialElements, char *pszFileName) ;
	void Release() ;
	bool SetD3DXHANDLE(D3DXHANDLE *pHandle, char *pszName) ;
} ;