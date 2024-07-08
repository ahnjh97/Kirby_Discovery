#include "stdafx.h"
#include "PopStar.h"
#include "MultiEffect.h"
#include "Particle.h"
CPopStar::CPopStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPopStar::CPopStar(const CPopStar& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CPopStar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPopStar::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	// 임시
	m_pModelCom->Set_Animation(1, 10.f, false, false);

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_fRimWidth = 5.f;
	m_bStencil = true;

	m_pTransformCom->Turn(_float4(0.f, 1.f, 0.f, 0.f), 1.f, 90.f);
	m_pTransformCom->Turn(_float4(0.f, 0.f, 1.f, 0.f), 1.f, 50.f);

	m_pTransformCom->Set_Scaled(10.f, 10.f, 10.f);

	//StarRiver
	CEffect::FX_DESC FXDesc{};

	FXDesc.vInitPos = _float3{15.f, -10.f, 0.f};
	FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
	if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_StarRiver"), &FXDesc)))
		return E_FAIL;

	FXDesc.vInitPos = _float3{ 15.f, -40.f, 0.f };
	FXDesc.vInitScale = { 6.f, 6.f, 6.f };
	if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_star dash test 3"), &FXDesc)))
		return E_FAIL;

	return S_OK;
}

_int CPopStar::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;




	return OBJ_NOEVENT;
}

void CPopStar::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);

}

HRESULT CPopStar::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		m_pModelCom->Render(i);
	}
	return S_OK;
}

HRESULT CPopStar::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_PopStar"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CPopStar::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CPopStar* CPopStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPopStar* pInstance = new CPopStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPopStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPopStar::Clone(void* pArg)
{
	CPopStar* pInstance = new CPopStar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CPopStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPopStar::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
