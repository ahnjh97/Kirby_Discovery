#include "stdafx.h"
#include "BlendMapObject.h"
#include "NonAnimDeco.h"

CBlendMapObject::CBlendMapObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice , pContext }
{
}

CBlendMapObject::CBlendMapObject(const CBlendMapObject& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CBlendMapObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBlendMapObject::Initialize(void* pArg)
{
	BLENDMAPOBJ_DESC tDesc{};
	if (pArg)
		tDesc = *(BLENDMAPOBJ_DESC*)pArg;

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(tDesc.tModel)))
		return E_FAIL;

	//// 모델 메쉬 쳐내는 작업
	//m_pModelCom->RemoveNonBlendMeshes(m_setBlendMeshIndices);

	// Normal 유무 검사해서 PassIndex 지정하는 작업
	m_pModelCom->DeterminePassIndices(m_vecPassIndices);

	return S_OK;
}

HRESULT CBlendMapObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (m_setBlendMeshIndices.end() == m_setBlendMeshIndices.find(i))
			continue;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

		if (FAILED(m_pShaderCom->Begin(m_vecPassIndices[i])))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CBlendMapObject::Add_Components(MODEL tModel)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	m_pModelCom = CModel::Create(m_pDevice, m_pContext, tModel);

	if (nullptr == m_pModelCom)
		return E_FAIL;

	return S_OK;
}

HRESULT CBlendMapObject::Bind_ShaderResources()
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

CBlendMapObject* CBlendMapObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBlendMapObject* pInstance = new CBlendMapObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBlendMapObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBlendMapObject::Clone(void* pArg)
{
	CBlendMapObject* pInstance = new CBlendMapObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBlendMapObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBlendMapObject::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
