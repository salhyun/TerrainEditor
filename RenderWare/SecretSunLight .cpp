#include "SecretSunLight.h"

#define TIME_CYCLE two_pi
#define TIME_NOON (TIME_CYCLE*0.0f)
#define TIME_DAWN (TIME_CYCLE*0.30f)
#define TIME_DUSK (TIME_CYCLE*0.70f)
#define TIME_MIDNIGHT 0.5f

CSecretSunLight::CSecretSunLight()
{
	Reset() ;
}

CSecretSunLight::~CSecretSunLight()
{
}

void CSecretSunLight::Reset()
{
	m_fTimeOfDay = 0.5f ;
	m_fTimeStep = 0.001f ;
	m_fSunIntensity = 1.0f ;
	m_fTurbidity = 1.0f ;
	m_vSunColor = D3DXVECTOR4(1.0f, 0.8f, 0.8f, 1.0f) ;
	m_vSunDir = D3DXVECTOR4(0.0f, 0.0f, 1.0f, 0.0f) ;
	m_vAmbientHue = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f) ;

    _ComputeSunLightColor(TIME_DAWN, 2, m_vDawnSunColor) ;
	_ComputeSunLightColor(TIME_NOON, 2, m_vNoonSunColor) ;
	
	m_vDawnSunColor.w = 100.0f ;
	m_vNoonSunColor.w = 100.0f ;

	m_vDeltaSunColor = m_vNoonSunColor-m_vDawnSunColor ;
	SetTimeOfDay(TIME_NOON) ;
}

void CSecretSunLight::SetTimeOfDay(float fTime)
{
    m_fTimeOfDay = fTime ;
	m_fTimeOfDay = fmodf(m_fTimeOfDay, TIME_CYCLE) ;
}

void CSecretSunLight::SetTurbidity(float fTurbidity)
{
	m_fTurbidity = fTurbidity ;
}

void CSecretSunLight::AddTimeOfDay(float add)
{
	m_fTimeOfDay += add ;
}

void CSecretSunLight::AddTurbidity(float add)
{
	m_fTurbidity += add ;
}

void CSecretSunLight::Process()
{
	_recalculateSunlightParameters() ;
}

void CSecretSunLight::_recalculateSunlightParameters()
{
	// the sun rotates on the world Y axis
	m_vSunDir.x = 0 ;
	m_vSunDir.y = cosf(m_fTimeOfDay) ;
	m_vSunDir.z = sinf(m_fTimeOfDay) ;
	m_vSunDir.w = 0 ;


	// we approximate the color of the sun by 
	// interpolating between the noon sun
	// (bright yellow) and a sunrise\sunset
	// reddish sun.

	// ramp up to the noon sun color
	// as the sun reaches the highest
	// point in the sky
	//float zenithFactor 
	//	= clamp(m_sunVector.z, 0.0f, 1.0f);
	float zenithFactor = clamp(m_vSunDir.y, 0.0f, 1.0f);


	m_vSunColor = m_vDawnSunColor+(m_vDeltaSunColor*zenithFactor);


	float fadeOut = 1.0f;

	if (m_vSunColor.y < 0.0f)
	{
		fadeOut = -m_vSunColor.y* 5.0f;
		fadeOut = clamp(1.0f-fadeOut, 0.0f, 1.0f);
	}

	m_vSunColor.w = m_fSunIntensity * 100.0f * fadeOut;

	m_vAmbientHue.x = zenithFactor*0.3f ;
	m_vAmbientHue.y = zenithFactor*0.2f ;
	m_vAmbientHue.z = 0.25f ;
	m_vAmbientHue.w = 0.0f ;
}

void CSecretSunLight::_ComputeSunLightColor(
	float timeOfDay, // 0-2pi
	float turbidity, // must be >= 2
	D3DXVECTOR4& output)
{
	// compute theta as the angle from
	// the noon (zenith) position in
	// radians
	float theta = (timeOfDay<pi) ? timeOfDay:two_pi-timeOfDay ;

	// angles greater than the horizon are clamped
	theta = clamp(theta, 0.0f, half_pi);

	// beta is a measure of aerosols.
	float beta = 
		0.04608365822050f * 
		turbidity - 0.04586025928522f;
    float opticalMass = 
		-1.0f/((float)cos(theta) 
		+ 0.15f
		*(float)pow(93.885f-theta/pi*180.0f,-1.253f));

	// constants for lambda
	// provided by Preetham et al,
	Vector3 lambda(0.65f, 0.57f,0.475f);

	// compute each color channel
	Vector3 v ;
    float tauR, tauA;

	// Rayleigh Scattering
		tauR = (float)exp(opticalMass 
				* 0.008735f 
				* (float)pow(lambda.x, 
					-4.08f));

		// Aerosal (water + dust) attenuation
		// paticle size ratio set at (1.3) 
		tauA = (float)exp(opticalMass 
				* beta 
				* (float)pow(lambda.x, 
					-1.3f)); 

		v.x = tauR * tauA; 

		// Rayleigh Scattering
		tauR = (float)exp(opticalMass 
				* 0.008735f 
				* (float)pow(lambda.y, 
					-4.08f));

		// Aerosal (water + dust) attenuation
		// paticle size ratio set at (1.3) 
		tauA = (float)exp(opticalMass 
				* beta 
				* (float)pow(lambda.y, 
					-1.3f)); 

		v.y = tauR * tauA; 

		// Rayleigh Scattering
		tauR = (float)exp(opticalMass 
				* 0.008735f 
				* (float)pow(lambda.z, 
					-4.08f));

		// Aerosal (water + dust) attenuation
		// paticle size ratio set at (1.3) 
		tauA = (float)exp(opticalMass 
				* beta 
				* (float)pow(lambda.z, 
					-1.3f)); 

		v.z = tauR * tauA; 

		v = v.Normalize() ;

		output.x = v.x ;
		output.y = v.y ;
		output.z = v.z ;
}

D3DXVECTOR4 CSecretSunLight::GetSunColor()
{
	return m_vSunColor ;
} ;
D3DXVECTOR4 CSecretSunLight::GetSunDir()
{
	return m_vSunDir ;
} ;