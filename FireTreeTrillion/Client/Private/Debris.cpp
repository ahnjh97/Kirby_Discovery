#include "stdafx.h"
#include "Debris.h"
#include "Kirby.h"

#include "MultiEffect.h"
#include "Particle.h"

CDebris::CDebris(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CDebris::CDebris(const CDebris& rhs)
	: CGameObject{ rhs }
{
}

void CDebris::Set_ParticleDebris(_fvector vPosition, _float fScale, _float2 fRandY, _float2 fRandXZ, _float fTotalTime)
{
	m_vRotationAxis = CUtils::Make_Random_Vector(1);

	m_fX = CUtils::Make_RandomFloat(fRandXZ.x, fRandXZ.y);
	_int iRand = CUtils::Make_RandomInt(0, 1);
	if (0 == iRand)
		m_fX *= -1;

	m_fY = CUtils::Make_RandomFloat(fRandY.x, fRandY.y);
	m_fZ = CUtils::Make_RandomFloat(fRandXZ.x, fRandXZ.y);
	_int iRand2 = CUtils::Make_RandomInt(0, 1);
	if (0 == iRand2)
		m_fZ *= -1;

	m_fScale = fScale;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPosition);
	m_pTransformCom->Set_Scaled(1.f * fScale, 1.f * fScale, 1.f * fScale);

    m_fTime = 0.f;
    m_fTotalTime = fTotalTime;
    m_bDead = false;
    m_bSwap = false;
}

void CDebris::Set_ParticleEffect(_fvector vPosition, _float fScale)
{
	m_fScale = fScale;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPosition);
	m_pTransformCom->Set_Scaled(1.f * fScale, 1.f * fScale, 1.f * fScale);

	m_fTotalTime = 3.f;
	m_fTime = 0.f;
	m_bDead = false;
	m_bSwap = true;
}

void CDebris::Set_DebrisInfo(_float4 vPos, _float fScale, _float fY, _float3 vDir, _float fTotalTime, _float fFallSpeed)
{
	m_vRotationAxis = CUtils::Make_Random_Vector(1);
	m_fScale = fScale;
	m_fY = fY;
	m_fX = vDir.x;
	m_fZ = vDir.z;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);
	m_fTime = 0.f;
	m_fTotalTime = fTotalTime;
	m_fFallSpeed = fFallSpeed;
	m_bDead = false;
	m_bSwap = false;
}

HRESULT CDebris::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDebris::Initialize(void* pArg)
{
	DEBRIS_DESC		GameObjectDesc{};
	if (nullptr != pArg)
		GameObjectDesc = *(DEBRIS_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
		return E_FAIL;

	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f);

	// m_pTransformCom->Set_Scaled(0.07f, 0.07f, 0.07f);

	m_bDead = true;

	m_fSamplingFactor = 0.1f;
	m_fTotalTime = 1.f;
	m_fFallSpeed = 16.f;
	//CEffect::FX_DESC FXDesc{};
	//FXDesc.pSocketMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	//FXDesc.vInitPos = { 0.f, 1.4f, -.5f };
	//FXDesc.vInitScale = { 20.f, 20.f, 20.f };
	//FXDesc.vInitRot = { 90.f, 0.f, 0.f };

	//if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_come on dash white"), &FXDesc)))
	//	return E_FAIL;

	//m_FXList.emplace_back(static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back()));
	//Safe_AddRef(m_FXList.back());

	//Add_Effect("come on dash white", FXDesc, true);

	return S_OK;
}

_int CDebris::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_NOEVENT;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	m_fTime += m_fTimeDelta;

	if (true == m_bSwap)
	{
		if (m_fTotalTime + 2.f < m_fTime)
			m_bDead = true;
		else if (m_fTotalTime < m_fTime)
		{
			_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
			vPos.m128_f32[1] -= m_fTimeDelta;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
	}
	else
	{
		if (m_fTotalTime > m_fTime)
		{
			m_pTransformCom->Turn(m_vRotationAxis, m_fTimeDelta * 8.f);

			//m_fY = m_fY * cos(m_fTime * XM_PI / (m_fTotalTime * 2.f)) * fTimeDelta
			m_fY -= m_fTime * GRAVITY * m_fTimeDelta * m_fFallSpeed;

			_float4 vDir = _float4(m_fX * m_fTimeDelta, m_fY * m_fTimeDelta, m_fZ * m_fTimeDelta, 0);
			m_pTransformCom->Move(vDir);

			_float fScale = m_fScale - (m_fTime / m_fTotalTime) * 0.5f * m_fScale;
			m_pTransformCom->Set_Scaled(fScale, fScale, fScale);
		}
		else
			m_bDead = true;
	}

	return OBJ_NOEVENT;
}

void CDebris::Late_Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return;

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CDebris::Render()
{
	if (true == m_bDead)
		return S_OK;

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

		if (FAILED(m_pShaderCom->Begin(MAP_NONBLEND_NONDISCARD)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CDebris::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(TEXT("Prototype_Component_Model_") + _wstrModelTag,
		TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDebris::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fZero, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CDebris* CDebris::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDebris* pInstance = new CDebris(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDebris"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDebris::Clone(void* pArg)
{
	CDebris* pInstance = new CDebris(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CDebris"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDebris::Free()
{
	__super::Free();

	//매칭된 이펙트들 모두 삭제
	if (!m_FXList.empty())
	{
		for (auto& FX : m_FXList)
		{
			FX->Set_Dead();
			Safe_Release(FX);
		}

		m_FXList.clear();
	}

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}