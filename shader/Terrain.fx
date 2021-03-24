//전역변수
float4x4 matWVP ;
float4x4 matWIT ;
float4x4 matWLP ;
float4x4 matWLPT ;
float4x4 matRotTex ;
float4 vDiffuse ;
float4 vLookAt ;
float4 vCameraPos ;
float afTilingTex[9] ;
float4 vWaterLevel ;
float4 vWaterColorTone ;
float fScaleY ;
float4 vHolecup ;

//광원밝기
float4 I_a = {0.3f, 0.3f, 0.3f, 0.f} ; //ambient
float4 I_d = {0.7f, 0.7f, 0.7f, 0.f} ; //diffuse

//반사율
float4 K_a = {1.f, 1.f, 1.f, 1.f} ;//ambient
float4 K_d = {1.f, 1.f, 1.f, 1.f} ;//diffuse

//경면광
float4 specular = {1.f, 1.f, 1.f, 1.f} ;
//float4 specular = {0.5f, 0.35f, 0.15f, 1.f} ;

float4 vSunDir = {0.578f,0.578f,0.578f,0.0f} ;
//float4 vSunColor = {0.578f,0.578f,0.578f,1.0f} ;
float4 vSunColor = {0.578f,0.578f,0.578f,1.0f} ;

texture BaseMap;
sampler BaseMapSamp = sampler_state
{
    Texture = <BaseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = wrap;
    AddressV = wrap;
};

