#include "Engine_Shader_Defines.hlsli"

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;

matrix	g_BoneMatrices[512];
texture2D	g_DiffuseTexture;
texture2D	g_NormalTexture;
texture2D   g_MRATexture;
texture2D   g_EmissiveTexture;


texture2D g_KirbyMouthTexture;
texture2D g_KirbyEyeTexture;

texture2D g_ObjNearClipTexture;

texture2D g_MaskTexture;
texture2D g_MaskTextureSub;

bool g_bStencil;
bool g_bRimLight;
float m_fRimWidth;
bool g_bMotionBlur;
float4 g_vMotionVelocity;
float g_fWhiteColorDiffuse;
float g_fOverPowerColor;

float4 g_vBulbColor;
bool g_isBulb;
bool g_bBulbOn;
float4 g_BulbPosition;

float4 g_vCamPosition;

float3 g_vDeformRimColor;
float2 g_vUVOffset;
float g_fDissolveRatio;

float g_fDimensionMin;
float g_fDimensionMax;

struct VS_IN
{
	float3		vPosition : POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float3		vTangent : TANGENT;

	uint4		vBlendIndices : BLENDINDEX;
	float4		vBlendWeights : BLENDWEIGHT;
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

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    float fWeightW = 1.f - (In.vBlendWeights.x + In.vBlendWeights.y + In.vBlendWeights.z);

    matrix BoneMatrix = g_BoneMatrices[In.vBlendIndices.x] * In.vBlendWeights.x +
		g_BoneMatrices[In.vBlendIndices.y] * In.vBlendWeights.y +
		g_BoneMatrices[In.vBlendIndices.z] * In.vBlendWeights.z +
		g_BoneMatrices[In.vBlendIndices.w] * fWeightW;

    vector vPosition = mul(vector(In.vPosition, 1.f), BoneMatrix);
    vector vNormal = mul(vector(In.vNormal, 0.f), BoneMatrix);
    vector vTangent = mul(vector(In.vTangent, 0.f), BoneMatrix);

    matrix matWV, matWVP;

    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vPosition, matWVP);
    Out.vNormal = normalize(mul(vNormal, g_WorldMatrix)).rgb;
    Out.vTexcoord = In.vTexcoord;
    Out.vWorldPos = mul(vPosition, g_WorldMatrix);
    Out.vProjPos = Out.vPosition;
    
    Out.vTangent = normalize(mul(vTangent, g_WorldMatrix)).xyz;
    Out.vBinormal = normalize(cross(Out.vNormal, Out.vTangent));
    
    return Out;
}

struct PS_IN
{
	float4		vPosition : SV_POSITION;
	float3		vNormal : NORMAL;
	float2		vTexcoord : TEXCOORD0;
	float4		vWorldPos : TEXCOORD1;
	float4		vProjPos : TEXCOORD2;
    
    float3      vTangent : TANGENT;
    float3      vBinormal : BINORMAL;
};

struct PS_OUT
{
	float4		vDiffuse : SV_TARGET0;
	float4		vNormal : SV_TARGET1;
	float4		vDepth : SV_TARGET2;
    float4		vRimLight : SV_TARGET3;
    float4		vEmissive : SV_TARGET4;
    float4		vStencil : SV_TARGET5;
    float4      vMotionBlur : SV_TARGET6;
    float4      vMRA : SV_TARGET7;

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

struct PS_OUT_DEFERRED
{
    float4 vDeferredInfo : SV_TARGET0;
    float4 vEmissive : SV_TARGET1;

};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vWhite = vector(1.f, 1.f, 1.f, 1.f);
    
    vector mixedColor = lerp(vMtrlDiffuse, vWhite, g_fWhiteColorDiffuse);
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = mixedColor;
    //Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT NO_NORMALMAP_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    //Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    Out.vMRA = vector(0.f, 0.f, 1.f, 1.f);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    
    return Out;
}

