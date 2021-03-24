#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "PerlinNoise.h"

bool IsPrimeNumber(long i) ;

long lRandomA[12] = {15731, 15733, 15737, 15739, 15749, 15761, 15767, 15773, 15787, 15791, 15797, 15803} ;
long lRandomB[12] = {789221, 789227, 789251, 789311, 789323, 789331, 789343, 789367, 789377, 789389, 789391, 789407} ;
long lRandomC[12] = {1376312627, 1376312629, 1376312657, 1376312687, 1376312689, 1376312753, 1376312783, 1376312789, 1376312813, 1376312857, 1376312879, 1376312881} ;
float fRandomD[12] = {1073741827.0f, 1073741831.0f, 1073741833.0f, 1073741839.0f, 1073741843.0f, 1073741857.0f, 1073741891.0f, 1073741909.0f, 1073741939.0f, 1073741953.0f, 1073741969.0f, 1073741971.0f} ;

CPerlinNoise::CPerlinNoise()
{
	m_nKind = 0 ;
	m_fPersistence = 1.0f/4.0f ;
	m_nNumOctaves = 6 ;

	m_bEnableRange = false ;
	m_fRangeMin = m_fRangeMax = m_fRangeLength = 0 ;
}
CPerlinNoise::~CPerlinNoise()
{
}
CPerlinNoise::CPerlinNoise(float fPersistence, int nNumOctaves, int nResolution)
{
	Initialize(fPersistence, nNumOctaves, nResolution) ;
}
void CPerlinNoise::InitRandomNumber()
{
	srand((unsigned)time(NULL)) ;

	m_lRandomA = lRandomA[rand()%12] ;
	m_lRandomB = lRandomB[rand()%12] ;
	m_lRandomC = lRandomC[rand()%12] ;
	m_fRandomD = fRandomD[rand()%12] ;

}
float CPerlinNoise::_RandomNumber(int num)
{
	num = (num<<13)^num ;
	return (float)( 1.0 - ( (num * (num * num * m_lRandomA + m_lRandomB) + m_lRandomC) & 0x7fffffff) / m_fRandomD) ;
    //return (float)( 1.0 - ( (num * (num * num * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) ;
}
float CPerlinNoise::_CosineInterpolate(float a, float b, float x)
{
	float t = x*3.1415927f ;
	float f = (1.0f-cosf(t))*0.5f ;
	return a*(1.0f-f)+b*f ;
}
//////////////////////
// 1차원
//////////////////////
float CPerlinNoise::_GenerateNoise1D(int x)
{
    return _RandomNumber(x) ;
}
float CPerlinNoise::_SmoothedNoise1D(float x)
{
	int nx = (int)x ;
	return (_GenerateNoise1D(nx-1)/4) + (_GenerateNoise1D(nx)/2) + (_GenerateNoise1D(nx+1)/4) ;
}
float CPerlinNoise::_InterpolatedNoise1D(float x)
{
	int nx = (int)x ;
	float fractional = x - (float)nx ;

	float v1 = _SmoothedNoise1D((float)nx) ;
	float v2 = _SmoothedNoise1D((float)(nx+1)) ;
	return _CosineInterpolate(v1, v2, fractional) ;
}
void CPerlinNoise::Initialize(float fPersistence, int nNumOctaves, int nResolution)
{
	//int i, i1, i2, c=0 ;
	//i1 = 1073741824 ;
	//i2 = i1+1000 ;
	//for(i=i1; i<i2; i++)
	//{
	//	if(IsPrimeNumber(i))
	//	{
	//		TRACE("%d.0f, ", i) ;
	//		if(++c >= 12)
	//			break ;
	//	}
	//}
	//TRACE("\r\n") ;

    m_fPersistence = fPersistence ;
	m_nNumOctaves = nNumOctaves ;
	m_nResolution = nResolution ;

	InitRandomNumber() ;
}
float CPerlinNoise::PerlinNoise1D(float x)
{
	int i ;
	float total=0, frequency, amplitude ;

	for(i=0 ; i<(m_nNumOctaves-1) ; i++)
	{
		frequency = powf(2.0f, (float)i) ;
		amplitude = powf(m_fPersistence, (float)i) ;
		total += _InterpolatedNoise1D(x*frequency)*amplitude ;
	}

	if(m_bEnableRange)
	{
		float temp = total + 1.0f ;
		total = m_fRangeMin+(temp*(m_fRangeLength/2.0f)) ;
	}

    return total ;
}
//////////////////////
// 2차원
//////////////////////
float CPerlinNoise::_GenerateNoise2D(int x, int y)
{
	return _RandomNumber(x+y*m_nResolution) ;
}
float CPerlinNoise::_SmoothedNoise2D(int x, int y)
{
	float corners = (_GenerateNoise2D(x-1, y-1)+_GenerateNoise2D(x+1, y-1)+_GenerateNoise2D(x-1, y+1)+_GenerateNoise2D(x+1, y+1))/16.0f ;
	float sides = (_GenerateNoise2D(x-1, y)+_GenerateNoise2D(x+1, y)+_GenerateNoise2D(x, y-1)+_GenerateNoise2D(x, y+1))/8.0f ;
	float center = _GenerateNoise2D(x, y)/4.0f ;
	return corners+sides+center ;
}
float CPerlinNoise::_InterpolatedNoise2D(float x, float y)
{
	int nx = (int)x ;
	float fractionalX = x-(float)nx ;

	int ny = (int)y ;
	float fractionalY = y-(float)ny ;

    float v1 = _SmoothedNoise2D(nx, ny) ;
	float v2 = _SmoothedNoise2D(nx+1, ny) ;
	float v3 = _SmoothedNoise2D(nx, ny+1) ;
	float v4 = _SmoothedNoise2D(nx+1, ny+1) ;

	float i1 = _CosineInterpolate(v1, v2, fractionalX) ;
	float i2 = _CosineInterpolate(v3, v4, fractionalX) ;

	return _CosineInterpolate(i1, i2, fractionalY) ;
}
float CPerlinNoise::PerlinNoise2D(float x, float y)
{
	int i ;
	float total=0, frequency, amplitude ;

	for(i=0 ; i<(m_nNumOctaves-1) ; i++)
	{
		frequency = powf(2.0f, (float)i) ;
		amplitude = powf(m_fPersistence, (float)i) ;
		total += _InterpolatedNoise2D(x*frequency, y*frequency)*amplitude ;
	}

	if(m_bEnableRange)
	{
		float temp = total + 1.0f ;
		total = m_fRangeMin+(temp*(m_fRangeLength/2.0f)) ;
	}

    return total ;
}
void CPerlinNoise::SetRange(float a, float b)
{
    m_fRangeMin = a ;
	m_fRangeMax = b ;
	m_fRangeLength = m_fRangeMax-m_fRangeMin ;
	m_bEnableRange = true ;
}
bool IsPrimeNumber(long i)
{
	long si,j;

	si = (long)sqrt((float)i);

	for (j=2; (j<=si); j++)
	{
		if (i%j == 0)
			return false ;
	}
	return true ;
}