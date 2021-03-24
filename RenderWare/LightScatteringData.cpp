/***************************************************************\

    ^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^
    ^^    ^^  ^^    ^^     ^^     ^^    ^^
    ^^        ^^^^^^^^     ^^     ^^^^^^^^
    ^^   ^^^  ^^    ^^     ^^     ^^    ^^
    ^^    ^^  ^^    ^^     ^^     ^^    ^^
    ^^^^^^^^  ^^    ^^  ^^^^^^^^  ^^    ^^

    sample code from the book...
    Real Time 3D Terrain Engines Using C++ and DirectX

    by Greg Snook
    greg@mightystudios.com

\***************************************************************/

#include "LightScatteringData.h"
#include "Vector4.h"
#include "def.h"

//
// light scattering constants
//

// Rayleigh scattering coefficient
static const Vector4 vRayleighBeta(
	0.000697153f,
	0.00117891f,
	0.00244460f, 1.0f); 

// Rayleigh Angular scattering 
// coefficient without phase term.
static const Vector4 vRayleighAngularBeta(
	4.16082e-005f,
	7.03612e-005f,
	0.000145901f, 1.0f);

//Mie scattering coefficient
static const Vector4 vMieBeta(
	0.00574060f,
	0.00739969f,
	0.0105143f, 1.0f);

//Mie Angular scattering
//coefficient without phase term.
static const Vector4 vMieAngularBeta(
	0.00133379f,
	0.00173466f,
	0.00249762f, 1.0f);

// Reflective spectrum data for soil.
// As calculated by Hoffman, Mitchell 
// and Preetham
static const Vector4 vSoilReflectivity(
	0.138f,
	0.113f, 
	0.08f, 1.0f);


/*	cLightScatteringData constructor
-----------------------------------------------------------------
    
    The scalar values provided allow for real-time
	changes to the atmosphere. As with Hoffman,
	Mitchell and Preetham's original work, we
	provide these controls for experimentation.
    
-----------------------------------------------------------------
*/
cLightScatteringData::cLightScatteringData()
:m_henyeyG(0.98f)
,m_rayleighBetaMultiplier(0.6f)
,m_mieBetaMultiplier(0.01f)
,m_inscatteringMultiplier(1.57f)
,m_extinctionMultiplier(1.33f)
,m_reflectivePower(0.1f)
{
	recalculateShaderData();
}


void cLightScatteringData::recalculateShaderData()
{
	float InvLog2 = 1.0f/(float)log(2.0f);
	//
	// rebuild the structure used by vertex shaders
	// to perform light scattering
	//

	m_shaderParams.vBeta1 = 
		vRayleighBeta * m_rayleighBetaMultiplier;

	TRACE("vBeta1(%g, %g, %g, %g)\r\n",
		m_shaderParams.vBeta1.x, m_shaderParams.vBeta1.y, m_shaderParams.vBeta1.z, m_shaderParams.vBeta1.w) ;

	m_shaderParams.vBeta2 = 
		vMieBeta * m_mieBetaMultiplier;

	TRACE("vBeta1(%g, %g, %g, %g)\r\n",
		m_shaderParams.vBeta2.x, m_shaderParams.vBeta2.y, m_shaderParams.vBeta2.z, m_shaderParams.vBeta2.w) ;

	m_shaderParams.vBetaD1 = 
		vRayleighAngularBeta * m_rayleighBetaMultiplier;

	TRACE("vBetaD1(%g, %g, %g, %g)\r\n",
		m_shaderParams.vBetaD1.x, m_shaderParams.vBetaD1.y, m_shaderParams.vBetaD1.z, m_shaderParams.vBetaD1.w) ;

	m_shaderParams.vBetaD2 = 
		vMieAngularBeta * m_mieBetaMultiplier;

	TRACE("vBetaD2(%g, %g, %g, %g)\r\n",
		m_shaderParams.vBetaD2.x, m_shaderParams.vBetaD2.y, m_shaderParams.vBetaD2.z, m_shaderParams.vBetaD2.w) ;

	m_shaderParams.vSumBeta1Beta2 = 
		m_shaderParams.vBeta1 +
		m_shaderParams.vBeta2;

	TRACE("vSumBeta1Beta2(%g, %g, %g, %g)\r\n",
		m_shaderParams.vSumBeta1Beta2.x, m_shaderParams.vSumBeta1Beta2.y, m_shaderParams.vSumBeta1Beta2.z, m_shaderParams.vSumBeta1Beta2.w) ;

	m_shaderParams.vLog2eBetaSum = 
		m_shaderParams.vSumBeta1Beta2 *
		InvLog2;

	m_shaderParams.vRcpSumBeta1Beta2.set(
		1.0f / m_shaderParams.vSumBeta1Beta2.x,
		1.0f / m_shaderParams.vSumBeta1Beta2.y,
		1.0f / m_shaderParams.vSumBeta1Beta2.z,
		0.0f);

	TRACE("vRcpSumBeta1Beta2(%g, %g, %g, %g)\r\n",
		m_shaderParams.vRcpSumBeta1Beta2.x, m_shaderParams.vRcpSumBeta1Beta2.y, m_shaderParams.vRcpSumBeta1Beta2.z, m_shaderParams.vRcpSumBeta1Beta2.w) ;

	// precalculate values used by the shader
	// for Henyey Greenstein approximation
	m_shaderParams.vHG.set(
		1.0f-m_henyeyG*m_henyeyG, 
		1.0f + m_henyeyG, 
		2.0f * m_henyeyG,
		1.0f);

	TRACE("vHG(%g, %g, %g, %g)\r\n",
		m_shaderParams.vHG.x, m_shaderParams.vHG.y, m_shaderParams.vHG.z, m_shaderParams.vHG.w) ;

	// set some basic constants used by the shader
	m_shaderParams.vConstants.set(
		1.0f,
		InvLog2,
		0.5f,
		0.0f);

	TRACE("vConstants(%g, %g, %g, %g)\r\n",
		m_shaderParams.vConstants.x, m_shaderParams.vConstants.y, m_shaderParams.vConstants.z, m_shaderParams.vConstants.w) ;

	// set the inscatter and extinction terms
	m_shaderParams.vTermMultipliers.set(
		m_inscatteringMultiplier,
		m_extinctionMultiplier,
		2.0f, // another constant
		0.0f);

    TRACE("vTermMultipliers(%g, %g, %g, %g)\r\n",
		m_shaderParams.vTermMultipliers.x, m_shaderParams.vTermMultipliers.y, m_shaderParams.vTermMultipliers.z, m_shaderParams.vTermMultipliers.w) ;	

	// set the reflective power of soil
	m_shaderParams.vSoilReflectivity = 
		vSoilReflectivity * m_reflectivePower;

	TRACE("vSoilReflectivity(%g, %g, %g, %g)\r\n",
		m_shaderParams.vSoilReflectivity.x, m_shaderParams.vSoilReflectivity.y, m_shaderParams.vSoilReflectivity.z, m_shaderParams.vSoilReflectivity.w) ;
}


