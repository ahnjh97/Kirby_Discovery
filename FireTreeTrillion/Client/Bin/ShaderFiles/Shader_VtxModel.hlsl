 #include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
texture2D g_DiffuseTexture;
texture2D g_MaskTexture;
texture2D g_NormalTexture;
texture2D g_DepthTexture;
texture2D g_MRATexture;
texture2D g_EmissiveTexture;

bool g_bMaptool = false;

uint g_iTriggerType;

texture2D g_ObjNearClipTexture;

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
float g_fMaskUVAngle = { 0.f };

float g_fWhiteColorDiffuse;
float g_fOverPowerColor;

bool g_bEmissive = { false };
float g_fEmissivePower;

float4 g_vCamPosition;

float g_fDissolve;


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

void MaskTest(vector vMaskValue)
{
     //마스크 자르기
    if (vMaskValue.a < g_fMaskThreshold)
        discard;
    else if (vMaskValue.r < g_fMaskThreshold)
        discard;
}

void MaskTest(float fMaskValue)
{
    if (fMaskValue < g_fMaskThreshold)
        discard;

}
void AlphaTest(vector vDiffuseValue, float fDiscardValue = .01)
{
    if (vDiffuseValue.a < fDiscardValue ||
        (vDiffuseValue.r < fDiscardValue && vDiffuseValue.g < fDiscardValue && vDiffuseValue.b < fDiscardValue))
        discard;
}

float SoftEffect(float fAlpha, vector vProjPos)
{
    //소프트 이펙트 보정
    float2 vTexcoord = (float2) 0.f;

    vTexcoord.x = (vProjPos.x / vProjPos.w) * 0.5f + 0.5f;
    vTexcoord.y = (vProjPos.y / vProjPos.w) * -0.5f + 0.5f;

    float4 vDepthDesc = g_DepthTexture.Sample(PointSampler, vTexcoord);
    float fOldViewZ = vDepthDesc.y * g_fFar;

    return fAlpha * saturate(fOldViewZ - vProjPos.w);
}

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float3 vTangent : TANGENT;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;

    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;


};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix matWV, matWVP;

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
    float4 vPosition : SV_POSITION;
    float3 vNormal : NORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;

    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
};

struct PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vRimLight : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
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
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    
    if (g_bMaptool)
        Out.vEmissive = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.0f, 0.0f);
    
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
    if (0.05f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    if (g_bEmissive == true)
    {
        Out.vDiffuse = vector(0.35f, 0.75f, 0.4f, 0.05f);
        Out.vDiffuse *= g_fEmissivePower;
    }
    else
        Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
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

PS_OUT FOR_BOSS_OBJECT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
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
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
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
    else if (g_iTriggerType == 3)
        Out.vDiffuse = vector(0, 0.45f, 0.45f, 1);
    else if (g_iTriggerType == 4) // Monster
        Out.vDiffuse = vector(1, 0, 0, 1);
    else if (g_iTriggerType == 5) // Simba Attqck
        Out.vDiffuse = vector(0, 0, 0, 1);
    else if (g_iTriggerType == 6) // Event
        Out.vDiffuse = vector(0, 1, 0, 1);
    else
        Out.vDiffuse = vector(1, 1, 1, 1);
    
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    
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
    vector vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord + g_vUVOffset);
    AlphaTest(vDiffuse, .2);

    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    
    Out.vMRA = float4(0.f, .8f, 0.f, 1.f);
    Out.vDiffuse.rgb = vDiffuse.rgb * g_vRColor;
    Out.vDiffuse.a = 1.f;
        
    if (Out.vMRA.b < 0.001)
        Out.vMRA.b = 1.f;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_BLEND_FX_LINEARDIFFUSE(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
 
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    AlphaTest(vDiffuse);

    Out.vColor.rgb = vDiffuse.rgb * g_vRColor;
    Out.vColor.a = vDiffuse.a * g_fAlpha * fSmoothedAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    
    if(Out.vColor.a < .1)
        discard;

    return Out;
}

PS_OUT_EFFECT PS_MAIN_BLEND_FX_CLAMPDIFFUSE(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    

    vector vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord + g_vUVOffset);
    AlphaTest(vDiffuse);

    Out.vColor.rgb = vDiffuse.rgb * g_vRColor;
    Out.vColor.a = vDiffuse.a * vMask.r * g_fAlpha * fSmoothedAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    
    if (Out.vColor.a < .1)
        discard;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_WHITE_FX_LINEARDIFFUSE(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    if (g_fAlpha == 0)
        discard;
    
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    //마스크 자르기
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    AlphaTest(vDiffuse);
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * fSmoothedAlpha * g_fAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    if (Out.vColor.a < .1)
        discard;
    
    return Out;
}


