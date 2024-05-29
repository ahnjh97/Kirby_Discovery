#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
texture2D	g_NormalTexture;
float       g_fSamplingFactor;
float       g_fTime;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float3		vTangent : TANGENT;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;

	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;


};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vNormal = normalize(mul(float4(In.vNormal, 0.f), g_WorldMatrix)).xyz;
	Out.vTangent = normalize(mul(float4(In.vTangent, 0.f), g_WorldMatrix)).xyz;
	Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));
    Out.vTexcoord = In.vTexcoord * g_fSamplingFactor;
	Out.vWorldPos = mul(float4(In.vPosition, 1.f), g_WorldMatrix);
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;

	float3		vTangent : TANGENT;
	float3		vBinormal : BINORMAL;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vRimLight : SV_TARGET3;
    float4 vFieldDepth : SV_TARGET4;
    float4 vStencil : SV_TARGET5;
};

struct PS_OUT_LIGHTDEPTH
{
    float4 vLightDepth : SV_TARGET0;
};

PS_OUT_LIGHTDEPTH PS_MAIN_LIGHTDEPTH(PS_IN In)
{
    PS_OUT_LIGHTDEPTH Out = (PS_OUT_LIGHTDEPTH) 0;

    Out.vLightDepth = float4(In.vProjPos.w / 1000.f, 0.f, 0.f, 0.f);

    return Out;
}

float3 vDamageColor = float3(2.f, 0.45f, 0);

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (0.3f >= vMtrlDiffuse.a)
		discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal;
    vNormal.xy = vNormalDesc.wy * 2.f - 1.f;
    vNormal.z = sqrt(1 - saturate(dot(vNormal.xy, vNormal.xy)));

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	float3 vWorldNormal = mul(vNormal, WorldMatrix);	

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    Out.vFieldDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.0f, 0.0f);
    
    if (g_fTime < 0.5f)
        Out.vDiffuse.rgb += vDamageColor * smoothstep(0.0f, 1.0f, g_fTime);
    else if (g_fTime < 1.f)
        Out.vDiffuse.rgb += vDamageColor * smoothstep(0.0f, 1.0f, (1 - g_fTime));
    
    if (Out.vDiffuse.a != 0 && Out.vDiffuse.r < 0.06f)
        discard;
    
    return Out;
}

PS_OUT NO_NORMALMAP_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.0f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    Out.vFieldDepth = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.0f, 0.0f);
    
    if (g_fTime < 0.5f)
        Out.vDiffuse.rgb += vDamageColor * smoothstep(0.0f, 1.0f, g_fTime);
    else if (g_fTime < 1.f)
        Out.vDiffuse.rgb += vDamageColor * smoothstep(0.0f, 1.0f, (1 - g_fTime));
    
    return Out;
}

technique11 DefaultTechnique
{
	// 노말이 있는 일반 논 애님 모델 ( 0 )
	pass Default
	{
		SetRasterizerState(RS_Default);
		SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN();
	}
	// 노말이 없는 일반 논 애님 모델 ( 1 )
    pass NonNormal_Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 NO_NORMALMAP_PS_MAIN();
    }
    // 그림자 그리기 ( 2 )
    pass LightDepth
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_LIGHTDEPTH();
    }
	// 노말이 있는 일반 블렌딩 객체 ( 3 )
    pass AlphaBlend
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
	// 노말이 없는 일반 블렌딩 객체 ( 4 )
    pass NonNormal_AlphaBlend
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 NO_NORMALMAP_PS_MAIN();
    }
}