void cLightScatteringData::setHenyeyG(float g)
{
	m_henyeyG = g;
	recalculateShaderData();
}

void cLightScatteringData::setRayleighScale(float s)
{
	m_rayleighBetaMultiplier = s;
	recalculateShaderData();
}

void cLightScatteringData::setMieScale(float s)
{
	m_mieBetaMultiplier = s;
	recalculateShaderData();
}

void cLightScatteringData::setInscatteringScale(float s)
{
	m_inscatteringMultiplier = s;
	recalculateShaderData();
}

void cLightScatteringData::setExtinctionScale(float s)
{
	m_extinctionMultiplier = s;
	recalculateShaderData();
}

void cLightScatteringData::setTerrainReflectionScale(float s)
{
	m_reflectivePower = s;
	recalculateShaderData();
}

void atmosphericLighting(Vector3 eyeVector, Vector3 sunVector, Vector3 norm, Vector4 sunColor, float s, Vector4 &vExt, Vector4 &vIns, sLightScatteringShaderParams atm)
{
	//
	// This shader assumes a world-space vertex is 
	// provided, and distance values represent 
	// real-world distances. A set of pre-
	// calculated atmospheric data is provided
	// in the atm structure.
	//

	Vector4 vConstants ;

	// compute cosine of theta angle
	//float cosTheta = dot(eyeVector, sunVector);
	float cosTheta = eyeVector.dot(sunVector) ;
	cosTheta = 0.7853981f ;

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e

	//temp = -(beta1+beta2)*s ;

	vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	vExt.x = exp(vExt.x);
	vExt.y = exp(vExt.y);
	vExt.z = exp(vExt.z);
	vExt.w = 0.0f ;

	// Compute theta terms used by inscattering.
	// compute phase2 theta as
    // (1-g^2)/(1+g-2g*cos(theta))^(3/2)
	// atm.vHG = [1-g^2, 1+g, 2g]
	float p1Theta = 1.0f+(cosTheta*cosTheta);
	float p2Theta = (atm.vHG.z*cosTheta)+atm.vHG.y;
	//TRACE("p1Theta=%g\r\n", p1Theta) ;
	//TRACE("p2Theta=%g\r\n", p2Theta) ;
	p2Theta = 1.0f/(sqrt(p2Theta));
	//TRACE("p2Theta=%g\r\n", p2Theta) ;
	p2Theta = (p2Theta*p2Theta*p2Theta) * atm.vHG.x;
	//TRACE("p2Theta=%g\r\n", p2Theta) ;

	// compute inscattering (vIns) as
	// (vBetaD1*p1Theta + vBetaD1*p2Theta) *
	// (1-vExt) * atm.vRcpSumBeta1Beta2
	// 
	// atm.vRcpSumBeta1Beta2 = 
	// 1.0f/ (Rayleigh+Mie)
	vConstants.set(1, 1, 1, 1) ;

	//vIns = ((atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta))*(atm.vConstants.x-vExt)*atm.vRcpSumBeta1Beta2;
	vIns = ((atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta))*(vConstants-vExt)*atm.vRcpSumBeta1Beta2;

	// scale inscatter and extinction 
	// for effect (optional)
	vIns = vIns*atm.vTermMultipliers.x;
	// scale extinction prior to its use
	// (this is optional)
	vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;

	// reduce inscattering on unlit surfaces
	// by modulating with a monochrome
	// Lambertian scalar. This is slightly
	// offset to allow some inscattering to
	// bleed into unlit areas
	//float NdL = dot(norm, sunVector);
	float NdL = norm.dot(sunVector) ;
	vIns= vIns*NdL;

	// apply sunlight color
	// and strength to each term
	// and output
	//vIns.xyz = vIns*sunColor*sunColor.w;
	vIns.x = vIns.x*sunColor.x*sunColor.w ;
	vIns.y = vIns.y*sunColor.y*sunColor.w ;
	vIns.z = vIns.z*sunColor.z*sunColor.w ;
	vIns.w = 0.0f;

	//vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.x = vExt.x*sunColor.x*sunColor.w ;
	vExt.y = vExt.y*sunColor.y*sunColor.w ;
	vExt.z = vExt.z*sunColor.z*sunColor.w ;
	vExt.w = 1.0f;
}