PS_OUT_EFFECT PS_MAIN_WHITE_FX_LINEARDIFFUSE_LINEARMASK(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    if (g_fAlpha == 0)
        discard;
    
    vector vMask = g_MaskTexture.Sample(LinearSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    //마스크 자르기
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    MaskTest(vMask.a);
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    AlphaTest(vDiffuse);
    
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * fSmoothedAlpha * g_fAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    if (Out.vColor.a < .1)
        discard;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_WHITE_FX_CLAMPDIFFUSE(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    if (g_fAlpha == 0)
        discard;
    
    vector vMask = g_MaskTexture.Sample(ClampSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    //마스크 자르기
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    
    vector vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    AlphaTest(vDiffuse);
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * fSmoothedAlpha * g_fAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    if (Out.vColor.a < .1)
        discard;
    
    return Out;
}


PS_OUT_EFFECT PS_MAIN_WHITE_FX_CLAMPDIFFUSE_LINEARMASK(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    if (g_fAlpha == 0)
        discard;
    
    vector vMask = g_MaskTexture.Sample(LinearSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    //마스크 자르기
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    
    vector vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    AlphaTest(vDiffuse);
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * fSmoothedAlpha * g_fAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    if (Out.vColor.a < .1)
        discard;
    
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

PS_OUT PS_MAIN_NEARCLIP(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vViewPos = g_WorldMatrix._41_42_43_44;
    vViewPos = mul(vViewPos, g_ViewMatrix);
    
    //if (vViewPos.z < 8.0)
    float fCameraDistance = 12.4f;
    if (vViewPos.z < fCameraDistance)
    {
        float2 vPixelTexcoord = (float2) 0.f;
        vPixelTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
        vPixelTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
        
        vector vNearClipDesc = g_ObjNearClipTexture.Sample(LinearSampler, vPixelTexcoord * 100);
        if (pow(saturate((vViewPos.z / fCameraDistance) - .1), 3) < vNearClipDesc.r)
            discard;
    }
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    
    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
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

PS_OUT_EFFECT PS_ALPHABLEND(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord + g_vUVOffset);
    AlphaTest(vDiffuse);

    Out.vColor.rgb = vDiffuse.rgb;
    Out.vColor.a = vDiffuse.a;

    if (.01 < Out.vColor.a)
        Out.vNonBlur = vector(0.f, 1.f, 0.f, 0.f);
    
    return Out;
}

PS_OUT PS_EMISSIVE_NORMAL_O(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.0f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    
    if (g_bMaptool)
        Out.vEmissive = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.0f, 0.0f);
    else
        Out.vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);
    
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

PS_OUT PS_EMISSIVE_NORMAL_X(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    if (0.0f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0, 1, 1, 1);

    if (g_bMaptool)
        Out.vEmissive = vector(In.vProjPos.z / In.vProjPos.w, 0.f, 0.0f, 0.0f);
    else
        Out.vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);

    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    return Out;
}

PS_OUT PS_FOR_STAR(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
    float3 vWorldNormal = mul(vNormal, WorldMatrix);
    
    float4 vWorldPos = In.vWorldPos;
    
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    
    float4 vLook = float4(g_vCamPosition.xyz, 1.f) - float4(vWorldPos.xyz, 1.f);
    float vDot = dot(normalize(vLook), normalize(float4(vWorldNormal, 0.f)));
    
    vDot = 1.0f - saturate(vDot); // 내적 값이 0에 가까울수록 vDot이 1에 가까워지도록 변환
    //vDot = vDot < 0.5 ? pow(2, 20 * vDot - 10) : (2 - pow(2, -20 * vDot + 10)) * 100;
    vDot = vDot < 0.5 ?
    2 * vDot * vDot :
    1 - pow(-2 * vDot + 2, 2) / 3;
    
    
    vector vRimLightColor = 0;
    
    // 밝은 부분
    if (vDot > 0.5)
        vRimLightColor = float4(1.f, 1.f, 1.f, 1.f) * vDot;
    // 밝지 않은 부분
    else if (vDot <= 0.5)
        vRimLightColor = lerp(float4(1.f, 0.9f, 0.6f, 1.f), float4(1.f, 0.5, 0.5f, 1.f), vDot * 2.f);

    Out.vDiffuse = saturate(vMtrlDiffuse * vRimLightColor);
    return Out;
}

PS_OUT PS_FOR_COIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
    float3 vWorldNormal = mul(vNormal, WorldMatrix);
    
    float4 vWorldPos = In.vWorldPos;
    
    Out.vDiffuse = float4(0.2f, 0.2f, 0.0f, 1.f);
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);

    float4 vLook = float4(g_vCamPosition.xyz, 1.f) - float4(vWorldPos.xyz, 1.f);
    float vDot = dot(normalize(vLook), normalize(float4(vWorldNormal, 0.f)));
    
    vDot = 1.0f - saturate(vDot);
    
    vDot = vDot < 0.5 ?
    2 * vDot * vDot :
    1 - pow(-2 * vDot + 2, 2) / 3;

    vector vRimLightColor = 0;
    
    if (vDot > 0.4)
        vRimLightColor = float4(0.9f, .9f, 0.6f, 1.f) * vDot;
    // 밝지 않은 부분
    else if (vDot <= 0.6)
        vRimLightColor = lerp(float4(0.6f, 0.2f, 0.15f, 1.f), float4(.9f, .9f, 0.5f, 1.f), pow(vDot, 0.5f));
        // 밝은 부분
    //if (vDot > 0.5)
    //    vRimLightColor = float4(1.f, 0.9f, 0.6f, 1.f) * vDot;
    //// 밝지 않은 부분
    //else if (vDot < 0.5)
    //    vRimLightColor = clamp(vDot, float4(1.f, 0.075f, 0.075f, 1.f), float4(1.f, 0.9, 0.6f, 1.f)) * (1.f - vDot);


    Out.vEmissive = saturate(/*vMtrlDiffuse * */vRimLightColor);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT_EFFECT PS_DISSOLVE(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    vector vMask =        g_MaskTexture.Sample(LinearSampler, In.vTexcoord);

    if (0.3f >= vMtrlDiffuse.a)
        discard;
    
    if (vMask.r < g_fDissolve)
        discard;

    vMtrlDiffuse.r = g_vRColor.x;
    vMtrlDiffuse.g = g_vRColor.y;
    vMtrlDiffuse.b = g_vRColor.z;
    vMtrlDiffuse.a = 1.f;
    
    Out.vColor = vMtrlDiffuse;
    if (Out.vColor.a < 0.02f)
        discard;
    
    return Out;
}

PS_OUT_EFFECT PS_MAIN_FOR_FINALBOSS_LASER(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    //0 이하로는 잘라버리기
    if(In.vWorldPos.y < 0)
        discard;
    
    if (g_fAlpha == 0)
        discard;
    
    vector vMask = g_MaskTexture.Sample(LinearSampler, RotateUV(In.vTexcoord + g_vMaskUVOffset, g_fMaskUVAngle));
    //마스크 자르기
    bool bMaskAlpha = false;
    if (vMask.a < .1f)
        bMaskAlpha = true;
    
    float fMaskValue = (bMaskAlpha) ? vMask.a : vMask.r;
    
    float fSmoothedAlpha = smoothstep(g_fMaskThreshold - 0.1, g_fMaskThreshold + 0.1, fMaskValue);
    if (fSmoothedAlpha < 0.01)
        discard;
    
    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    
    //알파가 0일때, 혹은 검은색일때 자르기
    AlphaTest(vDiffuse);
    
    
    Out.vColor.rgb = g_vRColor;
    Out.vColor.a = vDiffuse.a * fSmoothedAlpha * g_fAlpha;

    //소프트 이펙트 보정
    Out.vColor.a = SoftEffect(Out.vColor.a, In.vProjPos);
    if (Out.vColor.a < .1)
        discard;
    
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
    // 블렌드 할 모델 ( 5 )
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

    // 알파블렌드, 노말 X (13 -> 7)
    pass ALPHABLEND
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_ALPHABLEND();
    }

    // 디퍼드 인포 ( 9 -> 8 )
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

    // 가까이오면 잘리는 패스(12 -> 9 )
    pass NearClip
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_NEARCLIP();
    }

    // 커비 파츠 ( 11 -> 10 )
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

    // 몬스터의 파트 오브젝트 ( 14 -> 11 )
    pass MONSTER_PARTOBJECT
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_BOSS_OBJECT();
    }


	// 기본 이펙트 패스. 알파 테스팅 + 마스크 ( 7 -> 12 )
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

    //블렌드되는 이펙트. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 13 )
    pass BlendFX_LinearDiffuse
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND_FX_LINEARDIFFUSE();
    }

    //블렌드되는 이펙트. 알파 블렌딩 + 마스크 + 소프트 이펙트 ( 14 )
    pass BlendFX_ClampDiffuse
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLEND_FX_CLAMPDIFFUSE();
    }


    //화이트 이펙트. 알파 블렌딩 + 마스크 + 디퓨즈 리니어샘플( 10 -> 15 )
    pass WhiteFX_LinearDiffuse
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX_LINEARDIFFUSE();
    }

    ////화이트 이펙트. 알파 블렌딩 + 마스크 + 디퓨즈 클램프샘플 ( 10 -> 16 )
    pass WhiteFX_ClampDiffuse
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX_CLAMPDIFFUSE();
    }

    // Emissive, Normal O  ( 17 )
    pass Emissive_Normal_O
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE_NORMAL_O();
    }

    // Emissive, Normal X  ( 18 )
    pass Emissive_Normal_X
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE_NORMAL_X();
    }
    // Star ( 19 )
    pass STAR
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOR_STAR();
    }
    // Coin ( 20 )
    pass COIN
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOR_COIN();
    }
    // 디졸브 효과 ( 21 )
    pass DISSOLVE
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_DISSOLVE();
    }

    //화이트 이펙트. 알파 블렌딩 + 디퓨즈 리니어샘플 + 마스크 리니어샘플 + 소프트 이펙트( 22 )
    pass WhiteFX_LinearDiffuse_LinearMask
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX_LINEARDIFFUSE_LINEARMASK();
    }

    ////화이트 이펙트. 알파 블렌딩 + 디퓨즈 클램프샘플 + 마스크 리니어샘플 + 소프트 이펙트( 23 )
    pass WhiteFX_ClampDiffuse_LinearMask
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_WHITE_FX_CLAMPDIFFUSE_LINEARMASK();
    }

    pass For_Laser
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FOR_FINALBOSS_LASER();
    }
}