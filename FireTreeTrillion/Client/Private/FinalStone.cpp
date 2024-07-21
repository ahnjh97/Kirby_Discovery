#include "stdafx.h"
#include "FinalStone.h"
#include "CrashParticle.h"
#include "Camera_Main.h"
#include "Effect.h"

CFinalStone::CFinalStone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{pDevice, pContext}
{
}

CFinalStone::CFinalStone(const CFinalStone& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinalStone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinalStone::Initialize(void* pArg)
{
	FINALSTONEDESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (FINALSTONEDESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bRimLight = true;
	m_bStencil = true;
	m_bMotionBlur = true;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Desc->vPos);
	m_pTransformCom->Set_Scaled(Desc->fScale, Desc->fScale, Desc->fScale);

	m_pTransformCom->Turn(CUtils::Make_Random_Vector(1.f), 1.f, CUtils::Make_RandomFloat(120.f, 480.f));

	return S_OK;
}

_int CFinalStone::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	m_fGravity += fTimeDelta * GRAVITY;
	vPos.y -= m_fGravity;

	if (vPos.y <= 0.f)
	{
		m_bDead = true;

		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		pCamera->Make_Shake(1.5f);

		for (_int i = 0; i < 10; ++i)
		{
			_float4 vTemp = { 1.f, 0.f, 0.f, 0.f };
			_float4x4 RotMat = _float4x4::Identity;
			CUtils::Turn_OtherMatrix(RotMat, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			vTemp = XMVectorSetW(XMVector3Transform(vTemp, RotMat), 0.f);
			CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
			Crashdesc.vPos = vPos + vTemp;
			Crashdesc.vDir = vTemp;
			Crashdesc.vDir.Normalize();
			Crashdesc.vDir.y += 3.f;
			Crashdesc.vDir.Normalize();
			Crashdesc.fSpeed = CUtils::Make_RandomFloat(20.f, 40.f);
			Crashdesc.bGravity = true;
			Crashdesc.fScale = { CUtils::Make_RandomFloat(0.1f, 0.3f) };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
				return OBJ_NOEVENT;
		}

		for (_int i = 0; i < 8; ++i)
		{
			CEffect::FX_DESC FXDesc{};
			FXDesc.vInitPos = (_float3)vPos + (_float3)CUtils::Make_Random_Vector(2.f);
			FXDesc.vInitRot = CUtils::Make_Degree_FromDir((_float3)CUtils::Make_Random_Vector(1.f));
			_float fScale = CUtils::Make_RandomFloat(2.f, 4.f);
			FXDesc.vInitScale = { fScale, fScale, fScale };
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale collide smoke test3"), &FXDesc)))
				return OBJ_NOEVENT;
		}
	}

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	return OBJ_NOEVENT;
}

void CFinalStone::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CFinalStone::Render()
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

HRESULT CFinalStone::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFinalStone::Add_Components()
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

HRESULT CFinalStone::Bind_ShaderResources()
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

void CFinalStone::Compute_MotionBlur()
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


CFinalStone* CFinalStone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalStone* pInstance = new CFinalStone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinalStone"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinalStone::Clone(void* pArg)
{
	CFinalStone* pInstance = new CFinalStone(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinalStone"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinalStone::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
