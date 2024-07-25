#include "stdafx.h"
#include "KirbyBulb_State.h"
#include "Kirby_State_Function.h"

void Kirby_EyeState_Assist(CKirby::KIRBY_INFODESC* Kirbydesc)
{
    if (DESC(m_bLightOn) == true)
        DESC(m_eEyeState) = CKirby::EYE_CLOSE;
    else
        DESC(m_eEyeState) = CKirby::EYE_IDLE;
}

#pragma region IDLE STATE

CKirbyBulb_Idle_State::CKirbyBulb_Idle_State()
{
}

void CKirbyBulb_Idle_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBulb_Idle_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);

    pController->FreeFall(pTransformCom, fTimeDelta);
    Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

    if (DESC(m_pLight) != nullptr)
        DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());


    if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == true && DESC(m_bLightOn) == false)
    {
        DESC(m_pLight)->Interpolate_Light(_float4(1.f, 1.f, 1.f, 0.f), 14.f, 0.2f);
        pKirby->Change_State(CKirby::BULBSTATE_LIGHTON, 300.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = true;
        return;
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_bLightOn) == true)
    {
        DESC(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 6.f, 1.f);
        pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        m_pGameInstance->StopSound(CHANNEL_PLAYERVOICE);
        DESC(m_bLightOn) = false;
        return;
    }

    // 차량을 땅에 버리는 로직이다.
    if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
    {
        DESC(m_bDumpAbilityPress) = true;
        DESC(m_fDumpAbilityTime) += fTimeDelta;

        if (DESC(m_fDumpAbilityTime) > 1.f)
        {
            DESC(m_fDumpAbilityTime) = 0.f;
            DESC(m_fJumpVelocity) = 15.f;
            pKirby->Change_State(CKirby::STATE_SPITDEFORM, 60.f, false, false, CKirby::BODY_VACUUM);
            return;
        }
    }
    else
    {
        DESC(m_bDumpAbilityPress) = false;
    }


    if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) == true)
    {
        // 점프의 초기 파워
        DESC(m_fJumpVelocity) = 22.f;
        DESC(m_eEyeState) = CKirby::EYE_IDLE;
        DESC(m_fChangeVelocityZeroTime) = 0.f;
        // 공중에서 체공하는 시간 0.15초
        DESC(m_fHoldAirTime) = 0.f;
        // 점프키를 누르는 시간
        DESC(m_fJumpHoldTime) = 0.f;
        // 재입력 블락기능 초기화
        DESC(m_bRePressBlock) = false;

        pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        return;
    }

    if (JoyStick_On() == true)
    {
        if (DESC(m_bLightOn) == true)
        {
            pKirby->Change_State(CKirby::BULBSTATE_MOVEBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }
        else
        {
            pKirby->Change_State(CKirby::BULBSTATE_MOVE, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }
    }

}

void CKirbyBulb_Idle_State::OnStateExit()
{
}

CKirbyBulb_Idle_State* CKirbyBulb_Idle_State::Create()
{
    CKirbyBulb_Idle_State* pInstance = new CKirbyBulb_Idle_State();
    return pInstance;
}

void CKirbyBulb_Idle_State::Free()
{
    __super::Free();
}

#pragma endregion

#pragma region RUN STATE

CKirbyBulb_Run_State::CKirbyBulb_Run_State()
{
}

void CKirbyBulb_Run_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBulb_Run_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);

    Bbong_FX(fTimeDelta, pTransformCom);
    Make_BBongSound(DESC(m_fWalkSoundDelay), fTimeDelta);

    if (DESC(m_pLight) != nullptr)
        DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());

    if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == true && DESC(m_bLightOn) == false)
    {
        DESC(m_pLight)->Interpolate_Light(_float4(1.f, 1.f, 1.f, 0.f), 14.f, 0.2f);
        pKirby->Change_State(CKirby::BULBSTATE_LIGHTON, 300.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = true;
        return;
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_bLightOn) == true)
    {
        DESC(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 6.f, 1.f);
        pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = false;
        return;
    }

    // 차량을 땅에 버리는 로직이다.
    if (m_pGameInstance->Get_DIKeyState(DIK_V, KEY_PRESS))
    {
        DESC(m_bDumpAbilityPress) = true;
        DESC(m_fDumpAbilityTime) += fTimeDelta;

        if (DESC(m_fDumpAbilityTime) > 1.f)
        {
            DESC(m_fDumpAbilityTime) = 0.f;
            DESC(m_fJumpVelocity) = 15.f;
            pKirby->Change_State(CKirby::STATE_SPITDEFORM, 60.f, false, false, CKirby::BODY_VACUUM);
            return;
        }
    }
    else
    {
        DESC(m_bDumpAbilityPress) = false;
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) == true)
    {
        // 점프의 초기 파워
        DESC(m_fJumpVelocity) = 22.f;
        DESC(m_eEyeState) = CKirby::EYE_IDLE;
        DESC(m_fChangeVelocityZeroTime) = 0.f;
        // 공중에서 체공하는 시간 0.15초
        DESC(m_fHoldAirTime) = 0.f;
        // 점프키를 누르는 시간
        DESC(m_fJumpHoldTime) = 0.f;
        // 재입력 블락기능 초기화
        DESC(m_bRePressBlock) = false;

        pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        return;
    }


    pController->FreeFall(pTransformCom, fTimeDelta);
    Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

    if (pKirby->Get_State() == CKirby::BULBSTATE_MOVEBRIGHT || pKirby->Get_State() == CKirby::BULBSTATE_MOVE)
    {
        if (JoyStick_controller(Kirbydesc, pCamera) == true)
        {
            if (pKirby->Get_State() == CKirby::BULBSTATE_MOVEBRIGHT)
            {
                Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
                if (Kirbydesc->m_fMoveSpeed > 5.f)
                    Kirbydesc->m_fMoveSpeed = 5.f;

                // 타겟기준
                _vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
                pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
            }
            else if (pKirby->Get_State() == CKirby::BULBSTATE_MOVE)
            {
                Kirbydesc->m_fMoveSpeed += fTimeDelta * 70.f;
                if (Kirbydesc->m_fMoveSpeed > 8.3f)
                    Kirbydesc->m_fMoveSpeed = 8.3f;

                // 타겟기준
                _vector vMoveDelta = Kirbydesc->m_vTargetDir * fTimeDelta * Kirbydesc->m_fMoveSpeed;
                pController->Move_Dir(pTransformCom, vMoveDelta, fTimeDelta);
            }
        }
        else
        {
            if (DESC(m_bLightOn) == true)
            {
                pKirby->Change_State(CKirby::BULBSTATE_STOPBRIGHT, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
            else
            {
                pKirby->Change_State(CKirby::BULBSTATE_STOP, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
        }
    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_STOPBRIGHT || pKirby->Get_State() == CKirby::BULBSTATE_STOP)
    {
        Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

        if (JoyStick_controller(Kirbydesc, pCamera) == true)
        {
            if (DESC(m_bLightOn) == true)
            {
                pKirby->Change_State(CKirby::BULBSTATE_MOVEBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
            else
            {
                pKirby->Change_State(CKirby::BULBSTATE_MOVE, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
        }



        if (pKirby->isAnimFinish())
        {
            if (DESC(m_bLightOn) == true)
            {
                pKirby->Change_State(CKirby::BULBSTATE_WAITBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
            else
            {
                pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
                return;
            }
        }
    }
}

void CKirbyBulb_Run_State::OnStateExit()
{
}

CKirbyBulb_Run_State* CKirbyBulb_Run_State::Create()
{
    CKirbyBulb_Run_State* pInstance = new CKirbyBulb_Run_State();
    return pInstance;
}

void CKirbyBulb_Run_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region JUMP STATE

CKirbyBulb_Jump_State::CKirbyBulb_Jump_State()
{
}

void CKirbyBulb_Jump_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);
}

void CKirbyBulb_Jump_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);

    if (DESC(m_pLight) != nullptr)
        DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());

    if (pKirby->Get_State() == CKirby::BULBSTATE_LANDING || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGBRIGHT ||
        pKirby->Get_State() == CKirby::BULBSTATE_LANDINGEND || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGENDBRIGHT)
        Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
    else
        Jump_Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);

    if (JoyStick_controller(Kirbydesc, pCamera))
    {
        Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
    }
    else
    {
        if (pKirby->Get_State() == CKirby::BULBSTATE_LANDING || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGBRIGHT ||
            pKirby->Get_State() == CKirby::BULBSTATE_LANDINGEND || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGENDBRIGHT)
            Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
        else
            Jump_Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);
    }



    if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == true && DESC(m_bLightOn) == false)
    {
        if (pKirby->Get_State() == CKirby::BULBSTATE_JUMP)
            pKirby->Change_State(CKirby::BULBSTATE_LIGHTONAIR, 400.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        else
            pKirby->Change_State(CKirby::BULBSTATE_LIGHTON, 300.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = true;
        DESC(m_pLight)->Interpolate_Light(_float4(1.f, 1.f, 1.f, 0.f), 14.f, 0.2f);

    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_bLightOn) == true)
    {
        if (pKirby->Get_State() != CKirby::BULBSTATE_JUMP)
            pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = false;
        DESC(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 6.f, 1.f);

    }


    if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_UP))
    {
        // 때는 순간 트루가 되고, 점프 가능 시점에 다시 누를 때 까지 C에대한 누적 등 반응하지 않는다.
        DESC(m_bRePressBlock) = true;
    }

    if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN) &&
        (pKirby->Get_State() == CKirby::BULBSTATE_LANDING || 
            pKirby->Get_State() == CKirby::BULBSTATE_LANDINGBRIGHT || 
            pKirby->Get_State() == CKirby::BULBSTATE_LANDINGEND || 
            pKirby->Get_State() == CKirby::BULBSTATE_LANDINGENDBRIGHT) == false)
    {
        // 점프 예약을 한다.
        if (pController->Compute_Height() < 2.f &&
            (pKirby->Get_State() == CKirby::BULBSTATE_JUMP) == true)
        {
            DESC(m_bReserveJumpKey) = true;
        }
    }


    if (pKirby->Get_State() == CKirby::BULBSTATE_JUMP || pKirby->Get_State() == CKirby::BULBSTATE_LIGHTONAIR)
    {
        if (m_bJumpEffectTrigger == true && (pKirby->Get_State() == CKirby::BULBSTATE_JUMP))
        {
            Jump_FX(pTransformCom, true);
            m_bJumpEffectTrigger = false;
        }

        // 0.3초 동안만 누적이 된다.
        if (DESC(m_bRePressBlock) == false && m_pGameInstance->Get_DIKeyState(DIK_C, KEY_PRESS) && DESC(m_fJumpHoldTime) < 0.3f)
        {
            DESC(m_fJumpHoldTime) += fTimeDelta;
            DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
            pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
        }
        else
        {
            // 최소 점프 제한 키 유지 시간
            _float fJumpHoldTime = 0.1f;
            // 공중 체공 시간
            _float fHoldAirTime = 0.22f;
            // 최소단위 공중 올라가는 시간
            _float fChangeVelocityZeroTime = 0.09f;
            // 최소 점프시간 이하로 눌렀을 때
            if (DESC(m_fJumpHoldTime) < fJumpHoldTime)
            {
                DESC(m_fChangeVelocityZeroTime) += fTimeDelta;

                // 만약, 최소 점프 제한키 이하로 눌렀을 때, 최소 단위 공중 올라가는 시간이 지났다면
                if (DESC(m_fChangeVelocityZeroTime) > fChangeVelocityZeroTime)
                {
                    // 공중 체공시간동안 중력값은 보정된다.
                    DESC(m_fHoldAirTime) += fTimeDelta;
                    if (DESC(m_fHoldAirTime) < fHoldAirTime)
                    {
                        // 중력값 보정식
                        _float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
                        DESC(m_fJumpVelocity) = fStopVelocityPower + (fabs(DESC(m_fHoldAirTime) - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
                    }
                }
            }
            // 만약, 0.1초 이상 눌렀을 경우
            else
            {
                // 바로 공중 체공시간이 가산되어 중력값이 보정된다.
                DESC(m_fHoldAirTime) += fTimeDelta;
                if (DESC(m_fHoldAirTime) < fHoldAirTime)
                {
                    _float fStopVelocityPower = GRAVITY * fTimeDelta * 6.f;
                    DESC(m_fJumpVelocity) = fStopVelocityPower + (fabs(DESC(m_fHoldAirTime) - fHoldAirTime * 0.5f) * (-1.f / (fHoldAirTime * 0.5f)));
                }

            }
            DESC(m_fJumpVelocity) -= GRAVITY * fTimeDelta * DESC(m_fGravityOffset);
            pController->Jump(pTransformCom, DESC(m_fJumpVelocity), fTimeDelta);
        }

        if (pKirby->Get_State() == CKirby::BULBSTATE_LIGHTONAIR && pKirby->isAnimFinish())
        {
            pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }

        // 만약, 땅에 안전하게 착지했을 경우, 홀딩 시간에 따라 뽀잉 애니메이션이 분기된다.
        if (pController->Is_Terrain())
        {
            if (DESC(m_fJumpHoldTime) > 0.2f)
            {
                CMultiEffect::MULTI_FX_DESC FXDesc{};
                _float4 vKirbyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
                _float4 vKirbyLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

                FXDesc.vInitPos = { vKirbyPos.x, vKirbyPos.y + .4f, vKirbyPos.z };
                FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y + 20.f, 0.f };
                FXDesc.vInitScale = { 1.3f, 1.3f, 1.3f };

                if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Smoke Fast"), &FXDesc)))
                    return;

                FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y - 20.f, 0.f };
                if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Smoke Fast"), &FXDesc)))
                    return;

                m_pGameInstance->PlaySound_Free(L"Kirby_BigLanding.wav", 0.2f);

            }
            else
            {
                CMultiEffect::MULTI_FX_DESC FXDesc{};
                _float4 vKirbyPos = pTransformCom->Get_State(CTransform::STATE_POSITION);
                _float4 vKirbyLook = pTransformCom->Get_State(CTransform::STATE_LOOK);

                FXDesc.vInitPos = { vKirbyPos.x, vKirbyPos.y + .4f, vKirbyPos.z };
                FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y + 30.f, 0.f };
                FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };

                if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
                    return;

                FXDesc.vInitRot = { 0.f, CUtils::Make_Degree_FromDir(vKirbyLook).y - -30.f, 0.f };
                if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_BBong"), &FXDesc)))
                    return;

                m_pGameInstance->PlaySound_Free(L"Kirby_BigLanding.wav", 0.2f);

            }

            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_LANDINGBRIGHT, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_LANDING, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }
    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_LANDING || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGBRIGHT)
    {
        // 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
        _float fChangeRunTime = 0.08f;

        m_fChangeRunTime += fTimeDelta;


        if (DESC(m_bReserveJumpKey) == true)
        {
            // 점프의 초기 파워
            DESC(m_fJumpVelocity) = 22.f;
            DESC(m_eEyeState) = CKirby::EYE_IDLE;

            DESC(m_fChangeVelocityZeroTime) = 0.f;
            // 공중에서 체공하는 시간 0.15초
            DESC(m_fHoldAirTime) = 0.f;
            // 점프키를 누르는 시간
            DESC(m_fJumpHoldTime) = 0.f;

            // 재입력 블락기능 초기화
            DESC(m_bRePressBlock) = false;
            // 예약 초기화
            DESC(m_bReserveJumpKey) = false;
            pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        }

        // Idle일 때, C를 누르면 점프를 한다.
        if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
        {
            DESC(m_eEyeState) = CKirby::EYE_IDLE;
            // 점프의 초기 파워
            DESC(m_fJumpVelocity) = 22.f;

            DESC(m_fChangeVelocityZeroTime) = 0.f;
            // 공중에서 체공하는 시간 0.15초
            DESC(m_fHoldAirTime) = 0.f;
            // 점프키를 누르는 시간
            DESC(m_fJumpHoldTime) = 0.f;
            // 재입력 블락기능 초기화
            DESC(m_bRePressBlock) = false;
            pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        }

        // 바로 방향키를 갈겼다면
        if (m_fChangeRunTime > fChangeRunTime && JoyStick_controller(Kirbydesc, pCamera))
        {
            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_MOVEBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_MOVE, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);

            return;
        }


        if (pKirby->isAnimFinish())
        {
            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_LANDINGENDBRIGHT, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_LANDINGEND, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        }

    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_LANDINGEND || pKirby->Get_State() == CKirby::BULBSTATE_LANDINGENDBRIGHT)
    {
        // 최소 애니메이션이 재생되는 시간이다. ( 방향키를 누르면 0.2초 후 바로 Run 상태가 됨 )
        _float fChangeRunTime = 0.08f;

        m_fChangeRunTime += fTimeDelta;


        // Idle일 때, C를 누르면 점프를 한다.
        if (m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN))
        {
            DESC(m_eEyeState) = CKirby::EYE_IDLE;
            // 점프의 초기 파워
            DESC(m_fJumpVelocity) = 22.f;

            DESC(m_fChangeVelocityZeroTime) = 0.f;
            // 공중에서 체공하는 시간 0.15초
            DESC(m_fHoldAirTime) = 0.f;
            // 점프키를 누르는 시간
            DESC(m_fJumpHoldTime) = 0.f;
            // 재입력 블락기능 초기화
            DESC(m_bRePressBlock) = false;
            pKirby->Change_State(CKirby::BULBSTATE_JUMP, 60.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        }

        // 바로 방향키를 갈겼다면
        if (m_fChangeRunTime > fChangeRunTime && JoyStick_controller(Kirbydesc, pCamera))
        {
            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_MOVEBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_MOVE, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);

            return;
        }

        if (pKirby->isAnimFinish())
        {
            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_WAITBRIGHT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        }
    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_FALL)
    {
        pController->FreeFall(pTransformCom, fTimeDelta, DESC(m_fGravityOffset));
        m_fFallTime += fTimeDelta;

        if (pController->Is_Terrain())
        {
            if (DESC(m_bLightOn) == true)
                pKirby->Change_State(CKirby::BULBSTATE_LANDINGBRIGHT, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            else
                pKirby->Change_State(CKirby::BULBSTATE_LANDING, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);

            return;
        }
    }
}

