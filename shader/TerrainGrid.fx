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

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float2 Tex0 : TEXCOORD0 ;
	float2 Tex1 : TEXCOORD1 ;
	float4 Diffuse : TEXCOORD2 ;
} ;

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터

VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float4 Factor : TEXCOORD1, float3 Diffuse : TEXCOORD2)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;

	//position billboard
	float3 up, right ;
	
	right = Normal ;//Normal이 right인것임
	up = cross(right, vLookat.xyz) ;
	up = normalize(up) ;

	float4 pos ;

	right *= Factor.x ;
	up *= Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	//좌표변환
	Out.Pos = mul(pos, matWVP) ;
	
	Out.Tex0 = TexDecal ;
	
	Out.Tex1.x = TexDecal.x + Factor.z ;
	Out.Tex1.y = TexDecal.y ;
	
	Out.Diffuse.x = Diffuse.x ;
	Out.Diffuse.y = Diffuse.y ;
	Out.Diffuse.z = Diffuse.z ;
	Out.Diffuse.w = Factor.w ;

	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float4 color ;
	float3 rod ;
	float rodAlpha ;

	rodAlpha = tex2D(DecalMap01Samp, In.Tex0).w ;
	rod = In.Diffuse.xyz * rodAlpha ;
	
	if(In.Diffuse.w <= 0.0001f)//x-axis 
		color = tex2D(DecalMap02Samp, In.Tex1) ;
	else
		color = tex2D(DecalMap03Samp, In.Tex1) ;

	color.xyz = In.Diffuse.xyz+color.xyz ;
	
	color.w = (color.w > rodAlpha) ? color.w : rodAlpha ;

	return color ;
}

technique TShader
{
	pass P0//SHADER
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;
		
		ZEnable = false ;

		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
}