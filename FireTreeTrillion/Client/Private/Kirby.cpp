#include "stdafx.h"
#include "FSM.h"
#include "Kirby.h"

CKirby::CKirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CKirby::CKirby(const CKirby& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CKirby::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKirby::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 7.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pModelCom->Set_Animation(0, true);
	_vector vPos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);


}

_int CKirby::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;



	// FSM Á¦¾î
	Update_FSMState(fTimeDelta);
	m_pFSM->Update(this, fTimeDelta, m_eCurrentState);


	return _int();
}

void CKirby::Late_Tick(_float fTimeDelta)
{
}

HRESULT CKirby::Render()
{
	return E_NOTIMPL;
}

HRESULT CKirby::Render_LightDepth()
{
	return E_NOTIMPL;
}

void CKirby::Render_IMGUI()
{
}

HRESULT CKirby::Add_Components()
{
	return E_NOTIMPL;
}

HRESULT CKirby::Bind_ShaderResources()
{
	return E_NOTIMPL;
}

void CKirby::SetUp_FSM()
{
}

void CKirby::Update_FSMState(_float fTimeDelta)
{
}

void CKirby::Change_State(STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
}

CKirby* CKirby::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CKirby::Clone(void* pArg)
{
	return nullptr;
}

void CKirby::Free()
{
}
