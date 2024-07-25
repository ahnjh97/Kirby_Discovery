#include "stdafx.h"
#include "Finale_SpecialDebris_B.h"
#include "FinaleCut_ControlCenter.h"

#include "Bone.h"
#include "FinalePartical_Maker.h"
#include "MultiEffect.h"
#include "Camera_Main.h"

CFinale_SpecialDebris_B::CFinale_SpecialDebris_B(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinale_SpecialDebris_B::CFinale_SpecialDebris_B(const CFinale_SpecialDebris_B& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinale_SpecialDebris_B::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CFinale_SpecialDebris_B::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_bStencil = true;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(2550.f, 229.f, -136.f, 1.f));
	_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
	_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
	_float4 NewRight = XMVector3Cross(NewUp, NewLook);

	m_pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
	m_pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);


	return S_OK;
}

_int CFinale_SpecialDebris_B::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fAccTime = m_pGameInstance->Get_SecondTimer();

	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return OBJ_NOEVENT;

	_int iCutIndex = pCenter->Get_CutScene();

	if (iCutIndex == 4)
	{
		m_bRender = true;
		m_eCurCut = CUT4;
	}
	else if (iCutIndex == 5)
	{
		m_bRender = true;
		m_eCurCut = CUT5;
	}
	else if (iCutIndex == 6)
	{
		m_bRender = true;
		m_eCurCut = CUT6;

		if (m_bEffectOn == true)
		{
			CMultiEffect::MULTI_FX_DESC desc;
			desc.vInitPos = { 0.f, 1.4f, -0.5f };
			desc.vInitScale = { 9.f, 9.f, 9.f };
			desc.pSocketMatrix = &m_EffectSocket;

			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_HS_meteo dash line"), &desc)))
				return OBJ_NOEVENT;
			CEffect* pEffect = static_cast<CEffect*>(m_pGameInstance->Get_List(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Effect"))->back());
			m_pEffect = pEffect;
			Safe_AddRef(m_pEffect);

			m_bEffectOn = false;
		}
	}
	else if (iCutIndex == 7)
	{
		m_bRender = true;
		m_eCurCut = CUT7;

		if (m_pEffect != nullptr)
		{
			m_pEffect->Set_Dead();
			Safe_Release(m_pEffect);
			m_pEffect = nullptr;
		}

	}
	else
		m_bRender = false;

	Set_Animation();
	Make_Particle();
	Compute_My_Look();

	return OBJ_NOEVENT;
}

void CFinale_SpecialDebris_B::Late_Tick(_float fTimeDelta)
{
	if (m_bRender == false)
		return;

	m_pModelCom->Play_Animation(m_fAccTime);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

}

HRESULT CFinale_SpecialDebris_B::Render()
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
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;
		m_pModelCom->Render(i);
	}
	return S_OK;
}

HRESULT CFinale_SpecialDebris_B::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFinale_SpecialDebris_B::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_CutDebrisB"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CFinale_SpecialDebris_B::Bind_ShaderResources()
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

