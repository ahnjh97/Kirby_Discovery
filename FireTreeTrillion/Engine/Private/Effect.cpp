#include "Effect.h"
#include "GameInstance.h"
CEffect::CEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CGameObject{ pDevice, pContext }
{
}

CEffect::CEffect(const CEffect& rhs)
    :CGameObject{ rhs }
    ,m_strFXName{rhs.m_strFXName}
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
    m_bIsColorRender = effectDesc.bIsColorRender;
    m_bIsBloom = effectDesc.bIsBloom;

    //desc 읽어 기본 초기화
    m_strFXName = effectDesc.strFXName;
    m_strBufferTag = CUtils::StrToWstr(effectDesc.strBufferTag);
    m_strTexTag = CUtils::StrToWstr(effectDesc.strTexTag);
    m_strMaskTexTag = CUtils::StrToWstr(effectDesc.strMaskTexTag);


    m_vInitPos = effectDesc.vInitPos;
    m_vInitRot = effectDesc.vInitRot;
    m_vInitScale = effectDesc.vInitScale;


    m_iPassIdx = effectDesc.iPassIdx;
    m_iTexIdx = effectDesc.iTexIdx;
    m_iMaskTexIdx = effectDesc.iMaskTexIdx;

    m_Keyframes = effectDesc.Keyframes;

    //각 키프레임 관리
    m_iCurKeyframeIdxs = new _uint[m_Keyframes.size()];
    ZeroMemory(m_iCurKeyframeIdxs, sizeof(_uint) * m_Keyframes.size());


    m_fDuration.second = effectDesc.fDuration;
    m_fLifetime = effectDesc.fLifetime;
    m_eRenderGroup = effectDesc.eRenderGroup;
    m_eTimer = effectDesc.eTimer == TIMER_END ? TIMER_NONE : effectDesc.eTimer;
    m_vContinuousRotation = effectDesc.vContinuousRotation;

    //if (0.f < m_vContinuousRotation.Length())
    //    int a = 0;

    if (effectDesc.pSocketMatrix != nullptr)
    {
        m_pSoketMatrix = effectDesc.pSocketMatrix;
    }

    return S_OK;
}

void CEffect::Reset_Duration()
{
    m_fDuration.first = 0.f;

    for (size_t i = KF_POS; i < KF_END; ++i)
    {
        m_iCurKeyframeIdxs[i] = 0;
    }
}

void CEffect::Fill_SaveData(_Out_ SINGLE_FX_DATA* pFXData)
{


    pFXData->iNameStrLen = (_uint)m_strFXName.size();
    pFXData->strName = m_strFXName;

    pFXData->iBufferStrLen = (_uint)m_strBufferTag.size();
    pFXData->strBufferName = CUtils::WstrToStr(m_strBufferTag);

    pFXData->iTexStrLen = (_uint)m_strTexTag.size();
    pFXData->strTexName = CUtils::WstrToStr(m_strTexTag);

    pFXData->iMaskTexStrLen = (_uint)m_strMaskTexTag.size();
    pFXData->strMaskTexName = CUtils::WstrToStr(m_strMaskTexTag);

    pFXData->fDuration = m_fDuration.second;
    pFXData->fLifetime = m_fLifetime;

    pFXData->iPassIdx = m_iPassIdx;
    pFXData->iTexIdx = m_iTexIdx;
    pFXData->iMaskTexIdx = m_iMaskTexIdx;

    pFXData->bIsLoop = m_bIsLoop;
    pFXData->bIsBillboard = m_bIsBillboard;
    pFXData->bIsOrthographic = m_bIsOrthographic;
    pFXData->bIsColorRender = m_bIsColorRender;
    pFXData->bIsBloom = m_bIsBloom;

    pFXData->fRimLightThreshold = m_fRimLightThreshold;
    pFXData->vContinuousRotation = m_vContinuousRotation;
    if (m_vContinuousRotation.y != 0.f)
        int a = 0;

    pFXData->iPropertyMapNum = m_Keyframes.size();

    for (auto& keyframePair : m_Keyframes)
    {
        pFXData->vecKeyframeInfo.push_back({ keyframePair.first , keyframePair.second.size()});
        pFXData->vecKeyframes.push_back(keyframePair.second);
    }

    pFXData->eRenderGroup = m_eRenderGroup;
    pFXData->eTimer = m_eTimer;
    if (m_eTimer == TIMER_SECOND)
        int a = 0;
}

void CEffect::Add_Keyframe(FX_KEYFRAME& newKeyframe, KF_PROPERTY eProperty)
{
    auto iter = lower_bound(m_Keyframes[eProperty].begin(), m_Keyframes[eProperty].end(), newKeyframe,
        [](const FX_KEYFRAME& A, const FX_KEYFRAME& B) {
            return A.fTimeRatio < B.fTimeRatio;
        });

    m_Keyframes[eProperty].insert(iter, newKeyframe);
}

