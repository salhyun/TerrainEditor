//��������

float4x4 matWVP ;
float4x4 matWIT ;
float4 vDiffuse = {1.f, 1.f, 1.f, 1.f} ;
float4 vLookAt ;
float4 vCameraPos ;

//�������
float4 I_a = {0.3f, 0.3f, 0.3f, 0.f} ; //ambient
float4 I_d = {0.7f, 0.7f, 0.7f, 0.f} ; //diffuse

//�ݻ���
float4 K_a = {1.f, 1.f, 1.f, 1.f} ;//ambient
float4 K_d = {1.f, 1.f, 1.f, 1.f} ;//diffuse

//��鱤
float4 specular = {1.f, 1.f, 1.f, 1.f} ;
//float4 specular = {0.5f, 0.35f, 0.15f, 1.f} ;

float4 vSunDir = {0.578f,0.578f,0.578f,0.0f} ;
float4 vSunColor = {0.578f,0.578f,0.578f,1.0f} ;

texture DecalMap;
sampler DecalMapSamp = sampler_state
{
    Texture = <DecalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = wrap;
    AddressV = wrap;
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

float4 Lambert(float4 Pos, float3 Normal)
{
	float4 Color ;
	float3 L = normalize((float3)vSunDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	//max(0, dot(N, L))�� �ϸ� Color�� 0 �� �Ǿ������ �׷��� �����׽�Ʈ�� �Ұ�쿡 �׽�Ʈ�� ������� ���Ѵ�.
	//�׷��� max(0.01f, dot(N, L))�� ������ �̰ſ��� ������ ���ɼ��� ���⶧���� �ֽ��ؾߵ� �׸���.
	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
			+(specular*pow(max(0.01f, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

//�������̴� -> �ȼ����̴� : �� ����Ÿ���� �����Ͷ���������� ���ļ� ������ ������ PixelShader�� �Ѿ�� �ȴ�.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	//float4 Color : COLOR0 ;
	float4 vE : COLOR0 ;
	float4 vI : COLOR1 ;
	float2 Tex0 : TEXCOORD0 ;
	//float2 Tex1 : TEXCOORD1 ;
} ;

//Render

//Vertex Shader
//Pos ��Į��ǥ�� ������ ������ǥ
//Normal ��Į��ǥ�� ������ ��������

VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//��ǥ��ȯ
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = vDiffuse ;
	
	//float4 vTexCoord = {TexDecale.x, TexDecale.y, 0.0f, 1.0f} ;
	//vTexCoord = mul(vTexCoord, matTexCoordAni) ;
	//Out.TexDecale = vTexCoord.xy ;
	Out.Tex0 = TexDecal ;

	/*	
	float3 N, T, B ;
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;

	float3 E = normalize((float3)(vCameraPos.xyz-Pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;

	float3 L = normalize((float3)vSunDir) ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	*/
	
	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	return (tex2D(DecalMapSamp, In.Tex0)*In.vE)+In.vI ;

//	float3 N = 2.0f*tex2D(NormalMapSamp, In.TexDiffuse).xyz-1.0f ;//�������� -1 ���� +1 �� ���� �׷��� ���� ����� ����Ÿ�� 0-1���� ��, *2�� �ϰ� -1�� �Ѵ�.
//	float3 L = normalize(In.L) ;//rasterizer �� �ϰ� �Ǹ� �ڵ������� �ȴ�. �׷��� ���� ���� ����ȭ���� �ƴϱ� ������ ����ȭ�� �ѹ��� �Ѵ�
	
	//float3 R = reflect(-normalize(In.E), N) ;//�ݻ纤��
//	float3 H = normalize(L+normalize(In.E)) ;
//	float fDiffuse = 0.3f ;//-vLightDir.w ;//ambient
	
//	float Incide = dot(N, H) ;
//	float4 S = (0.1f*pow(max(0, dot(H, N)), 3)) ;

	//return In.Color*tex2D(DiffuseMapSamp, In.TexDiffuse) ;

//	return In.Color * tex2D(DiffuseMapSamp, In.TexDiffuse)
//			* (max(0, dot(N, L))+fDiffuse)//diffuse + ambient
//			+ S ;
			//+ (specular*pow(max(0, dot(H, N)), 3)) ;//specular ��������
			//+ (0.3f*pow(max(0, dot(H, N)), 3)) ;//specular ��������	
}

technique TShader
{
	pass P0//SHADER
	{
		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
}