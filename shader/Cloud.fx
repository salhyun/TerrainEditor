//전역변수

float4x4 matWVP ;
float4 vDiffuse ;
float4 vLookAt ;
float4 vCameraPos ;
float4x4 matLocal ;

float4 vCloudMove = {1.0f, 1.0f, 0.0f, 0.0f} ;

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

texture CloudLayerMap ;
sampler CloudLayerMapSamp = sampler_state
{
    Texture = <CloudLayerMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = wrap;        
    AddressV  = wrap;
    AddressW  = wrap;
};

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTCLOUD
{
	float4 Pos : POSITION ;
	float4 Color0 : COLOR0 ;
	float4 Color1 : COLOR1 ;
	float2 TexCloud0 : TEXCOORD1 ;
	float2 TexCloud1 : TEXCOORD2 ;
} ;

//Render
//Vertex Shader
VS_OUTPUTCLOUD VS_Cloud(float4 Pos : POSITION)
{
	VS_OUTPUTCLOUD Out = (VS_OUTPUTCLOUD)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	//float xx = saturate(dot(normalize(Pos.xyz), float3(vSunDir.xyz))) ;
	//Out.vI = float4(xx, xx, xx, 1.0f) ;	

	Out.TexCloud0 = Pos.xz+vCloudMove.zw ;
	Out.TexCloud1 = (Pos.xz+vCloudMove.xy)*1.7f ;
	
	Out.Color0 = vSunDir.y ;// float4(0.587f, 0.587f, 0.587f, 1.0f);// vSunDir.z ;
	//Out.Color1 = cos(0.7091f-(Pos.y+0.2791f)) ;//saturate(0.7f-(Pos.y+0.28f)) ;
	Out.Color1 = (0.9f-Pos.y) ;//칼라값이 1을 안넘는구만...

	return Out ;
}

float4 PS_Cloud(VS_OUTPUTCLOUD In) : COLOR
{
	float4 cloud0 = tex2D(CloudLayerMapSamp, In.TexCloud0) ;
	float4 cloud1 = tex2D(CloudLayerMapSamp, In.TexCloud1) ;
	
	float4 cloud_layer = (cloud0*cloud1.a)+In.Color1;// In.HorzColor.a ;
	
	return cloud_layer.a*(In.Color0) ;
	//return cloud0*cloud1.a ;
	//return In.vI+cloud_layer ;
}

technique TShader
{
	pass P0//SHADER_Cloud
	{
		// no culling
		CULLMODE = NONE;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;
		
		AlphaBlendEnable = true;
		//SrcBlend = srcAlpha;
		SrcBlend = srcColor;
		DestBlend = one;
		BlendOp = add;

		AlphaTestEnable = false;
	
		VertexShader = compile vs_2_0 VS_Cloud() ;
		PixelShader = compile ps_2_0 PS_Cloud() ;
	}
}