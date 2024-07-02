#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_MaskTexture;
texture2D g_DepthTexture;

vector g_vCamPosition;
vector g_vCamLook;

//컬러, 마스크 임계 등
float3 g_vRColor = { 1.f, 1.f, 1.f };
float3 g_vGColor = { 1.f, 1.f, 1.f };
float3 g_vBColor = { 1.f, 1.f, 1.f };

float g_fAlpha;
//float g_fMaskThreshold;


struct VS_IN
{
    float3 vPosition : POSITION;
    row_major float4x4 TransformMatrix : WORLD;
    bool bAlive : COLOR0;
};


struct VS_OUT
{
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    bool bAlive : COLOR0;
    float4 vProjPos : TEXCOORD1;
};

//struct VS_OUT_ALPHABLEND
//{
//    float4 vPosition : POSITION;
//    float2 vPSize : PSIZE;
//    float4 bAlive : COLOR0;
//    float4 vProjPos : TEXCOORD1;
//};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);

    Out.vPosition = mul(vPosition, g_WorldMatrix);
    Out.vPSize = float2(In.TransformMatrix._11, In.TransformMatrix._22);
    Out.bAlive = In.bAlive;
    
  
    matrix matWV, matWVP;
    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);
    Out.vProjPos = mul(Out.vPosition, matWVP);

    
    return Out;
}

//VS_OUT VS_MAIN_ALPHABLEND(VS_IN In)
//{
//    VS_OUT Out = (VS_OUT) 0;

//    vector vPosition = mul(vector(In.vPosition, 1.f), In.TransformMatrix);

//    Out.vPosition = mul(vPosition, g_WorldMatrix);
//    Out.vProjPos = Out.vPosition;
//    Out.vPSize = float2(In.TransformMatrix._11, In.TransformMatrix._22);
//    Out.bAlive = In.bAlive;

//    return Out;
//}

struct GS_IN
{
    float4 vPosition : POSITION;
    float2 vPSize : PSIZE;
    bool bAlive : COLOR0;
    float4 vProjPos : TEXCOORD1;
};

struct GS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    bool bAlive : COLOR0;
    float4 vProjPos : TEXCOORD1;
};

/* 정점을 생성한다. */
[maxvertexcount(6)]
void GS_MAIN(point GS_IN In[1], inout TriangleStream<GS_OUT> Vertices)
{
    GS_OUT Out[4];

    //float3 vLook = normalize((g_vCamPosition - In[0].vPosition).xyz);
    float3 vLook = normalize(-g_vCamLook).xyz;
    float3 vRight = normalize(cross(float3(0.f, 1.f, 0.f), vLook.xyz)) * In[0].vPSize.x * 0.5f;
    float3 vUp = normalize(cross(vLook, vRight)) * In[0].vPSize.y * 0.5f;
    
    //vRight = normalize(cross(vRight, vUp)) * In[0].vPSize.y * 0.5f;
    
    matrix matVP = mul(g_ViewMatrix, g_ProjMatrix);

    Out[0].vPosition = vector(In[0].vPosition.xyz + vRight + vUp, 1.f);
    Out[0].vTexcoord = float2(0.0f, 0.0f);
    Out[0].vPosition = mul(Out[0].vPosition, matVP);
    Out[0].bAlive = In[0].bAlive;
    Out[0].vProjPos = In[0].vProjPos;

    Out[1].vPosition = vector(In[0].vPosition.xyz - vRight + vUp, 1.f);
    Out[1].vTexcoord = float2(1.0f, 0.0f);
    Out[1].vPosition = mul(Out[1].vPosition, matVP);
    Out[1].bAlive = In[0].bAlive;
    Out[1].vProjPos = In[0].vProjPos;

    Out[2].vPosition = vector(In[0].vPosition.xyz - vRight - vUp, 1.f);
    Out[2].vTexcoord = float2(1.0f, 1.0f);
    Out[2].vPosition = mul(Out[2].vPosition, matVP);
    Out[2].bAlive = In[0].bAlive;
    Out[2].vProjPos = In[0].vProjPos;

    Out[3].vPosition = vector(In[0].vPosition.xyz + vRight - vUp, 1.f);
    Out[3].vTexcoord = float2(0.0f, 1.0f);
    Out[3].vPosition = mul(Out[3].vPosition, matVP);
    Out[3].bAlive = In[0].bAlive;
    Out[3].vProjPos = In[0].vProjPos;

    Vertices.Append(Out[0]);
    Vertices.Append(Out[1]);
    Vertices.Append(Out[2]);
    Vertices.RestartStrip();

    Vertices.Append(Out[0]);
    Vertices.Append(Out[2]);
    Vertices.Append(Out[3]);
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    bool bAlive : COLOR0;
    float4 vProjPos : TEXCOORD1;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
    float4 vNonBlur : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);

    if (Out.vColor.a <= 0.3f ||
		false == In.bAlive)
        discard;

    return Out;
}

PS_OUT PS_MAIN_DEFAULT_FX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    if (Out.vColor.a <= 0.1f ||
		false == In.bAlive)
        discard;
    
    return Out;
}

PS_OUT PS_MAIN_BLEND_FX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (vDiffuse.a < .01f || (vDiffuse.r < 0.1f && vDiffuse.g < 0.1f && vDiffuse.b < 0.1f) || !In.bAlive)
        discard;
	
    Out.vColor.rgb = vDiffuse.rgb * g_vRColor;
    Out.vColor.a = vDiffuse.a * g_fAlpha;
	
		//소프트 이펙트 보정
    float2 vTexcoord = (float2) 0.f;

    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;

    float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * g_fFar;

    Out.vColor.a = Out.vColor.a * saturate(fOldViewZ - In.vProjPos.w);
    
    if (0.01f <= Out.vColor.a)
        Out.vNonBlur = vector(0.f, 1.f, 0.f, 0.f);
	
    return Out;
}

PS_OUT PS_MAIN_WHITE_FX(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vBrightness = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (vBrightness.a < .01f || ( (vBrightness.r + vBrightness.g + vBrightness.b) / 3 < 0.1f) || !In.bAlive)
        discard;
	
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vBrightness.r * g_fAlpha; // 어두울수록 투명

    return Out;
}

PS_OUT PS_FOG(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (vDiffuse.a < .01f || ((vDiffuse.r + vDiffuse.g + vDiffuse.b) / 3 < 0.1f))
        discard;
    
	 //소프트 이펙트 보정
        float2 vTexcoord = (float2) 0.f;

    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;

    float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * g_fFar;

    Out.vColor.a = vDiffuse.a * saturate(fOldViewZ - In.vProjPos.w) * g_fAlpha;
    Out.vColor.rgb = vDiffuse.rgb;
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    
    return Out;
}

technique11 DefaultTechnique
{
    pass Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }


	// 기본 이펙트 패스. 알파 테스팅 + 마스크 ( 1 )
    pass DefaultFX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFAULT_FX();
    }

	// 화이트 이펙트 패스. 알파 테스팅 + 마스크 + no z text ( 2 )
    pass WhiteFX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX();
    }

	// 블렌드되는 이펙트. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 3 )
    pass BlendFX
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND_FX();
    }

    // Alive 상관없이 계속 그림. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 4 )
    pass Fog
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = compile gs_5_0 GS_MAIN();
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOG();
    }
}