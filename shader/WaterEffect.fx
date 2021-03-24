//전역변수

float4x4 matWVP ;
float4x4 matWorld ;
float4 vDiffuse ;
float4 vSunDir ;
float4 vSunColor ;
float4 vLookAt ;
float4 vCameraPos ;
float4x4 matWLP ;
float4x4 matWLPT ;
float fScreenPosOffset ;
float fProjOffset ;
float fSunStrength ;
float fSunShininess ;

//4개의 각각 다른 시간, 규모를 가진 범프맵을 사용한다.
float4 avBumpMove[2] ;//각각의 방향으로 이동
float4 vTilingBumpTex ;
float fElapsedTime ;

float4x4 matLODWVP ;

//광원밝기
float4 I_a = {0.3f, 0.3f, 0.3f, 0.f} ; //ambient
float4 I_d = {0.7f, 0.7f, 0.7f, 0.f} ; //diffuse

//반사율
float4 K_a = {1.f, 1.f, 1.f, 1.f} ;//ambient
float4 K_d = {1.f, 1.f, 1.f, 1.f} ;//diffuse

//경면광
//float4 specular = {1.f, 1.f, 1.f, 1.f} ;
float4 specular = {0.5f, 0.5f, 0.5f, 1.f} ;
//float4 specular = {0.5f, 0.35f, 0.15f, 1.f} ;

texture DetailMap;
sampler DetailMapSamp = sampler_state
{
    Texture = <DetailMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = Wrap;
    AddressV = Wrap;
};
texture ReflectMap;
sampler ReflectMapSamp = sampler_state
{
    Texture = <ReflectMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = CLAMP;
    AddressV = CLAMP;
};
texture UnderWaterMap;
sampler UnderWaterMapSamp = sampler_state
{
    Texture = <UnderWaterMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = CLAMP;
    AddressV = CLAMP;
};
texture Ripple1NormalMap;
sampler Ripple1NormalMapSamp = sampler_state
{
	Texture = <Ripple1NormalMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = Wrap;
    AddressV = Wrap;
};
texture3D VolRipple1NormalMap;
sampler3D VolRipple1NormalMapSamp = sampler_state
{
	Texture = <VolRipple1NormalMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = WRAP;
    AddressV = WRAP;
    AddressW = WRAP;    
};

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTBUMP
{
	float4 Pos : POSITION ;
	float4 Color : COLOR0 ;
	float2 TexDecal : TEXCOORD0 ;
	float3 L : TEXCOORD1 ;
	float3 E : TEXCOORD2 ;
	float4 TexBump0 : TEXCOORD3 ;
	float4 TexBump1 : TEXCOORD4 ;
	float2 TexBump2 : TEXCOORD5 ;
	float3 ScreenPos : TEXCOORD6 ;
} ;

