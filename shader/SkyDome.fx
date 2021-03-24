//전역변수

float4x4 matWVP ;
float4 vDiffuse ;
float4 vLookAt ;
float4 vCameraPos ;
float4 vTexAni ;
float4 vSunToCamera ;

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
float4 vSunColor = {0.578f,0.578f,0.578f,1.0f} ;

texture SkyDomeMap;
sampler SkyDomeMapSamp = sampler_state
{
	Texture = <SkyDomeMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = Wrap;
    AddressV = Wrap;
};

texture SunMap;
sampler SunMapSamp = sampler_state
{
	Texture = <SunMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = Wrap;
    AddressV = Wrap;
};


//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTSKYDOME
{
	float4 Pos : POSITION ;
	float2 TexDecal : TEXCOORD0 ;
} ;

//Render

//Vertex Shader
VS_OUTPUTSKYDOME VS_SkyDome(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUTSKYDOME Out = (VS_OUTPUTSKYDOME)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	//Out.TexDecal = TexDecal ;

	Out.TexDecal.x = TexDecal.x+vTexAni.x ;
	Out.TexDecal.y = TexDecal.y ;	
	
	return Out ;
}

float4 PS_SkyDome(VS_OUTPUTSKYDOME In) : COLOR
{
	return tex2D(SkyDomeMapSamp, In.TexDecal) ;
}

VS_OUTPUTSKYDOME VS_Sun(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float2 Factor : TEXCOORD1)
{
	VS_OUTPUTSKYDOME Out = (VS_OUTPUTSKYDOME)0 ;

	//position billboard
	float3 up = {0, 1, 0}, right ;
	right = cross(vSunToCamera.xyz, up) ;
	up = cross(right, vSunToCamera.xyz) ;

	float4 pos ;

	right *= Factor.x ;
	up *= Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	//좌표변환
	Out.Pos = mul(pos, matWVP) ;
	Out.TexDecal = TexDecal ;

	return Out ;
}

float4 PS_Sun(VS_OUTPUTSKYDOME In) : COLOR
{
	return tex2D(SunMapSamp, In.TexDecal) ;
}

technique TShader
{
	pass P0//SHADER_SkyDome
	{	
		// no culling
		CULLMODE = NONE;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;
	
		VertexShader = compile vs_2_0 VS_SkyDome() ;
		PixelShader = compile ps_2_0 PS_SkyDome() ;
	}
	
	pass P1//SHADER_Sun
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		// no culling
		CULLMODE = NONE;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;
	
		VertexShader = compile vs_2_0 VS_Sun() ;
		PixelShader = compile ps_2_0 PS_Sun() ;
	}
}