void CEffect::Delete_Keyframe(KF_PROPERTY eProperty, _uint iKeyframeIdx)
{
    m_Keyframes[eProperty].erase(m_Keyframes[eProperty].begin() + iKeyframeIdx);
    m_iCurKeyframeIdxs[eProperty] = 0;
}

void CEffect::Add_RenderGroup()
{
    if (m_bIsBillboard)
        Billboard_Effect();

    if ((CRenderer::RENDERGROUP)m_eRenderGroup != CRenderer::RENDER_END)
        m_pGameInstance->Add_RenderGroup((CRenderer::RENDERGROUP)m_eRenderGroup, this);

    if (m_bIsBloom)
        m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
}

_bool CEffect::Update_Duration(_float fTimeDelta)
{
    return _bool();
}

_bool CEffect::Update_LifeTime(_float fTimeDelta)
{
    return _bool();
}

_bool CEffect::Calculate_Duration(_float _fTimeDelta)
{
    m_fDuration.first += _fTimeDelta;

    if (m_fDuration.second <= m_fDuration.first)
    {
        m_fDuration.first = m_fDuration.second;

        if (m_fDuration.second == FX_MAXDURATION)
        {
            m_fDuration.first = 0.f;
            return false;
        }
        return true;
    }
    return false;
}

_bool CEffect::Calculate_Lifetime(_float _fTimeDelta)
{
    if ( m_fDuration.first < m_fLifetime.first)
        return false;
    
    if (m_fLifetime.second <= m_fDuration.first)
    {
        if (m_bIsLoop || m_fDuration.second == FX_MAXDURATION)
        {
            _float fLength = m_fLifetime.second - m_fLifetime.first;
            m_fLifetime.first += fLength;
            m_fLifetime.second += fLength;
        }
        else
        {
            m_fLifeRatio = 1.f;
            return true;
        }
    }

    m_fLifeRatio = (m_fDuration.first - m_fLifetime.first) / (m_fLifetime.second - m_fLifetime.first);

    return false;
}

_float3 CEffect::Calculate_CurValue_Lerp(_float fTimeDelta, KF_PROPERTY eProperty, _bool bIsInEditor)
{
    vector<FX_KEYFRAME>& curKeyframes = m_Keyframes[eProperty];
    _float3 vResultValue = curKeyframes[0].vValue;

    //지금 키프레임이 마지막 키프레임?
    if (curKeyframes.size() - 1 <= m_iCurKeyframeIdxs[eProperty])
    {
        if (m_bIsLoop)
        {
            m_iCurKeyframeIdxs[eProperty] = 0;
        }
        //루프하지 않는다면, 마지막 키프레임 값으로 제한한다.
        else
        {
            vResultValue = curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue;
            return vResultValue;
        }
    }

    //현재 lifetime이 현재 키프레임 시간보다 작거나, 다음 키프레임 시간보다 클 때 보정
    if (curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].fTimeRatio <= m_fLifeRatio ||
        m_fLifeRatio <= curKeyframes[m_iCurKeyframeIdxs[eProperty]].fTimeRatio)
    {

        _uint iTempIdx = 0;
        while (iTempIdx < curKeyframes.size() - 1 &&
            curKeyframes[iTempIdx + 1].fTimeRatio <= m_fLifeRatio)
            ++iTempIdx;


        m_iCurKeyframeIdxs[eProperty] = iTempIdx;
    }

    //마지막 키프레임이면 하지마!!
    if (curKeyframes.size() - 1 <= m_iCurKeyframeIdxs[eProperty])
        return vResultValue;


    //진~짜 보간합니다 레츠고
    _float fCurKFRatio = curKeyframes[ m_iCurKeyframeIdxs[eProperty] ].fTimeRatio;
    _float fNextKFRatio = curKeyframes[ m_iCurKeyframeIdxs[eProperty] + 1 ].fTimeRatio;

    _float fInterpolateRatio = (m_fLifeRatio - fCurKFRatio) / (fNextKFRatio - fCurKFRatio); //비율을 구한다.

    switch (curKeyframes[m_iCurKeyframeIdxs[eProperty]].eEasing)
    {
    case EASE_IN:
        fInterpolateRatio = EASE_IN(fInterpolateRatio);
        break;
    case EASE_IN_FAST:
        fInterpolateRatio = EASE_IN_FAST(fInterpolateRatio);
        break;
    case EASE_OUT:
        fInterpolateRatio = EASE_OUT(fInterpolateRatio);
        break;
    case EASE_OUT_FAST:
        fInterpolateRatio = EASE_OUT_FAST(fInterpolateRatio);
        break;
    case EASE_INOUT:
        fInterpolateRatio = EASE_INOUT(fInterpolateRatio);
        break;
    case EASE_INOUT_FAST:
        fInterpolateRatio = EASE_INOUT_FAST(fInterpolateRatio);
        break;
    default: //그냥 Linear도 여기 포함
        break;
    }

    vResultValue = _float3::Lerp(curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue, curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].vValue, fInterpolateRatio);

    return vResultValue;
}

