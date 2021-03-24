//��������

float4x4 matWVP ;
float3 vLookat ;
float fElapsedTime ;

texture3D VolBillboardMap;
sampler3D VolBillboardMapSamp = sampler_state
{
	Texture = <VolBillboardMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;    
};

//�������̴� -> �ȼ����̴� : �� ����Ÿ���� �����Ͷ���������� ���ļ� ������ ������ PixelShader�� �Ѿ�� �ȴ�.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float2 Tex0 : TEXCOORD0 ;
} ;

VS_OUTPUT VS(float4 Pos : POSITION, float2 TexDecal : TEXCOORD0, float2 Factor : TEXCOORD1)
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

	//��ǥ��ȯ
	Out.Pos = mul(pos, matWVP) ;
	Out.Tex0 = TexDecal ;

	return Out ;
}

float4 PS(VS_OUTPUT In) : COLOR
{
	float3 tex = float3(In.Tex0.x, In.Tex0.y, fElapsedTime) ;
	return tex3D(VolBillboardMapSamp, tex) ;
}

technique TShader
{
	pass P0//SHADER
	{
		AlphaBlendEnable = true ;
		SrcBlend = SrcAlpha ;
		DestBlend = InvSrcAlpha ;

		VertexShader = compile vs_2_0 VS() ;
		PixelShader = compile ps_2_0 PS() ;
	}
}