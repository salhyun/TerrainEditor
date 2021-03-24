#pragma once

#include "Vector3.h"
#include "Vector4.h"

struct sLightScatteringShaderParams
{
	Vector4 vBeta1;
	Vector4 vBeta2;
	Vector4 vBetaD1;
	Vector4 vBetaD2;
	Vector4 vSumBeta1Beta2;
	Vector4 vLog2eBetaSum;
	Vector4 vRcpSumBeta1Beta2;
	Vector4 vHG;
	Vector4 vConstants;
	Vector4 vTermMultipliers;
	Vector4 vSoilReflectivity;
};

class cLightScatteringData
{
public:

	// Data Types & Constants...

    cLightScatteringData();
    ~cLightScatteringData(){};


	void setHenyeyG(float g);
	void setRayleighScale(float s);
	void setMieScale(float s);
	void setInscatteringScale(float s);
	void setExtinctionScale(float s);
	void setTerrainReflectionScale(float s);

	float getHenyeyG()const;
	float getRayleighScale()const;
	float getMieScale()const;
	float getInscatteringScale()const;
	float getExtinctionScale()const;
	float getTerrainReflectionScale()const;

	const sLightScatteringShaderParams* getShaderData()const;

	// the current light scattering
	// structure provided to the
	// vertex shaders
	sLightScatteringShaderParams m_shaderParams;

private:

	// 'g' values used in the approximation
	// function designed by Henyey Greenstein
	float m_henyeyG;		

	// scalars to control the size of
	// coefficients used for Rayleigh
	// and Mie light scattering
	float m_rayleighBetaMultiplier;
	float m_mieBetaMultiplier;

	// scalars to control the overall
	// amount of light scattering for
	// both inscattering and
	// extinction
	float m_inscatteringMultiplier;
	float m_extinctionMultiplier;

	// a scalar to adjust the reflective
	// nature of the terrain itself
	float m_reflectivePower;

	// the private function which updates the
	// the internal structure
	void recalculateShaderData();

};




//
// Accessors
//
inline float cLightScatteringData::getHenyeyG()const
{
	return m_henyeyG;
}

inline float cLightScatteringData::getRayleighScale()const
{
	return m_rayleighBetaMultiplier;
}

inline float cLightScatteringData::getMieScale()const
{
	return m_mieBetaMultiplier;
}

inline float cLightScatteringData::getInscatteringScale()const
{
	return m_inscatteringMultiplier;
}

inline float cLightScatteringData::getExtinctionScale()const
{
	return m_extinctionMultiplier;
}

inline float cLightScatteringData::getTerrainReflectionScale()const
{
	return m_reflectivePower;
}

inline const sLightScatteringShaderParams* cLightScatteringData::getShaderData()const
{
	return &m_shaderParams;
}

void atmosphericLighting(Vector3 eyeVector, Vector3 sunVector, Vector3 norm, Vector4 sunColor, float s, Vector4 &vExt, Vector4 &vIns, sLightScatteringShaderParams atm) ;

//- End of cLightScatteringData -------------------------------------
//$Log: $