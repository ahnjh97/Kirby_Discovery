#include "stdafx.h"
#include "FXToolDirector.h"
#include "SingleEffect.h"

#include "MultiEffect.h"
#include "Camera_Free.h"


CFXToolDirector::CFXToolDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CFXToolDirector::CFXToolDirector(const CFXToolDirector& rhs)
	:CGameObject{rhs}
{
}

void CFXToolDirector::Make_Effect(SINGLE_FX_DATA& EffectData)
{
}

void CFXToolDirector::Make_Effect(MULTI_FX_DATA& EffectData)
{
}

CEffect* CFXToolDirector::Find_Effect(string strEffectName)
{
	return nullptr;
}

HRESULT CFXToolDirector::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFXToolDirector::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};

	if (nullptr == pArg)
	{
		GameObjectDesc.fSpeedPerSec = 5.f;
		GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	}
	else
	{
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	}

	//if (FAILED(__super::Initialize(&GameObjectDesc)))
	//	return E_FAIL;


	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED_MSG(hr, "Failed To Initialize : CCamera_Main");


	hr = Add_Components();
	CHECK_FAILED_MSG(hr, "Failed To Add Components : CCamera_Main");



	return S_OK;
}

_int CFXToolDirector::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CFXToolDirector::Late_Tick(_float fTimeDelta)
{
	Render_FXHierarchy();
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CFXToolDirector::Render()
{
	return S_OK;
}

void CFXToolDirector::Render_IMGUI()
{
	__super::Render_IMGUI();
}

void CFXToolDirector::Render_FXHierarchy()
{
	Begin(u8"½Ì±Û FX");

	End();
}

HRESULT CFXToolDirector::Add_Components()
{
	return S_OK;
}

CFXToolDirector* CFXToolDirector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFXToolDirector* pInstance = new CFXToolDirector(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFXToolDirector::Clone(void* pArg)
{
	CFXToolDirector* pInstance = new CFXToolDirector(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CFXToolDirector"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFXToolDirector::Free()
{
	__super::Free();
}
