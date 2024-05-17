#include "stdafx.h"
#include "..\Public\Player.h"


#include "Body_Player.h"
#include "Weapon.h"

CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer & rhs)
	: CGameObject{ rhs }
{

}

HRESULT CPlayer::Initialize_Prototype()
{
	

	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	if (FAILED(Add_PartObjects()))
		return E_FAIL;

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (GetKeyState(VK_LEFT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
	}

	if (GetKeyState(VK_RIGHT) & 0x8000)
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
	}

	if (GetKeyState(VK_DOWN) & 0x8000)
	{
		m_pTransformCom->Go_Backward(fTimeDelta);
	}

	if (GetKeyState(VK_UP) & 0x8000)
	{
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);

		m_eState |= STATE_RUN;
		if (m_eState & STATE_IDLE)
			m_eState ^= STATE_IDLE;
	}
	else
	{
		m_eState |= STATE_IDLE;
		if(m_eState & STATE_RUN)
			m_eState ^= STATE_RUN;
	}

	for (auto& Pair : m_PartObjects)
		Pair.second->Tick(fTimeDelta);

	_vector		vWorldPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);


	if (GetKeyState(VK_LBUTTON) & 0x8000)
	{
		POINT		ptMouse;
		GetCursorPos(&ptMouse);
		ScreenToClient(g_hWnd, &ptMouse);

		_float2		vMousePos = _float2(ptMouse.x, ptMouse.y);

		vWorldPos = m_pGameInstance->Compute_WorldPos(vMousePos, TEXT("Target_FieldDepth"));
	}
	
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vWorldPos);


	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
	
}

void CPlayer::Late_Tick(_float fTimeDelta)
{


	for (auto& Pair : m_PartObjects)
		Pair.second->Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

#ifdef _DEBUG
	m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif
}

HRESULT CPlayer::Render()
{

	return S_OK;
}

HRESULT CPlayer::Add_Components()
{
	/* Com_Collider */
	CBounding_AABB::BOUNDING_AABB_DESC		ColliderDesc{};

	/* 로컬상의 정보를 셋팅한다. */
	ColliderDesc.vSize = _float3(0.8f, 1.2f, 0.8f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Add_PartObjects()
{


	/* For.Part_Body */
	CPartObject*		pBodyObject = { nullptr };
	CBody_Player::BODY_DESC	BodyDesc{};

	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BodyDesc.pState = &m_eState;

	pBodyObject = dynamic_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Body_Player"), &BodyDesc));
	if (nullptr == pBodyObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Body"), pBodyObject);

	/* For.Part_Weapon */
	CPartObject*		pWeaponObject = { nullptr };
	CWeapon::WEAPON_DESC	WeaponDesc{};

	CModel*		pModel = (CModel*)pBodyObject->Get_Component(TEXT("Com_Model"));

	WeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	WeaponDesc.pSocket = pModel->Get_BonePtr("SWORD");

	pWeaponObject = dynamic_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Weapon"), &WeaponDesc));
	if (nullptr == pWeaponObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Weapon"), pWeaponObject);

	/* For.Part_Effect */
	CPartObject*		pEffectObject = { nullptr };
	CPartObject::PARTOBJECT_DESC EffectDesc{};

	EffectDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();

	pEffectObject = dynamic_cast<CPartObject*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Blue"), &EffectDesc));
	if (nullptr == pEffectObject)
		return E_FAIL;

	m_PartObjects.emplace(TEXT("Part_Effect"), pEffectObject);
	
	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);

	Safe_Release(m_pColliderCom);

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);

	m_PartObjects.clear();
}
