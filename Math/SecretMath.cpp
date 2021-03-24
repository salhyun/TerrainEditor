#include "SecretMath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

float g_fEpsilon = 0.0001f ;

int round_int(float value)
{
    if(float_less(value, 0))
		return (int)(value-0.5f) ;

	return (int)(value+0.5f) ;
}

float float_round(float value, float cipher)
{
    //value *= powf(10.0f, 4.0f) ;
	//value = (float_greater(value, 0.0f)) ? floor(value+0.500001f) : ceil(value-0.500001f) ;
	//value *= powf(10.0f, -4.0f) ;
	//return value ;
	//return (floor((value)*powf(10.0f,cipher)+0.5f)/powf(10.0f,cipher)) ;
	//소수점 3째자리에서 반올림
//    float ret ;
//	( *((unsigned long *)&value) & 0x80000000) ? ret = (floor((value)*powf(10.0f,cipher)+0.5f)/powf(10.0f,cipher))-0.000001f : ret = (floor((value)*powf(10.0f,cipher)+0.5f)/powf(10.0f,cipher))+0.000001f ;
//	return ret ;

	if( *((unsigned long *)&value) & 0x80000000)//minus
		return (floor((value)*powf(10.0f,cipher)+0.5f)/powf(10.0f,cipher))-0.0000001f ;
	return (floor((value)*powf(10.0f,cipher)+0.5f)/powf(10.0f,cipher))+0.0000001f ;
}
float float_abs(float value)
{
	//unsigned long *pl = (unsigned long *)&ret ;
	(*((unsigned long *)&value)) &= (~0x80000000) ;
	return value ;
}
double float_abs(double value)
{
	//unsigned long *pl = (unsigned long *)&ret ;
	(*((unsigned long long *)&value)) &= (~0x8000000000000000) ;
	return value ;
}
bool float_positive(float value)
{
	if((*((unsigned long *)&value)) & 0x80000000)
		return false ;
	return true ;
}
float clamp(float value, float min, float max)
{
	if(float_less(value, min))
		value = min ;
	else if(float_greater(value, max))
		value = max ;
	return value ;
}
float float_greatest(float x, float y, float z)
{
	if(float_greater(x, y))
	{
		if(float_greater(x, z))
			return x ;
		else
			return z ;
	}
	else
	{
		if(float_greater(y, z))
			return y ;
		else
			return z ;
	}
	return 0.0f ;
}
float float_random(float range, bool bOffsetZero, float detail)
{
    float ret = (rand()%(int)(range*detail)) / detail ;
	if(bOffsetZero)
		ret -= (range*0.5f) ;

    return ret ;
}

void printfloat(float f)
{
     unsigned t;

     char temp[35],bin[35];

     // 비트를 다루기 쉽도록 정수형 변수에 대입한다.

     t=*(unsigned *)&f;

     // 선행 제로를 포함한 32자리의 2진수 문자열로 변환

     itoa(t,bin,2);

     memset(temp,'0',35);

     strcpy(temp+32-strlen(bin),bin);

     // 부호, 지수 다음에 공백을 하나씩 넣음

     bin[0]=temp[0];

     bin[1]=' ';

     strncpy(bin+2,temp+1,8);

     bin[10]=' ';

     strcpy(bin+11,temp+9);

     //printf("실수=%f(%s), ",f,bin);
	 //TRACE("실수=%f(%s), ",f,bin);

     // 지수 출력

     //printf("지수부 = %d\n",(t >> 23 & 0xff) - 127);
	 //TRACE("지수부 = %d\r\n",(t >> 23 & 0xff) - 127);
}