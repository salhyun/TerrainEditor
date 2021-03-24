#pragma once

#include <math.h>
#include <assert.h>

#define sqr(x) ((x)*(x))

extern float g_fEpsilon ; //이건 이제 손대지 말자...

const float natural_log = 2.7182818284f ;

const float pi = 3.1415926535f ;
const float two_pi = pi*2.0f ;
const float half_pi = pi/2.0f ;
const float quarter_pi = pi/4.0f ;

int round_int(float value) ;
float float_round(float value, float cipher=3.0f) ;
float clamp(float value, float min, float max) ;
float float_greatest(float x, float y, float z) ;
float float_abs(float value) ;
double float_abs(double value) ;
bool float_positive(float value) ;
float float_random(float range, bool bOffsetZero=false, float detail=512.0f) ;

void printfloat(float f) ;

//#define float_eq(x,v)	( ((v) - epsilon) < (x) && (x) < ((v) + epsilon) )		// float equality test
#define float_eq(x,v) ( float_abs(v-x) <= g_fEpsilon )

#define float_less(x, v) ( (x) < (v) && !float_eq(x,v) )
#define float_greater(x, v) ( (x) > (v) && !float_eq(x,v) )

#define float_less_eq(x, v) ( (x) < (v) || float_eq(x,v) )
#define float_greater_eq(x, v) ( (x) > (v) || float_eq(x,v) )

#define deg_rad(x) (x*3.141592f/180.0f)
#define rad_deg(x) (x*180.0f/3.141592f)

//const float FLOAT_PRECISION					= (float)(3.4e-8);
//const float PI								= (float)(3.14159265358979323846264338327950288419716939937511);
//const float TWO_PI							= (float)(2.0*PI);
//const float HALF_PI							= (float)(PI/2.0);
//const float QUARTER_PI						= (float)(PI/4.0);
//const float EIGHTH_PI						= (float)(PI/8.0);
//const float PI_SQUARED						= (float)(9.86960440108935861883449099987615113531369940724079);
//const float PI_INVERSE						= (float)(0.31830988618379067153776752674502872406891929148091);
//const float PI_OVER_180						= (float)(PI/180);
//const float PI_DIV_180						= (float)(180/PI);
//const float NATURAL_LOGARITHM_BASE			= (float)(2.71828182845904523536028747135266249775724709369996);
//const float EULERS_CONSTANT					= (float)(0.57721566490153286060651);
//const float SQUARE_ROOT_2					= (float)(1.41421356237309504880168872420969807856967187537695);
//const float INVERSE_ROOT_2					= (float)(0.707106781186547524400844362105198);
//const float SQUARE_ROOT_3					= (float)(1.73205080756887729352744634150587236694280525381038);
//const float SQUARE_ROOT_5					= (float)(2.23606797749978969640917366873127623544061835961153);
//const float SQUARE_ROOT_10					= (float)(3.16227766016837933199889354443271853371955513932522);
//const float CUBE_ROOT_2						= (float)(1.25992104989487316476721060727822835057025146470151);
//const float CUBE_ROOT_3						= (float)(1.44224957030740838232163831078010958839186925349935);
//const float FOURTH_ROOT_2					= (float)(1.18920711500272106671749997056047591529297209246382);
//const float NATURAL_LOG_2					= (float)(0.69314718055994530941723212145817656807550013436026);
//const float NATURAL_LOG_3					= (float)(1.09861228866810969139524523692252570464749055782275);
//const float NATURAL_LOG_10					= (float)(2.30258509299404568401799145468436420760110148862877);
//const float NATURAL_LOG_PI					= (float)(1.14472988584940017414342735135305871164729481291531);
//const float BASE_TEN_LOG_PI					= (float)(0.49714987269413385435126828829089887365167832438044);
//const float NATURAL_LOGARITHM_BASE_INVERSE	= (float)(0.36787944117144232159552377016146086744581113103177);
//const float NATURAL_LOGARITHM_BASE_SQUARED	= (float)(7.38905609893065022723042746057500781318031557055185);
//const float GOLDEN_RATIO						= (float)((SQUARE_ROOT_5 + 1.0) / 2.0);
//const float DEGREE_RATIO						= (float)(PI_DIV_180);
//const float RADIAN_RATIO						= (float)(PI_OVER_180);
//const float GRAVITY_CONSTANT					= 9.81f;