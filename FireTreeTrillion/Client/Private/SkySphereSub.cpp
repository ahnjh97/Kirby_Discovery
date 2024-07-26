#include "stdafx.h"
#include "GameInstance.h"
#include "SkySphereSub.h"
#include "SkySphere.h"

CSkySphereSub::CSkySphereSub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CSkySphereSub::CSkySphereSub(const CSkySphereSub& rhs)
	:CGameObject{ rhs }
{
}

HRESULT CSkySphereSub::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkySphereSub::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	CSkySphere::SKYSPHERE_DESC SkySphereDesc{};
	if (nullptr != pArg)
		SkySphereDesc = *(CSkySphere::SKYSPHERE_DESC*)pArg;

	if (FAILED(Add_Components()))
		return E_FAIL;

	//레벨 별 상태 변경을 위한 값 저장 (현재는 무의미한데, 트리거 정보 받아올 경우에 조건 처리 필요)
	//보스전 필드 가기 전의 이동 필드는 해당 서브스피어 렌더가 필요없기 떄문.
	/*
	m_eCurLevel = (LEVEL)*m_pGameInstance->Get_CurrentLevelID();
	if (LEVEL_FINALBOSS != m_eCurLevel)
	{
		for (auto& iMod : m_pModelCom)
			iMod->Set_Hide(TRUE); //default FALSE
	}
	*/
	return S_OK;
}	

_int CSkySphereSub::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CSkySphereSub::Late_Tick(_float fTimeDelta)
{
	//보스전 필드 진입 트리거에 해당 조건을 체크. 해당 레벨과 트리거 조건이 일치할 때, 서브 스피어 렌더ON
	//현재는 레벨만 체크 중인 상태
	/*
	if (LEVEL_FINALBOSS == m_eCurLevel)
	{
		for (auto& iMod : m_pModelCom)
			iMod->Set_Hide(FALSE); //default FALSE
	}
	*/
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSkySphereSub::Render()
{
	//return S_OK;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/*
	for (auto& iMod : m_pModelCom)
	{
		_bool IsHidden = iMod->IsHidden();
		if (TRUE == IsHidden)
			return S_OK;
	}
	*/
	
	//서브 스피어는 메쉬 하나
	HRESULT hr;
	for (_uint iMod = 0; iMod < MOD_NONE; ++iMod)
	{
		hr = m_pModelCom[iMod]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0, TextureType_DIFFUSE);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);

		hr = m_pModelCom[iMod]->Render(0);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CSkySphereSub::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"), 
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbBuildingFrame"), 
		TEXT("Com_ModelFrame"), (CComponent**)&m_pModelCom[MOD_FRAME]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_LbFarPiller"),
		TEXT("Com_ModelPiller"), (CComponent**)&m_pModelCom[MOD_PILLER]);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CSkySphereSub::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CSkySphereSub* CSkySphereSub::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSkySphereSub* pInstance = new CSkySphereSub(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSkySphereSub"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSkySphereSub::Clone(void* pArg)
{
	CSkySphereSub* pInstance = new CSkySphereSub(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CSkySphereSub"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkySphereSub::Free()
{
	Safe_Release(m_pShaderCom);

	for (auto& iMod : m_pModelCom)
		Safe_Release(iMod);

	__super::Free();
}
