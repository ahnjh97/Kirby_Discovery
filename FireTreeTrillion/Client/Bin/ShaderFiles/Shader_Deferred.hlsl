#include "Engine_Shader_Defines.hlsli"
#define PI 3.14159265359

/* 전역변수 : 쉐이더 외부에 있는 데이터를 쉐이더 안으로 받아온다. */
matrix g_WorldMatrix, g_ViewMatrix, g_ProjMatrix;
matrix g_LightViewMatrix, g_LightProjMatrix;
matrix g_ViewMatrixInv, g_ProjMatrixInv;

//색 보정 글로별 번수
bool g_bApplyCorrection = true;

// 디퍼드 옵션 설정
bool g_bRenderShadow = {true};
bool g_bRenderSSAO = {true};
bool g_bRenderDOF = {true};
bool g_bRenderMotionBlur = {true};


float g_fExposure = 1.4f;
float g_fHue = 1.f;
float g_fSaturation = 1.f;
float g_fBrightness = 1.f;
float g_fGamma = .5f;
float g_fVibrance = .5f;

float3 g_vColorBalance = { 1.f, 1.f, 1.f };
float3 g_vWhiteBalance = { .5f, .5f, .5f };
float g_fContrast = .5f;

//toner
float3 g_vShadowColor = { 1.f, 1.f, 1.f };
float g_fShadowIntensity = { 0.f };
float3 g_vMidtoneColor = { 1.f, 1.f, 1.f };
float g_fMidtoneIntensity = { 0.f };
float3 g_vHighlightColor = { 1.f, 1.f, 1.f };
float g_fHighlightIntensity = { 0.f };
float g_fShadowThreshold = { .25f };
float g_fHighlightThreshold = { .75f };

//PBR
static const float Epsilon = 0.0001;
static const uint NumLights = 3;
static const float3 Fdielectric = 0.04;
texture2D g_MRATexture;


static const float fWeight[13] =
{
    0.0561, 0.1353, 0.278, 0.4868, 0.7261, 0.9231, 1,
	0.9231, 0.7261, 0.4868, 0.278, 0.1353, 0.0561
};

static const float fTotal = 6.2108;


texture2D g_Texture;
texture2D g_NormalTexture;
texture2D g_DiffuseTexture;

texture2D g_LinearTexture;
texture2D g_SpecularTexture;

//**** 싸오 ****//
texture2D g_SSAOTexture;
texture2D g_RandomNormalTexture;

const float2 vCoordDir[4] = { float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1) };
// 원근 허용범위
float g_scale = 0.5f;
// 콘모양 제어
float g_bias = 0.f;
// 탐색 반경
float g_sample_radius = 3.f;
// 음영의 강함
float g_intensity = 3.f;

////////////////

TextureCube g_EnvTexture;
Texture2D g_LUTTexture;

texture2D g_DepthTexture;
texture2D g_LightDepthTexture;

texture2D g_FieldDepthTexture;
texture2D g_StencilTexture;
texture2D g_RimLightTexture;

texture2D g_EffectTexture;
texture2D g_BlurTexture;

texture2D g_SkyTexture;

texture2D g_BlendTexture;
texture2D g_NonLightTexture;

texture2D g_FinalTexture;
texture2D g_UITexture;

texture2D g_RadialBlur;
float g_fRadialblurRaduis;
float2 g_fRadialblurCenter;

texture2D g_DOFBlur;
texture2D g_DOFBlur_Result;
float2 g_vDOFFocus;
texture2D g_DiffuseMotionBlur;
texture2D g_MotionBlur;

texture2D g_DeferredInfoTexture;

float4 g_vLightDir;
float4 g_vLightPos;
float g_fLightRange;

float4 g_vLightDiffuse;
float4 g_vLightAmbient;
float4 g_vLightSpecular;


bool g_bRimTest;

float g_fBlackBackGround;

float4 g_vCamPosition;


//////////////////////////////////// For PBR 

float ndfGGX(float cosLh, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}


// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}


// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}


