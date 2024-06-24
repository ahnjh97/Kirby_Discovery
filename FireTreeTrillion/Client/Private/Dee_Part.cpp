#include "stdafx.h"
#include "Dee_Part.h"
#include "Bone.h"

CDee_Part::CDee_Part(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPartObject{ pDevice, pContext }
{
}

CDee_Part::CDee_Part(const CDee_Part& rhs)
	:CPartObject{ rhs }
{
}

HRESULT CDee_Part::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDee_Part::Initialize(void* pArg)
{
	DEEPART_DESC* pPartDesc = (DEEPART_DESC*)pArg;

	m_pSocket = pPartDesc->pSocket;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components(pPartDesc->wstrModelName)))
		return E_FAIL;

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0.f, 0.1f, 0.01f, 1.f));


	return S_OK;
}

_int CDee_Part::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	Compute_MotionBlur();

	m_WorldMatrix = m_pTransformCom->Get_WorldMatrix() * *m_pSocket->Get_CombinedTransformationMatrix() * *m_pParentMatrix;

	return OBJ_NOEVENT;
}

void CDee_Part::Late_Tick(_float fTimeDelta)
{
	if (m_pGameInstance->isInFrustum_WorldSpace(Pos(m_WorldMatrix.Translation()), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CDee_Part::Render()
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

		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CDee_Part::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDee_Part::Add_Components(wstring wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);


	//마을(TOWN/PARTTIME)에 있을 때만 파트 오브젝트 로드하기
	if (*m_pCurrentLevelID == LEVEL_TOWN || *m_pCurrentLevelID == LEVEL_PARTTIME)
	{
		wstring wstrTag = TEXT("Prototype_Component_Model_");

		wstrTag += wstrModelName;
		/* For.Com_Model */
		hr = __super::Add_Component(wstrTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CDee_Part::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 추후 변경
	_bool bStencil = true;
	_bool bRimLight = true;
	_bool bMotionBlur = true;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CDee_Part::Compute_MotionBlur()
{
}

CDee_Part* CDee_Part::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDee_Part* pInstance = new CDee_Part(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDee_Part"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDee_Part::Clone(void* pArg)
{
	CDee_Part* pInstance = new CDee_Part(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDee_Part"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDee_Part::Free()
{
	__super::Free();

	Safe_Release(m_pSocket);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