/*
void atmosphericLighting(Vector3 eyeVector, Vector3 sunVector, Vector3 norm, Vector4 sunColor, float s, Vector4 &vExt, Vector4 &vIns, sLightScatteringShaderParams atm)
{
	//
	// This shader assumes a world-space vertex is 
	// provided, and distance values represent 
	// real-world distances. A set of pre-
	// calculated atmospheric data is provided
	// in the atm structure.
	//

	Vector4 vConstants ;
	
	// compute cosine of theta angle
	//float cosTheta = dot(eyeVector, sunVector);
	float cosTheta = eyeVector.dot(sunVector) ;

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e
	vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	vExt.x = exp(vExt.x);
	vExt.y = exp(vExt.y);
	vExt.z = exp(vExt.z);
	vExt.w = 0.0f;

	// Compute theta terms used by inscattering.
	// compute phase2 theta as
    // (1-g^2)/(1+g-2g*cos(theta))^(3/2)
	// atm.vHG = [1-g^2, 1+g, 2g]
	float p1Theta = (cosTheta*cosTheta)+atm.vConstants.x;
	float p2Theta = (atm.vHG.z*cosTheta)+atm.vHG.y;
	TRACE("p1Theta=%g\r\n", p1Theta) ;
	TRACE("p2Theta=%g\r\n", p2Theta) ;
	p2Theta = 1.0f/(sqrt(p2Theta));
	TRACE("p2Theta=%g\r\n", p2Theta) ;
	p2Theta = (p2Theta*p2Theta*p2Theta) * atm.vHG.x;
	TRACE("p2Theta=%g\r\n", p2Theta) ;

	// compute inscattering (vIns) as
	// (vBetaD1*p1Theta + vBetaD1*p2Theta) *
	// (1-vExt) * atm.vRcpSumBeta1Beta2
	// 
	// atm.vRcpSumBeta1Beta2 = 
	// 1.0f/ (Rayleigh+Mie)
	vConstants.set(atm.vConstants.x, atm.vConstants.x, atm.vConstants.x, atm.vConstants.x) ;

	//vIns = ((atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta))*(atm.vConstants.x-vExt)*atm.vRcpSumBeta1Beta2;

	Vector4 numerator, denominator ;

	numerator = ((atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta)) ;
	numerator = numerator*(vConstants-vExt) ;

	denominator = atm.vSumBeta1Beta2 ;

	vIns = ((atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta))*(vConstants-vExt)*atm.vRcpSumBeta1Beta2;

	// scale inscatter and extinction 
	// for effect (optional)
	vIns = vIns*atm.vTermMultipliers.x;
	// scale extinction prior to its use
	// (this is optional)
	vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;

	// reduce inscattering on unlit surfaces
	// by modulating with a monochrome
	// Lambertian scalar. This is slightly
	// offset to allow some inscattering to
	// bleed into unlit areas
	//float NdL = dot(norm, sunVector);
	float NdL = norm.dot(sunVector) ;
	vIns= vIns*NdL;

	// apply sunlight color
	// and strength to each term
	// and output
	//vIns.xyz = vIns*sunColor*sunColor.w;
	vIns.x = vIns.x*sunColor.x*sunColor.w ;
	vIns.y = vIns.y*sunColor.y*sunColor.w ;
	vIns.z = vIns.z*sunColor.z*sunColor.w ;
	vIns.w = 0.0f;

	//vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.x = vExt.x*sunColor.x*sunColor.w ;
	vExt.y = vExt.y*sunColor.y*sunColor.w ;
	vExt.z = vExt.z*sunColor.z*sunColor.w ;
	vExt.w = 1.0f;
}
*/

//***************************************************************
// end of file      ( cOutdoorLighting.cpp )

//----------------------------------------------------------
//$Log: $