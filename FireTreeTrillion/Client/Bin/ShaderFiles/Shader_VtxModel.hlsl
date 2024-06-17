 #include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D	g_DiffuseTexture;
texture2D   g_MaskTexture;
texture2D	g_NormalTexture;
texture2D   g_DepthTexture;
texture2D   g_MRATexture;
uint        g_iTriggerType;


bool g_bStencil;
bool g_bRimLight;
float m_fRimWidth;
bool g_bMotionBlur;
float4 g_vMotionVelocity;

//컬러, 마스크 임계 등 이펙트 관련 변수
float3 g_vRColor = { 1.f, 1.f, 1.f };
float3 g_vGColor = { 1.f, 1.f, 1.f };
float3 g_vBColor = { 1.f, 1.f, 1.f };

float g_fAlpha = { 1.f };
float g_fMaskThreshold = { 0.f };

float2 g_vUVOffset = { 0.f, 0.f };
float2 g_vMaskUVOffset = { 0.f, 0.f };
float g_fMaskUVAngle = { 0.f};

float g_fWhiteColorDiffuse;
float g_fOverPowerColor;


// 회전된 UV를 계산
float2 RotateUV(float2 vCoord, float fAngle)
{
    float2 vCenter = (0.5, 0.5);
    
    float fSinAngle = sin(fAngle);
    float fCosAngle = cos(fAngle);
    float2x2 RotationMatrix = float2x2(fCosAngle, -fSinAngle, fSinAngle, fCosAngle);

    //float2 vDir = vCoord - vCenter;
    vCoord -= vCenter;
    vCoord = mul(vCoord, RotationMatrix);
    vCoord += vCenter;
    
    return mul(vCoord, RotationMatrix);
}


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
	Out.vTexcoord = In.vTexcoord;
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
    float4 vMotionBlur : SV_TARGET6;
    float4 vMRA : SV_TARGET7;

};

struct PS_OUT_EFFECT
{
    float4 vColor : SV_TARGET0;
    float4 vNonBlur : SV_TARGET1;
};

struct PS_OUT_LIGHTDEPTH
{
    float4 vLightDepth : SV_TARGET0;
};

PS_OUT_LIGHTDEPTH PS_MAIN_LIGHTDEPTH(PS_IN In)
{
    PS_OUT_LIGHTDEPTH Out = (PS_OUT_LIGHTDEPTH) 0;

    Out.vLightDepth = float4(In.vProjPos.w / 2000.f, 0.f, 0.f, 0.f);

    return Out;
}

PS_OUT PS_MAIN(PS_IN In)
{
	PS_OUT			Out = (PS_OUT)0;

	vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
	if (0.3f >= vMtrlDiffuse.a)
		discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

	float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

	float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

	float3 vWorldNormal = mul(vNormal, WorldMatrix);	

	Out.vDiffuse = vMtrlDiffuse;
	Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
	Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    if (Out.vMRA.b < 0.001)
        Out.vMRA.b = 1.f;
    
	return Out;
}

PS_OUT FOR_KIRBY_PARTOBJECT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    if (Out.vMRA.b < 0.001)
        Out.vMRA.b = 1.f;
    
    return Out;
}


PS_OUT NO_NORMALMAP_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);

    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    
    if (Out.vMRA.b < 0.001)
        Out.vMRA.b = 1.f;
    
    return Out;
}

PS_OUT SKY_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
        
	// 구현부
    Out.vDiffuse = vMtrlDiffuse;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_BLUR(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    
    Out.vColor = vMtrlDiffuse;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_NONBLUR(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    
    Out.vColor = vMtrlDiffuse;
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    return Out;
}

PS_OUT TRIGGER(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    if (g_iTriggerType == 0)
        Out.vDiffuse = vector(1, 0.75f, 0.8f, 1);
    else if (g_iTriggerType == 1)
        Out.vDiffuse = vector(0, 0, 1, 1);
    else
        Out.vDiffuse = vector(1, 1, 1, 1);
    
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    
    return Out;
}

PS_OUT PS_MAIN_DEFAULT_FX(PS_IN In)
{
    
    PS_OUT Out = (PS_OUT) 0;

    //마스크 값으로 자르기
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    if (vMask.a < .01f && vMask.a < g_fMaskThreshold)
        discard;
    else if (vMask.r < .1f && vMask.r < g_fMaskThreshold)
        discard;
    
    //diffuse 알파 테스팅
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    if (vDiffuse.a < .2f)
        discard;
    


    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / 1000.0f, 0.0f, 0.0f);
    
    Out.vMRA = float4(0.f, .8f, 0.f, 1.f);
    Out.vDiffuse.rgb = vDiffuse.rgb * g_vRColor;
    Out.vDiffuse.a = 1.f;
        
    if (Out.vMRA.b < 0.001)
        Out.vMRA.b = 1.f;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_BLEND_FX(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    
    if(vMask.a < g_fMaskThreshold)
        discard;
    else if (vMask.r < g_fMaskThreshold)
        discard;
    

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    if (vDiffuse.a < .01f || (vDiffuse.r < 0.1f && vDiffuse.g < 0.1f && vDiffuse.b < 0.1f) )
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

    //if(.01 < Out.vColor.a)
    //    Out.vNonBlur = vector(0.f, 1.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_WHITE_FX(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    
    //마스크 자르기
    if (vMask.a < g_fMaskThreshold)
        discard;
    else if (vMask.r < g_fMaskThreshold)
        discard;
    

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    if (vDiffuse.a < .01f || (vDiffuse.r < 0.1f && vDiffuse.g < 0.1f && vDiffuse.b < 0.1f))
        discard;

    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * g_fAlpha;

    return Out;
}


PS_OUT_LIGHTDEPTH PS_MAIN_DEFERREDINFO(PS_IN In)
{
    PS_OUT_LIGHTDEPTH Out = (PS_OUT_LIGHTDEPTH) 0;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (vMtrlDiffuse.a == 0.f)
        discard;

    Out.vLightDepth = float4(0.f, 1.f, 0.f, 1.f);
    
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
	// 스카이박스 ( 3 )
    pass SKY
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_Sky, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 SKY_MAIN();
    }

    // 블룸 처리 할 모델 ( 4 )
    pass Blur_Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);


        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLUR();
    }
    // 블랜드 할 모델 ( 5 )
    pass NonBlur_Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NONBLUR();
    }
    // 트리거 ( 6 )
    pass Trigger
    {
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 TRIGGER();
    }

	// 기본 이펙트 패스. 알파 테스팅 + 마스크 ( 7 )
    pass DefaultFX
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFAULT_FX();
    }

    //블렌드되는 이펙트. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 8 )
    pass BlendFX
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND_FX();
    }

    // 디퍼드 인포 ( 9 )
    pass DeferredInfo_Depth
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DEFERREDINFO();
    }

    //화이트 이펙트. 알파 블렌딩 + 마스크 ( 10 )
    pass WhiteFX
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX();
    }
    // 커비 파츠 ( 11 )
    pass KIRBY_PARTOBJECT
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_KIRBY_PARTOBJECT();
    }
}