_float4 CEffect::Calculate_CurValue_Slerp(_float fTimeDelta, KF_PROPERTY eProperty, _bool bIsInEditor)
{
    vector<FX_KEYFRAME>& curKeyframes = m_Keyframes[eProperty];
    _float4 vResultValue = curKeyframes[0].vValue;

    //지금 키프레임이 마지막 키프레임?
    if (curKeyframes.size() - 1 <= m_iCurKeyframeIdxs[eProperty])
    {
        if (m_bIsLoop)
        {
            m_iCurKeyframeIdxs[eProperty] = 0;
        }
        //루프하지 않는다면, 마지막 키프레임 값으로 제한한다.
        else
        {
            vResultValue = Quaternion::CreateFromYawPitchRoll(curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue);
            return vResultValue;
        }
    }

    //현재 lifetime이 현재 키프레임 시간보다 작거나, 다음 키프레임 시간보다 클 때 보정
    if (curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].fTimeRatio <= m_fLifeRatio ||
        m_fLifeRatio <= curKeyframes[m_iCurKeyframeIdxs[eProperty]].fTimeRatio)
    {

        _uint iTempIdx = 0;
        while (iTempIdx < curKeyframes.size() - 1 &&
            curKeyframes[iTempIdx + 1].fTimeRatio <= m_fLifeRatio)
            ++iTempIdx;

        m_iCurKeyframeIdxs[eProperty] = iTempIdx;
    }

    //마지막 키프레임이면 하지마!!
    if (curKeyframes.size() - 1 <= m_iCurKeyframeIdxs[eProperty])
        return vResultValue;


    //진~짜 보간합니다 레츠고
    _float fCurKFRatio = curKeyframes[m_iCurKeyframeIdxs[eProperty]].fTimeRatio;
    _float fNextKFRatio = curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].fTimeRatio;

    _float fInterpolateRatio = (m_fLifeRatio - fCurKFRatio) / (fNextKFRatio - fCurKFRatio); //비율을 구한다.

    switch (curKeyframes[m_iCurKeyframeIdxs[eProperty]].eEasing)
    {
    case EASE_IN:
        fInterpolateRatio = EASE_IN(fInterpolateRatio);
        break;
    case EASE_IN_FAST:
        fInterpolateRatio = EASE_IN_FAST(fInterpolateRatio);
        break;
    case EASE_OUT:
        fInterpolateRatio = EASE_OUT(fInterpolateRatio);
        break;
    case EASE_OUT_FAST:
        fInterpolateRatio = EASE_OUT_FAST(fInterpolateRatio);
        break;
    case EASE_INOUT:
        fInterpolateRatio = EASE_INOUT(fInterpolateRatio);
        break;
    case EASE_INOUT_FAST:
        fInterpolateRatio = EASE_INOUT_FAST(fInterpolateRatio);
        break;
    default: //그냥 Linear도 여기 포함
        break;
    }

    _float3 vFirstRadian{ curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue };
    vFirstRadian = { ToRadian(vFirstRadian.x), ToRadian(vFirstRadian.y) , ToRadian(vFirstRadian.z) };
    _float3 vSecondRadian{ curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].vValue };
    vSecondRadian = { ToRadian(vSecondRadian.x), ToRadian(vSecondRadian.y) , ToRadian(vSecondRadian.z) };

    Quaternion vFirstQuat, vSecondQuat, vResultQuat;
    vFirstQuat = Quaternion::CreateFromYawPitchRoll(vFirstRadian);
    vSecondQuat = Quaternion::CreateFromYawPitchRoll(vSecondRadian);

    vResultValue = Quaternion::Slerp(vFirstQuat, vSecondQuat, fInterpolateRatio);

    return vResultValue;
}

void CEffect::Billboard_Effect()
{
    // 빌보드
    _float3   vScale = m_pTransformCom->Get_Scaled();

    _matrix CamMatrix = m_pGameInstance->Get_Transform_Inv(CPipeLine::D3DTS_VIEW);

    _vector vRight, vUp, vLook;

    vRight = CamMatrix.r[0];
    vUp = CamMatrix.r[1];
    vLook = CamMatrix.r[2];


    vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 1.f), vLook);
    vLook = XMVector3Cross(vRight, vUp);
    m_pTransformCom->Set_State(CTransform::STATE_RIGHT, XMVector3Normalize(vRight) * vScale.x);
    m_pTransformCom->Set_State(CTransform::STATE_UP, XMVector3Normalize(vUp) * vScale.y);
    m_pTransformCom->Set_State(CTransform::STATE_LOOK, XMVector3Normalize(vLook) * vScale.z);

}

void CEffect::Free()
{
    __super::Free();

    Safe_Delete_Array(m_iCurKeyframeIdxs);
}
