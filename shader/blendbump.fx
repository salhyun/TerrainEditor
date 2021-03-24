//전역변수

float4x4 matWVP ;
float4x4 matWIT ;
float4 vDiffuse = {1.f, 1.f, 1.f, 1.f} ;
float4 vLookAt ;
float4 vCameraPos ;

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

float4 sun_vec = {0.578f,0.578f,0.578f,0.0f} ;

int anParam[4] ;

texture BaseMap;
sampler BaseMapSamp = sampler_state
{
    Texture = <BaseMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;

    AddressU = wrap;
    AddressV = wrap;
};

texture BunkerMap;
sampler BunkerMapSamp = sampler_state
{
	Texture = <BunkerMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = wrap;
    AddressV = wrap;
};
texture BunkerAlphaMap;
sampler BunkerAlphaMapSamp = sampler_state
{
	Texture = <BunkerAlphaMap>;
	MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    
    AddressU = wrap;
    AddressV = wrap;
};

float4 Lambert(float4 Pos, float3 Normal)
{
	float4 Color ;
	float3 L = normalize((float3)vSunDir) ;
	float3 N = Normal ;
	float3 E = normalize(vCameraPos.xyz-Pos.xyz) ;
	float3 H = normalize(L+E) ;

	//max(0, dot(N, L))로 하면 Color가 0 이 되어버리고 그러면 알파테스트를 할경우에 테스트를 통과하지 못한다.
	//그래서 max(0.01f, dot(N, L))로 했지만 이거역시 수정될 가능성이 높기때문에 주시해야될 항목임.
	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
			+(specular*pow(max(0.01f, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

//정점셰이더 -> 픽셀셰이더 : 각 데이타들은 레스터라이저계산을 거쳐서 보간된 값으로 PixelShader로 넘어가게 된다.
struct VS_OUTPUTBUMP
{
	float4 Pos : POSITION ;
	float4 Color : COLOR0 ;
	float2 Tex0 : TEXCOORD0 ;
	float2 Tex1 : TEXCOORD1 ;
	//float3 E : TEXCOORD2 ;
} ;

//Render

//Vertex Shader
//Pos 로칼좌표계 기준의 정점좌표
//Normal 로칼좌표계 기준의 법선벡터

VS_OUTPUTBUMP VS_Bump(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 TexDiffuse : TEXCOORD0)
{
	VS_OUTPUTBUMP Out = (VS_OUTPUTBUMP)0 ;

	//좌표변환
	Out.Pos = mul(Pos, matWVP) ;
	
	Out.Color = vDiffuse ;
	
	//float4 vTexCoord = {TexDecale.x, TexDecale.y, 0.0f, 1.0f} ;
	//vTexCoord = mul(vTexCoord, matTexCoordAni) ;
	//Out.TexDecale = vTexCoord.xy ;
	Out.Tex0 = TexDiffuse*64 ;
	Out.Tex1 = TexDiffuse ;

	/*	
	float3 N, T, B ;
	N = Normal ;
	T = Tangent ;
	B = cross(N, T) ;

	float3 E = normalize((float3)(vCameraPos.xyz-Pos.xyz)) ;
	Out.E.x = dot(E, T) ;
	Out.E.y = dot(E, B) ;
	Out.E.z = dot(E, N) ;

	float3 L = normalize((float3)vSunDir) ;
	Out.L.x = dot(L, T) ;
	Out.L.y = dot(L, B) ;
	Out.L.z = dot(L, N) ;
	*/
	
	return Out ;
}

float4 PS_Bump(VS_OUTPUTBUMP In) : COLOR
{
	float4 f4Color, f4Base, f4Alpha = {0, 0, 0, 0} ;
	
	f4Color = f4Base = tex2D(BaseMapSamp, In.Tex0) ;
	
	f4Alpha = tex2D(BunkerAlphaMapSamp, In.Tex1) ;
	f4Color = (1-f4Alpha)*f4Color + f4Alpha*tex2D(BunkerMapSamp, In.Tex0) ;
	
	return f4Color ;

//	float3 N = 2.0f*tex2D(NormalMapSamp, In.TexDiffuse).xyz-1.0f ;//법선맵은 -1 에서 +1 의 범위 그러나 현재 저장된 데이타는 0-1사이 값, *2를 하고 -1을 한다.
//	float3 L = normalize(In.L) ;//rasterizer 를 하게 되면 자동보간이 된다. 그래서 원래 가진 정규화값이 아니기 때문에 정규화를 한번더 한다
	
	//float3 R = reflect(-normalize(In.E), N) ;//반사벡터
//	float3 H = normalize(L+normalize(In.E)) ;
//	float fDiffuse = 0.3f ;//-vLightDir.w ;//ambient
	
//	float Incide = dot(N, H) ;
//	float4 S = (0.1f*pow(max(0, dot(H, N)), 3)) ;

	//return In.Color*tex2D(DiffuseMapSamp, In.TexDiffuse) ;

//	return In.Color * tex2D(DiffuseMapSamp, In.TexDiffuse)
//			* (max(0, dot(N, L))+fDiffuse)//diffuse + ambient
//			+ S ;
			//+ (specular*pow(max(0, dot(H, N)), 3)) ;//specular 유광제질
			//+ (0.3f*pow(max(0, dot(H, N)), 3)) ;//specular 무광제질	
}

technique TShader
{
	pass P0//SHADER_bump
	{
		VertexShader = compile vs_2_0 VS_Bump() ;
		PixelShader = compile ps_2_0 PS_Bump() ;
	}
}