PS_OUT FOR_KIRBY_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);
    
    if (g_isBulb == true)
    {
        float4 vBulbWorldPos = g_BulbPosition;
        vBulbWorldPos = mul(vBulbWorldPos, g_ViewMatrix);
        vBulbWorldPos = mul(vBulbWorldPos, g_ProjMatrix);
        vBulbWorldPos /= vBulbWorldPos.w;
        vBulbWorldPos.x *= g_fTexW / g_fTexH;
        
        float4 vProjPos = In.vProjPos;
        vProjPos /= vProjPos.w;
        vProjPos.x *= g_fTexW / g_fTexH;
        
        
        float fViewZ = In.vProjPos.w;
        
        float fLightRange = 0;
        float fRedRange = 0;
        float4 vTotal = 0;
        // 발광
        if (g_bBulbOn == true)
        {
            fLightRange = 5.f / fViewZ;
            fRedRange = 10.f / fViewZ;
            
            float fDistance = length(vProjPos.xy - vBulbWorldPos.xy);
            float fLightAtt = saturate((fLightRange - fDistance) / fLightRange);
            float fRedAtt = saturate((fRedRange - fDistance) / fRedRange);
            vTotal = saturate(float4(1.f, 1.f, 1.f, 1.f) * fLightAtt + float4(1.f, 0.f, 0.8f, 1.f) * fRedAtt);

        }
        else
        {
            fLightRange = 3.5f / fViewZ;
            fRedRange = 10.f / fViewZ;
            
            float fDistance = length(vProjPos.xy - vBulbWorldPos.xy);
            float fLightAtt = saturate((fLightRange - fDistance) / fLightRange);
            float fRedAtt = saturate((fRedRange - fDistance) / fRedRange);
            vTotal = saturate(float4(1.f, 0.8f, 0.f, 1.f) * fLightAtt + float4(1.f, .1f, .3f, 1.f) * fRedAtt);
        }
        
        vTotal /= 2.f;
        Out.vEmissive = vTotal;

    }
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);
    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT FOR_FINALEKIRBY_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 0.3f, 1.f, 1.f);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);
    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT FOR_KIRBY_PS_HAMMER_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT FOR_MOUTH_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_KirbyMouthTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    
    return Out;

}

PS_OUT FOR_KIRBYMOUTH_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_KirbyMouthTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    //if (g_bMotionBlur == true)
    //    Out.vMotionBlur = g_vMotionVelocity;

    
    return Out;
}

PS_OUT FOR_EYE_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_KirbyEyeTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);

  
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    return Out;

}

PS_OUT FOR_KIRBYEYE_PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_KirbyEyeTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);

  
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    return Out;

}

PS_OUT_LIGHTDEPTH PS_MAIN_LIGHTDEPTH(PS_IN In)
{
	PS_OUT_LIGHTDEPTH		Out = (PS_OUT_LIGHTDEPTH)0;

	Out.vLightDepth = float4(In.vProjPos.w / 2000.f, 0.f, 0.f, 0.f);

	return Out;	
}

PS_OUT_DEFERRED PS_MAIN_DEFERREDINFO(PS_IN In)
{
    PS_OUT_DEFERRED Out = (PS_OUT_DEFERRED) 0;
    
    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (vMtrlDiffuse.a == 0.f)
        discard;

    Out.vDeferredInfo = float4(0.f, 1.f, 0.f, 1.f);

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

PS_OUT PS_MAIN_NEARCLIP(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vViewPos = g_WorldMatrix._41_42_43_44;
    vViewPos = mul(vViewPos, g_ViewMatrix);
    
    if(vViewPos.z < 8.0)
    {
        float2 vPixelTexcoord = (float2) 0.f;
        vPixelTexcoord.x = (In.vProjPos.x / In.vProjPos.w) * 0.5f + 0.5f;
        vPixelTexcoord.y = (In.vProjPos.y / In.vProjPos.w) * -0.5f + 0.5f;
        
        vector vNearClipDesc = g_ObjNearClipTexture.Sample(LinearSampler, vPixelTexcoord * 100);
        if ( pow (saturate((vViewPos.z / 8) -.1), 3) < vNearClipDesc.r)
            discard;
    }
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT PS_LINEAR_NORMAL_O(PS_IN In)
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
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT_EFFECT PS_ALPHABLEND(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.0f >= vMtrlDiffuse.a)
        discard;

    Out.vColor = vMtrlDiffuse;
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    return Out;
}

PS_OUT PS_LINEAR_NORMAL_O_NONDISCARD(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    //if (0.f >= vMtrlDiffuse.a)
    //    discard;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}

PS_OUT PS_SIMBAEYE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = { 0.85f, 0.5f, 1.f, 1.f };
    
    Out.vDiffuse = vMtrlDiffuse;
  
    return Out;
}
PS_OUT PS_BULBLIGHT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_vBulbColor;

    Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse + g_fOverPowerColor;
    Out.vNormal = vector(In.vNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = vector(0.f, 1.f, 1.f, 1.f);

  
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;
    
    return Out;

}


PS_OUT PS_FOR_POPSTAR(PS_IN In)
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
    
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    
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
        vRimLightColor = float4(1.f, 0.9f, 0.6f, 1.f) * vDot;
    // 밝지 않은 부분
    else if (vDot < 0.5)
        vRimLightColor = clamp(vDot, float4(1.f, 0.075f, 0.075f, 1.f), float4(1.f, 0.9, 0.6f, 1.f)) * (1.f - vDot);

    Out.vDiffuse = saturate(vMtrlDiffuse * vRimLightColor);
    
    return Out;
}

