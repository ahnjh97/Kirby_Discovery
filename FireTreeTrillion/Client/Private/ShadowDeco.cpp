#include "stdafx.h"
#include "ShadowDeco.h"

CShadowDeco::CShadowDeco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CShadowDeco::CShadowDeco(const CShadowDeco& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CShadowDeco::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CShadowDeco::Initialize(void* pArg)
{
	SHADOWDECO_DESC tDesc{};
	if (pArg)
		tDesc = *(SHADOWDECO_DESC*)pArg;

	tDesc.matWorld = tDesc.pDecoModel->Get_WorldMatrixForOctree();
	m_pModelCom = tDesc.pDecoModel;
	Safe_AddRef(m_pModelCom);

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(tDesc.wstrModelName)))
		return E_FAIL;

	m_bStencil = true;

	return S_OK;
}

_int CShadowDeco::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CShadowDeco::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 10.0f))
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CShadowDeco::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CShadowDeco::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CShadowDeco::Bind_ShaderResources()
{
	return S_OK;
}

CShadowDeco* CShadowDeco::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShadowDeco* pInstance = new CShadowDeco(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CShadowDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CShadowDeco::Clone(void* pArg)
{
	CShadowDeco* pInstance = new CShadowDeco(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CShadowDeco"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShadowDeco::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
