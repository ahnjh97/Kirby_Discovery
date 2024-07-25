#include "stdafx.h"
#include "SimbaRock.h"
#include "stdafx.h"
#include "Gully.h"
#include "HitBox.h"
#include "Camera_Main.h"

CSimbaRock::CSimbaRock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSimbaRock::CSimbaRock(const CSimbaRock& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CSimbaRock::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSimbaRock::Initialize(void* pArg)
{
	GAMEOBJECT_DESC tDesc{};
	if (nullptr != pArg)
	{
		tDesc = *(GAMEOBJECT_DESC*)pArg;
		tDesc.fSpeedPerSec = 7.f;
		tDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	}

	m_fSamplingFactor = 0.1f;

	if (FAILED(__super::Initialize(&tDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(tDesc.wstrModelName)))
		return E_FAIL;
		
	m_bHide = true;

	return S_OK;
}

_int CSimbaRock::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if(false == m_bHide)
		m_fTime += m_pGameInstance->Get_SecondTimer();

	if (false == m_bHide && 2.35f < m_fTime)
		m_bHide = true;
		
	return OBJ_NOEVENT;
}

void CSimbaRock::Late_Tick(_float fTimeDelta)
{
	if (true == m_bHide)
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSimbaRock::Render()
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

		if (FAILED(m_pShaderCom->Begin(MAP_SIMBA_ROCK))) // SimbaRock¿ë ÆÐ½º
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CSimbaRock::SetUpSimbaRock(_fvector vPos)
{
	m_fScale = CUtils::Make_RandomFloat(0.2f, 0.28f);
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
	m_pTransformCom->Turn(CUtils::Make_Random_Vector(1.f), 1, CUtils::Make_RandomFloat(0, 360.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	m_bHide = false;
	m_fTime = 0.f;
}

void CSimbaRock::TestRock(_fvector vPos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	m_bHide = false;
	m_fTime = 0.f;
}

HRESULT CSimbaRock::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr{};
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelName,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CSimbaRock::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_fSamplingFactor, sizeof(m_fSamplingFactor))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CSimbaRock* CSimbaRock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSimbaRock* pInstance = new CSimbaRock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSimbaRock"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSimbaRock::Clone(void* pArg)
{
	CSimbaRock* pInstance = new CSimbaRock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSimbaRock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSimbaRock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
