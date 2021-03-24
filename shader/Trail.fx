//전역변수

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

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUT
{
	float4 Pos : POSITION ;
	float3 Tex0 : TEXCOORD0 ;
} ;

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터

VS_OUTPUT VS(float4 Pos : POSITION, float3 TexDecal : TEXCOORD0)
{
	VS_OUTPUT Out = (VS_OUTPUT)0 ;
	
	//좌표변환
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