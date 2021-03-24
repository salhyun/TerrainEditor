#pragma once

class CPerlinNoise
{
public :
	enum DIMENSION { DIMENSION_ONE=0, DIMENSION_TWO, DIMENSION_THREE } ;

private :
	int m_nKind ;
	float m_fPersistence ;
	int	m_nNumOctaves ;
	int m_nResolution ;
    
	long m_lRandomA, m_lRandomB, m_lRandomC ;
	float m_fRandomD ;

    bool m_bEnableRange ;
	float m_fRangeMin, m_fRangeMax, m_fRangeLength ;

	float _RandomNumber(int num) ;

    float _CosineInterpolate(float a, float b, float x) ;

	float _GenerateNoise1D(int x) ;
	float _SmoothedNoise1D(float x) ;
	float _InterpolatedNoise1D(float x) ;

	float _GenerateNoise2D(int x, int y) ;
	float _SmoothedNoise2D(int x, int y) ;
	float _InterpolatedNoise2D(float x, float y) ;

public :
	CPerlinNoise() ;
	CPerlinNoise(float fPersistence, int nNumOctaves, int nResolution) ;
	~CPerlinNoise() ;

	void Initialize(float fPersistence, int nNumOctaves, int nResolution) ;
	float PerlinNoise1D(float x) ;
	float PerlinNoise2D(float x, float y) ;

	void InitRandomNumber() ;
	void SetRange(float a, float b) ;
} ;