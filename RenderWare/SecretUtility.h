#pragma once

#include "D3Ddef.h"
#include "Vector3.h"

struct SNormalRGB
{
	BYTE b, g, r ;

	SNormalRGB()
	{
		r = g = b = 0 ;
	}
	SNormalRGB(BYTE _r, BYTE _g, BYTE _b)
	{
		set(_r, _g, _b) ;
	}
	void set(BYTE _r, BYTE _g, BYTE _b)
	{
		r = _r ;
		g = _g ;
		b = _b ;
	}

} ;

class CTga ;

void GetNormalandHeight(CTga *ptga, unsigned long x, unsigned long y, Vector3 &vNormal, float &fHeight) ;
Vector3 GetNormal(CTga *ptga, unsigned long x, unsigned long y) ;
float GetHeight(CTga *ptga, unsigned long x, unsigned long y) ;

bool CreateNormalBumpData(char *pszFileName) ;

SNormalRGB GetNormalRGB(CTga *ptga, unsigned long x, unsigned long y) ;
bool CreateNormalMap(char *pszFileName) ;