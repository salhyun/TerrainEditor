//전역변수

float4x4 matWVP ;
float4x4 matWIT ;
float4 vDiffuse = {1.f, 1.f, 1.f, 1.f} ;
float4 vLightPos ;
float4 vLightDir ;
float4 vLookAt ;
float4 vCameraPos ;
float4x4 matWLP ;
float4x4 matWLPT ;
float4x4 matTexCoordAni ;
int anObjectID[2] ;
float epsilon ;
float fDistFromCamera ;
int nEigenIndex ;
float fAlphaTestDiffuse ;
float fAlphaBlendingByForce=1.0f ;

float fElapsedTime=0.0f ;
float fTimeScale=2.0f ;
float fFrequency=10.0f ;
float fAmplitude=0.8f ;

//스키닝을 위한 상수레지스터
float4x4 amatSkin[2] ;

//광원밝기
//float4 I_a = {0.3f, 0.3f, 0.3f, 1.f} ; //ambient
//float4 I_a = {0.4f, 0.4f, 0.4f, 1.f} ; //ambient
float4 I_a = {0.2f, 0.2f, 0.2f, 1.f} ; //ambient
//float4 I_a = {0.1f, 0.1f, 0.1f, 1.f} ; //ambient
float4 I_d = {0.7f, 0.7f, 0.7f, 1.f} ; //diffuse

//반사율
float4 K_a = {1.f, 1.f, 1.f, 1.f} ;//ambient
float4 K_d = {1.f, 1.f, 1.f, 1.f} ;//diffuse

//경면광
float4 specular = {1.f, 1.f, 1.f, 1.f} ;
float4 vLightColor = {1.f, 1.f, 1.f, 1.f} ;

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