PS_OUT PS_EMISSIVE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;

    vector vWhite = vector(1.f, 1.f, 1.f, 1.f);
    
    vector mixedColor = lerp(vMtrlDiffuse, vWhite, g_fWhiteColorDiffuse);
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = mixedColor;
    Out.vEmissive = g_EmissiveTexture.Sample(LinearSampler, In.vTexcoord);
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

    return Out;
}


PS_OUT PS_FOR_DEFORMRIM(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);
    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;
    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);
    float3 vWorldNormal = mul(vNormal, WorldMatrix);
    vector vMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord + g_vUVOffset);
    
    float4 vWorldPos = In.vWorldPos;
    
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
        
    float4 vLook = float4(g_vCamPosition.xyz, 1.f) - float4(vWorldPos.xyz, 1.f);
    float vDot = dot(normalize(vLook), normalize(float4(vWorldNormal, 0.f)));
    
    vDot = 1.0f - saturate(vDot); // 내적 값이 0에 가까울수록 vDot이 1에 가까워지도록 변환
    
    vDot = 1 - pow(-2 * vDot + 2, 2) / 3;
    //vDot = vDot < 0.5 ?
    //2 * vDot * vDot :
    //1 - pow(-2 * vDot + 2, 2) / 3;
    //vDot = pow(vDot, 10.f);
    
    vector vRimLightColor = 0;
    
    if (vDot < 0.03f)
        discard;
    
    if (vMask.r < g_fDissolveRatio)
        discard;
    
    vRimLightColor.rgb = g_vDeformRimColor.rgb * vDot;
    vRimLightColor.a = vDot;
    
    vRimLightColor += float4(0.4, 0.4, 0.4, 0);

    Out.vDiffuse = saturate(vRimLightColor) * max(vMask, 0.2f);
    return Out;
}


PS_OUT PS_FOR_DIMENSIONGATE(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        discard;
    //vector vMask = g_MaskTexture.Sample(LinearSampler, In.vTexcoord);
    
    float2 UV = In.vTexcoord;
    float2 UVVector = float2(0.5, 0.5) - In.vTexcoord;
    float2 EditUVVector = UVVector *= 0.55;
    UV = In.vTexcoord + EditUVVector;
    //bool b = false;
    //vector vMask2 = g_MaskTextureSub.Sample(LinearSampler, RotateUV(UV, g_fDimensionMax));

    //if (vMask.r < 0.18f)
    //{
    //    discard;
    //}
    //else if (vMask.r >= 0.15f && vMask.r < 0.3)
    //{
    //    b = true;
    //    if (vMask2.r >= 0.27f)
    //        discard;
    //}
    
    //Out.vDiffuse = saturate(float4(vMtrlDiffuse.rgb, 1.f));
    
    //if (b == true)
    //{
    //    Out.vDiffuse *= 0.5f;
    //}
    
    ///
    
    vector vMask = g_MaskTextureSub.Sample(LinearSampler, RotateUV(UV, g_fDimensionMax));

    if (vMask.r > g_fDimensionMax)
        discard;
    
    //float flength = length(UVVector);
    Out.vDiffuse = saturate(float4(vMtrlDiffuse.rgb, 1.f));
    //Out.vDiffuse.a *= pow(1.f - flength, 3.f);
    return Out;
}

PS_OUT_EFFECT PS_GlassCrack(PS_IN In)
{
    PS_OUT_EFFECT Out = (PS_OUT_EFFECT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.0f >= vMtrlDiffuse.a)
        discard;
    
    vector vWhite = vector(1.f, 1.f, 1.f, 1.f);
    vector mixedColor = lerp(vMtrlDiffuse, vWhite, g_fWhiteColorDiffuse);
    
    Out.vColor = mixedColor;
    Out.vNonBlur = float4(0.f, 1.f, 0.f, 0.f);
    return Out;
}

