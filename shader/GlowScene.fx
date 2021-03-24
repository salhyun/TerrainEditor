float afMapSize[2] ;

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
texture OriginalMap;
sampler OriginalMapSamp = sampler_state
{
    Texture = <OriginalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

struct VS_OUTPUT_INTENSITY
{
	float4 Pos : POSITION ;
	float2 Tex : TEXCOORD0 ;
} ;

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
//강렬함 추출하기
//////////////////////////////////////////////////////////////////////
VS_OUTPUT_INTENSITY VS_Intensity(float4 Pos : POSITION, float2 Tex : TEXCOORD0)
{
	VS_OUTPUT_INTENSITY Out = (VS_OUTPUT_INTENSITY)0 ;
	Out.Pos = Pos ;
	Out.Tex = Tex ;
	return Out ;
}
float4 PS_Intensity(VS_OUTPUT_INTENSITY In) : COLOR
{
	float4 Color = tex2D(SrcMapSamp, In.Tex) ;
	
	float fIntensity = (Color.x+Color.y+Color.z)/3.0f ;
	Color.xyz = pow(fIntensity, 3) ;
	return Color ;
}

/////////////////////////////////////////////////////////////////////////////
//뭉기게 쉐이더
/////////////////////////////////////////////////////////////////////////////
VS_OUTPUT_QUAD VS_Soft(float4 Pos : POSITION, float4 Tex : TEXCOORD0)
{
	VS_OUTPUT_QUAD Out = (VS_OUTPUT_QUAD)0 ;

	Out.Pos = Pos ;
	
	Out.Tex0 = Tex ;//center
	Out.Tex1 = Tex+float2(0, -1.0f/afMapSize[1]) ;//up
	Out.Tex2 = Tex+float2(-1.0f/afMapSize[0], -1.0f/afMapSize[1]) ;//left-up
	Out.Tex3 = Tex+float2(-1.0f/afMapSize[0], 0) ;//left
	Out.Tex4 = Tex+float2(-1.0f/afMapSize[0], +1.0f/afMapSize[1]) ;//left-down
	
	/*
	Out.Tex0 = Tex+float2(-3.0f/fMapSize, -3.0f/fMapSize) ;
	Out.Tex1 = Tex+float2(-3.0f/fMapSize, -1.0f/fMapSize) ;
	Out.Tex2 = Tex+float2(-3.0f/fMapSize,  1.0f/fMapSize) ;
	Out.Tex3 = Tex+float2(-3.0f/fMapSize,  3.0f/fMapSize) ;

	Out.Tex4 = Tex+float2(-1.0f/fMapSize, -3.0f/fMapSize) ;
	Out.Tex5 = Tex+float2(-1.0f/fMapSize, -1.0f/fMapSize) ;
	Out.Tex6 = Tex+float2(-1.0f/fMapSize,  1.0f/fMapSize) ;
	Out.Tex7 = Tex+float2(-1.0f/fMapSize,  3.0f/fMapSize) ;
	*/
	
	return Out ;
}

float afWeightValue[] = { 0.0659699f, 0.256846f, 0.0659699f, 0.256846f, 1.0f, 0.256846f, 0.0659699f, 0.256846f, 0.0659699f} ;

//                        left-up     up        right-up   left      center  right    left-down     down    right-down
//float afWeightValue[] = {0.214936f, 0.463612f, 0.214936f, 0.463612f, 1.0f, 0.463612f, 0.214936f, 0.463612f, 0.214936f} ;
//                           0        1            2         3         4      5           6          7          8
float4 PS_Soft(VS_OUTPUT_QUAD In) : COLOR
{
	float4 Color ;
	//float2 dvu = float2(4.0f/256.0f, 0) ;
	//float2 dvu = float2(4.0f/fMapSize, 0) ;

	Color = (tex2D(SrcMapSamp, In.Tex0)*afWeightValue[4])//center
			+(tex2D(SrcMapSamp, In.Tex1)*afWeightValue[1])//up
			+(tex2D(SrcMapSamp, In.Tex2)*afWeightValue[0])//left-up
			+(tex2D(SrcMapSamp, In.Tex3)*afWeightValue[3])//left
			+(tex2D(SrcMapSamp, In.Tex4)*afWeightValue[6])//left-down
			
			+(tex2D(SrcMapSamp, In.Tex1+float2(+1.0f/afMapSize[0], 0))*afWeightValue[2])//right-up
			+(tex2D(SrcMapSamp, In.Tex3+float2(+2.0f/afMapSize[0], 0))*afWeightValue[5])//right
			+(tex2D(SrcMapSamp, In.Tex4+float2(+1.0f/afMapSize[0], 0))*afWeightValue[7])//down
			+(tex2D(SrcMapSamp, In.Tex4+float2(+2.0f/afMapSize[0], 0))*afWeightValue[8]) ;//right-down
			
	return Color/9 ;

	/*
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
	*/
}

///////////////////////////////////////////////////
//원본버퍼와 뭉개진버퍼를 합치기
///////////////////////////////////////////////////
VS_OUTPUT_INTENSITY VS_Modulate(float4 Pos : POSITION, float2 Tex : TEXCOORD0)
{
	VS_OUTPUT_INTENSITY Out = (VS_OUTPUT_INTENSITY)0 ;
	Out.Pos = Pos ;
	Out.Tex = Tex ;
	return Out ;
}
float4 PS_Modulate(VS_OUTPUT_INTENSITY In) : COLOR
{
	float4 SrcColor = tex2D(SrcMapSamp, In.Tex) ;
	float4 OriginalColor = tex2D(OriginalMapSamp, In.Tex) ;
	
	OriginalColor.xyz = saturate(OriginalColor.xyz+SrcColor.xyz) ;

	return OriginalColor ;
}

technique TShader
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS_Intensity() ;
		PixelShader = compile ps_2_0 PS_Intensity() ;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 VS_Soft() ;
		PixelShader = compile ps_2_0 PS_Soft() ;
	}
	pass P2
	{
		VertexShader = compile vs_2_0 VS_Modulate() ;
		PixelShader = compile ps_2_0 PS_Modulate() ;
	}
}