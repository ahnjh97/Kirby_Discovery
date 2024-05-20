#include "Effect.h"
#include "GameInstance.h"
CEffect::CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject{ pDevice, pContext }
{
}

CEffect::CEffect(const CEffect& rhs)
    :CGameObject{ rhs }
{
}

HRESULT CEffect::Initialize(void* pArg)
{
    FX_DESC effectDesc{};

    effectDesc.fSpeedPerSec = 10.f;
    effectDesc.fRotationPerSec = ToRadian(90.0f);


    if (nullptr != pArg)
    {
        effectDesc = *(FX_DESC*)pArg;
    }


    HRESULT hr = __super::Initialize(&effectDesc);
    CHECK_FAILED_MSG(hr, "Failed To Initialize : CCamera_Main");

    //기본 bool 설정
    m_bIsLoop = effectDesc.bIsLoop;
    m_bIsBillboard = effectDesc.bIsBillboard;
    m_bIsOrthographic = effectDesc.bIsOrthographic;
    m_bIsNonLight = effectDesc.bIsNonLight;
    m_bIsBloom = effectDesc.bIsBloom;

    //desc 읽어 기본 초기화
    m_strFXName = CUtils::StrToWstr(effectDesc.strFXName);
    m_strBufferTag = CUtils::StrToWstr(effectDesc.strBufferTag);
    m_strTexTag = CUtils::StrToWstr(effectDesc.strBufferTag);
    m_strMaskTexTag = CUtils::StrToWstr(effectDesc.strBufferTag);


    m_vInitPos = effectDesc.vInitPos;
    m_vInitRot = effectDesc.vInitRot;
    m_vInitScale = effectDesc.vInitScale;


    m_iPassIdx = effectDesc.iPassIdx;
    m_iTexIdx = effectDesc.iTexIdx;
    m_iMaskTexIdx = effectDesc.iMaskTexIdx;

    m_vCurRColor = effectDesc.vRColor;
    m_vCurGColor = effectDesc.vGColor;
    m_vCurBColor = effectDesc.vBColor;

    //m_keyfra = effectDesc.Keyframes;

    return S_OK;
}

void CEffect::Fill_SaveData(_Out_ FX_DATA* pEffectData)
{
}

void CEffect::Add_Keyframe(FX_KEYFRAME& newKeyframe)
{
}


_bool CEffect::Update_Duration(_float fTimeDelta)
{
    return _bool();
}

_bool CEffect::Update_LifeTime(_float fTimeDelta)
{
    return _bool();
}

void CEffect::Calculate_CurValue(_float fTimeDelta, _bool bIsInEditor)
{
}

void CEffect::Free()
{
}