// Shlick's approximation of the Fresnel factor.
float3 fresnelSchlick(float3 F0, float cosTheta)
{
    float factor = pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    // Increase the impact of the fresnel factor
    //return F0 + (1.0 - F0) * factor * 1.5; // Adjust the multiplier as needed
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

//////////////////////////////////////


/// For SSAO

float2 getRandom(in float2 uv)
{
    return normalize(g_RandomNormalTexture.Sample(LinearSampler, float2(g_fTexW, g_fTexH) * uv / float2(900.f, 900.f)).xy * 2.0f - 1.0f);
}

float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
    float2 vUV = tcoord + uv;
    
    vector vSSAO_DepthDesc = g_DepthTexture.Sample(PointSampler, vUV);
    float fSSAO_ViewZ = vSSAO_DepthDesc.y * g_fFar;
    float4 vSSAO_WorldPos;
    /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vSSAO_WorldPos.x = vUV.x * 2.f - 1.f;
    vSSAO_WorldPos.y = vUV.y * -2.f + 1.f;
    vSSAO_WorldPos.z = vSSAO_DepthDesc.x;
    vSSAO_WorldPos.w = 1.f;
	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vSSAO_WorldPos *= fSSAO_ViewZ;
	/* 로컬위치 * 월드행렬 * 뷰행렬 */
    vSSAO_WorldPos = mul(vSSAO_WorldPos, g_ProjMatrixInv);
	/* 로컬위치 * 월드행렬 */
    vSSAO_WorldPos = mul(vSSAO_WorldPos, g_ViewMatrixInv);
    
    float3 diff = vSSAO_WorldPos.xyz - p;
    const float3 v = normalize(diff);
    const float d = length(diff) * g_scale;
    
    return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
}



float4 Blur_X(float2 vTexCoord)
{
    float4 vOut = (float4) 0;

    float2 vUV = (float2) 0;
    
    
    if (1.f == g_BlendTexture.Sample(ClampSampler, vTexCoord).g)
        return vOut;
    
    float fTotal = 0.0;

    for (int i = -6; i < 7; ++i)
    {
        vUV = vTexCoord + float2(1.f / g_fTexW * i, 0);
        if (1.f == g_BlendTexture.Sample(ClampSampler, vUV).g)
            continue;

        vOut += fWeight[6 + i] * (g_EffectTexture.Sample(ClampSampler, vUV) + g_SpecularTexture.Sample(ClampSampler, vUV));
        fTotal += fWeight[6 + i];
    }

    vOut /= fTotal;
    return vOut;
}

float4 FreeBlur_X(float2 vTexCoord, texture2D tTexture, float fRadius)
{
    float4 vOut = (float4) 0;
    float2 vUV = (float2) 0;

    float fTotal = 0.0;

    for (int i = -6; i < 7; ++i)
    {
        vUV = vTexCoord + float2(1.f / g_fTexW * i * fRadius, 0);

        vOut += fWeight[6 + i] * (tTexture.Sample(ClampSampler, vUV));
        fTotal += fWeight[6 + i];
    }

    vOut /= fTotal;

    return vOut;
}

float4 Blur_Y(float2 vTexCoord)
{
    float4 vOut = (float4) 0;

    float2 vUV = (float2) 0;
    
    if (1.f == g_BlendTexture.Sample(ClampSampler, vTexCoord).g)
        return vOut;

    float fTotal = 0.0;
    
    for (int i = -6; i < 7; ++i)
    {
        vUV = vTexCoord + float2(0, 1.f / (g_fTexH / 2.f) * i);
        if (1.f == g_BlendTexture.Sample(ClampSampler, vUV).g)
            continue;

        vOut += fWeight[6 + i] * g_EffectTexture.Sample(ClampSampler, vUV);
        fTotal += fWeight[6 + i];

    }

    vOut /= fTotal;
    return vOut;
}

float4 FreeBlur_Y(float2 vTexCoord, texture2D tTexture, float fRadius)
{
    float4 vOut = (float4) 0;
    float2 vUV = (float2) 0;

    float fTotal = 0.0;

    for (int i = -6; i < 7; ++i)
    {
        vUV = vTexCoord + float2(0, 1.f / (g_fTexH) * i * fRadius);

        vOut += fWeight[6 + i] * (tTexture.Sample(ClampSampler, vUV));
        fTotal += fWeight[6 + i];
    }

    vOut /= fTotal;

    return vOut;
}


float3 ToneMapping(float3 vHDRColor, float fExposure)
{
    vHDRColor *= fExposure;
    
    return vHDRColor / (vHDRColor + float3(1.f, 1.f, 1.f));
}

