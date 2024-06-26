#include "stdafx.h"
#include "FinalBossSpear.h"
#include "Bone.h"
#include "RayArrow.h"
#include "FinalBoss.h"

CFinalBossSpear::CFinalBossSpear(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPartObject{ pDevice, pContext }
{
}

CFinalBossSpear::CFinalBossSpear(const CFinalBossSpear& rhs)
	: CPartObject{ rhs }
{
}

HRESULT CFinalBossSpear::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinalBossSpear::Initialize(void* pArg)
{
	FINALBOSSSPEAR_DESC* pWeaponDesc = (FINALBOSSSPEAR_DESC*)pArg;

	m_pSocket = pWeaponDesc->pSocket;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(0.f, 0.1f, 0.01f, 1.f));

	//m_pModelCom->Invalidate_Bones();

	m_fDelayTime = 0.1f;
	m_fCreateTime = 0.1f;

	return S_OK;
}

_int CFinalBossSpear::Tick(_float fTimeDelta)
{
	if (m_bDead)
		return OBJ_DEAD;

	//Compute_MotionBlur();

	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix()) * XMLoadFloat4x4(m_pParentMatrix));

	CFinalBoss* pFinalBoss = static_cast<CFinalBoss*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Monster")));
	if (CFinalBoss::FINALBOSS_RAYARROWSTART == pFinalBoss->Get_State() || CFinalBoss::FINALBOSS_RAYARROWSTARTAIR == pFinalBoss->Get_State())
	{
		if (0.0f < pFinalBoss->Get_AnimRatio() && 0.25f > pFinalBoss->Get_AnimRatio())
		{
			m_fDelayTime += fTimeDelta;
			if (m_fCreateTime < m_fDelayTime)
			{
				++m_iCnt;
				m_fDelayTime = 0.f;
				CRayArrow::RAYARROW_DESC RayArrow = {};
				_float4x4 WorldMatrix = m_WorldMatrix;
				WorldMatrix._41 -= m_WorldMatrix._31 * 4.5f;
				WorldMatrix._42 -= m_WorldMatrix._32 * 4.5f;
				WorldMatrix._43 -= m_WorldMatrix._33 * 4.5f;
				RayArrow.vPosition = _float4(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43, 1.f);
				RayArrow.fAngle = m_fAngle;
				if(CFinalBoss::FINALBOSS_RAYARROWSTART == pFinalBoss->Get_State())
					RayArrow.fHeight = 30.f;
				else
					RayArrow.fHeight = 70.f;
				RayArrow.vSide = pFinalBoss->Get_TransformCom()->Get_State_Vector(CTransform::STATE_RIGHT) * m_fSide;
				RayArrow.fDelayTime = m_fFirstFireTime;
				//RayArrow.vLook = _float3(m_pParentMatrix->_31, m_pParentMatrix->_32, m_pParentMatrix->_33);
				if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Arrow"), TEXT("Prototype_GameObject_RayArrow"), &RayArrow)))
				{
					MSG_BOX(TEXT("Failed to Clone : CRayArrow"));
					return E_FAIL;
				}

				m_fAngle += 13.f;
				m_fSide -= 10.f;
				m_fFirstFireTime -= 0.4f;

				if (1 == m_iCnt)
					m_fCreateTime = 0.16f;
				else if (2 == m_iCnt)
					m_fCreateTime = 0.08f;
				else
					m_fCreateTime = 0.11f;
			}
		}
		else if (0.47f < pFinalBoss->Get_AnimRatio() && 0.65f > pFinalBoss->Get_AnimRatio())
		{
			m_fDelayTime += fTimeDelta;
			if (m_fCreateTime < m_fDelayTime)
			{
				m_fAngle -= 13.f;
				m_fSide += 10.f;
				m_fSecondFireTime -= 0.4f;
				++m_iCnt;
				m_fDelayTime = 0.f;
				CRayArrow::RAYARROW_DESC RayArrow = {};
				_float4x4 WorldMatrix = m_WorldMatrix;
				WorldMatrix._41 -= m_WorldMatrix._31 * 4.5f;
				WorldMatrix._42 -= m_WorldMatrix._32 * 4.5f;
				WorldMatrix._43 -= m_WorldMatrix._33 * 4.5f;
				RayArrow.vPosition = _float4(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43, 1.f);
				RayArrow.fAngle = m_fAngle;
				if (CFinalBoss::FINALBOSS_RAYARROWSTART == pFinalBoss->Get_State())
					RayArrow.fHeight = 30.f;
				else
					RayArrow.fHeight = 70.f;
				RayArrow.vSide = pFinalBoss->Get_TransformCom()->Get_State_Vector(CTransform::STATE_RIGHT) * m_fSide;
				RayArrow.fDelayTime = m_fSecondFireTime;
				//RayArrow.vLook = _float3(m_pParentMatrix->_21, m_pParentMatrix->_22, m_pParentMatrix->_23);
				if (FAILED(m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Arrow"), TEXT("Prototype_GameObject_RayArrow"), &RayArrow)))
				{
					MSG_BOX(TEXT("Failed to Clone : CRayArrow"));
					return E_FAIL;
				}

				if (6 == m_iCnt)
					m_fCreateTime = 0.08f;
				else
					m_fCreateTime = 0.1f;
			}
		}
		else
		{
			m_fAngle = 0.f;
			m_fSide = 0.f;
			m_fFirstFireTime = 4.2f;
			m_fSecondFireTime = 4.f;
		}
	}
	else
	{
		m_iCnt = 0;
		m_fDelayTime = 0.f;
	}

	return OBJ_NOEVENT;
}

void CFinalBossSpear::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, m_WorldMatrix._44), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CFinalBossSpear::Render()
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
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFinalBossSpear::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_PartObject(m_pShaderCom, &m_WorldMatrix, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFinalBossSpear::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FinalBossSpear"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CFinalBossSpear::Bind_ShaderResources()
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

void CFinalBossSpear::Compute_MotionBlur()
{
	_vector vPos = CUtils::Get_State_Vector_Matrix(m_WorldMatrix, CUtils::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vPreScreenPos = vCurScreenPos;
}

_float4 CFinalBossSpear::Compute_BoneWorldMatrix()
{
	_float4x4 WorldMatrix = m_pTransformCom->Get_WorldMatrix();
	WorldMatrix._41 -= m_pTransformCom->Get_State_Float4(CTransform::STATE_RIGHT).x * 5.f;
	WorldMatrix._42 -= m_pTransformCom->Get_State_Float4(CTransform::STATE_RIGHT).y * 5.f;
	WorldMatrix._43 -= m_pTransformCom->Get_State_Float4(CTransform::STATE_RIGHT).z * 5.f;

	XMStoreFloat4x4(&WorldMatrix, WorldMatrix * XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix()) * XMLoadFloat4x4(m_pParentMatrix));

	return _float4(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43, WorldMatrix._44);
}

CFinalBossSpear* CFinalBossSpear::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalBossSpear* pInstance = new CFinalBossSpear(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinalBossSpear"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinalBossSpear::Clone(void* pArg)
{
	CFinalBossSpear* pInstance = new CFinalBossSpear(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinalBossSpear"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinalBossSpear::Free()
{
	__super::Free();

	Safe_Release(m_pSocket);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
