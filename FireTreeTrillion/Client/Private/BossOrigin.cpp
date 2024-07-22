#include "stdafx.h"
#include "BossOrigin.h"
#include "EventCenter.h"
#include "Level_Loading.h"

CBossOrigin::CBossOrigin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBossOrigin::CBossOrigin(const CBossOrigin& rhs)
	: CGameObject{ rhs }
{
}

void CBossOrigin::Activate(CGameObject* pObj)
{
	m_bActivated = true;
}

HRESULT CBossOrigin::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossOrigin::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* desc = {};

	if (pArg != nullptr)
		desc = (GAMEOBJECT_DESC*)pArg;

	HRESULT hr{};
	hr = __super::Initialize(desc);
	CHECK_FAILED(hr);

	if (desc != nullptr)
		Add_Components(desc->wstrModelName);

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_bStencil = true;
	
	for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); i++)
	{
		string strMeshName = m_pModelCom->Get_MeshName(i);
		if ("L_EyeBallM__EyeC" == strMeshName || "R_EyeBallM__EyeC" == strMeshName)
			m_vecEyeMeshes.push_back(i);
		else
			m_vecMeshes.push_back(i);
	}

	m_pTransformCom->Set_Scaled(3.f, 3.f, 3.f);

	m_pModelCom->Set_Animation(BO_WAIT, 60.f, true, false);

	function<void(CGameObject*)> func = bind(&CBossOrigin::Activate, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_BOSSORIGIN, this, func);

	return S_OK;
}

_int CBossOrigin::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (true == m_bActivated)
		m_fTime += m_pGameInstance->Get_SecondTimer();

	if (3.f < m_fTime && true == m_bActivated) {
		m_pModelCom->Set_Animation(BO_GETOUT, 50.f, false, true);
		m_bActivated = false;
	}

	if (BO_GETOUT == m_pModelCom->Get_CurAnimIndex() && true == m_pModelCom->IsFinished()) { // 지영누나 여기야 페이드아웃 부탁
		m_pModelCom->Set_Animation(BO_WAIT_EYEOPEN, 60.f, false, false);

		MB(L"페이드아웃 부탁해요", L"Hey JiYoung");
	}

	return OBJ_NOEVENT;
}

void CBossOrigin::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CBossOrigin::Render()
{
	HRESULT hr{};
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	for (auto& i : m_vecMeshes)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}
	
	for (auto& i : m_vecEyeMeshes)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(ANIMMODEL_BOSSORIGINEYE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CBossOrigin::Add_Components(wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrModeltag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModeltag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CBossOrigin::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CBossOrigin* CBossOrigin::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossOrigin* pInstance = new CBossOrigin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBossOrigin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBossOrigin::Clone(void* pArg)
{
	CBossOrigin* pInstance = new CBossOrigin(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBossOrigin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBossOrigin::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

