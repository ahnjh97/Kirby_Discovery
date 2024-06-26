#include "stdafx.h"
#include "NonAnimDeco.h"

CNonAnimDeco::CNonAnimDeco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CNonAnimDeco::CNonAnimDeco(const CNonAnimDeco& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CNonAnimDeco::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNonAnimDeco::Initialize(void* pArg)
{
	GAMEOBJECT_DESC tDesc{};
	if (pArg)
		tDesc = *(GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(tDesc.wstrModelName)))
		return E_FAIL;

	return S_OK;
}

_int CNonAnimDeco::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CNonAnimDeco::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CNonAnimDeco::Render()
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
		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

		if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CNonAnimDeco::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelTag;
	if (FAILED(__super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CNonAnimDeco::Bind_ShaderResources()
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

CNonAnimDeco* CNonAnimDeco::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CNonAnimDeco* pInstance = new CNonAnimDeco(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CNonAnimDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNonAnimDeco::Clone(void* pArg)
{
	CNonAnimDeco* pInstance = new CNonAnimDeco(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CNonAnimDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CNonAnimDeco::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
