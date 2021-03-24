//��������

float4x4 matWVP ;
float4 vCameraPos ;
float4 vAxisY, vAxisX ;
float4x4 matWLP ;
float4x4 matWLPT ;

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
texture NoiseMap;
sampler NoiseMapSamp = sampler_state
{
    Texture = <NoiseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

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

//�������̴� -> �ȼ����̴� : �� ����Ÿ���� �����Ͷ���������� ���ļ� ������ ������ PixelShader�� �Ѿ�� �ȴ�.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float2 Tex0 : TEXCOORD0 ;
	float4 texShadowUV : TEXCOORD1 ;
	float2 Depth : TEXCOORD2 ;
} ;

//Render

//Vertex Shader
//Pos ��Į��ǥ�� ������ ������ǥ
//Normal ��Į��ǥ�� ������ ��������

VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, float2 TexDecal : TEXCOORD0, float2 Factor : TEXCOORD1, float4 Reserved : TEXCOORD2)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;
	
	//float3 up = {0, 1, 0}, right = normalize(vCameraPos.xyz-Pos.xyz) ;
	//right = normalize(cross(right, up)) ;
	
	//Directional billboard
	//float3 up = {0, 1, 0}, right ;
	//right = vAxisX.xyz ;
	//up = vAxisY.xyz ;
	
	//position billboard
	float3 up = {0, 1, 0}, right, view = normalize(Pos.xyz-vCameraPos.xyz) ;
	right = normalize(cross(up, view)) ;
	up = normalize(cross(view, right)) ;

	float4 pos ;

	right *= Factor.x ;
	up *= Factor.y ;
	
	pos.xyz = Pos.xyz+right.xyz+up.xyz ;
	pos.w = Pos.w ;

	//��ǥ��ȯ
	Out.Pos = mul(pos, matWVP) ;
	
	Out.Tex0 = TexDecal ;
	
	//float4 p = mul(pos, matWLP) ;
	//Out.Depth.x = p.z ;
	//Out.Depth.y = p.w ;
	Out.Depth.x = Reserved.w ;
	
	Out.texShadowUV = mul(pos, matWLPT) ;

	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float shadow = tex2Dproj(SrcShadowMapSamp, In.texShadowUV).x ;
	shadow = (shadow <= 0.5f) ? 0.5f : shadow ;

	float4 color = tex2D(DecalMapSamp, In.Tex0) ;//*tex2D(NoiseMapSamp, In.Tex0).w ;
	color.xyz *= shadow ;
	color.w *= In.Depth.x ;

	return color ;
}

technique TShader
{
	pass P0//SHADER
	{
		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
}