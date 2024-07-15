#include "stdafx.h"
#include "PopStar.h"
#include "MultiEffect.h"

#include "FinaleCut_ControlCenter.h"

CPopStar::CPopStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPopStar::CPopStar(const CPopStar& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CPopStar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPopStar::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	// 임시
	m_pModelCom->Set_Animation(WAIT, 10.f, false, false);

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_fRimWidth = 5.f;
	m_bStencil = true;

	m_pTransformCom->Turn(_float4(0.f, 1.f, 0.f, 0.f), 1.f, 90.f);
	m_pTransformCom->Turn(_float4(0.f, 0.f, 1.f, 0.f), 1.f, 50.f);
	m_pTransformCom->Set_Scaled(8.f, 8.f, 8.f);


	//StarRiver	
	CEffect::FX_DESC FXDesc{};

	//FXDesc.vInitPos = _float3{15.f, -10.f, 0.f};
	//FXDesc.vInitPos = _float3{ 15.f, -40.f, 0.f };

	FXDesc.pSocketMatrix = &m_EffectSocket;
	FXDesc.vInitScale = { 1.5f, 1.5f, 1.5f };
	if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_StarRiver"), &FXDesc)))
		return E_FAIL;

	FXDesc.vInitScale = { 4.f, 4.f, 4.f };
	if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_star dash test 3"), &FXDesc)))
		return E_FAIL;

	GAMEOBJECT_DESC Smalldesc = {};
	Smalldesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	_float4 vSmallLook = CUtils::Get_State_Vector_Matrix(Smalldesc.matWorld, CUtils::STATE_LOOK);
	_float4 vSmallRight = CUtils::Get_State_Vector_Matrix(Smalldesc.matWorld, CUtils::STATE_RIGHT);
	_float4 vSmallPos = CUtils::Get_State_Vector_Matrix(Smalldesc.matWorld, CUtils::STATE_POSITION);

	_float4 vNewSmallPos = vSmallPos + (vSmallLook * -200.f) + vSmallRight * 30.f; //+ (vSmallUp * -50.f);
	CUtils::Set_State_Matrix(Smalldesc.matWorld, CUtils::STATE_POSITION, vNewSmallPos);
	if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_PopStar"), TEXT("Prototype_GameObject_PopStar_StarSmall"), &Smalldesc)))
		return E_FAIL;

	return S_OK;
}

_int CPopStar::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;


	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return OBJ_NOEVENT;

	_int iCutIndex = pCenter->Get_CutScene();


	m_EffectSocket = m_pTransformCom->ComputeBoneWorldMatrix(m_pModelCom->Get_BonePtr("PopStarL"), _float3(), true);
	//_float3 vAngle = { 90.f, 0.f, 0.f };
	//_float4x4 RotMat = _float4x4::CreateFromYawPitchRoll(CUtils::Degree_ToRadian(vAngle));
	//m_EffectSocket *= RotMat;


	if (iCutIndex == 1)
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(2550.f, 255.f, -136.f, 1.f));
		_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
		_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
		_float4 NewRight = XMVector3Cross(NewUp, NewLook);
		m_pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
		m_pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
		m_pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);

		m_eCurCut = CUT1;
	}
	else if (iCutIndex == 2)
	{
		m_eCurCut = CUT2;
	}
	else if (iCutIndex == 3)
	{
		m_eCurCut = CUT3;
	}
	else if (iCutIndex == 4)
	{
		m_eCurCut = CUT4;
	}
	else if (iCutIndex == 5)
	{
		m_eCurCut = CUT5;
	}
	else if (iCutIndex == 6)
	{
		m_eCurCut = CUT6;
	}
	else if (iCutIndex == 7)
	{
		m_eCurCut = CUT7;
	}
	else if (iCutIndex == 8)
	{
		m_eCurCut = CUT8;
	}
	else if (iCutIndex == 9)
	{
		m_eCurCut = CUT9;
	}
	else if (iCutIndex == 10)
	{
		m_eCurCut = CUT10;
	}
	else if (iCutIndex == 11)
	{
		m_eCurCut = CUT11;
	}
	else if (iCutIndex == 12)
	{
		m_eCurCut = CUT12;
	}
	else if (iCutIndex == 13)
	{
		m_eCurCut = CUT13;
	}
	else if (iCutIndex == 14)
	{
		m_eCurCut = CUT14;
	}
	else if (iCutIndex == 15)
	{
		m_eCurCut = CUT15;
	}
	else if (iCutIndex == 16)
	{

	}
	else if (iCutIndex == 17)
	{
		m_eCurCut = CUT17;
	}
	else if (iCutIndex == 18)
	{
		m_eCurCut = CUT18;
	}
	else if (iCutIndex == 19)
	{
		m_eCurCut = CUT19;
	}
	else if (iCutIndex == 20)
	{
		m_eCurCut = CUT20;
	}

	Set_Animation();

	return OBJ_NOEVENT;
}

void CPopStar::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
	m_RenderCount = 1;
}

HRESULT CPopStar::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	if (m_RenderCount == 1)
	{
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
		m_RenderCount = 0;
	}
	else if (m_RenderCount == 0)
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
				return E_FAIL;
			if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(ANIMMODEL_POPSTAR)))
				return E_FAIL;
			m_pModelCom->Render(i);
		}
		m_RenderCount = 0;
	}
	return S_OK;
}

void CPopStar::Set_Animation()
{
	if (m_eCurCut == m_ePreCut)
		return;

	m_pModelCom->Set_Animation(m_eCurCut, 50.f, false, false);
	m_ePreCut = m_eCurCut;

}

HRESULT CPopStar::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_PopStar"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CPopStar::Bind_ShaderResources()
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

CPopStar* CPopStar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPopStar* pInstance = new CPopStar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPopStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPopStar::Clone(void* pArg)
{
	CPopStar* pInstance = new CPopStar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CPopStar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPopStar::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