float3 RGB2HSV(float3 c)
{
    float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float3 HSV2RGB(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
}

float3 ColorGrading(float3 vColor, float fHue, float fSaturation, float fBrightness)
{
    float3 vHSV = RGB2HSV(vColor);
    vHSV.x += fHue;
    vHSV.y *= fSaturation;
    vHSV.z *= fBrightness;
    return HSV2RGB(vHSV);
}
float3 GammaCorrection(float3 vColor, float fGamma)
{
    return pow(vColor, 1.0 / fGamma);
}

float3 AdjustVibrance(float3 vColor, float fVibrance)
{
    float luminance = dot(vColor, float3(0.299, 0.587, 0.114));
    float3 grey = float3(luminance, luminance, luminance);
    return lerp(grey, vColor, fVibrance);
}

float3 AdjustContrast(float3 vColor, float fContrast)
{
    float3 vMidpoint = float3(0.5, 0.5, 0.5);
    return (vColor - vMidpoint) * fContrast + vMidpoint;
}
float3 AdjustColorBalance(float3 vColor, float3 vBalance)
{
    return vColor * vBalance;
}
float3 AdjustWhiteBalance(float3 vColor, float3 vWhitePoint)
{
    return vColor / vWhitePoint;
}
float3 AdjustShadow(float3 vColor, float3 vShadowColor, float fShadowIntensity, float fLuminance)
{
    
    float intensity = fShadowIntensity * smoothstep(g_fShadowThreshold - 0.1, g_fShadowThreshold, fLuminance);
    return lerp(vColor, vShadowColor, intensity);
    
    
    //if (g_fShadowThreshold < fLuminance )
    //    fShadowIntensity *= pow( 1 - ((fLuminance - g_fShadowThreshold) * 10) , 1);

    //return lerp(vColor, vShadowColor, fShadowIntensity );
}

float3 AdjustMidtone(float3 vColor, float3 vMidtoneColor, float fMidtoneIntensity, float fLuminance)
{
    
    float lowerThreshold = g_fShadowThreshold;
    float upperThreshold = g_fHighlightThreshold;
    float intensity = fMidtoneIntensity * smoothstep(lowerThreshold, upperThreshold, fLuminance);
    return lerp(vColor, vMidtoneColor, intensity);
    
    
    //if ( fLuminance < g_fShadowThreshold)
    //    fMidtoneIntensity *= pow((g_fShadowThreshold - fLuminance) * 10, 1);
    //if (g_fHighlightThreshold < fLuminance)
    //    fMidtoneIntensity *= pow(1 - (fLuminance - g_fHighlightThreshold) *10, 1);
    
    //return lerp(vColor, vMidtoneColor, fMidtoneIntensity);
}

float3 AdjustHighlight(float3 vColor, float3 vHighlightColor, float fHighlightIntensity, float fLuminance)
{
    
    float intensity = fHighlightIntensity * smoothstep(g_fHighlightThreshold, g_fHighlightThreshold + 0.1, fLuminance);
    return lerp(vColor, vHighlightColor, intensity);
    
    
    //if (fLuminance < g_fHighlightThreshold)
    //    fHighlightIntensity *= pow(( g_fHighlightThreshold - fLuminance) * 10 , 1);
    
    //return lerp(vColor, vHighlightColor, fHighlightIntensity);
}

struct VS_IN
{
    float3 vPosition : POSITION;
    float2 vTexcoord : TEXCOORD;
};

struct VS_OUT
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
};

/* 정점 쉐이더 */
VS_OUT VS_MAIN(VS_IN In)
{
    VS_OUT Out = (VS_OUT) 0;

    matrix matWV, matWVP;

    matWV = mul(g_WorldMatrix, g_ViewMatrix);
    matWVP = mul(matWV, g_ProjMatrix);

    Out.vPosition = mul(vector(In.vPosition, 1.f), matWVP);
    Out.vTexcoord = In.vTexcoord;

    return Out;
}

struct PS_IN
{
    float4 vPosition : SV_POSITION;
    float2 vTexcoord : TEXCOORD;
};

struct PS_OUT
{
    float4 vColor : SV_TARGET0;
};

PS_OUT PS_MAIN(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = g_Texture.Sample(LinearSampler, In.vTexcoord);

    return Out;
}

struct PS_OUT_LIGHT
{
    float4 vResultColor : SV_TARGET0;
    float4 vSpecular : SV_TARGET1;
    float4 vSSAO : SV_TARGET2;
};