float4 Lambert(float4 Pos, float3 Normal, float power=10.0f)
{
	float4 Color ;
	float3 L = normalize((float3)vLightDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	//max(0, dot(N, L))로 하면 Color가 0 이 되어버리고 그러면 알파테스트를 할경우에 테스트를 통과하지 못한다.
	//그래서 max(0.01f, dot(N, L))로 했지만 이거역시 수정될 가능성이 높기때문에 주시해야될 항목임.
	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
			+(specular*pow(max(0.01f, dot(N, H)), power)) ;//Blinn-Phong HalfVector

	return Color ;
}
float4 LambertwithLightColor(float4 Pos, float3 Normal, float4 LightColor)
{
	float4 Color ;
	float3 L = normalize((float3)vLightDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	//max(0, dot(N, L))로 하면 Color가 0 이 되어버리고 그러면 알파테스트를 할경우에 테스트를 통과하지 못한다.
	//그래서 max(0.01f, dot(N, L))로 했지만 이거역시 수정될 가능성이 높기때문에 주시해야될 항목임.
	//Color = (I_a*K_a)
	//    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
	//		+(specular*pow(max(0.01f, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
			
	Color = (I_a*K_a)
	    	+(LightColor*K_d*max(0.01f, (dot(N, L)*0.5f)+0.5f) )
			+(LightColor*pow(max(0.01f, dot(N, H)), 30)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

float4 Lambert_PointLight(float4 Pos, float3 Normal)
{
	float4 Color ;
	float3 L = normalize(vLightPos-Pos) ;//-normalize((float3)vLightDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	//max(0, dot(N, L))로 하면 Color가 0 이 되어버리고 그러면 알파테스트를 할경우에 테스트를 통과하지 못한다.
	//그래서 max(0.01f, dot(N, L))로 했지만 이거역시 수정될 가능성이 높기때문에 주시해야될 항목임.
	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
			+(specular*pow(max(0.01f, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

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
texture DecalMap;
sampler DecalMapSamp = sampler_state
{
    Texture = <DecalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
    
    //MipMapLodBias = -3.0f;
};
texture NoMipmapFilterMap;
sampler NoMipmapFilterMapSamp = sampler_state
{
    Texture = <NoMipmapFilterMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = CLAMP;
    AddressV = CLAMP;
    
    //MipMapLodBias = -3.0f;
};

texture NormalMap;
sampler NormalMapSamp = sampler_state
{
	Texture = <NormalMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float4 Color : COLOR0 ;
	float2 TexDecal : TEXCOORD0 ;
} ;

struct VS_OUTPUTBUMP
{
	float4 Pos : POSITION ;
	float4 Color : COLOR0 ;
	float2 TexDecal : TEXCOORD0 ;
	float3 L : TEXCOORD1 ;
	float3 E : TEXCOORD2 ;
} ;

struct VS_OUTPUTAMBIENTOCCLUSION
{
	float4 Pos : POSITION ;
	float4 Color : COLOR0 ;
	float2 TexDecal : TEXCOORD1 ;
} ;

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터
VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	Out.Color = Lambert(Pos, Normal) ;
	
	//float4 vTexCoord = {TexDecal.x, TexDecal.y, 0.0f, 1.0f} ;
	//vTexCoord = mul(vTexCoord, matTexCoordAni) ;
	//Out.TexDecal = vTexCoord.xy ;
	
	Out.TexDecal = TexDecal ;

	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float4 Decal = tex2D(DecalMapSamp, In.TexDecal) ;

	In.Color *= Decal ;
//	if(In.Color.w <= 0.1f)
//		In.Color.w = 1.0f ;

	return In.Color ;
}

VS_OUTPUTBUMP VS_Bump(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUTBUMP Out = (VS_OUTPUTBUMP)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = vDiffuse ;
	
	float4 vTexCoord = {TexDecal.x, TexDecal.y, 0.0f, 1.0f} ;
	vTexCoord = mul(vTexCoord, matTexCoordAni) ;
	Out.TexDecal = vTexCoord.xy ;
	//Out.TexDecal = TexDecal ;
	
	float3 N, T, B ;
	
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;

	float3 E = normalize((float3)(vCameraPos.xyz-Pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;

	float3 L = normalize(vLightPos-Pos) ;//-normalize((float3)vLightDir) ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	
	return Out ;
}

float4 PS_Bump(VS_OUTPUTBUMP In) : COLOR
{
	float3 N = 2.0f*tex2D(NormalMapSamp, In.TexDecal).xyz-1.0f ;//법선맵은 -1 에서 +1 의 범위 그러나 현재 저장된 데이타는 0-1사이 값, *2를 하고 -1을 한다.
	float3 L = normalize(In.L) ;//rasterizer 를 하게 되면 자동보간이 된다. 그래서 원래 가진 정규화값이 아니기 때문에 정규화를 한번더 한다
	
	//float3 R = reflect(-normalize(In.E), N) ;//반사벡터
	float3 H = normalize(L+normalize(In.E)) ;
	float fDiffuse = 0.7f ;//-vLightDir.w ;//ambient
	
	float Incide = dot(N, H) ;
	float4 S = (specular*pow(max(0, dot(H, N)), 3)) ;

	return In.Color * tex2D(DecalMapSamp, In.TexDecal)
			* (max(0, dot(N, L))+fDiffuse)//diffuse + ambient
			+ S ;
			//+ (specular*pow(max(0, dot(H, N)), 3)) ;//specular 유광제질
			//+ (0.3f*pow(max(0, dot(H, N)), 3)) ;//specular 무광제질
}

//텍스쳐를 사용하지 않는 메쉬
VS_OUTPUT VS_pass1(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	Out.Color.x = 1 ;
	Out.Color.y = 1 ;
	Out.Color.z = 1 ;
	Out.Color.w = 1 ;
	return Out ;
}
float4 PS_pass1(VS_OUTPUT In) : COLOR
{
	return In.Color ;
}

//버텍스 블렌딩
VS_OUTPUT VS_blend(
float4 Pos : POSITION,
float3 Normal : NORMAL,
float2 TexDecal : TEXCOORD0,
float4 Blend : TEXCOORD1,
float4 Indices : TEXCOORD2)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//좌표변환
	//Out.Pos = mul(Pos, matWVP) ;

	float4x4 matSkin ;
	float4 pos ;
	float3 normal ;

	pos.xyzw = 0.0f ;
	normal.xyz = 0.0f ;

	//[0] 가중치 적용
	matSkin = mul(matWIT, amatSkin[(int)Indices.x]) ;
	pos += mul( mul( Pos, matSkin ), Blend.x) ;
	normal += mul( mul( Normal, (float3x3)matSkin), Blend.x) ;

	//[1] 가중치 적용
	if(Blend.y > 0.0f)
	{
		matSkin = mul(matWIT, amatSkin[(int)Indices.y]) ;
		pos += mul( mul( Pos, matSkin ), Blend.y) ;
		normal += mul( mul( Normal, (float3x3)matSkin), Blend.y) ;
	}
		
	//[2] 가중치 적용
	if(Blend.z > 0.0f)
	{
		matSkin = mul(matWIT, amatSkin[(int)Indices.z]) ;
		pos += mul( mul( Pos, matSkin ), Blend.z) ;
		normal += mul( mul( Normal, (float3x3)matSkin), Blend.z) ;
	}	
		
	//[3] 가중치 적용
	if(Blend.w > 0.0f)
	{
		matSkin = mul(matWIT, amatSkin[(int)Indices.w]) ;
		pos += mul( mul( Pos, matSkin ), Blend.w) ;
		normal += mul( mul( Normal, (float3x3)matSkin), Blend.w) ;
	}
	
	Out.Color = Lambert(Pos, Normal) ;
	Out.TexDecal = TexDecal ;

	return Out ;
}

VS_OUTPUTAMBIENTOCCLUSION VS_AO(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUTAMBIENTOCCLUSION Out = (VS_OUTPUTAMBIENTOCCLUSION)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = Lambert(Pos, Normal) ;
	Out.TexDecal = TexDecal ;

	//float4 Ambient = float4(AmbientDegree.x/3, AmbientDegree.x/3, AmbientDegree.x/3, AmbientDegree.x/3) ;
	//float4 Ambient = float4(AmbientDegree.x, AmbientDegree, AmbientDegree, 0) ;
	
	//Out.Color = float4(max(0, Out.Color.x+Ambient.x), max(0, Out.Color.y+Ambient.y), max(0, Out.Color.z+Ambient.z), max(0, Out.Color.w+Ambient.w)) ;
	//Out.Color = float4(max(0, Out.Color.x-Ambient.x), max(0, Out.Color.y-Ambient.y), max(0, Out.Color.z-Ambient.z), max(0, Out.Color.w-Ambient.w)) ;
	Out.Color = float4(AmbientDegree, AmbientDegree, AmbientDegree, 1) ;
	//Out.Color *= (Ambient*1.2f) ;

	return Out ;
}
float4 PS_AO(VS_OUTPUTAMBIENTOCCLUSION In) : COLOR
{
	//float4 Decal = tex2D(DecalMapSamp, In.TexDecal) ;
	//In.Color *= Decal ;	
	return In.Color ;
}

struct VS_OUTPUTIDSHADOW
{
	float4 Pos : POSITION ;
	float2 Depth : TEXCOORD0 ;
	float2 TexDecal : TEXCOORD1 ;
} ;

VS_OUTPUTIDSHADOW VS_IdShadow(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUTIDSHADOW Out = (VS_OUTPUTIDSHADOW)0 ;
	if(fElapsedTime > 0.0f)
	{
		Pos.x += sin(Pos.x * fFrequency + (fElapsedTime/fTimeScale)) * 2.0f * fAmplitude * (2.0f-TexDecal.x)/4.0f ;
		Pos.y += cos(Pos.y * fFrequency + (fElapsedTime/fTimeScale)) * 2.0f * fAmplitude * (2.0f-TexDecal.y)/8.0f ;
	}
	Out.Pos = mul(Pos, matWLP) ;
	Out.Depth.x = Out.Pos.z ;
	Out.Depth.y = Out.Pos.w ;
	Out.TexDecal = TexDecal ;
		
	return Out ;
}
void PS_IdShadow(VS_OUTPUTIDSHADOW In, out float4 Color : COLOR)
{
	//float4 color ;
	//color.x = anObjectID[0] ;
	//color.y = anObjectID[1] ;
	//color.z = In.Depth.x ;
	//color.w = In.Depth.y ;
	
	//color = (In.Depth.x-afViewPlanes[0])/(afViewPlanes[1]-afViewPlanes[0]) ;
	Color = In.Depth.x/In.Depth.y ;
	Color.w = tex2D(DecalMapSamp, In.TexDecal).w ;
}

struct VS_OUTPUTIDMAP
{
	float4 Pos : POSITION ;
	//float4 Color : COLOR0 ;
	//float2 TexDecal : TEXCOORD0 ;
	//float2 TexPos : TEXCOORD1 ;
	//float4 TexShadow : TEXCOORD2 ;
} ;

void VS_IdMap(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, out float4 oPos : POSITION)
{
	//좌표변환
	oPos = mul(Pos, matWVP) ;
	
	//Out.Color = Lambert(Pos, Normal) ;
	//float4 TexPos = mul(Pos, matWLP) ;
	//Out.TexPos = float2(TexPos.z, TexPos.w) ;
	//Out.TexShadow = mul(Pos, matWLPT) ;
	//Out.TexDecal = TexDecal ;
}

void PS_IdMap(out float4 Color : COLOR)
{
	//D3DFMT_R32F
	Color = nEigenIndex ;
	
	//D3DFMT_G16R16F
	//Color.ar = anObjectID[0] ;
	//Color.gb = anObjectID[0]+1 ;

	//float4 color ;
	//float4 zero = {0.01f, 0.01f, 0.01f, 0} ;
	//float epsilon = 1.0f/1024.0f ;
	//float offset = 1.0f/24.0f ;
	
	//float4 Decal = tex2D(DecalMapSamp, In.TexDecal) ;
	//float4 shadow = tex2Dproj(ShadowMapSamp, In.TexShadow) ;
	
	//if( (abs(In.TexPos.x-shadow.x) <= epsilon) && (abs(In.TexPos.y-shadow.y) <= epsilon) )
	//{
	//	color = ((In.TexPos.z-offset) > ((shadow.z/shadow.w)*In.TexPos.w)) ? zero : In.Color ;
	//}
	//else
	//	color = zero ;

	//return color*Decal ;
}

struct VS_OUTPUT_SUNLIT
{
	float4 Pos : POSITION ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 TexDecal : TEXCOORD0 ;
	float4 Lembert : TEXCOORD1 ;
	float4 texShadowUV : TEXCOORD3 ;
	float2 Depth : TEXCOORD4 ;
} ;

VS_OUTPUT_SUNLIT VS_twosidedSunLit(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUT_SUNLIT Out = (VS_OUTPUT_SUNLIT)0 ;

	//좌표변환
	if(fElapsedTime > 0.0f)
	{
		Pos.x += sin(Pos.x * fFrequency + (fElapsedTime/fTimeScale)) * 2.0f * fAmplitude * (2.0f-TexDecal.x)/4.0f ;
		Pos.y += cos(Pos.y * fFrequency + (fElapsedTime/fTimeScale)) * 2.0f * fAmplitude * (2.0f-TexDecal.y)/8.0f ;
	}
	Out.Pos = mul(Pos, matWVP) ;
	//Out.Color = Lambert(Pos, Normal) ;

	float3 eyeVector = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//float s = length(eyeVector) ;//*0.05f ;
	eyeVector = normalize(eyeVector) ;
	
	if(dot(eyeVector.xyz, Normal.xyz) < -0.25f)
		Normal = -Normal ;

	atmosphericLighting(eyeVector, vLightDir, Normal, vLightColor, fDistFromCamera, Out.vE, Out.vI) ;
	//Out.N = Normal ;
	//Out.N.xz = 0 ;
	//Out.N.y = 1 ;
	//Out.eyeVector.xyz = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//Out.eyeVector.w = AmbientDegree ;
	
	float3 H = normalize(vLightDir + eyeVector) ;
	Out.Lembert.xyz = max(0.01f, dot(Normal, vLightDir)*vDiffuse.x) ;// + pow(max(0.01f, dot(Normal, H)), 3) ) ;
	Out.Lembert.w = AmbientDegree ;
	
	//float2 texCoord = mul(TexDecal, matTexCoordAni) ;
	//Out.TexDecal = texCoord ;
	Out.TexDecal = TexDecal ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;
	float4 pos = mul(Pos, matWLP) ;
	Out.Depth.x = pos.z ;
	Out.Depth.y = pos.w ;

	return Out ;
}
float4 PS_twosidedSunLit(VS_OUTPUT_SUNLIT In) : COLOR
{
	float4 decal = tex2D(DecalMapSamp, In.TexDecal) ;
	
	/*	
	float directlight, specular ;
	float3 H = normalize(vLightDir + normalize(In.eyeVector.xyz)) ;
	float3 N = normalize(In.N) ;
	float3 ambient_color ;
		
	//decal.xyz = ( decal.xyz + (K_d*max(0.01f, dot(N, vLightDir))) + (specular*pow(max(0.01f, dot(N, H)), 10)) ) ;
	
	//decal.xyz = decal.xyz * (K_d*max(0.01f, dot(N, vLightDir))) ;
	
	directlight = max(0.01f, dot(N, vLightDir)*vDiffuse.x) ;
	specular = 0 ;// pow(max(0.01f, dot(N, H)), 10) ;

	ambient_color = decal.xyz*In.eyeVector.w ;

	//decal.x = saturate(decal.x * (directlight+specular)) ;
	//decal.y = saturate(decal.y * (directlight+specular)) ;
	//decal.z = saturate(decal.z * (directlight+specular)) ;

	decal.xyz = saturate(decal.xyz * (directlight+specular)) ;		
	decal.xyz = saturate(ambient_color + decal.xyz) ;
	*/
	
	float3 ambient_color = decal.xyz*In.Lembert.w ;
	decal.xyz = saturate(decal.xyz * In.Lembert.xyz) ;
	decal.xyz = saturate(ambient_color + decal.xyz + fAlphaTestDiffuse) ;

	/*
	//Shadow map
	float4 shadow = {1, 1, 1, 1} ;
	if(In.texShadowUV.x < 0.0f || In.texShadowUV.x > In.Depth.y || In.texShadowUV.y < 0.0f || In.texShadowUV.y > In.Depth.y || In.Depth.x < 0.0f || In.Depth.x > In.Depth.y)
		;
	else
	{
		float a, b=0, offset=1.0f/1024.0f ;
		float4 texShadow ;
		
		texShadow = In.texShadowUV ;
		texShadow.x -= offset ;
		texShadow.y -= offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x += offset ;
		texShadow.y -= offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x -= offset ;
		texShadow.y += offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x += offset ;
		texShadow.y += offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		shadow = b*0.25f ;
		shadow.w = 1.0f ;
	}
	*/
	
	float4 color = ((decal*In.vE)+In.vI) ;
	if(fAlphaBlendingByForce < 1.0f)
		color.w = fAlphaBlendingByForce ;
	
	return color ;

	//return ((decal*In.vE)+In.vI) ;// * shadow ;
	//return decal;//*shadow ; 알파블렌딩 되는 거는 그림자없이!
}

VS_OUTPUT_SUNLIT VS_SunLit(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUT_SUNLIT Out = (VS_OUTPUT_SUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	//Out.Lembert = Lambert(Pos, Normal) ;
	
	//float4 pos = mul(Pos, matWIT) ;
	float3 eyeVector = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//float s = length(eyeVector)*0.05f ;
	eyeVector = normalize(eyeVector) ;

	atmosphericLighting(eyeVector, vLightDir, Normal, vLightColor, fDistFromCamera, Out.vE, Out.vI) ;
	//atmosphericLighting_noNdL(eyeVector, vLightDir, vLightColor, s, Out.vE, Out.vI) ;
	
	//Out.Lembert = LambertwithLightColor(pos, Normal, vLightColor) ;
	//Out.N = Normal ;	
	//Out.eyeVector.xyz = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//Out.eyeVector.w = AmbientDegree ;
	
	float3 H = normalize(vLightDir + eyeVector) ;
	Out.Lembert.xyz = max(0.01f, dot(Normal, vLightDir)*vDiffuse.x) ;// + pow(max(0.01f, dot(Normal, H)), 3) ) ;
	Out.Lembert.w = AmbientDegree ;
	
	//float2 texCoord = mul(TexDecal, matTexCoordAni) ;
	//Out.TexDecal = texCoord ;
	Out.TexDecal = TexDecal ;
	
	Out.texShadowUV = mul(Pos, matWLPT) ;
	float4 pos = mul(Pos, matWLP) ;
	Out.Depth.x = pos.z ;
	Out.Depth.y = pos.w ;

	return Out ;
}

float4 PS_SunLit(VS_OUTPUT_SUNLIT In) : COLOR
{
	float4 decal = tex2D(DecalMapSamp, In.TexDecal) ;
	
	/*
	float directlight, specular ;
	float3 H = normalize(vLightDir + normalize(In.eyeVector.xyz)) ;
	float3 N = normalize(In.N) ;
	float3 ambient_color ;
		
	//decal.xyz = ( decal.xyz + (K_d*max(0.01f, dot(N, vLightDir))) + (specular*pow(max(0.01f, dot(N, H)), 15)) ) ;
	
	//decal.xyz = decal.xyz * (K_d*max(0.01f, dot(N, vLightDir))) ;
	
	directlight = max(0.01f, dot(N, vLightDir)*vDiffuse.x) ;
	specular = 0 ;// pow(max(0.01f, dot(N, H)), 10) ;
	
	//ambient_color = lerp(decal.xyz*In.eyeVector.w, 0, 0.5f) ;
	ambient_color = decal.xyz*In.eyeVector.w ;

	//decal.x = saturate(decal.x * (directlight+specular)) ;
	//decal.y = saturate(decal.y * (directlight+specular)) ;
	//decal.z = saturate(decal.z * (directlight+specular)) ;
	
	decal.xyz = saturate(decal.xyz * (directlight+specular)) ;
	decal.xyz = saturate(ambient_color + decal.xyz) ;

	//decal.xyz *= ( max(0.01f, dot(N, vLightDir)) + pow(max(0.01f, dot(N, H)), 7) ) ;
	//decal.xyz *= In.eyeVector.w ;

	//float3 ambient={0.01f, 0.01f, 0.01f} ;
	//decal.xyz = lerp(ambient, decal.xyz, directlight) ;
	//decal.xyz *= In.Color.xyz ;
	*/
	
	float3 ambient_color = decal.xyz*In.Lembert.w ;
	decal.xyz = saturate(decal.xyz * In.Lembert.xyz) ;
	decal.xyz = saturate(ambient_color + decal.xyz + fAlphaTestDiffuse) ;

	float4 shadow = {1, 1, 1, 1} ;
	if(In.texShadowUV.x < 0.0f || In.texShadowUV.x > In.Depth.y || In.texShadowUV.y < 0.0f || In.texShadowUV.y > In.Depth.y || In.Depth.x < 0.0f || In.Depth.x > In.Depth.y)
		;
	else
	{
		//float a, b ;
		//a = (In.Depth.x-afViewPlanes[0])/(afViewPlanes[1]-afViewPlanes[0]) ;
		//b = tex2Dproj(ShadowMapSamp, In.texShadowUV).x ;
		//shadow.xyz = (a>(b+epsilon)) ? 0.25f : 1.0f ;
		
		//shadow.xyz = ((In.Depth.x-afViewPlanes[0])>( (b*(afViewPlanes[1]-afViewPlanes[0]))+epsilon )) ? 0.25f : 1.0f ;
		
		//if( (((a+epsilon)>b) && ((a-epsilon)<b)) || (a>b) )
		//	shadow.xyz = 0.25f ;

		float4 a ;
		float b=0, offset=1.0f/1024.0f ;
		float4 texShadow ;
		
		texShadow = In.texShadowUV ;
		texShadow.x -= offset ;
		texShadow.y -= offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a.x*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x += offset ;
		texShadow.y -= offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a.x*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x -= offset ;
		texShadow.y += offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a.x*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		texShadow = In.texShadowUV ;
		texShadow.x += offset ;
		texShadow.y += offset ;
		a = tex2Dproj(ShadowMapSamp, texShadow) ;
		b += (In.Depth.x > ((a.x*In.Depth.y)+epsilon) ) ? 0.5f : 1 ;
		
		//epsilon=1.0f 라면 b에다가 1이 들어가서 shadow가 1이 됨.

		shadow = b*0.25f ;
		shadow.w = 1.0f ;

		//shadow = tex2Dproj(ShadowMapSamp, In.texShadowUV).x ;
		//shadow = (In.Depth.x > ((shadow*In.Depth.y)+epsilon) ) ? tex2Dproj(SrcShadowMapSamp, In.texShadowUV) : 1 ;
		//shadow.w = 1 ;
	}

	//decal.xyz = decal.xyz + In.Lembert.xyz ;
	return ((decal*In.vE)+In.vI)*shadow ;
	
	//return decal*shadow ;
}

struct VS_OUTPUT_NOLIGHTINGSUNLIT
{
	float4 Pos : POSITION ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 TexDecal : TEXCOORD0 ;
} ;
VS_OUTPUT_NOLIGHTINGSUNLIT VS_NoLightingSunLit(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float1 AmbientDegree : TEXCOORD1)
{
	VS_OUTPUT_NOLIGHTINGSUNLIT Out = (VS_OUTPUT_NOLIGHTINGSUNLIT)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	//Out.Color = Lambert(Pos, Normal) ;

	float4 pos = mul(Pos, matWIT) ;
	
	float3 eyeVector = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//float s = length(eyeVector) ;
	eyeVector = normalize(eyeVector) ;

	atmosphericLighting_noNdL(eyeVector, vLightDir, vLightColor, fDistFromCamera, Out.vE, Out.vI) ;
	//Out.vE.xyzw = 1 ;
	//Out.vI.xyzw = 0 ;

	Out.TexDecal = TexDecal ;

	return Out ;
}
float4 PS_NoLightingSunLit(VS_OUTPUT_NOLIGHTINGSUNLIT In) : COLOR
{
	float4 decal = tex2D(DecalMapSamp, In.TexDecal) ;
	decal.xyz += vDiffuse.xyz ;
	return (decal*In.vE)+In.vI ;
	//return tex2D(DecalMapSamp, In.TexDecal) ;
}
float4 PS_Sizzling(VS_OUTPUT_NOLIGHTINGSUNLIT In) : COLOR
{
	//float4 decal = tex2D(DecalMapSamp, In.TexDecal) ;
	//return (decal*In.vE)+In.vI ;
	//return tex2D(NoMipmapFilterMapSamp, In.TexDecal) ;
	float4 decal = tex2D(NoMipmapFilterMapSamp, In.TexDecal) ;
	decal.xyz += vDiffuse.xyz ;	
	return (decal*In.vE)+In.vI ;
}
struct VS_OUTPUT_BOLLBOARD
{
	float4 Pos : POSITION ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 TexDecal : TEXCOORD0 ;
} ;

VS_OUTPUT_BOLLBOARD VS_Billboard(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float2 Factor : TEXCOORD1)
{
	VS_OUTPUT_BOLLBOARD Out = (VS_OUTPUT_BOLLBOARD)0 ;

	//좌표변환
	float4 right={amatSkin[0]._11, amatSkin[0]._12, amatSkin[0]._13, amatSkin[0]._14} ;
	float4 up={amatSkin[0]._21, amatSkin[0]._22, amatSkin[0]._23, amatSkin[0]._24} ;
	float4 view_dir={amatSkin[0]._31, amatSkin[0]._32, amatSkin[0]._33, amatSkin[0]._34} ;
	float4 pos ;
	
	right.xyz = cross(view_dir.xyz, up.xyz) ;
	
	right = right*Factor.x ;
	up = up*Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	Out.Pos = mul(pos, matWVP) ;
	//Out.Pos = mul(Pos, matWVP) ;
	//Out.Color = Lambert(Pos, Normal) ;
	
	//pos = mul(Pos, matWIT) ;
	float3 eyeVector = (float3)(vCameraPos.xyz-Pos.xyz) ;
	//float s = length(eyeVector)*0.05f ;
	eyeVector = normalize(eyeVector) ;
	atmosphericLighting_noNdL(eyeVector, vLightDir, vLightColor, fDistFromCamera, Out.vE, Out.vI) ;

	Out.TexDecal = TexDecal ;

	return Out ;
}

float4 PS_Billboard(VS_OUTPUT_BOLLBOARD In) : COLOR
{
	float4 decal = tex2D(DecalMapSamp, In.TexDecal) ;
	decal.xyz += vDiffuse.xyz ;
	return (decal*In.vE)+In.vI ;
	//return decal ;
}
float4 PS_Billboard_NoMipmapFilter(VS_OUTPUT_BOLLBOARD In) : COLOR
{
	float4 decal = tex2D(NoMipmapFilterMapSamp, In.TexDecal) ;
	decal.xyz += vDiffuse.xyz ;
	return (decal*In.vE)+In.vI ;
	//return decal ;
}

VS_OUTPUTIDSHADOW VS_BillBoardShadow(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float2 Factor : TEXCOORD1)
{
	VS_OUTPUTIDSHADOW Out = (VS_OUTPUTIDSHADOW)0 ;

	//좌표변환
	float4 right={amatSkin[0]._11, amatSkin[0]._12, amatSkin[0]._13, amatSkin[0]._14} ;
	float4 up={amatSkin[0]._21, amatSkin[0]._22, amatSkin[0]._23, amatSkin[0]._24} ;
	float4 pos ;
	
	right = right*Factor.x ;
	up = up*Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	Out.Pos = mul(pos, matWLP) ;
	Out.Depth.x = Out.Pos.z ;
	Out.Depth.y = Out.Pos.w ;
	Out.TexDecal = TexDecal ;
	return Out ;
}
float4 PS_BillBoardShadow(VS_OUTPUTIDSHADOW In) : COLOR
{
	float4 color = In.Depth.x/In.Depth.y ;
	//color.w = tex2D(DecalMapSamp, In.TexDecal).w ;
	color.w = tex2D(NoMipmapFilterMapSamp, In.TexDecal).w ;
	return color ;
}

technique TShader
{
	pass P0//SHADER_useTex
	{
		//AlphaBlendEnable = true ;
		//SrcBlend = SrcAlpha ;
		//DestBlend = InvSrcAlpha ;
		
		AlphaTestEnable = true ;
		AlphaRef = 0x00000000 ;
		AlphaFunc = NotEqual ;
				
		//ColorOp[0] = Modulate ;
		//ColorArg0[0] = Texture ;
		//ColorArg1[0] = Diffuse ;
		
		//AlphaOp[0] = Modulate ;
		//AlphaArg0[0] = Texture ;
		//AlphaArg1[0] = Diffuse ;

		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
	pass P1//SHADER_withoutTex
	{
		VertexShader = compile vs_2_0 VS_pass1() ;
		PixelShader = compile ps_2_0 PS_pass1() ;
	}
	pass P2//SHADER_vsSkinning
	{
		VertexShader = compile vs_2_0 VS_blend() ;
		PixelShader = compile ps_2_0 PS() ;
	}
	pass P3//SHADER_bump
	{
		VertexShader = compile vs_2_0 VS_Bump() ;
		PixelShader = compile ps_2_0 PS_Bump() ;
	}
	pass P4//SHADER_useAmbient
	{
		AlphaTestEnable = true ;
		AlphaRef = 0x00000000 ;
		AlphaFunc = NotEqual ;
	
		VertexShader = compile vs_2_0 VS_AO() ;
		PixelShader = compile ps_2_0 PS_AO() ;
	}
	pass P5//SHADER_shadowID 그림자를 드리우기위해 깊이값을 랜더링
	{
		VertexShader = compile vs_2_0 VS_IdShadow() ;
		PixelShader = compile ps_2_0 PS_IdShadow() ;
	}
	pass P6//SHADER_IDMapping 사용하지 않음.
	{
		VertexShader = compile vs_2_0 VS_IdMap() ;
		PixelShader = compile ps_2_0 PS_IdMap() ;
	}
	pass P7//SHADER Light Scattering
	{
		VertexShader = compile vs_2_0 VS_SunLit() ;
		PixelShader = compile ps_2_0 PS_SunLit() ;
	}
	pass P8//SHADER No Lighting, only Scattering
	{
		VertexShader = compile vs_2_0 VS_NoLightingSunLit() ;
		PixelShader = compile ps_2_0 PS_NoLightingSunLit() ;
	}
	pass P9//SHADER Light Scattering two-sided
	{
		VertexShader = compile vs_2_0 VS_twosidedSunLit() ;
		PixelShader = compile ps_2_0 PS_twosidedSunLit() ;
	}
	pass P10//SHADER Billboard
	{
		VertexShader = compile vs_2_0 VS_Billboard() ;
		PixelShader = compile ps_2_0 PS_Billboard() ;
	}
	pass P11//SHADER Billboard no mipmapfilter
	{
		VertexShader = compile vs_2_0 VS_Billboard() ;
		PixelShader = compile ps_2_0 PS_Billboard_NoMipmapFilter() ;
	}
	pass P12//SHADER_shadowID Billboard
	{
		VertexShader = compile vs_2_0 VS_BillBoardShadow() ;
		PixelShader = compile ps_2_0 PS_BillBoardShadow() ;
	}
	pass P13//SHADER No Lighting, only Scattering, No MipmapFilitering
	{
		VertexShader = compile vs_2_0 VS_NoLightingSunLit() ;
		PixelShader = compile ps_2_0 PS_Sizzling() ;
	}
}