PS_OUT PS_SimbaEyeDefault(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vMtrlDiffuse = g_DiffuseTexture.Sample(ClampSampler, In.vTexcoord);
    if (0.3f >= vMtrlDiffuse.a)
        vMtrlDiffuse.rgb = float3(1, 1, 1);

    vector vWhite = vector(1.f, 1.f, 1.f, 1.f);
    
    vector vNormalDesc = g_NormalTexture.Sample(LinearSampler, In.vTexcoord);

    float3 vNormal = vNormalDesc.xyz * 2.f - 1.f;

    float3x3 WorldMatrix = float3x3(In.vTangent, In.vBinormal, In.vNormal);

    float3 vWorldNormal = mul(vNormal, WorldMatrix);

    Out.vDiffuse = vMtrlDiffuse;
    //Out.vDiffuse = vMtrlDiffuse + g_fWhiteColorDiffuse;
    Out.vNormal = vector(vWorldNormal * 0.5f + 0.5f, 0.f);
    Out.vDepth = vector(In.vProjPos.z / In.vProjPos.w, In.vProjPos.w / g_fFar, 0.0f, 0.0f);
    Out.vMRA = g_MRATexture.Sample(LinearSampler, In.vTexcoord);
    if (Out.vMRA.z == 0)
        Out.vMRA.z = 0.001f;
    
    if (g_bStencil == true)
        Out.vStencil = vector(1.f, 0.f, 0.0f, 1.f);
    
    if (g_bRimLight == true)
        Out.vRimLight = vector(0.f, m_fRimWidth, 1.f, 1.f);

    if (g_bMotionBlur == true)
        Out.vMotionBlur = g_vMotionVelocity;

  
    return Out;
}
technique11 DefaultTechnique
{
    // 기본적인 애니메이션 모델 ( 0 )
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
    // 기본적인 노말 맵이 없는 애니메이션 모델 ( 1 )
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
        
    // For Mouth (3)
    pass Mouth
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_MOUTH_PS_MAIN();
    }

    // For Eye (4)
    pass Eye
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_EYE_PS_MAIN();
    }
    // 블룸 처리 할 모델 ( 5 )
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
    // 블랜드 할 모델 ( 6 )
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

    // 디퍼드 인포 7
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
    // 커비 전용 랜더 ( 8 )
    pass Kirby_Default
    {
        SetRasterizerState(RS_NonCull);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_KIRBY_PS_MAIN();
    }

    // For Kirby Mouth ( 9 )
    pass Kirby_Mouth
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_KIRBYMOUTH_PS_MAIN();
    }

    // For Kirby Eye ( 10 )
    pass Kirby_Eye
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_KIRBYEYE_PS_MAIN();
    }

    //(11)
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

    // Normal O + 리니어 샘플러 (12)
    pass Linear_Normal_O
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_LINEAR_NORMAL_O();
    }

    // 커비들의 애니메이션 웨펀들을 위한 패스 (13)
    pass For_Kirby_Weapons
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_KIRBY_PS_HAMMER_MAIN();
    }

    // 피날레 커비 실험용 ( 14 )
    pass For_FinaleKirby_Weapons
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 FOR_FINALEKIRBY_PS_MAIN();
    }

    // AlphaBlend (15)
    pass AlphaBlend
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

    // Normal O + 리니어 샘플러 + 디스카드x (16)
    pass Linear_Normal_O_NonDiscard
        {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_LINEAR_NORMAL_O_NONDISCARD();
    }

    // SimbaEye (17)
    pass SimbaEye
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SIMBAEYE();
    }
    // Bulb Light ( 18 )
    pass BulbLight
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_BULBLIGHT();
    }

    // Popstar ( 19 )
    pass Popstar
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOR_POPSTAR();
    }

    // Emissive (20)
    pass Emissive
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_EMISSIVE();
    }

    // DeformRimEffect (21)
    pass DeformRimEffect
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOR_DEFORMRIM();

    }

    // OriginCage GlassCrack (22)
    pass GlassCrack
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_GlassCrack();
    }

    // DimensionGate (23)
    pass DimensionGate
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_FOR_DIMENSIONGATE();
    }

    // SimbaEyeDefault (24)
    pass SimbaEyeDefault
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_SimbaEyeDefault();
    }
}