void CKirbyBulb_Jump_State::OnStateExit()
{
    m_fFallTime = 0.f;
    m_fChangeRunTime = 0.f;
    m_bJumpEffectTrigger = true;
}

CKirbyBulb_Jump_State* CKirbyBulb_Jump_State::Create()
{
    CKirbyBulb_Jump_State* pInstance = new CKirbyBulb_Jump_State();
    return pInstance;
}

void CKirbyBulb_Jump_State::Free()
{
    __super::Free();

}

#pragma endregion



#pragma region LIGHT STATE

CKirbyBulb_Light_State::CKirbyBulb_Light_State()
{
}

void CKirbyBulb_Light_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

    if (_iAnimIndex == CKirby::BULBSTATE_LIGHTON)
    {
        m_pGameInstance->PlaySound_Free(L"KirbyBulb_LightOn.wav", 0.5f);
        m_pGameInstance->LoopSound(CHANNEL_PLAYERVOICE, L"KirbyBulb_Lighting.wav", 0.05f);
    }
}

void CKirbyBulb_Light_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);

    if (DESC(m_pLight) != nullptr)
        DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());


    if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == true && DESC(m_bLightOn) == false)
    {
        pKirby->Change_State(CKirby::BULBSTATE_LIGHTON, 300.f, false, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = true;
        DESC(m_pLight)->Interpolate_Light(_float4(1.f, 1.f, 1.f, 0.f), 14.f, 0.2f);

        return;
    }
    else if (m_pGameInstance->Get_DIKeyState(DIK_X, KEY_PRESS) == false && DESC(m_bLightOn) == true)
    {
        pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        DESC(m_bLightOn) = false;
        DESC(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 6.f, 1.f);
        return;
    }

    if (pKirby->Get_State() == CKirby::BULBSTATE_LIGHTON)
    {
        pController->FreeFall(pTransformCom, fTimeDelta);
        Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
        if (JoyStick_controller(Kirbydesc, pCamera) == true)
            Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
        else
            Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);


        if (pKirby->isAnimFinish())
        {
            pKirby->Change_State(CKirby::BULBSTATE_WAITBRIGHT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }
    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_LIGHTOFF)
    {
        pController->FreeFall(pTransformCom, fTimeDelta);
        Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
        if (JoyStick_controller(Kirbydesc, pCamera) == true)
            Moving_Logic(Kirbydesc, pTransformCom, pController, fTimeDelta);
        else
            Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

        if (pKirby->isAnimFinish())
        {
            pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, true, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }
    }
}

