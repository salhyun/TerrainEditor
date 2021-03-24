//전역변수

float4x4 matWVP ;
float4x4 matWLP ;
float4x4 matWLPT ;
float4 vCameraPos ;
float4 vLightDir ;
float4 vLightColor ;
float4 vDiffuse ;
float4 vLookat ;

texture DecalMap01;
sampler DecalMap01Samp = sampler_state
{
    Texture = <DecalMap01>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = wrap;
    AddressV = wrap;
};
texture DecalMap02;
sampler DecalMap02Samp = sampler_state
{
    Texture = <DecalMap02>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = wrap;
    AddressV = wrap;
};
texture DecalMap03;
sampler DecalMap03Samp = sampler_state
{
    Texture = <DecalMap03>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = wrap;
    AddressV = wrap;
};
texture DecalMap04;
sampler DecalMap04Samp = sampler_state
{
    Texture = <DecalMap04>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = wrap;
    AddressV = wrap;
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
void atmosphericLighting_noNdL(
	uniform float3 eyeVector,
	uniform float3 sunVector,
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
	//float NdL = dot(norm, sunVector);
	//vIns= vIns*NdL;
	vIns *= 0.75f ;

	// apply sunlight color
	// and strength to each term
	// and output
	vIns.xyz = vIns*sunColor*sunColor.w;
	vIns.w = 0.0f;

	vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.w = 1.0f;
}

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 Tex0 : TEXCOORD0 ;
	float4 texShadowUV : TEXCOORD1 ;
	float4 DecalIndex : TEXCOORD2 ;
} ;

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터

VS_OUTPUT VS(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float3 Factor : TEXCOORD1, float3 Diffuse : TEXCOORD2)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//position billboard
	float3 up = {0, 1, 0}, right ;
	right = cross(vLookat.xyz, up) ;

	float4 pos ;

	right *= Factor.x ;
	up *= Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	//좌표변환
	Out.Pos = mul(pos, matWVP) ;
	
	Out.Tex0 = TexDecal ;
	//Out.DecalIndex.x = Factor.z ;
	//Out.DecalIndex.y = Factor.w ;
	Out.DecalIndex.x = Diffuse.x ;
	Out.DecalIndex.y = Diffuse.y ;
	Out.DecalIndex.z = Diffuse.z ;
	Out.DecalIndex.w = Factor.z ;
	
	float3 eyeVector = (float3)(vCameraPos.xyz-Pos.xyz) ;
	float fDistfromCamera = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting_noNdL(eyeVector, vLightDir, vLightColor, fDistfromCamera, Out.vE, Out.vI) ;

	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float4 color ;
	
	if(abs(In.DecalIndex.w-0) <= 0.0001f)
		color = tex2D(DecalMap01Samp, In.Tex0) ;
	else if(abs(In.DecalIndex.w-0.03125f) <= 0.0001f)
		color = tex2D(DecalMap02Samp, In.Tex0) ;
	else if(abs(In.DecalIndex.w-0.0625f) <= 0.0001f)
		color = tex2D(DecalMap03Samp, In.Tex0) ;	
	else if(abs(In.DecalIndex.w-0.09375f) <= 0.0001f)
		color = tex2D(DecalMap04Samp, In.Tex0) ;
			
	color.xyz += In.DecalIndex.xyz ;

	return (color*In.vE)+In.vI ;
}

struct VS_OUTPUTSHADOW
{
	float4 Pos : POSITION ;
	float2 Tex0 : TEXCOORD0 ;
	float2 Depth : TEXCOORD1 ;
	float4 DecalIndex : TEXCOORD2 ;
} ;

VS_OUTPUTSHADOW VS_Shadow(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float3 Factor : TEXCOORD1, float3 Diffuse : TEXCOORD2)
{
	VS_OUTPUTSHADOW Out = (VS_OUTPUTSHADOW)0 ;

	//position billboard
	float3 up = {0, 1, 0}, right ;
	right = cross(vLookat.xyz, up) ;

	float4 pos ;

	right *= Factor.x ;
	up *= Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	//좌표변환
	Out.Pos = mul(pos, matWLP) ;
	Out.Tex0 = TexDecal ;
	Out.Depth.x = Out.Pos.z ;
	Out.Depth.y = Out.Pos.w ;
	
	//Out.DecalIndex.x = Factor.z ;
	//Out.DecalIndex.y = Factor.w ;
	Out.DecalIndex.x = Diffuse.x ;
	Out.DecalIndex.y = Diffuse.y ;
	Out.DecalIndex.z = Diffuse.z ;
	Out.DecalIndex.w = Factor.z ;

	return Out ;
}
float4 PS_Shadow(VS_OUTPUTSHADOW In) : COLOR
{
	float4 color ;
	
	color = In.Depth.x/In.Depth.y ;
	
	if(abs(In.DecalIndex.w-0) <= 0.0001f)
		color.w = tex2D(DecalMap01Samp, In.Tex0).w ;
	else if(abs(In.DecalIndex.w-0.03125f) <= 0.0001f)
		color.w = tex2D(DecalMap02Samp, In.Tex0).w ;
	else if(abs(In.DecalIndex.w-0.0625f) <= 0.0001f)
		color.w = tex2D(DecalMap03Samp, In.Tex0).w ;	
	else if(abs(In.DecalIndex.w-0.09375f) <= 0.0001f)
		color.w = tex2D(DecalMap04Samp, In.Tex0).w ;

	return color ;				
}

technique TShader
{
	pass P0//SHADER
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		ZWriteEnable = false ;
		ZFunc = less ;
	
		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
	pass P1
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		VertexShader = compile vs_2_0 VS_Shadow() ;
		PixelShader = compile ps_2_0 PS_Shadow() ;
	}
}