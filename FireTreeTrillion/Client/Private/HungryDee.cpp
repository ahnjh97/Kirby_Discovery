#include "stdafx.h"
#include "HungryDee.h"

CHungryDee::CHungryDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CWaddleDee{ pDevice, pContext }
{
}

CHungryDee::CHungryDee(const CHungryDee& rhs)
	:CWaddleDee{ rhs }
{
}

HRESULT CHungryDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHungryDee::Initialize(void* pArg)
{
	return S_OK;
}

_int CHungryDee::Tick(_float fTimeDelta)
{
	return _int();
}

void CHungryDee::Late_Tick(_float fTimeDelta)
{
}

HRESULT CHungryDee::Render()
{
	return S_OK;
}

HRESULT CHungryDee::Render_LightDepth()
{
	return S_OK;
}

void CHungryDee::Add_AnimEvent()
{
}

void CHungryDee::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CHungryDee::Render_IMGUI()
{
}

HRESULT CHungryDee::Add_Components()
{
	return S_OK;
}

HRESULT CHungryDee::Add_PartObjects()
{
	return S_OK;
}

HRESULT CHungryDee::Bind_ShaderResources()
{
	return S_OK;
}

void CHungryDee::SetUp_FSM()
{
}

_bool CHungryDee::Custom_Face(_uint iMeshIndex)
{
	return _bool();
}

CHungryDee* CHungryDee::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CGameObject* CHungryDee::Clone(void* pArg)
{
	return nullptr;
}

void CHungryDee::Free()
{
}
