#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

texture2D	g_DiffuseTexture;
texture2D	g_MaskTexture;
texture2D	g_DepthTexture;

//컬러, 마스크 임계 등 이펙트 관련 변수
float3 g_vRColor = { 1.f, 1.f, 1.f};
float3 g_vGColor = { 1.f, 1.f, 1.f};
float3 g_vBColor = { 1.f, 1.f, 1.f};

float g_fAlpha = { 1.f };
float g_fMaskThreshold = { 0.f };

float2 g_vUVOffset = { 0.f, 0.f };
float2 g_vMaskUVOffset = { 0.f, 0.f };
float g_fMaskUVAngle = { 0.f};

float g_fMaskRatio = { 1.f };

float g_fAlarmColor = { 0.f };


// 회전된 UV를 계산
float2 RotateUV(float2 vCoord, float fAngle)
{
    float2 vCenter = float2(0.5, 0.5); // 중점 좌표 설정

    float fSinAngle = sin(fAngle);
    float fCosAngle = cos(fAngle);
    float2x2 RotationMatrix = float2x2(fCosAngle, -fSinAngle, fSinAngle, fCosAngle);

    // 텍스처 좌표를 중점을 기준으로 이동시키고 회전 변환 적용
    vCoord -= vCenter;
    vCoord = mul(vCoord, RotationMatrix);
    vCoord += vCenter;

    return vCoord;
}


struct VS_IN
{
	float3		vPosition : POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct VS_OUT
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
	VS_OUT		Out = (VS_OUT)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;

	return Out;
}


struct VS_OUT_ALPHABLEND
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};


VS_OUT_ALPHABLEND VS_MAIN_ALPHABLEND(VS_IN In)
{
	VS_OUT_ALPHABLEND		Out = (VS_OUT_ALPHABLEND)0;

	matrix		matWV, matWVP;

	matWV = mul(g_WorldMatrix, g_ViewMatrix);
	matWVP = mul(matWV, g_ProjMatrix);

	Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
	Out.vTexcoord = In.vTexcoord;
	Out.vProjPos = Out.vPosition;

	return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
    float4 vNonBlur : SV_TARGET1;
};

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	
	//알파 테스트
	if(Out.vColor.a < .05f)
        discard;
    
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
	
	return Out;
}

struct PS_IN_ALPHABLEND
{
	float4		vPosition : SV_POSITION;
	float2		vTexcoord : TEXCOORD0;
	float4		vProjPos : TEXCOORD1;
};


PS_OUT PS_MAIN_ALPHABLEND(PS_IN_ALPHABLEND In)
{
	PS_OUT			Out = (PS_OUT)0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    
    //알파 값 예외처리
    if (Out.vColor.a < 0.1f)
        discard;
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a *= g_fAlpha;

    if (0.01f <= Out.vColor.a)
        Out.vNonBlur = vector(0.f, 1.f, 0.f, 0.f);
	
	return Out;
}

PS_OUT PS_MAIN_BLOOM(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	
	if(0.f == Out.vColor.a)
        discard;
		
    return Out;
}

PS_OUT PS_MAIN_BLEND_FX(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;
	
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    
    if (vMask.a < g_fMaskThreshold)
        discard;
    else if (vMask.r < g_fMaskThreshold)
        discard;
    

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    if (vDiffuse.a < .01f || (vDiffuse.r < 0.1f && vDiffuse.g < 0.1f && vDiffuse.b < 0.1f))
        discard;

    Out.vColor.rgb = vDiffuse.rgb * g_vRColor;
    Out.vColor.a = vDiffuse.a * g_fAlpha;
	
	 //소프트 이펙트 보정
    float2 vTexcoord = (float2) 0.f;

    vTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;

    float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * 1000.f;

    Out.vColor.a = Out.vColor.a * saturate(fOldViewZ - In.vProjPos.w);
    
    if ( 0.01f <= Out.vColor.a )
        Out.vNonBlur = vector(0.f, 1.f, 0.f, 0.f);
	
    return Out;
}

PS_OUT PS_MAIN_DEFAULT_FX(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    //마스크 값으로 자르기
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    if (vMask.a < g_fMaskThreshold)
        discard;
    else if (vMask.r < g_fMaskThreshold)
        discard;
    
    //diffuse 알파 테스팅
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    if (vDiffuse.a < .01f)
        discard;
    
    
    Out.vColor = vDiffuse;
	
    return Out;
}

PS_OUT PS_MAIN_WHITEFX(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    //마스크 값으로 자르기
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    //if (vMask.a < g_fMaskThreshold)
    //    discard;
    //else if (vMask.r < g_fMaskThreshold)
    //    discard;
    
    
    //diffuse 알파 테스팅
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    if (vDiffuse.a < .01f || (vDiffuse.r < 0.1f && vDiffuse.g < 0.1f && vDiffuse.b < 0.1f))
        discard;
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * g_fAlpha
    * (bMaskAlpha) ? clamp(vMask.a - g_fMaskThreshold, 0.f, vMask.a - g_fMaskThreshold) : clamp(vMask.r - g_fMaskThreshold, 0.f, vMask.r - g_fMaskThreshold);
	
    return Out;
}

PS_OUT PS_MAIN_FOR_HP(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    //마스크 값으로 자르기
    vector vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vColor.a < 0.1f)
        discard;

    if (vMask.r > g_fMaskRatio)
        discard;
    
    //diffuse 알파 테스팅
    //vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a *= g_fAlpha;
    
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_MAIN_FOR_HPDAMAGE(PS_IN_ALPHABLEND In)
{
    PS_OUT Out = (PS_OUT) 0;

    //마스크 값으로 자르기
    vector vMask = g_MaskTexture.Sample(ClampSampler, In.vTexcoord);
    Out.vColor = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vColor.a < 0.1f)
        discard;

    if (vMask.r > g_fMaskRatio)
        discard;
    
    //diffuse 알파 테스팅
    //vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vColor.rgb = g_vRColor + g_fAlarmColor;
    Out.vColor.a *= g_fAlpha;
    
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    
    return Out;
}



technique11 DefaultTechnique
{
	// 기본 패스. 알파 테스팅 ( 0 )
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

	// 기본 알파 블렌딩 패스 ( 1 )
	pass Blend
	{
		SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
		SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

		VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
		GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
		HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
		DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
		PixelShader = compile ps_5_0 PS_MAIN_ALPHABLEND();
	}

    //블렌드되는 이펙트. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 2 )
    pass BlendFX
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND_FX();
    }

	// For Bloom ( 3 )
    pass Bloom
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLOOM();
    }

	// 기본 이펙트 패스. 알파 테스팅 + 마스크 ( 4 )
    pass DefaultFX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFAULT_FX();
    }

	// Z test 안함 (5)
    pass Blend_NOZTEST
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_ALPHABLEND();
    }

	// 화이트 이펙트 패스. 알파 테스팅 + 마스크 + no z text ( 6 )
    pass WhiteFX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITEFX();
    }

    // Mask를 이용한 UI ( 7 )
    pass UI_MASK
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FOR_HP();
    }

    // Mask를 이용한 UI 2 ( 8 )
    pass UI_MASK2
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN_ALPHABLEND();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FOR_HPDAMAGE();
    }

}