/* 빛 하나당 480000 수행되는 쉐이더. */

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    float fMetalness = g_MRATexture.Sample(LinearSampler, In.vTexcoord).x;
    float fRoughness = g_MRATexture.Sample(LinearSampler, In.vTexcoord).y;
    float fAmbientOcclusion = g_MRATexture.Sample(LinearSampler, In.vTexcoord).z;
    
    vector vDepthDesc = g_DepthTexture.Sample(LinearSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * g_fFar;
    
    vDiffuse = pow(vDiffuse, 2.2f);
    
    float4 vWorldPos;
   /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;
   /* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos *= fViewZ;
   /* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);
   /* 로컬위치 * 월드행렬 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);
    
    // 월드 포지션에서 빛이 반사되어 우리 눈에 들어오는 방향 벡터
    float3 Lo = normalize(g_vCamPosition - vWorldPos);
    
    vector vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    // 월드 상의 노말벡터를 가져온다.
    float3 N = vNormalDesc.xyz * 2.f - 1.f;
    
    
    // 표면의 법선벡터와 빛이 반사되어 우리 눈에 들어오는 방향 벡터간의 내적
    float cosLo = max(0.0, dot(N, Lo));
    
    // 리플렉트
    float3 Lr = 2.0 * cosLo * N - Lo;
    
    // Fresnel reflectance at normal incidence (for metals use albedo color).
    float3 F0 = lerp(Fdielectric, vDiffuse.rgb, fMetalness);
    
    
    // Direct lighting calculation for analytical lights.
    float3 directLighting = 0.0;
    {
    
        float3 Li = -1.f * normalize(g_vLightDir);
        float3 Lradiance = 1.f;

	// Half-vector between Li and Lo.
        float3 Lh = normalize(Li + Lo);

    
	// Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.1, saturate(dot(N, Li) + 0.7f));
        float cosLh = max(0.0, dot(N, Lh));

        float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
        float D = ndfGGX(cosLh, fRoughness);
        float G = gaSchlickGGX(cosLi, cosLo, fRoughness);
    
    
        float3 kd = lerp(1.0 - F, 0.0, fMetalness);


    // 스페큘러가 높을수록 Diffuse를 잃는다.
        float3 diffuseBRDF = kd * vDiffuse.rgb;

	// Cook-Torrance specular BRDF. (공식임 ㅎㅎ)
        float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

	// 최종적인 결과물
        directLighting = (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }
    //////////////////    // Ambient lighting (IBL).
    
    float3 ambientLighting = 0.0;
    {
        float3 irradiance = g_EnvTexture.Sample(LinearSampler, N).rgb * fMetalness;
        float3 F = fresnelSchlick(F0, cosLo);
        float3 kd = lerp(1.0 - F, 0.0, fMetalness);
        float3 diffuseIBL = kd * vDiffuse.rgb * irradiance + 0.001;
        
        float2 specularBRDF = g_LUTTexture.Sample(LinearSampler, float2(cosLo, fRoughness)).rg;
        
        //float2 specularBRDF = float2(1.0, 0.0);
        
        //float3 specularIBL = (F0 * 1.0 + 0.0) * irradiance; // Adjusted for simplification
        float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * irradiance;
        //specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * irradiance;
        
        
        ambientLighting = diffuseIBL + specularIBL;
    }
    
    float4 vLightspecular = 0.0;
    {
        float3 vReflect = reflect(normalize(g_vLightDir.xyz), N);
        float fLightspecular = saturate(pow(max(dot(normalize(vReflect), normalize(Lo)), 0.0), 20.f * ( max(0.5, 1.f - fRoughness * 2.f) )) * saturate(1.f - fRoughness * 1.05f));
        vLightspecular = fLightspecular;

        vLightspecular += vDiffuse * vLightspecular.a;

    }
    
    // SSAO 옵션이 true인 경우에만
    if (g_bRenderSSAO == true)
    {
        float4 vSSAO = 0.0;
        {
            Out.vSSAO.rgb = 1.0;

            float2 rand = getRandom(In.vTexcoord);
            float radius = g_sample_radius / fViewZ;
        
            // SSAO Calculation
            int iterations = 4;

            if (fViewZ > 120.f)
                iterations = 2;
        
            for (int i = 0; i < iterations; ++i)
            {
                float2 coord1 = reflect(vCoordDir[i], rand) * radius;
                float2 coord2 = float2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
            
                vSSAO += doAmbientOcclusion(In.vTexcoord, coord1 * 0.25, vWorldPos.xyz, N);
                vSSAO += doAmbientOcclusion(In.vTexcoord, coord2 * 0.5, vWorldPos.xyz, N);
                vSSAO += doAmbientOcclusion(In.vTexcoord, coord1 * 0.75, vWorldPos.xyz, N);
                vSSAO += doAmbientOcclusion(In.vTexcoord, coord2, vWorldPos.xyz, N);
            }
        
            vSSAO /= (float) iterations * 4.0;
        }
        
        Out.vSSAO -= vSSAO;
    }
    
    Out.vResultColor = float4(directLighting + ambientLighting, 1.f) * fAmbientOcclusion;
    Out.vSpecular = vLightspecular;
    return Out;
}

PS_OUT_LIGHT PS_MAIN_POINT(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    float fMetalness = g_MRATexture.Sample(LinearSampler, In.vTexcoord).x;
    float fRoughness = g_MRATexture.Sample(LinearSampler, In.vTexcoord).y;

    vDiffuse = pow(vDiffuse, 2.2f);
       

    return Out;
}

PS_OUT_LIGHT PS_MAIN_DIRECTIONAL_FOR_TOOL(PS_IN In)
{
    PS_OUT_LIGHT Out = (PS_OUT_LIGHT) 0;

    vector vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);

    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    float fViewZ = vDepthDesc.y * g_fFar;
	
    Out.vSSAO = g_vLightDiffuse * saturate(max(dot(normalize(g_vLightDir) * -1.f, vNormal), 0.f) + g_vLightAmbient);

    float4 vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    float4 vLook = vWorldPos - g_vCamPosition;
    float4 vReflect = reflect(normalize(g_vLightDir), vNormal);

    float fSpecular = pow(max(dot(normalize(vLook) * -1.f, normalize(vReflect)), 0.f), 30.f);

    Out.vSpecular = g_vLightSpecular * fSpecular;

    return Out;
}

/* 최종적으로 480000 수행되는 쉐이더. */
PS_OUT PS_MAIN_FINAL(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_LinearTexture.Sample(LinearSampler, In.vTexcoord);
    
    Out.vColor = pow(vDiffuse, 1.0f / 2.2f);
    
    if (g_bRenderSSAO == true)
        Out.vColor *= g_SSAOTexture.Sample(LinearSampler, In.vTexcoord);
    
	/* 현재 픽셀의 월드상의 위치를 구한다. */

	/* ProjPos.w == View.Z */
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    
    float fViewZ = vDepthDesc.y * g_fFar;

    float4 vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    // 그림자 옵션 ON일 경우에만
    if (g_bRenderShadow == true)
    {
	/* 라이트 뷰, 투영을 곱한다. */
        vector vPosition = mul(vWorldPos, g_LightViewMatrix);
        vPosition = mul(vPosition, g_LightProjMatrix);

        float2 vTexcoord;

        vTexcoord.x = (vPosition.x / vPosition.w) * 0.5f + 0.5f;
        vTexcoord.y = (vPosition.y / vPosition.w) * -0.5f + 0.5f;

    
        vector vLightDepthDesc = g_LightDepthTexture.Sample(ClampSampler, vTexcoord);

	/* vPosition.w : 현재 내가 그릴려고 했던 픽셀의 광원기준의 깊이. */
	/* vLightDepthDesc.x * 2000.f : 현재 픽셀을 광원기준으로  그릴려고 했던 위치에 이미 그려져있떤 광원 기준의 깊이.  */
        if (vPosition.w > (vLightDepthDesc.x * 2000.f) && g_StencilTexture.Sample(LinearSampler, In.vTexcoord).x == 0.f)
        {
            Out.vColor *= 0.6f;
        }
    }

    //////// 림 라이트
    vector vRimLightDesc = g_RimLightTexture.Sample(ClampSampler, In.vTexcoord);
    float4 vLook = g_vCamPosition - vWorldPos;
    vector vNormalDesc = g_NormalTexture.Sample(PointSampler, In.vTexcoord);
    float4 vNormal = float4(vNormalDesc.xyz * 2.f - 1.f, 0.f);
    
    if (vRimLightDesc.g > 0.01f && vRimLightDesc.b == 1.f)
        Out.vColor += (1.f - (clamp(pow(dot(normalize(vLook), normalize(vNormal)), vRimLightDesc.g), 0.f, 1.f)));
    /////////
    
        
    vector vNonLight = g_NonLightTexture.Sample(LinearSampler, In.vTexcoord);
    vector vBlend = g_BlendTexture.Sample(LinearSampler, In.vTexcoord);
    vector vBlur = g_BlurTexture.Sample(LinearSampler, In.vTexcoord);
    vector vEffect = g_EffectTexture.Sample(LinearSampler, In.vTexcoord);
    vector vSky = g_SkyTexture.Sample(LinearSampler, In.vTexcoord);
    
    // 스카이박스와 이펙트의 결합 ( 블룸 및 블랜드 )
    if (0.0f == vDiffuse.a)
        Out.vColor += vSky * (1.f - vEffect.a);
    
    // 빛 연산이 되지 않고, Alpha값이 1인 객체들을 그대로 그린다.
    if (0.0f < vNonLight.a)
        Out.vColor = vNonLight;
        
    // Blend 라는 뜻
    if (vBlend.g == 1.f)
        Out.vColor *= (1.f - vEffect.a);

    // 기존 디퓨즈와 가산되어 그려진다.
    Out.vColor += vEffect + (vBlur * 2);
        
    
    
    if (g_DeferredInfoTexture.Sample(LinearSampler, In.vTexcoord).g == 1.f && g_StencilTexture.Sample(LinearSampler, In.vTexcoord).r != 1.f)
    {
        int ShadowTotal = 0;
        float2 vUV = (float2) 0;

        for (int i = -6; i < 7; ++i)
        {
            for (int j = -6; j < 7; ++j)
            {
                float2 Offset = float2(j, i);
                float2 TexOffset = Offset * float2(1.0f / g_fTexW * 0.8f, 1.0f / g_fTexH * 0.8f);
                vUV = In.vTexcoord + TexOffset;
                
                if (g_DeferredInfoTexture.Sample(LinearSampler, vUV).g != 1.f)
                    ShadowTotal++;
            }
        }
        
        // 0 ~ 196번 최대   0번일수록 0.6   196번 일수록 1이여야 한다.
        Out.vColor *= saturate(0.6f + (0.4f / 90.f * ShadowTotal));
    }
    else if (g_DeferredInfoTexture.Sample(LinearSampler, In.vTexcoord).g != 1.f)
    {
        Out.vColor *= g_fBlackBackGround;
    }
    
    return Out;
}

PS_OUT PS_MAIN_FINAL_FOR_TOOL(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    vector vDiffuse = g_DiffuseTexture.Sample(LinearSampler, In.vTexcoord);
    vector vShade = g_SSAOTexture.Sample(LinearSampler, In.vTexcoord);
    
    Out.vColor = vDiffuse * vShade;
    
	/* ProjPos.w == View.Z */
    vector vDepthDesc = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    
    float fViewZ = vDepthDesc.y * g_fFar;

    float4 vWorldPos;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 / View.z */
    vWorldPos.x = In.vTexcoord.x * 2.f - 1.f;
    vWorldPos.y = In.vTexcoord.y * -2.f + 1.f;
    vWorldPos.z = vDepthDesc.x;
    vWorldPos.w = 1.f;

	/* 로컬위치 * 월드행렬 * 뷰행렬 * 투영행렬 */
    vWorldPos *= fViewZ;

	/* 로컬위치 * 월드행렬 * 뷰행렬 */
    vWorldPos = mul(vWorldPos, g_ProjMatrixInv);

	/* 로컬위치 * 월드행렬 */
    vWorldPos = mul(vWorldPos, g_ViewMatrixInv);

    // 그림자 옵션 ON일 경우에만
    vector vPosition = mul(vWorldPos, g_LightViewMatrix);
    vPosition = mul(vPosition, g_LightProjMatrix);

    float2 vTexcoord;

    vTexcoord.x = (vPosition.x / vPosition.w) * 0.5f + 0.5f;
    vTexcoord.y = (vPosition.y / vPosition.w) * -0.5f + 0.5f;

    vector vLightDepthDesc = g_LightDepthTexture.Sample(ClampSampler, vTexcoord);

	/* vPosition.w : 현재 내가 그릴려고 했던 픽셀의 광원기준의 깊이. */
	/* vLightDepthDesc.x * 2000.f : 현재 픽셀을 광원기준으로  그릴려고 했던 위치에 이미 그려져있떤 광원 기준의 깊이.  */
    if (vPosition.w > (vLightDepthDesc.x * 2000.f) && g_StencilTexture.Sample(LinearSampler, In.vTexcoord).x == 0.f)
    {
        Out.vColor *= 0.6f;
    }    
        
    vector vNonLight = g_NonLightTexture.Sample(LinearSampler, In.vTexcoord);
    vector vBlend = g_BlendTexture.Sample(LinearSampler, In.vTexcoord);
    vector vBlur = g_BlurTexture.Sample(LinearSampler, In.vTexcoord);
    vector vEffect = g_EffectTexture.Sample(LinearSampler, In.vTexcoord);
    vector vSky = g_SkyTexture.Sample(LinearSampler, In.vTexcoord);
    
    // 스카이박스와 이펙트의 결합 ( 블룸 및 블랜드 )
    if (0.0f == vDiffuse.a)
        Out.vColor += vSky * (1.f - vEffect.a);
    
    // 빛 연산이 되지 않고, Alpha값이 1인 객체들을 그대로 그린다.
    if (0.0f < vNonLight.a)
        Out.vColor = vNonLight;
        
    // Blend 라는 뜻
    if (vBlend.g == 1.f)
        Out.vColor *= (1.f - vEffect.a);

    // 기존 디퓨즈와 가산되어 그려진다.
    Out.vColor += vEffect + (vBlur * 2);
        
    
    return Out;
}

PS_OUT PS_MAIN_BLUR_X(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = Blur_X(In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MAIN_BLUR_Y(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;

    Out.vColor = Blur_Y(In.vTexcoord);
    
    return Out;
}

PS_OUT PS_MAIN_RADIAL_BLUR(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    // 0~1 텍스트 쿠드 좌표로 환산하여 넣어준다.
    float2 fCenter = g_fRadialblurCenter;
    float2 fDirection = fCenter - In.vTexcoord;
    
    // 블러의 강도를 표현한다. 빠르게 감쇠시켜야한다.
    float fRadialblurRaduis = g_fRadialblurRaduis;
    
    float2 vUV = (float2) 0;

    for (int i = -6; i < 7; ++i)
    {
        vUV = In.vTexcoord + fDirection * i * float2(1.f / g_fTexW * fRadialblurRaduis, 1.f / g_fTexH * fRadialblurRaduis);
        Out.vColor += fWeight[6 + i] * (g_RadialBlur.Sample(ClampSampler, vUV));
    }
   
    Out.vColor /= fTotal;

    return Out;
  
}

PS_OUT PS_MAIN_COLORCORRECT(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector vColor = g_FinalTexture.Sample(LinearSampler, In.vTexcoord);
    //vColor.a = 1.f;
    
    if (g_bApplyCorrection)
    {
        //톤매핑
        vColor.rgb = ToneMapping(vColor.rgb, g_fExposure);
        
        float fLuminance = dot(vColor.rgb, float3(0.299, 0.587, 0.114));
        
        float3 vTonerColor = 0.f;
        //vTonerColor.rgb = vColor.rgb;
        
        int iTotalNum = 0;
        if (fLuminance < g_fShadowThreshold + .1f)
        {
            vTonerColor += AdjustShadow(vColor.rgb, g_vShadowColor, g_fShadowIntensity, fLuminance) - vColor.rgb;
            ++iTotalNum;
            //vColor.rgb = AdjustShadow(vColor.rgb, g_vShadowColor, g_fShadowIntensity, fLuminance);
        }
        if (g_fShadowThreshold - .1f < fLuminance && fLuminance < g_fHighlightThreshold + .1f)
        {
            vTonerColor += AdjustMidtone(vColor.rgb, g_vMidtoneColor, g_fMidtoneIntensity, fLuminance) - vColor.rgb;
            ++iTotalNum;
            //vColor.rgb = AdjustMidtone(vColor.rgb, g_vMidtoneColor, g_fMidtoneIntensity, fLuminance);
        }
        if (g_fHighlightThreshold - .1f < fLuminance)
        {
            vTonerColor += AdjustHighlight(vColor.rgb, g_vHighlightColor, g_fHighlightIntensity, fLuminance) - vColor.rgb;
            ++iTotalNum;
            //vColor.rgb = AdjustHighlight(vColor.rgb, g_vHighlightColor, g_fHighlightIntensity, fLuminance);
        }
        
        if (0 < iTotalNum)
            vColor.rgb += (vTonerColor / iTotalNum);
        
        
        vColor.rgb = AdjustWhiteBalance(vColor.rgb, g_vWhiteBalance);
        
        
        vColor.rgb = AdjustContrast(vColor.rgb, g_fContrast);
        
        
        //Color Balance
        vColor.rgb = AdjustColorBalance(vColor.rgb, g_vColorBalance);
        
        //HSV
        vColor.rgb = ColorGrading(vColor.rgb, g_fHue, g_fSaturation, g_fBrightness);
        
        //활기
        vColor.rgb = AdjustVibrance(vColor.rgb, g_fVibrance);
    
        //감마
        vColor.rgb = GammaCorrection(vColor.rgb, g_fGamma);
        
        vColor = saturate(float4(vColor.rgb, 1));

    }
    
    Out.vColor = vColor;
    
    return Out;
}


PS_OUT PS_MAIN_DOFBlur(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    if (g_bRenderDOF == false)
    {
        Out.vColor = g_DOFBlur.Sample(LinearSampler, In.vTexcoord);
        return Out;
    }
        
    
    // 초점 대상의 Depth값
    vector vDepthFocus = g_DepthTexture.Sample(PointSampler, g_vDOFFocus);
    float fKirbyViewZ = vDepthFocus.y * g_fFar;
    
    // 현재 픽셀의 Depth값
    vector vMyDepth = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    float fMyViewZ = vMyDepth.y * g_fFar;

    float2 vUV = (float2) 0;
    float fDOFTotal = 0.f;
    
     // 초점 깊이와 현재 깊이의 차이
    float fDepthDifference = abs(fKirbyViewZ - fMyViewZ);    
    float fDOFWeight = 0.f;
        
    //커비보다 내가 가까우면
    if(fMyViewZ < fKirbyViewZ)
    {
        float fDOFFar = fKirbyViewZ;
        // DOFWeight 계산 (스케일링 및 클램핑)
        fDOFWeight = saturate( pow(fDepthDifference / fDOFFar, 5.0) * 20.0);
    }
    else
    {
        float fDOFFar = g_fFar - fKirbyViewZ;
        fDOFWeight = saturate( pow(fDepthDifference / 100, 15.0) * 2.0);
    }
    
    
    Out.vColor = FreeBlur_X(In.vTexcoord, g_DOFBlur, fDOFWeight);
    // 공기 원근법 적용
    float airFactor = saturate(pow(fMyViewZ / 200.0, 3.0) - .4);
    float3 airColor = float3(-0.05, 0.01, 0.08); // 공기색 (파란색 계열)
    
    Out.vColor += float4(airFactor * airColor, 0.f);
    return Out;
    
    
}

PS_OUT PS_MAIN_DOFBlur_Result(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
        
    if (g_bRenderDOF == false)
    {
        Out.vColor = g_DOFBlur_Result.Sample(LinearSampler, In.vTexcoord);
        return Out;
    }
    
    // 초점 대상의 Depth값
    vector vDepthFocus = g_DepthTexture.Sample(PointSampler, g_vDOFFocus);
    float fKirbyViewZ = vDepthFocus.y * g_fFar;
    
    // 현재 픽셀의 Depth값
    vector vMyDepth = g_DepthTexture.Sample(PointSampler, In.vTexcoord);
    float fMyViewZ = vMyDepth.y * g_fFar;

    float2 vUV = (float2) 0;
    float fDOFTotal = 0.f;
    
     // 초점 깊이와 현재 깊이의 차이
    float fDepthDifference = abs(fKirbyViewZ - fMyViewZ);
    
    float fDOFWeight = 0.f;
        
    //커비보다 내가 가까우면
    if (fMyViewZ < fKirbyViewZ)
    {
        float fDOFFar = fKirbyViewZ;
        // DOFWeight 계산 (스케일링 및 클램핑)
        fDOFWeight = saturate(pow(fDepthDifference / fDOFFar, 5.0) * 20.0);
    }
    else
    {
        float fDOFFar = g_fFar - fKirbyViewZ;
        fDOFWeight = saturate(pow(fDepthDifference / 100, 15.0) * 2.0);
    }
    
    
    Out.vColor = FreeBlur_Y(In.vTexcoord, g_DOFBlur_Result, fDOFWeight);
    return Out;    
    
}

PS_OUT PS_MAIN_MotionBlur(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    
    float4 vMotionBlurSample = g_MotionBlur.Sample(LinearSampler, In.vTexcoord);
    
    // 모션이 없거나, 모션블러가 꺼져있다면?
    if (length(vMotionBlurSample) < 0.0001f || g_bRenderMotionBlur == false)
    {
        Out.vColor = g_DiffuseMotionBlur.Sample(ClampSampler, In.vTexcoord);
        return Out;
    }
    
    float2 vMyBlurDir = vMotionBlurSample.xy;
    
    
    float fMotionblurRaduis = 500.f;
    
    if (vMotionBlurSample.z == 1.f)
        fMotionblurRaduis = 1000.f;
    
    float2 vUV = (float2) 0;

    for (int i = -6; i < 7; ++i)
    {
        vUV = In.vTexcoord + (vMyBlurDir * i * float2(1.f / g_fTexW * fMotionblurRaduis, 1.f / g_fTexH * fMotionblurRaduis));
        Out.vColor += fWeight[6 + i] * (g_DiffuseMotionBlur.Sample(ClampSampler, vUV));
    }
    Out.vColor /= fTotal;

    return Out;
}

PS_OUT PS_UI_Default(PS_IN In)
{
    PS_OUT Out = (PS_OUT) 0;
    
    vector vColor = g_FinalTexture.Sample(LinearSampler, In.vTexcoord);
 
    vector vUIColor = g_UITexture.Sample(LinearSampler, In.vTexcoord);   
    
    Out.vColor = vColor;
    
    if (vUIColor.a != 0.f)
    {
        Out.vColor *= (1 - vUIColor.a);
        Out.vColor += vUIColor;
    }
        
    return Out;
}

technique11 DefaultTechnique
{
    pass Debug
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN();
    }
    // 태양광 계산 ( 1 )
    pass Light_Directional
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL();
    }
    // 점 광원 계산 ( 2 )
    pass Light_Point
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Blend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_POINT();
    }
    // 최종 셰이더 ( 3 )
    pass Final
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = /*compile gs_5_0 GS_MAIN()*/NULL;
        HullShader = /*compile hs_5_0 HS_MAIN()*/NULL;
        DomainShader = /*compile ds_5_0 DS_MAIN()*/NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FINAL();
    }
    // 블러 X ( 4 )
    pass Blur_X
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLUR_X();
    }
    // 블러 Y ( 5 )
    pass Blur_Y
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_BLUR_Y();
    }


    // 레디얼 블러 ( 6 )
    pass Radial_Blur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_RADIAL_BLUR();
    }

    // 색 후처리 ( 7 )
    pass ColorCorrection
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_COLORCORRECT();
    }

    // DOF 블러 ( 8 )
    pass DOF_Blur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DOFBlur();
    }

    // 모션 블러 ( 9 )
    pass Motion_Blur
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_MotionBlur();
    }

    // (10)
    pass UI_Default
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_UI_Default();
    }

    // DOF 블러 최종 ( 11 )
    pass DOF_Blur_Result
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DOFBlur_Result();
    }

//// For TOOL

    // Tool 에서의 방향성 광원 ( 12 )
    pass Light_Directional_For_Tool
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_DIRECTIONAL_FOR_TOOL();
    }

    // Tool 에서의 Render Result ( 13 )
    pass Final_For_Tool
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_NO_TEST_WRITE, 0);
        SetBlendState(BS_Default, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);

        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAIN_FINAL_FOR_TOOL();
    }

}