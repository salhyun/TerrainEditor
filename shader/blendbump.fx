//��������

float4x4 matWVP ;
float4x4 matWIT ;
float4 vDiffuse = {1.f, 1.f, 1.f, 1.f} ;
float4 vLookAt ;
float4 vCameraPos ;

//�������
float4 I_a = {0.3f, 0.3f, 0.3f, 0.f} ; //ambient
float4 I_d = {0.7f, 0.7f, 0.7f, 0.f} ; //diffuse

//�ݻ���
float4 K_a = {1.f, 1.f, 1.f, 1.f} ;//ambient
float4 K_d = {1.f, 1.f, 1.f, 1.f} ;//diffuse

//��鱤
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

	//max(0, dot(N, L))�� �ϸ� Color�� 0 �� �Ǿ������ �׷��� �����׽�Ʈ�� �Ұ�쿡 �׽�Ʈ�� ������� ���Ѵ�.
	//�׷��� max(0.01f, dot(N, L))�� ������ �̰ſ��� ������ ���ɼ��� ���⶧���� �ֽ��ؾߵ� �׸���.
	Color = (I_a*K_a)
	    	+(vDiffuse*K_d*max(0.01f, dot(N, L)))
			+(specular*pow(max(0.01f, dot(N, H)), 10)) ;//Blinn-Phong HalfVector
				
	return Color ;
}

//�������̴� -> �ȼ����̴� : �� ����Ÿ���� �����Ͷ���������� ���ļ� ������ ������ PixelShader�� �Ѿ�� �ȴ�.
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
//Pos ��Į��ǥ�� ������ ������ǥ
//Normal ��Į��ǥ�� ������ ��������

VS_OUTPUTBUMP VS_Bump(float4 Pos : POSITION, float3 Normal : NORMAL, float3 Tangent :TANGENT, float2 TexDiffuse : TEXCOORD0)
{
	VS_OUTPUTBUMP Out = (VS_OUTPUTBUMP)0 ;

	//��ǥ��ȯ
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

//	float3 N = 2.0f*tex2D(NormalMapSamp, In.TexDiffuse).xyz-1.0f ;//�������� -1 ���� +1 �� ���� �׷��� ���� ����� ����Ÿ�� 0-1���� ��, *2�� �ϰ� -1�� �Ѵ�.
//	float3 L = normalize(In.L) ;//rasterizer �� �ϰ� �Ǹ� �ڵ������� �ȴ�. �׷��� ���� ���� ����ȭ���� �ƴϱ� ������ ����ȭ�� �ѹ��� �Ѵ�
	
	//float3 R = reflect(-normalize(In.E), N) ;//�ݻ纤��
//	float3 H = normalize(L+normalize(In.E)) ;
//	float fDiffuse = 0.3f ;//-vLightDir.w ;//ambient
	
//	float Incide = dot(N, H) ;
//	float4 S = (0.1f*pow(max(0, dot(H, N)), 3)) ;

	//return In.Color*tex2D(DiffuseMapSamp, In.TexDiffuse) ;

//	return In.Color * tex2D(DiffuseMapSamp, In.TexDiffuse)
//			* (max(0, dot(N, L))+fDiffuse)//diffuse + ambient
//			+ S ;
			//+ (specular*pow(max(0, dot(H, N)), 3)) ;//specular ��������
			//+ (0.3f*pow(max(0, dot(H, N)), 3)) ;//specular ��������	
}

technique TShader
{
	pass P0//SHADER_bump
	{
		VertexShader = compile vs_2_0 VS_Bump() ;
		PixelShader = compile ps_2_0 PS_Bump() ;
	}
}