void CKirbyBulb_Light_State::OnStateExit()
{
}

CKirbyBulb_Light_State* CKirbyBulb_Light_State::Create()
{
    CKirbyBulb_Light_State* pInstance = new CKirbyBulb_Light_State();
    return pInstance;
}

void CKirbyBulb_Light_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region DAMAGE STATE

CKirbyBulb_Damage_State::CKirbyBulb_Damage_State()
{
}

void CKirbyBulb_Damage_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyBulb_Damage_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);


    DESC(m_eEyeState) = CKirby::EYE_CLOSE;
    _float fDamageJumpPower = pKirby->Get_DamageJumpPower();
    pController->Jump(pTransformCom, fDamageJumpPower, fTimeDelta);
    fDamageJumpPower -= GRAVITY * fTimeDelta * 3.f;
    pKirby->Set_DamageJumpPower(fDamageJumpPower);

    if (pController->Is_Terrain() || pKirby->isAnimFinish())
    {
        DESC(m_bCarJump) = false;
        DESC(m_eEyeState) = CKirby::EYE_IDLE;
        pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
        return;
    }
}

void CKirbyBulb_Damage_State::OnStateExit()
{
}

CKirbyBulb_Damage_State* CKirbyBulb_Damage_State::Create()
{
    CKirbyBulb_Damage_State* pInstance = new CKirbyBulb_Damage_State();
    return pInstance;
}

