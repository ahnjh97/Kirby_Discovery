#include "stdafx.h"
#include "CrashParticle.h"

CCrashParticle::CCrashParticle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CCrashParticle::CCrashParticle(const CCrashParticle& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CCrashParticle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrashParticle::Initialize(void* pArg)
{
	CRASHPARTICLEDESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (CRASHPARTICLEDESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bGravity = Desc->bGravity;
	m_vDir = Desc->vDir;
	m_fSpeed = m_fPreSpeed = Desc->fSpeed;
	m_fScale = Desc->fScale;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Desc->vPos);


	m_bMotionBlur = true;
	m_bRimLight = true;
	m_fRimWidth = 0.1f;
	m_bStencil = true;

	m_fTurn = CUtils::Make_RandomFloat(120.f, 720.f);
	m_fTurnAxis = CUtils::Make_Random_Vector(1.f);
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

	return S_OK;
}

_int CCrashParticle::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	Compute_MotionBlur();
	m_fRunTime += fTimeDelta;

	if (m_bGravity == true)
	{
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_fGravity += 1.5f * m_fTimeDelta;

		_float4 vDelta = (m_vDir * m_fTimeDelta * m_fSpeed);
		_float4 vGravity = _float4(0.f, m_fGravity, 0.f, 0.f);

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDelta - vGravity);
		m_pTransformCom->Turn(m_fTurnAxis, m_fTimeDelta, m_fTurn);

		if (m_fRunTime > 4.f)
		{
			m_fScale -= m_fTimeDelta * 4.f;

			if (m_fScale <= 0.f)
			{
				m_bDead = true;
				return OBJ_NOEVENT;
			}
			m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
		}

	}
	else if (m_bGravity == false)
	{
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float4 vDelta = (m_vDir * m_fTimeDelta * m_fSpeed);

		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vDelta);
		m_pTransformCom->Turn(m_fTurnAxis, m_fTimeDelta, m_fTurn);

		m_fSpeed -= m_fTimeDelta * m_fPreSpeed * 2.f;
		if (m_fSpeed < 0.f)
			m_fSpeed = 0.f;

		if (m_fRunTime > 0.5f)
		{
			m_fScale -= m_fTimeDelta * 4.f;

			if (m_fScale <= 0.f)
			{
				m_bDead = true;
				return OBJ_NOEVENT;
			}
			m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
		}
	}

	m_pTransformCom->Turn(m_fTurnAxis, m_fTimeDelta, m_fTurn);

	return OBJ_NOEVENT;
}

void CCrashParticle::Late_Tick(_float fTimeDelta)
{

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CCrashParticle::Render()
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
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CCrashParticle::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CCrashParticle::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_RoadParticle"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CCrashParticle::Bind_ShaderResources()
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

void CCrashParticle::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vMotionVelocity.z = 0.f;

	m_vPreScreenPos = vCurScreenPos;
}

CCrashParticle* CCrashParticle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCrashParticle* pInstance = new CCrashParticle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCrashParticle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCrashParticle::Clone(void* pArg)
{
	CCrashParticle* pInstance = new CCrashParticle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCrashParticle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrashParticle::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