float4 Lambert(float4 Pos, float3 Normal)
{
	float4 Color ;
	float3 L = normalize((float3)(vSunDir)) ;//-normalize((float3)vLightDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0, dot(N, L)))
			+(specular*pow(max(0, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터
VS_OUTPUTBUMP VS_Lake(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUTBUMP Out = (VS_OUTPUTBUMP)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = vDiffuse ;
	Out.TexDecal = TexDecal ;

	Out.TexBump0.x = TexDecal.x*vTilingBumpTex.x+avBumpMove[0].x ;
	Out.TexBump0.y = TexDecal.y*vTilingBumpTex.x+avBumpMove[0].y ;
	
	Out.TexBump0.z = TexDecal.x*vTilingBumpTex.x+avBumpMove[0].z ;
	Out.TexBump0.w = TexDecal.y*vTilingBumpTex.x+avBumpMove[0].w ;
	
	Out.TexBump1.x = TexDecal.x*vTilingBumpTex.x+avBumpMove[1].x ;
	Out.TexBump1.y = TexDecal.y*vTilingBumpTex.x+avBumpMove[1].y ;
	
	Out.TexBump1.z = TexDecal.x*vTilingBumpTex.x+avBumpMove[1].z ;
	Out.TexBump1.w = TexDecal.y*vTilingBumpTex.x+avBumpMove[1].w ;
	
	Out.TexBump2 = TexDecal*vTilingBumpTex.y ;

	float4 pos = mul(float4(Pos.x, 0, Pos.z, 1), matLODWVP) ;
	Out.ScreenPos = pos.xyz/pos.w ;
	Out.ScreenPos.xy = (0.5+fProjOffset) + 0.5*Out.ScreenPos.xy*float2(1,-1);
	Out.ScreenPos.z = fScreenPosOffset/Out.ScreenPos.z;

	float3 N, T, B ;
	
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;
	
	pos = mul(Pos, matWorld) ;
	
	//transform tangent space
	float3 E = normalize((float3)(vCameraPos.xyz-pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;
	
	//transform tangent space
	//float3 L = normalize((float3)(vSunPos.xyz-Pos.xyz)) ;
	float3 L = vSunDir.xyz ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	
	return Out ;
}

float4 PS_Lake(VS_OUTPUTBUMP In) : COLOR
{
	float4 color ;
	float2 texBump ;
	float4 bump0, bump1, bump2, bump3 ;

	float3 vol_tex ;
	
	//vol_tex = float3(In.TexDecal.x, In.TexDecal.y, fElapsedTime) ;
	vol_tex = float3(In.TexBump2.x*0.25f, In.TexBump2.y*0.25f, fElapsedTime) ;
	bump0 = tex3D(VolRipple1NormalMapSamp, vol_tex) ;
	
	//vol_tex = float3(In.TexDecal.x*2.0f, In.TexDecal.y*2.0f, fElapsedTime+0.25f) ;
	vol_tex = float3(In.TexBump2.x*0.5f, In.TexBump2.y*0.5f, fElapsedTime+0.25f) ;
	bump1 = tex3D(VolRipple1NormalMapSamp, vol_tex) ;
	
	//vol_tex = float3(In.TexDecal.x*4.0f, In.TexDecal.y*4.0f, fElapsedTime+0.5f) ;
	vol_tex = float3(In.TexBump2.x*0.75f, In.TexBump2.y*0.75f, fElapsedTime+0.5f) ;
	bump2 = tex3D(VolRipple1NormalMapSamp, vol_tex) ;
	
	//vol_tex = float3(In.TexDecal.x*8.0f, In.TexDecal.y*8.0f, fElapsedTime+0.75f) ;
	vol_tex = float3(In.TexBump2.x, In.TexBump2.y, fElapsedTime+0.75f) ;
	bump3 = tex3D(VolRipple1NormalMapSamp, vol_tex) ;
		
	//bump0 = (bump0+bump1+bump2+bump3)*0.25f ;
	bump0 = (bump0+bump1)*0.5f ;

	float depth = tex2D(UnderWaterMapSamp, In.ScreenPos.xy).w ;//물가쪽일수록 factor가 작아져서 물가쪽결함을 막는다.

	float3 E = normalize(In.E) ;
	float3 L = normalize(In.L) ;//rasterizer 를 하게 되면 자동보간이 된다. 그래서 원래 가진 정규화값이 아니기 때문에 정규화를 한번더 한다
	//bump0 = (bump0+bump1+bump2+bump3)*0.25f ;
	float3 N = 2.0f*bump0.xyz-1.0f ;//법선맵은 -1 에서 +1 의 범위 그러나 현재 저장된 데이타는 0-1사이 값, *2를 하고 -1을 한다.

	//float3 R = reflect(-E, N) ;
	//float3 sunlight = fSunStrength*pow(saturate(dot(R, L)), fSunShininess)*float3(1.2, 1.2f, 1.2f) ;
	
	float4 ReflectColor = tex2D(ReflectMapSamp, In.ScreenPos.xy-(In.ScreenPos.z*N.yx)) ;
	float4 RefractColor = tex2D(UnderWaterMapSamp, In.ScreenPos.xy-(saturate(depth+0.5f)*In.ScreenPos.z*N.xy)) ;

	float3 H = normalize(L+E) ;
	float fDiffuse = 0.7f ;//-vLightDir.w ;//ambient
	float Incide = dot(N, H) ;
	float4 S = (specular*pow(max(0, dot(H, N)), 100)) ;
	
	float3 sunlight = fSunStrength*pow(saturate(dot(H, N)), fSunShininess)*float3(1.2, 1.2f, 1.2f) ;
	
	//detail map
	texBump.xy = In.TexBump0.zw ;
	float4 DetailColor = tex2D(DetailMapSamp, texBump+(N.xy*0.3f)) ;
	//float4 DetailColor = tex2D(DetailMapSamp, In.TexDecal+(N.xy*0.3f)) ;
	
	//the fresnel term (air-to-water)
	float fresnel = 1.0f-dot(E, N) ;
	fresnel = 0.2037f+0.97963f*pow(fresnel, 6) ;
	fresnel = fresnel*depth ;
	
	ReflectColor.xyz += sunlight.xyz ;
	color = lerp(RefractColor, ReflectColor, saturate(fresnel)) ;
	color = lerp(color, DetailColor, DetailColor.w) ;
	color.w = depth ;

	color = color * (max(0, dot(N, L))+float4(0.25f, 0.25f, 0.25f, 0.25f))//diffuse + ambient
			+ S ;

	//color = tex2D(UnderWaterMapSamp, In.TexUnderWater).wwww ;
	//color.w = 1 ;
			
	//color.xyz = tex2D(UnderWaterMapSamp, In.ScreenPos.xy).xyz ;
	//color.w = 1 ;

	//texBump.xy = In.TexBump1.zw ;
	//bump0 = tex2D(Ripple1NormalMapSamp, texBump) ;
	//N = 2.0f*bump0.xzy-1.0f ;
	
	//color = tex2D(UnderWaterMapSamp, In.ScreenPos.xy-(In.ScreenPos.z*N.xz)) ;
	//color = tex2D(UnderWaterMapSamp, In.ScreenPos.xy) ;

	//color = tex2D(ReflectMapSamp, In.ScreenPos.xy-(In.ScreenPos.z*N.zx)) ;
	//color = tex2D(ReflectMapSamp, In.ScreenPos.xy) ;
	
	//color.xyz = sunlight.xyz ;

	//return bump0 ;
	return color ;
}

VS_OUTPUTBUMP VS_River(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 TexDecal : TEXCOORD0)
{
	VS_OUTPUTBUMP Out = (VS_OUTPUTBUMP)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = vDiffuse ;
	Out.TexDecal = TexDecal ;

	Out.TexBump0.x = TexDecal.x*vTilingBumpTex.x+avBumpMove[0].x ;
	Out.TexBump0.y = TexDecal.y*vTilingBumpTex.x+avBumpMove[0].y ;
	
	Out.TexBump0.z = TexDecal.x*vTilingBumpTex.x+avBumpMove[0].z ;
	Out.TexBump0.w = TexDecal.y*vTilingBumpTex.x+avBumpMove[0].w ;
	
	Out.TexBump1.x = TexDecal.x*vTilingBumpTex.x+avBumpMove[1].x ;
	Out.TexBump1.y = TexDecal.y*vTilingBumpTex.x+avBumpMove[1].y ;
	
	Out.TexBump1.z = TexDecal.x*vTilingBumpTex.x+avBumpMove[1].z ;
	Out.TexBump1.w = TexDecal.y*vTilingBumpTex.x+avBumpMove[1].w ;
	
	//Out.TexBump2 = TexDecal*vTilingBumpTex.y ;

	float4 pos = mul(float4(Pos.x, 0, Pos.z, 1), matWVP) ;
	Out.ScreenPos = pos.xyz/pos.w ;
	Out.ScreenPos.xy = 0.5 + 0.5*Out.ScreenPos.xy*float2(1,-1);
	Out.ScreenPos.z = fScreenPosOffset/Out.ScreenPos.z;

	float3 N, T, B ;
	
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;
	
	pos = mul(Pos, matWorld) ;
	
	//transform tangent space
	float3 E = normalize((float3)(vCameraPos.xyz-pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;
	
	//transform tangent space
	//float3 L = normalize((float3)(vSunPos.xyz-Pos.xyz)) ;
	float3 L = vSunDir.xyz ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	
	return Out ;
}

float4 PS_River(VS_OUTPUTBUMP In) : COLOR
{
	float4 color ;
	float2 texBump ;
	float4 bump0, bump1, bump2, bump3 ;
	
	texBump.xy = In.TexBump0.xy ;
	bump0 = tex2D(Ripple1NormalMapSamp, texBump) ;
	texBump.xy = In.TexBump0.zw ;
	bump1 = tex2D(Ripple1NormalMapSamp, texBump) ;
	texBump.xy = In.TexBump1.xy ;
	bump2 = tex2D(Ripple1NormalMapSamp, texBump) ;
	texBump.xy = In.TexBump1.zw ;
	bump3 = tex2D(Ripple1NormalMapSamp, texBump) ;
	bump0 = (bump0+bump1+bump2+bump3)*0.25f ;

	float depth = tex2D(UnderWaterMapSamp, In.ScreenPos.xy).w ;//물가쪽일수록 factor가 작아져서 물가쪽결함을 막는다.

	float3 E = normalize(In.E) ;
	float3 L = normalize(In.L) ;//rasterizer 를 하게 되면 자동보간이 된다. 그래서 원래 가진 정규화값이 아니기 때문에 정규화를 한번더 한다
	//bump0 = (bump0+bump1+bump2+bump3)*0.25f ;
	float3 N = 2.0f*bump0.xyz-1.0f ;//법선맵은 -1 에서 +1 의 범위 그러나 현재 저장된 데이타는 0-1사이 값, *2를 하고 -1을 한다.

	//float3 R = reflect(-E, N) ;
	//float3 sunlight = fSunStrength*pow(saturate(dot(R, L)), fSunShininess)*float3(1.2, 1.2f, 1.2f) ;
	
	float4 ReflectColor = tex2D(ReflectMapSamp, In.ScreenPos.xy-(In.ScreenPos.z*N.yx)) ;
	float4 RefractColor = tex2D(UnderWaterMapSamp, In.ScreenPos.xy-(saturate(depth+0.5f)*In.ScreenPos.z*N.xy)) ;

	float3 H = normalize(L+E) ;
	float fDiffuse = 0.7f ;//-vLightDir.w ;//ambient
	float Incide = dot(N, H) ;
	float4 S = (specular*pow(max(0, dot(H, N)), 100)) ;
	
	float3 sunlight = fSunStrength*pow(saturate(dot(H, N)), fSunShininess)*float3(1.2, 1.2f, 1.2f) ;
	
	//detail map
	texBump.xy = In.TexBump0.zw ;
	float4 DetailColor = tex2D(DetailMapSamp, texBump+(N.xy*0.3f)) ;
	//float4 DetailColor = tex2D(DetailMapSamp, In.TexDecal+(N.xy*0.3f)) ;
	
	//the fresnel term (air-to-water)
	float fresnel = 1.0f-dot(E, N) ;
	fresnel = 0.2037f+0.97963f*pow(fresnel, 6) ;
	fresnel = fresnel*depth ;
	
	ReflectColor.xyz += sunlight.xyz ;
	color = lerp(RefractColor, ReflectColor, saturate(fresnel)) ;
	color = lerp(color, DetailColor, DetailColor.w) ;
	color.w = depth ;

	color = color * (max(0, dot(N, L))+float4(0.25f, 0.25f, 0.25f, 0.25f))//diffuse + ambient
			+ S ;
			
	return color ;
}

technique TShader
{
	pass P0//SHADER_River
	{
		VertexShader = compile vs_2_0 VS_River() ;
		PixelShader = compile ps_2_0 PS_River() ;
	}
	pass P1//SHADER_Lake
	{
		VertexShader = compile vs_2_0 VS_Lake() ;
		PixelShader = compile ps_2_0 PS_Lake() ;
	}
}