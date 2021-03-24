float fMapSize=1024.0f ;

texture SrcMap;
sampler SrcMapSamp = sampler_state
{
    Texture = <SrcMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

struct VS_OUTPUT_QUAD
{
	float4 Pos : POSITION ;
	float2 Tex0 : TEXCOORD0 ;
	float2 Tex1 : TEXCOORD1 ;
	float2 Tex2 : TEXCOORD2 ;
	float2 Tex3 : TEXCOORD3 ;
	float2 Tex4 : TEXCOORD4 ;
	float2 Tex5 : TEXCOORD5 ;
	float2 Tex6 : TEXCOORD6 ;
	float2 Tex7 : TEXCOORD7 ;
} ;

//////////////////////////////////////////////////////////////////////
//¿§Áö¸¦ ¸¸µé±â À§ÇÑ ½¦ÀÌ´õ
//////////////////////////////////////////////////////////////////////
VS_OUTPUT_QUAD VS_Edge(float4 Pos : POSITION, float2 Tex : TEXCOORD0)
{
	VS_OUTPUT_QUAD Out = (VS_OUTPUT_QUAD)0 ;

	Out.Pos = Pos ;
	float offset=4.0f ;

	Out.Tex0 = Tex+float2(-offset/fMapSize, -offset/fMapSize) ;
	Out.Tex1 = Tex+float2(+offset/fMapSize, +offset/fMapSize) ;
	Out.Tex2 = Tex+float2(-offset/fMapSize, +offset/fMapSize) ;
	Out.Tex3 = Tex+float2(+offset/fMapSize, -offset/fMapSize) ;

	return Out ;
}

float4 PS_Edge(VS_OUTPUT_QUAD In) : COLOR
{
	float4 Color ;

	float d0 = tex2D(SrcMapSamp, In.Tex0)-tex2D(SrcMapSamp, In.Tex1) ;
	float d1 = tex2D(SrcMapSamp, In.Tex2)-tex2D(SrcMapSamp, In.Tex3) ;

	Color = abs(pow(d0, 1))+abs(pow(d1, 1)) ;
	return Color ;
}

/////////////////////////////////////////////////////////////////////////////
//¿§Áö ¹¶±â°Ô ½¦ÀÌ´õ
/////////////////////////////////////////////////////////////////////////////
VS_OUTPUT_QUAD VS_Soft(float4 Pos : POSITION, float4 Tex : TEXCOORD0)
{
	VS_OUTPUT_QUAD Out = (VS_OUTPUT_QUAD)0 ;

	Out.Pos = Pos ;

	Out.Tex0 = Tex+float2(-3.0f/fMapSize, -3.0f/fMapSize) ;
	Out.Tex1 = Tex+float2(-3.0f/fMapSize, -1.0f/fMapSize) ;
	Out.Tex2 = Tex+float2(-3.0f/fMapSize,  1.0f/fMapSize) ;
	Out.Tex3 = Tex+float2(-3.0f/fMapSize,  3.0f/fMapSize) ;

	Out.Tex4 = Tex+float2(-1.0f/fMapSize, -3.0f/fMapSize) ;
	Out.Tex5 = Tex+float2(-1.0f/fMapSize, -1.0f/fMapSize) ;
	Out.Tex6 = Tex+float2(-1.0f/fMapSize,  1.0f/fMapSize) ;
	Out.Tex7 = Tex+float2(-1.0f/fMapSize,  3.0f/fMapSize) ;
	return Out ;
}

float4 PS_Soft(VS_OUTPUT_QUAD In) : COLOR
{
	float4 Color ;
	//float2 dvu = float2(4.0f/256.0f, 0) ;
	float2 dvu = float2(4.0f/fMapSize, 0) ;

	Color = tex2D(SrcMapSamp, In.Tex0)
			+tex2D(SrcMapSamp, In.Tex1)
			+tex2D(SrcMapSamp, In.Tex2)
			+tex2D(SrcMapSamp, In.Tex3)
			+tex2D(SrcMapSamp, In.Tex4)
			+tex2D(SrcMapSamp, In.Tex5)
			+tex2D(SrcMapSamp, In.Tex6)
			+tex2D(SrcMapSamp, In.Tex7)
			
			+tex2D(SrcMapSamp, In.Tex0+dvu)
			+tex2D(SrcMapSamp, In.Tex1+dvu)
			+tex2D(SrcMapSamp, In.Tex2+dvu)
			+tex2D(SrcMapSamp, In.Tex3+dvu)
			+tex2D(SrcMapSamp, In.Tex4+dvu)
			+tex2D(SrcMapSamp, In.Tex5+dvu)
			+tex2D(SrcMapSamp, In.Tex6+dvu)
			+tex2D(SrcMapSamp, In.Tex7+dvu) ;
					
	return Color/14.0f ;
}

technique TShader
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Edge() ;
		PixelShader = compile ps_2_0 PS_Edge() ;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 VS_Soft() ;
		PixelShader = compile ps_2_0 PS_Soft() ;
	}
}