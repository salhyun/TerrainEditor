//전역변수

float4x4 matWVP ;
float4 vDiffuse ;
float4 vLookAt ;
float4 vCameraPos ;
float4 vSunDir ;
float4 vSunColor ;

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
	float cosTheta = dot(eyeVector.rgb, sunVector.rgb);

	// compute extinction term vExt
	// -(beta_1+beta_2) * s * log_2 e
	//vExt = -atm.vSumBeta1Beta2 * s * atm.vConstants.y;
	//float temp = -0.0025f*s ;
	//vExt = float4(temp, temp, temp, 1.0f) ;
	//vExt = -atm.vtemp*s ;
	
	//float4 vSumBeta1Beta2 = {0.000475698f, 0.000781343f, 0.0015719f, 0.0114668f};
	//vSumBeta1Beta2 /= 10.0f ;
	//vExt = -(vSumBeta1Beta2) * s * atm.vConstants.y;
	vExt = -(atm.vSumBeta1Beta2) * s * atm.vConstants.y;
	
	vExt.x = exp(vExt.x);
	vExt.y = exp(vExt.y);
	vExt.z = exp(vExt.z);
	//vExt.w = 0.0f;

	// Compute theta terms used by inscattering.
	// compute phase2 theta as
    // (1-g^2)/(1+g-2g*cos(theta))^(3/2)
	// atm.vHG = [1-g^2, 1+g, 2g]
	float p1Theta = (cosTheta*cosTheta)+atm.vConstants.x;
	
	//float p2Theta = (1.96f*cosTheta)+1.98f;
	//p2Theta = 1.0f/(sqrt(p2Theta));
	//p2Theta = (p2Theta*p2Theta*p2Theta) * 0.0396f;
	
	float p2Theta = (atm.vHG.z*cosTheta)+atm.vHG.y;
	p2Theta = 1.0f/(sqrt(p2Theta));
	p2Theta = (p2Theta*p2Theta*p2Theta) * atm.vHG.x;


	// compute inscattering (vIns) as
	// (vBetaD1*p1Theta + vBetaD1*p2Theta) *
	// (1-vExt) * atm.vRcpSumBeta1Beta2
	// 
	// atm.vRcpSumBeta1Beta2 = 
	// 1.0f/ (Rayleigh+Mie)
	//float4 vBetaD1={2.49649e-005f, 4.22167e-005f, 8.75406e-005f, 0.6f};
	//float4 vBetaD2={1.33379e-005f, 1.73466e-005f, 2.49762e-005f, 0.01f};
	//나누는 숫자를 높이면 잿빛하늘이 되고 낮추면 파란색이 많이 들어감.
	//vBetaD1 /= 10.0f ;
	//vBetaD2 /= 10.0f ;

	//vIns = (vBetaD1*p1Theta)+(vBetaD2*p2Theta);
	vIns = (atm.vBetaD1*p1Theta)+(atm.vBetaD2*p2Theta);
	
	//float4 Rcp={2102.18, 1279.85, 636.172, 0};
	//Rcp *= 15.0f ;//곱하는 숫자를 낮추면 태양이 되는 부분의 크기가 작아짐. 원래 15.0f
	//vIns = vIns*(1.0f-vExt)*Rcp;
	vIns = vIns*(1.0f-vExt)*atm.vRcpSumBeta1Beta2;
	
	//vIns = ((atm.vBeta1*p1Theta)
	//		+(atm.vBeta2*p2Theta))
	//		*(atm.vConstants.x-vExt)
	//		*atm.vRcpSumBeta1Beta2;

	// scale inscatter and extinction 
	// for effect (optional)
	vIns = saturate(vIns*atm.vTermMultipliers.x);
	// scale extinction prior to its use
	// (this is optional)
	//vExt = vExt*atm.vTermMultipliers.y*atm.vSoilReflectivity;
	vExt = saturate(vExt*atm.vTermMultipliers.y);

	// reduce inscattering on unlit surfaces
	// by modulating with a monochrome
	// Lambertian scalar. This is slightly
	// offset to allow some inscattering to
	// bleed into unlit areas
	//float NdL = dot(norm, sunVector);
	//vIns= vIns*NdL;

	// apply sunlight color
	// and strength to each term
	// and output
	float4 vsun_color={0.578f, 0.578f, 0.578f, 1.0f};
	vsun_color *= 10.0f ;

	//vIns.xyz = saturate(vIns)*(vsun_color*vsun_color.w);
	//vIns.w = 1.0f;
	//vExt = vExt*vsun_color*vsun_color.w;//*(1-falloff);	
	
	vIns.xyz = saturate(vIns)*sunColor*sunColor.w;
	vIns.w = 1.0f;

	vExt.xyz = vExt*sunColor*sunColor.w;
	vExt.w = 1.0f;
}

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTSKY
{
	float4 Pos : POSITION ;
	float4 vI : COLOR0 ;
	float4 vE : COLOR1 ;
} ;

//Render
//Vertex Shader
VS_OUTPUTSKY VS_Sky(float4 Pos : POSITION)
{
	VS_OUTPUTSKY Out = (VS_OUTPUTSKY)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;

	atmosphericLighting(normalize((float3)Pos.xyz), -vSunDir, vSunColor, 3000.0f, Out.vE, Out.vI) ;

	return Out ;
}

float4 PS_Sky(VS_OUTPUTSKY In) : COLOR
{
	return In.vI ;
}

technique TShader
{
	pass P0//SHADER_Sky
	{
		// no culling
		CULLMODE = NONE;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;
		
		StencilEnable = true;
		StencilFunc = always;
		StencilPass = replace;
		StencilRef = 0;

		AlphaBlendEnable = false;
		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 VS_Sky() ;
		PixelShader = compile ps_2_0 PS_Sky() ;
	}
}