//##################//
//    LayerMap00    //
//##################//
texture LayerMap00;
sampler LayerMap00Samp = sampler_state
{
    Texture = <LayerMap00>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    MipMapLodBias = -2.5f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap01    //
//##################//
texture LayerMap01;
sampler LayerMap01Samp = sampler_state
{
    Texture = <LayerMap01>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap02    //
//##################//
texture LayerMap02;
sampler LayerMap02Samp = sampler_state
{
    Texture = <LayerMap02>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
    MipMapLodBias = -3.0f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap03    //
//##################//
texture LayerMap03;
sampler LayerMap03Samp = sampler_state
{
    Texture = <LayerMap03>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = POINT;
    MipMapLodBias = -3.0f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap04    //
//##################//
texture LayerMap04;
sampler LayerMap04Samp = sampler_state
{
    Texture = <LayerMap04>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap05    //
//##################//
texture LayerMap05;
sampler LayerMap05Samp = sampler_state
{
    Texture = <LayerMap05>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    //MipMapLodBias = -3.0f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap06    //
//##################//
texture LayerMap06;
sampler LayerMap06Samp = sampler_state
{
    Texture = <LayerMap06>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    //MipMapLodBias = -3.0f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
//##################//
//    LayerMap07    //
//##################//
texture LayerMap07;
sampler LayerMap07Samp = sampler_state
{
    Texture = <LayerMap07>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture ShadowMap;
sampler ShadowMapSamp = sampler_state
{
    Texture = <ShadowMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
texture SrcShadowMap;
sampler SrcShadowMapSamp = sampler_state
{
    Texture = <SrcShadowMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
texture NoiseMap00;
sampler NoiseMap00Samp = sampler_state
{
    Texture = <NoiseMap00>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture NoiseMap01;
sampler NoiseMap01Samp = sampler_state
{
    Texture = <NoiseMap01>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    MipMapLodBias = -1.35f ;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture AlphaMap00;
sampler AlphaMap00Samp = sampler_state
{
    Texture = <AlphaMap00>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture AlphaMap01;
sampler AlphaMap01Samp = sampler_state
{
    Texture = <AlphaMap01>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture AlphaMap02;
sampler AlphaMap02Samp = sampler_state
{
    Texture = <AlphaMap02>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture DecalMap;
sampler DecalMapSamp = sampler_state
{
    Texture = <DecalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    //MipMapLodBias = -3.0f ;

    AddressU = Wrap;
    AddressV = Wrap;
};

texture RoadMap;
sampler RoadMapSamp = sampler_state
{
    Texture = <RoadMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};

struct SLightScatteringShaderParams
{
	float4 vBeta1;
	float4 vBeta2;
	float4 vBetaD1;
	float4 vBetaD2;
	float4 vSumBeta1Beta2;
	float4 vLog2eBetaSum;
	float4 vRcpSumBeta1Beta2;
	float4 vHG;
	float4 vConstants;
	float4 vTermMultipliers;
	float4 vSoilReflectivity;
};

SLightScatteringShaderParams atm ;

void atmosphericLighting(
	uniform float3 eyeVector,
	uniform float3 sunVector,
	uniform float3 norm,
	uniform float4 sunColor,
	uniform float s,
	out float4 vExt,
	out float4 vIns)
{
	//
	// This shader assumes a world-space vertex is 
	// provided, and distance values represent 
	// real-world distances. A set of pre-
	// calculated atmospheric data is provided
	// in the atm structure.
	//
	
	// compute cosine of theta angle
	float cosTheta = dot(eyeVector, sunVector);

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e
	//vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	//float temp = -0.0025f*s ;
	//vExt = float4(temp, temp, temp, 1.0f) ;
	vExt = -atm.vSumBeta1Beta2*s*atm.vConstants.y ;
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
	p2Theta = 1.0f/(sqrt(p2Theta));
	p2Theta = (p2Theta*p2Theta*p2Theta) * atm.vHG.x;


	// compute inscattering (vIns) as
	// (vBetaD1*p1Theta + vBetaD1*p2Theta) *
	// (1-vExt) * atm.vRcpSumBeta1Beta2
	// 
	// atm.vRcpSumBeta1Beta2 = 
	// 1.0f/ (Rayleigh+Mie)
	vIns = ((atm.vBetaD1*p1Theta)+
			(atm.vBetaD2*p2Theta))
			*(atm.vConstants.x-vExt)
			*atm.vRcpSumBeta1Beta2;

	// scale inscatter and extinction 
	// for effect (optional)
	vIns = vIns*atm.vTermMultipliers.x;
	// scale extinction prior to its use
	// (this is optional)
	//vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;
	vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity ;

	// reduce inscattering on unlit surfaces
	// by modulating with a monochrome
	// Lambertian scalar. This is slightly
	// offset to allow some inscattering to
	// bleed into unlit areas
	float NdL = dot(norm, sunVector);
	vIns= vIns*NdL;

	// apply sunlight color
	// and strength to each term
	// and output
	vIns.xyz = vIns*sunColor*sunColor.w;
	vIns.w = 0.0f;

	vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.w = 1.0f;
}

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTSUNLIT
{
	float4 Pos : POSITION ;
	//float4 Color : COLOR0 ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 texTerrain : TEXCOORD0 ;
	float2 texAlpha : TEXCOORD1 ;
	float4 texShadowUV : TEXCOORD2 ;
	float3 Depth : TEXCOORD3 ;
	float3 color : TEXCOORD4 ;
} ;

float4 Lambert(float4 Pos, float3 Normal, float fDiffuse=1.0f)
{
	float4 Color ;
	float3 L = normalize((float3)(vSunDir)) ;//normalize(vLightPos-Pos) ;//-normalize((float3)vLightDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	Color = (I_a*K_a)
	    	+(K_d*max(0, dot(N, L)*fDiffuse)) ;
			//+(specular*pow(max(0, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터
//VS_OUTPUTSUNLIT VS_Bump(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 texTerrain : TEXCOORD0, float2 texAlpha : TEXCOORD1)
VS_OUTPUTSUNLIT VS_Sunlit(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texTerrain : TEXCOORD0)
{
	VS_OUTPUTSUNLIT Out = (VS_OUTPUTSUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	//Out.Color = vDiffuse ;
	float4 lambert_color = Lambert(Pos, Normal, vDiffuse.x) ;
	
	float3 eyeVector = float3(vCameraPos.xyz-Pos.xyz) ;
	float s = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting(eyeVector, vSunDir, Normal, vSunColor, s, Out.vE, Out.vI) ;
	
	//float3 H = normalize(vSunDir + eyeVector) ;
	//Out.color.xyz = specular*pow(max(0.01f, dot(Normal, H)), 90) ;
	
	//float sh = dot(Normal, normalize(vSunDir))*0.5f ;
	//Out.Color.xyz = 0.5f+sh ;
	//Out.Color.w = 1 ;
	//Out.Color = Lambert(Pos, Normal) ;

	Out.texTerrain = texTerrain ;
	Out.texAlpha = texTerrain ;
	
	float4 p = mul(Pos, matWIT) ;
	Out.Depth.x = p.y ;
	//Out.Depth.y = p.w ;
	Out.Depth.z = lambert_color.x ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;

	/*
	float3 N, T, B ;
	
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;

	float3 E = normalize((float3)(vCameraPos.xyz-Pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;

	float3 L = normalize(vLightPos-Pos) ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	*/
	
	return Out ;
}
float4 PS_Sunlit(VS_OUTPUTSUNLIT In) : COLOR
{
	float fDiffuse, Incide, fAlpha ;
	float2 texLayer ;
	float4 f4Color = {0, 0, 0, 0} ;
	
	float4 alpha00 = tex2D(AlphaMap00Samp, In.texAlpha) ;
	float4 alpha01 = tex2D(AlphaMap01Samp, In.texAlpha) ;
	
	texLayer = In.texTerrain*afTilingTex[0] ;
	//ob
	f4Color = tex2D(BaseMapSamp, texLayer) ;
	
	//float3 a={77.0f/255.0f, 140.0f/255.0f, 3.0f/255.0f}, b={200.0f/255.0f, 237.0f/255.0f, 69.0f/255.0f}, c ;
	//float3 a={255.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f}, b={0.0f/255.0f, 0.0f/255.0f, 255.0f/255.0f}, c ;
	//c = lerp(a, b, tex2D(NoiseMap00Samp, In.texAlpha).xyz) ;
	//float3 c = tex2D(NoiseMap00Samp, In.texAlpha).xyz ;
	//f4Color.xyz *= c ;
	
	//rough
	fAlpha = alpha00.w ;
	texLayer = In.texTerrain*afTilingTex[1] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap00Samp, texLayer) ;
	
	//해저드와 티 레이어 순서를 바꾼다
	//tee
	fAlpha = alpha00.y ;
	texLayer = In.texTerrain*afTilingTex[3] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap02Samp, texLayer) ;
	
	//hazard
	fAlpha = alpha00.x ;
	texLayer = In.texTerrain*afTilingTex[2] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap01Samp, texLayer) ;
	
	//fairway
	fAlpha = alpha00.z ;
	float2 texRot = mul(In.texAlpha, matRotTex) ;
	texLayer = In.texTerrain*afTilingTex[4] ;
	float4 layer = tex2D(LayerMap03Samp, texLayer) ;
	layer.xyz += (tex2D(NoiseMap01Samp, texRot).z) ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*layer ;
	
	//bunker
	fAlpha = alpha01.w ;
	texLayer = In.texTerrain*afTilingTex[5] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap04Samp, texLayer) ;
	//float4 a = tex2D(LayerMap04Samp, texLayer) ;
	//a.xyz += In.color ;
	//a *= fAlpha ;
	//f4Color = (1-fAlpha)*f4Color + a ;

	//green
	fAlpha = alpha01.x ;
	texLayer = In.texTerrain*afTilingTex[6] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap05Samp, texLayer) ;
	
	//greenside
	fAlpha = alpha01.y ;
	texLayer = In.texTerrain*afTilingTex[7] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap06Samp, texLayer) ;
	
	//roughside
	fAlpha = alpha01.z ;
	texLayer = In.texTerrain*afTilingTex[8] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap07Samp, texLayer) ;

	float3 c = tex2D(NoiseMap00Samp, In.texAlpha).xyz ;
	f4Color.xyz *= c ;

	float shadow=1 ;
//	if(In.texShadowUV.x < 0.0f || In.texShadowUV.x > In.Depth.y || In.texShadowUV.y < 0.0f || In.texShadowUV.y > In.Depth.y || In.Depth.x < 0.0f || In.Depth.x > In.Depth.y)
//		shadow = 1 ;
//	else
//	{
		//shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).x ;
		shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).r ;
		//float shadow_vert = tex2Dproj(ShadowMapSamp, In.texShadowUV).g ;
		//shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).r ;
		//shadow = (abs(1.0f-shadow) <= 0.001f) ? 0.1f : 1.0f ;
		
		shadow = (shadow <= 0.5f) ? 0.5f : shadow ;
		
		//float terrain_vert = In.Depth.x/1024.0f ;
		//shadow = (terrain_vert < shadow_vert) ? shadow : 1.0f ;
		
		//shadow = (abs(1.0f-vert)<=0.001f) ? shadow : 1.0f ;
//	}

//	float shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).x ;
//	if((shadow*In.Depth.y+epsilon) < In.Depth.x)
//		shadow = (shadow <= 0.5f) ? 0.5f : shadow ;
//	else
//		shadow = 1.0f ;

	//f4Color = max(0.0f, f4Color*vDiffuse*In.Depth.z) ;
	f4Color = max(0.0f, f4Color*In.Depth.z) ;
	
	//return f4Color*shadow ;//without scattering
	//f4Color *= ((vDiffuse*In.vE)+In.vI)*shadow ;

	f4Color = ((f4Color*In.vE)+In.vI)*shadow ;
	f4Color.w = 0.0f ;

	return f4Color ;	
	//return (f4Color*(vDiffuse*In.vE)+In.vI)*shadow ;//with scattering
}
struct VS_OUTPUTSHADOW
{
	float4 Pos : POSITION ;
	float2 Depth : TEXCOORD0 ;
} ;
VS_OUTPUTSHADOW VS_Shadow(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texTerrain : TEXCOORD0)
{
	VS_OUTPUTSHADOW Out = (VS_OUTPUTSHADOW)0 ;

	Out.Pos = mul(Pos, matWLP) ;
	Out.Depth.x = Out.Pos.z ;
	Out.Depth.y = Out.Pos.w ;
	
	return Out ;
}
float4 PS_Shadow(VS_OUTPUTSHADOW In) : COLOR
{
	float4 color = In.Depth.x/In.Depth.y ;
	//color = (In.Depth.x-afViewPlanes[0])/(afViewPlanes[1]-afViewPlanes[0]) ;
	return color ;
}
VS_OUTPUTSUNLIT VS_RoadSunlit(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texTerrain : TEXCOORD0)
{
	VS_OUTPUTSUNLIT Out = (VS_OUTPUTSUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	//Out.Color = vDiffuse ;
	float4 lambert_color = Lambert(Pos, Normal, vDiffuse.x) ;
	
	float3 eyeVector = float3(vCameraPos.xyz-Pos.xyz) ;
	float s = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting(eyeVector, vSunDir, Normal, vSunColor, s, Out.vE, Out.vI) ;

	Out.texTerrain = texTerrain ;
	Out.texAlpha = texTerrain ;
	float4 p = mul(Pos, matWLP) ;
	Out.Depth.x = p.z ;
	//Out.Depth.y = p.w ;
	Out.Depth.y = lambert_color.x ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;
	
	return Out ;
}
float4 PS_RoadSunlit(VS_OUTPUTSUNLIT In) : COLOR
{
	float4 color = tex2D(RoadMapSamp, In.texTerrain) ;

	float shadow=1 ;
	shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).r ;
	shadow = (shadow <= 0.5f) ? 0.5f : shadow ;
	
	//color.xyz = max(0.0f, color.xyz*vDiffuse.xyz*In.Depth.y) ;
	//color.xyz *= ((vDiffuse.xyz*In.vE.xyz)+In.vI.xyz)*shadow ;
	
	color.xyz = max(0.0f, color.xyz*In.Depth.y) ;
	color.xyz = ((color.xyz*In.vE.xyz)+In.vI.xyz)*shadow ;
	
	return color ;
	//return (color*(vDiffuse*In.vE)+In.vI)*shadow ;//with scattering
}

VS_OUTPUTSUNLIT VS_DepthWater(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texTerrain : TEXCOORD0)
{
	VS_OUTPUTSUNLIT Out = (VS_OUTPUTSUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;

	float4 lambert_color = Lambert(Pos, Normal, vDiffuse.x) ;
	
	float3 eyeVector = float3(vCameraPos.xyz-Pos.xyz) ;
	float s = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting(eyeVector, vSunDir, Normal, vSunColor, s, Out.vE, Out.vI) ;

	Out.texTerrain = texTerrain ;
	Out.texAlpha = texTerrain ;
	float4 p = mul(Pos, matWLP) ;
	Out.Depth.x = p.z ;
	Out.Depth.y = lambert_color.x ;
	Out.Depth.z = Pos.y ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;
	
	return Out ;
}
float4 PS_DepthWater(VS_OUTPUTSUNLIT In) : COLOR
{
	float fDiffuse, Incide, fAlpha ;
	float2 texLayer ;
	float4 f4Color = {0, 0, 0, 0} ;
	
	float4 alpha00 = tex2D(AlphaMap00Samp, In.texAlpha) ;
	float4 alpha01 = tex2D(AlphaMap01Samp, In.texAlpha) ;
	
	texLayer = In.texTerrain*afTilingTex[0] ;
	//ob
	f4Color = tex2D(BaseMapSamp, texLayer) ;
	
	//float3 a={77.0f/255.0f, 140.0f/255.0f, 3.0f/255.0f}, b={200.0f/255.0f, 237.0f/255.0f, 69.0f/255.0f}, c ;
	//float3 a={255.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f}, b={0.0f/255.0f, 0.0f/255.0f, 255.0f/255.0f}, c ;
	//c = lerp(a, b, tex2D(NoiseMap00Samp, In.texAlpha).xyz) ;
	float3 c = tex2D(NoiseMap00Samp, In.texAlpha).xyz ;
	//f4Color.xyz *= 1.25f ;
	f4Color.xyz *= c ;
	
	//rough
	fAlpha = alpha00.w ;
	texLayer = In.texTerrain*afTilingTex[1] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap00Samp, texLayer) ;
	
	//hazard
	fAlpha = alpha00.x ;
	texLayer = In.texTerrain*afTilingTex[2] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap01Samp, texLayer) ;
	
	//tee
	fAlpha = alpha00.y ;
	texLayer = In.texTerrain*afTilingTex[3] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap02Samp, texLayer) ;
	
	//fairway	
	fAlpha = alpha00.z ;
	float2 texRot = mul(In.texAlpha, matRotTex) ;
	texLayer = In.texTerrain*afTilingTex[4] ;
	float4 layer = tex2D(LayerMap03Samp, texLayer) ;
	layer.xyz += (tex2D(NoiseMap01Samp, texRot).z) ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*layer ;
	
	//bunker
	fAlpha = alpha01.w ;
	texLayer = In.texTerrain*afTilingTex[5] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap04Samp, texLayer) ;

	//green
	fAlpha = alpha01.x ;
	texLayer = In.texTerrain*afTilingTex[6] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap05Samp, texLayer) ;
	
	//greenside
	fAlpha = alpha01.y ;
	texLayer = In.texTerrain*afTilingTex[7] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap06Samp, texLayer) ;
	
	//roughside
	fAlpha = alpha01.z ;
	texLayer = In.texTerrain*afTilingTex[8] ;
	f4Color = (1-fAlpha)*f4Color + fAlpha*tex2D(LayerMap07Samp, texLayer) ;

	float shadow=1 ;
	//shadow = tex2D(SrcShadowMapSamp, In.texShadowUV).x ;
	shadow = tex2D(SrcShadowMapSamp, In.texShadowUV).r ;
	shadow = (shadow <= 0.5f) ? 0.5f : shadow ;

	//f4Color = max(0.0f, f4Color*vDiffuse*In.Depth.y) ;
	//return f4Color*shadow ;//without scattering
	//f4Color *= ((vDiffuse*In.vE)+In.vI)*shadow ;
	
	f4Color = max(0.0f, f4Color*In.Depth.y) ;
	f4Color = ((f4Color*In.vE)+In.vI)*shadow ;
	
	float4 color ;
	float r1 ;
	float3 r2 ;
	r1 = (vWaterLevel.y-In.Depth.z)*vWaterLevel.w ;//(vWaterLevel.y-In.Depth.z)물의 깊이를 계산하고 vWaterLevel.w로 규모조정을 한다.
	color.w = saturate(r1) ;
	color.w = color.w*color.w ;
	
	color.xyz = lerp(f4Color.xyz, vWaterColorTone.xyz, color.w) ;

	return color ;
}

VS_OUTPUTSUNLIT VS_Holecup(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texDecal : TEXCOORD0, float2 texNoise : TEXCOORD1)
{
	VS_OUTPUTSUNLIT Out = (VS_OUTPUTSUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;

	//Out.Color = vDiffuse ;
	float4 lambert_color = Lambert(Pos, Normal, vDiffuse.x) ;
	
	float3 eyeVector = float3(vCameraPos.xyz-Pos.xyz) ;
	float s = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting(eyeVector, vSunDir, Normal, vSunColor, s, Out.vE, Out.vI) ;

	Out.texTerrain = texDecal ;
	Out.texAlpha = texNoise ;
	float4 p = mul(Pos, matWLP) ;
	Out.Depth.x = p.z ;
	Out.Depth.y = p.w ;
	Out.Depth.z = lambert_color.x ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;

	/*
	float3 N, T, B ;
	
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;

	float3 E = normalize((float3)(vCameraPos.xyz-Pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;

	float3 L = normalize(vLightPos-Pos) ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	*/
	
	return Out ;
}

float4 PS_Holecup(VS_OUTPUTSUNLIT In) : COLOR
{
	float4 f4Color = {0, 0, 0, 0} ;

	f4Color = tex2D(DecalMapSamp, In.texTerrain) ;
	
	float3 c = tex2D(NoiseMap00Samp, In.texAlpha).xyz ;
	f4Color.xyz *= c ;

	//float shadow=1 ;
	//if(In.texShadowUV.x < 0.0f || In.texShadowUV.x > In.Depth.y || In.texShadowUV.y < 0.0f || In.texShadowUV.y > In.Depth.y || In.Depth.x < 0.0f || In.Depth.x > In.Depth.y)
		//shadow = 1 ;
	//else
	//{
		//shadow = tex2D(SrcShadowMapSamp, In.texShadowUV).x ;
		//shadow = (shadow <= 0.5f) ? 0.5f : shadow ;
	//}

	float shadow = tex2Dproj(ShadowMapSamp, In.texShadowUV).r ;
	shadow = (shadow <= 0.5f) ? 0.5f : shadow ;

	f4Color.xyz = max(0.0f, f4Color.xyz*In.Depth.zzz) ;
	//return f4Color*shadow ;//without scattering
	//f4Color *= ((vDiffuse*In.vE)+In.vI)*shadow ;

	f4Color.xyz = ((f4Color.xyz*In.vE.xyz)+In.vI.xyz)*shadow ;
	
	return f4Color ;	
	//return (f4Color*(vDiffuse*In.vE)+In.vI)*shadow ;//with scattering
}

struct VS_OUTPUTALTITUDE
{
	float4 Pos : POSITION ;
	float2 texAlpha : TEXCOORD0 ;
	float2 Altitude : TEXCOORD1 ;
} ;

float3 vAltitudeLow = {0.0f, 1.0f, 0.0f} ;
float3 vAltitudeHigh = {1.0f, 0.0f, 0.0f} ;

VS_OUTPUTALTITUDE VS_Altitude(float4 Pos : POSITION, float3 Normal : NORMAL, float2 texTerrain : TEXCOORD0)
{
	VS_OUTPUTALTITUDE Out = (VS_OUTPUTALTITUDE)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	Out.texAlpha = texTerrain ;
	Out.Altitude.x = Pos.y - vHolecup.y ;
	return Out ;
}
float4 PS_Altitude(VS_OUTPUTALTITUDE In) : COLOR
{
	float4 color ;
	
	float4 alpha01 = tex2D(AlphaMap01Samp, In.texAlpha) ;
	
	//if(alpha01.x < 0.1f)
		//color.xyzw = 0.0f ;
	//else
	//{
		//color.xyz = lerp(vAltitudeLow, vAltitudeHigh, saturate((In.Altitude.x+2.0f)*0.25f)) ;
		//color.w = 1.0f ;
	//}
	
	color.xyz = lerp(vAltitudeLow, vAltitudeHigh, saturate((In.Altitude.x+2.0f)*0.25f)) ;
	color.w = alpha01.x ;

	return color ;
}

technique TShader
{
	pass P0//SHADER_Sunlit
	{
		VertexShader = compile vs_2_0 VS_Sunlit() ;
		PixelShader = compile ps_2_0 PS_Sunlit() ;
	}
	pass P1//SHADER_Shadow
	{
		VertexShader = compile vs_2_0 VS_Shadow() ;
		PixelShader = compile ps_2_0 PS_Shadow() ;
	}
	pass P2//SHADER_RoadSunlit
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		VertexShader = compile vs_2_0 VS_RoadSunlit() ;
		PixelShader = compile ps_2_0 PS_RoadSunlit() ;
	}
	pass P3//SHADER_DepthWater
	{
		//AlphaBlendEnable = true ;
		//SrcBlend = DestColor ;
		//DestBlend = Zero ;
		//SrcBlend = SrcAlpha ;
		//DestBlend = One ;
		
		VertexShader = compile vs_2_0 VS_DepthWater() ;
		PixelShader = compile ps_2_0 PS_DepthWater() ;
	}
	pass P4//SHADER_Holecup
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		VertexShader = compile vs_2_0 VS_Holecup() ;
		PixelShader = compile ps_2_0 PS_Holecup() ;
	}
	pass P5//SHADER_Altitude
	{
		VertexShader = compile vs_2_0 VS_Altitude() ;
		PixelShader = compile ps_2_0 PS_Altitude() ;
	}
}