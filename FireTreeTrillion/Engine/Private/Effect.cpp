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
    m_bIsColorRender = effectDesc.bIsNonLight;
    m_bIsBloom = effectDesc.bIsBloom;

    //desc 읽어 기본 초기화
    m_strFXName = effectDesc.strFXName;
    m_strBufferTag = CUtils::StrToWstr(effectDesc.strBufferTag);
    m_strTexTag = CUtils::StrToWstr(effectDesc.strBufferTag);
    m_strMaskTexTag = CUtils::StrToWstr(effectDesc.strBufferTag);


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

    //_int iSize = m_iCurKeyframeIdxs[0];
    //iSize = m_iCurKeyframeIdxs[1];
    //iSize = m_iCurKeyframeIdxs[2];
    //iSize = m_iCurKeyframeIdxs[3];
    //ZeroMemory(m_iCurKeyframeIdxs, )
    //_int iSize = m;
    m_vCurRColor = effectDesc.vRColor;
    m_vCurGColor = effectDesc.vGColor;
    m_vCurBColor = effectDesc.vBColor;

    //m_keyfra = effectDesc.Keyframes;

    return S_OK;
}

void CEffect::Fill_SaveData(_Out_ SINGLE_FX_DATA* pEffectData)
{
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
        return true;
    }

    return false;
}

_bool CEffect::Calculate_Lifetime(_float _fTimeDelta)
{
    if ( m_fDuration.first < m_fLifeTime.first)
        return false;

    //m_fLifeTime.first += _fTimeDelta;
    
    if (m_fLifeTime.second <= m_fDuration.first)
    {
        if (m_bIsLoop)
        {
            _float fLength = m_fLifeTime.second - m_fLifeTime.first;
            m_fLifeTime.first += fLength;
            m_fLifeTime.second += fLength;
        }
        else
        {
            m_fLifeRatio = 1.f;
            return true;
        }
    }

    m_fLifeRatio = (m_fDuration.first - m_fLifeTime.first) / (m_fLifeTime.second - m_fLifeTime.first);

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

    //_float4 vFirstQuat = XMQuaternionRotationRollPitchYawFromVector(m_pGameInstance->MakeRollPitchYaw(m_KeyFrames[m_iCurKFIdx].vRotation));
    //_float4 vFirstQuat =   curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue.r;
    //_float4 vSecondQuat = XMQuaternionRotationRollPitchYawFromVector(m_pGameInstance->MakeRollPitchYaw(m_KeyFrames[m_iCurKFIdx + 1].vRotation));
    //_float4 vResultQuat = XMQuaternionSlerp(vFirstQuat, vSecondQuat, fInterpolateRatio);
    //_float3 vFirstDegree, vSecondDegree;

    _float3 vFirstRadian{ curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue };
    vFirstRadian = { ToRadian(vFirstRadian.x), ToRadian(vFirstRadian.y) , ToRadian(vFirstRadian.z) };
    _float3 vSecondRadian{ curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].vValue };
    vSecondRadian = { ToRadian(vSecondRadian.x), ToRadian(vSecondRadian.y) , ToRadian(vSecondRadian.z) };

    Quaternion vFirstQuat, vSecondQuat, vResultQuat;
    vFirstQuat = Quaternion::CreateFromYawPitchRoll(vFirstRadian);
    vSecondQuat = Quaternion::CreateFromYawPitchRoll(vSecondRadian);

    vResultValue = Quaternion::Slerp(vFirstQuat, vSecondQuat, fInterpolateRatio);

    //return 

    ////회전
    //_vector vInitialRot = XMQuaternionRotationRollPitchYawFromVector(m_pGameInstance->MakeRollPitchYaw(m_vInitialRot));
    //_vector vResultRotQuat = XMQuaternionMultiply(vCurRotation, vInitialRot);

    ////_vector vResultRollPitchYaw = m_pGameInstance->QuaternionToEulerAngles(vResultRotQuat);
    ////XMQuaternionMultiply(vInitialRot, vCurRotation);
    //VecToF3(m_vCurRotation, m_pGameInstance->QuaternionToEulerAngles(vResultRotQuat));
    //VecToF4(m_vCurRotQuat, vResultRotQuat);




    //vResultValue = _float3::Lerp(curKeyframes[m_iCurKeyframeIdxs[eProperty]].vValue, curKeyframes[m_iCurKeyframeIdxs[eProperty] + 1].vValue, fInterpolateRatio);


    return vResultValue;
}

void CEffect::Free()
{
    __super::Free();
}