void CFinale_SpecialDebris_B::Make_Particle()
{
	if (m_eCurCut == CUT6 && m_pModelCom->Get_CurTrackPosition() >= 325.f && m_BlackTrigger == true)
	{
		m_pGameInstance->Set_ObjectBlack(0.2f, 0.5f);
		m_BlackTrigger = false;
	}


	if (m_eCurCut == CUT6 && m_pModelCom->Get_CurTrackPosition() >= 375.f)
	{
		if (m_bParticleTrigger == true)
		{
			m_pGameInstance->PlaySound_Free(L"덤프트럭으로 부수는 소리.wav", 0.5f);

			CBone* pBone = m_pModelCom->Get_BonePtr("AllL");
			_float4x4 pBoneLocalMatrix = *pBone->Get_CombinedTransformationMatrix();
			_float4x4 pBoneWorldMatrix = pBoneLocalMatrix * m_pTransformCom->Get_WorldFloat4x4();
			_float4 vPos = CUtils::Get_State_Vector_Matrix(pBoneWorldMatrix, CUtils::STATE_POSITION);

			_float4 vEffectPos = vPos;
			vEffectPos.y -= 25.f;

			CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_FinalePartical_Maker")));
			pMaker->Make_Partical(40, vPos, 15.f, 6.f, 3.f, _float4(1.f, 1.f, 0.f, 0.f), 180.f, CUtils::Make_RandomFloat(100.f, 150.f));
			pMaker->Make_Partical(15, vPos, 40.f, 6.f, 3.f, _float4(1.f, 0.2f, 0.f, 0.f), 180.f, CUtils::Make_RandomFloat(120.f, 150.f));


			for (_int i = 0; i < 15; ++i)
			{
				CEffect::FX_DESC FXDesc{};

				FXDesc.vInitPos = static_cast<_float3>(vEffectPos) + (_float3)CUtils::Make_Random_Vector(3.f);
				FXDesc.vInitRot = CUtils::Make_Degree_FromDir((_float3)CUtils::Make_Random_Vector(1.f));

				_float fScale = CUtils::Make_RandomFloat(30.f, 40.f);
				FXDesc.vInitScale = { fScale, fScale, fScale };
				if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_finale collide smoke test3"), &FXDesc)))
					return;
			}

			CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
			pCamera->Make_Shake(8.f, 0.7f);
			m_pGameInstance->Setting_RadialBlur(50.f, 60.f);

			m_pGameInstance->Set_ObjectBlack(0.6f, 0.3f);
			m_bParticleTrigger = false;
		}
	}
}

void CFinale_SpecialDebris_B::Set_Animation()
{
	if (m_eCurCut == m_ePreCut)
		return;

	switch (m_eCurCut)
	{
	case CUT4:
		m_pModelCom->Set_Animation(CUT4, 40.f, false, false);
		break;
	case CUT5:
		m_pModelCom->Set_Animation(CUT5, 40.f, false, false);
		break;
	case CUT6:
		m_pModelCom->Set_Animation(CUT6, 70.f, false, false);
		break;
	case CUT7:
		m_pModelCom->Set_Animation(CUT7, 50.f, false, false);
		break;
	default:
		break;
	}

	m_ePreCut = m_eCurCut;

}

void CFinale_SpecialDebris_B::Compute_My_Look()
{
	m_EffectSocket = _float4x4::Identity;

	CBone* pBone = m_pModelCom->Get_BonePtr("AllL");
	m_vCurPos = m_pTransformCom->ComputeBoneWorldPos(pBone);

	if (m_bInitializeLook == true)
	{
		m_vPrePos = m_vCurPos - _float4(-1.f, 0.f, 0.f, 0.f);
		m_bInitializeLook = false;
	}

	_float4 vNewLook = m_vCurPos - m_vPrePos;
	vNewLook.Normalize();
	_float4 vNewRight = XMVector3Cross(_float4(0.f, 1.f, 0.f, 0.f), vNewLook);
	vNewRight.Normalize();
	_float4 vNewUp = XMVector3Cross(vNewLook, vNewRight);
	vNewUp.Normalize();

	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_POSITION, m_vCurPos);
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_LOOK, vNewLook);
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_UP, vNewUp);
	CUtils::Set_State_Matrix(m_EffectSocket, CUtils::STATE_RIGHT, vNewRight);

	m_vPrePos = m_vCurPos;

}

CFinale_SpecialDebris_B* CFinale_SpecialDebris_B::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinale_SpecialDebris_B* pInstance = new CFinale_SpecialDebris_B(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinale_SpecialDebris_B"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinale_SpecialDebris_B::Clone(void* pArg)
{
	CFinale_SpecialDebris_B* pInstance = new CFinale_SpecialDebris_B(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinale_SpecialDebris_B"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinale_SpecialDebris_B::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pEffect);
}