void CKirbyBulb_Damage_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region VACUUM STATE

CKirbyBulb_Vacuum_State::CKirbyBulb_Vacuum_State()
{
}

void CKirbyBulb_Vacuum_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

    if (_iAnimIndex == CKirby::BULBVACUUMSTATE_DEFORM)
    {
        m_pGameInstance->Set_BlackBackGround(true);
        m_pGameInstance->Set_SecondTimerRatio(0.f);
        static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr())->Set_FOVY(20.f);
    }
}

void CKirbyBulb_Vacuum_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();
    Kirby_EyeState_Assist(Kirbydesc);

    Deceleration(Kirbydesc, pTransformCom, pController, fTimeDelta);

    if (pKirby->Get_State() == CKirby::BULBVACUUMSTATE_DEFORM)
    {
        if (m_bLightReset == true)
        {
            if (DESC(m_pLight) != nullptr)
            {
                DESC(m_pLight)->Set_DeadLight(true);
                Safe_Release(DESC(m_pLight));
                DESC(m_pLight) = nullptr;
            }
            m_bLightReset = false;
        }
        m_fTime += fTimeDelta;

        if (m_fTime > 0.36666666666666666666f)
        {
            LIGHT_DESC			LightDesc{};
            LightDesc.eType = LIGHT_DESC::TYPE_POINT;
            LightDesc.vPosition = pTransformCom->Get_State_Float4(CTransform::STATE_POSITION) + pTransformCom->Get_State(CTransform::STATE_UP) * 2.f;
            LightDesc.fRange = 7.f;
            LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
            LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
            LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
            if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
                return;
            DESC(m_pLight) = CGameInstance::Get_Instance()->Get_LightLastAddress();
            Safe_AddRef(DESC(m_pLight));


            if (DESC(m_pLight) != nullptr)
                DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());
            DESC(m_bLightOn) = true;

            pKirby->Change_State(CKirby::BULBSTATE_DEMOENDFIRST, 60.f, false, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);
            return;
        }

    }
    else if (pKirby->Get_State() == CKirby::BULBSTATE_DEMOENDFIRST)
    {
        if (m_bLightReset == true)
        {
            CMultiEffect::MULTI_FX_DESC FXDesc{};
            FXDesc.vInitPos = { 0.f, 0.f, 0.f };
            FXDesc.vInitScale = { 1.f, 1.f, 1.f };
            pKirby->Add_Effect("YW Deform Effect2", FXDesc, false);
            m_pGameInstance->PlaySound_Free(L"Kirby_DeformEvent.wav", 0.5f);
            m_bLightReset = false;
        }

        Turn_Interpolate(Kirbydesc, pTransformCom, fTimeDelta);
        if (DESC(m_pLight) != nullptr)
            DESC(m_pLight)->Update_LightPos(pKirby->Get_BulbLightPos());

        if (pKirby->isAnimFinish())
        {
            m_pGameInstance->Set_BlackBackGround(false);
            m_pGameInstance->Set_SecondTimerRatio(1.f);

            CCamera_Main* pCamMain = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
            if (pCamMain == nullptr)
                ALARM_FAIL("망했다 카메라 없다");

            pCamMain->Set_FOVY(30.f);
            pKirby->Change_State(CKirby::BULBSTATE_WAIT, 60.f, true, false, CKirby::BODY_BULBDEFAULT, CKirby::OFFSET_BULB);

            DESC(m_bLightOn) = false;
            DESC(m_pLight)->Interpolate_Light(_float4(0.7f, 0.2f, 0.2f, 0.f), 6.f, 1.f);

            return;
        }
    }
}

