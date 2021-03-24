//��������

float4x4 matWVP ;
float4 vCameraPos ;

texture DecalMap;
sampler DecalMapSamp = sampler_state
{
    Texture = <DecalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Clamp;
    AddressV = Clamp;
};

//�������̴� -> �ȼ����̴� : �� ����Ÿ���� �����Ͷ���������� ���ļ� ������ ������ PixelShader�� �Ѿ�� �ȴ�.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float3 Tex0 : TEXCOORD0 ;
} ;

//Render

//Vertex Shader
//Pos ��Į��ǥ�� ������ ������ǥ
//Normal ��Į��ǥ�� ������ ��������

VS_OUTPUT VS(float4 Pos : POSITION, float3 TexDecal : TEXCOORD0)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;
	
	//��ǥ��ȯ
	Out.Pos = mul(Pos, matWVP) ;
	Out.Tex0 = TexDecal ;
	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float2 tex={In.Tex0.x, 	In.Tex0.y} ;
	float4 color = tex2D(DecalMapSamp, tex) ;
	color.w -= In.Tex0.z ;
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