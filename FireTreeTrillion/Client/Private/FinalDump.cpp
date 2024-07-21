#include "stdafx.h"
#include "FinalDump.h"
#include "Camera_Main.h"
#include "CrashParticle.h"
#include "Effect.h"

CFinalDump::CFinalDump(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinalDump::CFinalDump(const CFinalDump& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinalDump::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFinalDump::Initialize(void* pArg)
{
	DUMPDESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (DUMPDESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	m_pModelCom->Set_Animation(0, 0.2f, false, false);
	m_pTransformCom->Set_WorldMatrix(Desc->Matrix);


	m_bRimLight = false;
	m_bStencil = true;
	m_bMotionBlur = false;

	return S_OK;
}

_int CFinalDump::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;


	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_pModelCom->Set_Animation(0, 0.2f, false, false);

	if (m_bBound == false && m_bOnTerrain == false)
	{
		m_fGravity += fTimeDelta * GRAVITY;
		vPos.y -= m_fGravity;

		if (vPos.y <= -0.3f)
		{
			vPos.y = -0.3f;
			m_fGravity = 0.f;
			m_bBound = true;

			_float4 vEffectPos = vPos;
			vEffectPos += m_pTransformCom->Get_State(CTransform::STATE_RIGHT) * 16.f;

			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake(1.5f);

			for (_int i = 0; i < 20; ++i)
			{
				_float4 vTemp = { 2.5f, 0.f, 0.f, 0.f };
				_float4x4 RotMat = _float4x4::Identity;
				CUtils::Turn_OtherMatrix(RotMat, XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
				vTemp = XMVectorSetW(XMVector3Transform(vTemp, RotMat), 0.f);
				CCrashParticle::CRASHPARTICLEDESC Crashdesc = {};
				Crashdesc.vPos = vEffectPos + vTemp;
				Crashdesc.vDir = vTemp;
				Crashdesc.vDir.Normalize();
				Crashdesc.vDir.y += 3.f;
				Crashdesc.vDir.Normalize();
				Crashdesc.fSpeed = CUtils::Make_RandomFloat(30.f, 50.f);
				Crashdesc.bGravity = true;
				Crashdesc.fScale = { CUtils::Make_RandomFloat(0.2f, 0.5f) };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_CrashParticle"), TEXT("Prototype_GameObject_CrashParticle"), &Crashdesc)))
					return OBJ_NOEVENT;
			}


			for (_int i = 0; i < 15; ++i)
			{
				CEffect::FX_DESC FXDesc{};
				FXDesc.vInitPos = (_float3)vEffectPos + (_float3)CUtils::Make_Random_Vector(2.f);
				FXDesc.vInitRot = CUtils::Make_Degree_FromDir((_float3)CUtils::Make_Random_Vector(1.f));
				_float fScale = CUtils::Make_RandomFloat(3.f, 7.f);
				FXDesc.vInitScale = { fScale, fScale, fScale };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale collide smoke test3"), &FXDesc)))
					return OBJ_NOEVENT;
			}
		}
	}
	else if (m_bBound == true && m_bOnTerrain == false)
	{
		m_fGravity += fTimeDelta * GRAVITY;
		_float vBoundDelta = 0.5f - m_fGravity;
		vPos.y += vBoundDelta;

		if (vPos.y < -0.3f)
		{
			vPos.y = -0.3f;
			m_bOnTerrain = true;
		}
	}
	else if (m_bOnTerrain == true)
	{
		// 반딧불 같은 이펙트 ?
	}

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	return OBJ_NOEVENT;
}

void CFinalDump::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 5.0f))
	{
		m_pModelCom->Play_Animation(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CFinalDump::Render()
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
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (i == 10)
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_X)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
				return E_FAIL;
		}

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CFinalDump::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CFinalDump::Render_IMGUI()
{
}
#endif

HRESULT CFinalDump::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_DumpCar"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CFinalDump::Bind_ShaderResources()
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

CFinalDump* CFinalDump::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalDump* pInstance = new CFinalDump(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinalDump"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinalDump::Clone(void* pArg)
{
	CFinalDump* pInstance = new CFinalDump(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinalDump"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinalDump::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