void CKirbyBulb_Vacuum_State::OnStateExit()
{
    m_fTime = 0.f;
    m_bLightReset = true;
}

CKirbyBulb_Vacuum_State* CKirbyBulb_Vacuum_State::Create()
{
    CKirbyBulb_Vacuum_State* pInstance = new CKirbyBulb_Vacuum_State();
    return pInstance;
}

void CKirbyBulb_Vacuum_State::Free()
{
    __super::Free();
}

#pragma endregion


#pragma region CONTENTS STATE

CKirbyBulb_Contents_State::CKirbyBulb_Contents_State()
{
}

void CKirbyBulb_Contents_State::OnStateEnter(CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation, _uint _iOffSet)
{
    __super::OnStateEnter(_pModel, _iAnimIndex, _fAnimSpeed, _bLoop, _bInterpolation, _iOffSet);

}

void CKirbyBulb_Contents_State::OnStateUpdate(CGameObject* pGameObject, _float fTimeDelta)
{
    CKirby* pKirby = static_cast<CKirby*>(pGameObject);
    CCharacterController* pController = dynamic_cast<CCharacterController*>(pGameObject->Get_Component(TEXT("Com_Controller")));
    CTransform* pTransformCom = pGameObject->Get_TransformCom();
    CKirby::KIRBY_INFODESC* Kirbydesc = pKirby->Get_KirbyInfo();
    CGameObject* pCamera = (CGameObject*)m_pGameInstance->Get_CurCameraPtr();


}

void CKirbyBulb_Contents_State::OnStateExit()
{
}

CKirbyBulb_Contents_State* CKirbyBulb_Contents_State::Create()
{
    CKirbyBulb_Contents_State* pInstance = new CKirbyBulb_Contents_State();
    return pInstance;
}

void CKirbyBulb_Contents_State::Free()
{
    __super::Free();
}

#pragma endregion
