#pragma once

#include "D3Ddef.h"
#include "GeoLib.h"
#include "Vector3.h"
#include "Vector4.h"

class CSecretSunLight
{
public :

private :
	float m_fTimeOfDay, m_fTimeStep ;
	float m_fSunIntensity, m_fTurbidity ;
	D3DXVECTOR4 m_vSunColor, m_vSunDir, m_vAmbientHue ;
	D3DXVECTOR4 m_vDawnSunColor, m_vNoonSunColor, m_vDeltaSunColor ;

	void _ComputeSunLightColor(float timeOfDay, float turbidity, D3DXVECTOR4& output) ;
	void _recalculateSunlightParameters() ;

public :
	CSecretSunLight() ;
	~CSecretSunLight() ;

	void SetTimeOfDay(float fTime) ;
	void SetTurbidity(float fTrubidity) ;
	void AddTimeOfDay(float add) ;
	void AddTurbidity(float add) ;
	void Process() ;
	void Reset() ;

	D3DXVECTOR4 GetSunColor() ;
	D3DXVECTOR4 GetSunDir() ;

	
} ;