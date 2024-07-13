#include "stdafx.h"
#include "Finale_SpecialDebris_C.h"
#include "FinaleCut_ControlCenter.h"


CFinale_SpecialDebris_C::CFinale_SpecialDebris_C(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFinale_SpecialDebris_C::CFinale_SpecialDebris_C(const CFinale_SpecialDebris_C& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFinale_SpecialDebris_C::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CFinale_SpecialDebris_C::Initialize(void* pArg)
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

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(2550.f, 239.f, -136.f, 1.f));
	_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
	_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
	_float4 NewRight = XMVector3Cross(NewUp, NewLook);

	m_pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
	m_pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);


	return S_OK;
}

_int CFinale_SpecialDebris_C::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fAccTime = m_pGameInstance->Get_SecondTimer();

	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return OBJ_NOEVENT;

	_int iCutIndex = pCenter->Get_CutScene();

	if (iCutIndex == 10)
	{
		m_bRender = true;
		m_eCurCut = CUT10;
	}
	else if (iCutIndex == 11)
	{
		m_bRender = true;
		m_eCurCut = CUT11;
	}
	else if (iCutIndex == 12)
	{
		m_bRender = true;
		m_eCurCut = CUT12;
	}
	else if (iCutIndex == 13)
	{
		m_bRender = true;
		m_eCurCut = CUT13;
	}
	else if (iCutIndex == 14)
	{
		m_bRender = true;
		m_eCurCut = CUT14;
	}
	else if (iCutIndex == 15)
	{
		m_bRender = true;
		m_eCurCut = CUT15;
	}
	else if (iCutIndex == 16)
	{
		m_bRender = true;
		m_eCurCut = CUT16;
	}
	else if (iCutIndex == 17)
	{
		m_bRender = true;
		m_eCurCut = CUT17;
	}
	else if (iCutIndex == 18)
	{
		m_bRender = true;
		m_eCurCut = CUT18;
	}
	else if (iCutIndex == 19)
	{
		m_bRender = true;
		m_eCurCut = CUT19;
	}
	else if (iCutIndex == 20)
	{
		m_bRender = true;
		m_eCurCut = CUT20;
	}
	else
		m_bRender = false;

	Set_Animation();


	return OBJ_NOEVENT;
}

void CFinale_SpecialDebris_C::Late_Tick(_float fTimeDelta)
{
	if (m_bRender == false)
		return;

	m_pModelCom->Play_Animation(m_fAccTime);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

}

HRESULT CFinale_SpecialDebris_C::Render()
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

HRESULT CFinale_SpecialDebris_C::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFinale_SpecialDebris_C::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_CutDebrisC"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CFinale_SpecialDebris_C::Bind_ShaderResources()
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

_int CFinale_SpecialDebris_C::Make_Partical()
{


	return 0;
}

void CFinale_SpecialDebris_C::Set_Animation()
{
	if (m_eCurCut == m_ePreCut)
		return;

	switch (m_eCurCut)
	{
	case CUT10:
		m_pModelCom->Set_Animation(CUT10, 50.f, false, false);
		break;
	case CUT11:
		m_pModelCom->Set_Animation(CUT11, 50.f, false, false);
		break;
	case CUT12:
		m_pModelCom->Set_Animation(CUT12, 50.f, false, false);
		break;
	case CUT13:
		m_pModelCom->Set_Animation(CUT13, 50.f, false, false);
		break;
	case CUT14:
		m_pModelCom->Set_Animation(CUT14, 50.f, false, false);
		break;
	case CUT15:
		m_pModelCom->Set_Animation(CUT15, 50.f, false, false);
		break;
	case CUT16:
		m_pModelCom->Set_Animation(CUT16, 50.f, false, false);
		break;
	case CUT17:
		m_pModelCom->Set_Animation(CUT17, 50.f, false, false);
		break;
	case CUT18:
		m_pModelCom->Set_Animation(CUT18, 50.f, false, false);
		break;
	case CUT19:
		m_pModelCom->Set_Animation(CUT19, 50.f, false, false);
		break;
	case CUT20:
		m_pModelCom->Set_Animation(CUT20, 50.f, false, false);
		break;
	default:
		break;
	}

	m_ePreCut = m_eCurCut;

}

CFinale_SpecialDebris_C* CFinale_SpecialDebris_C::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinale_SpecialDebris_C* pInstance = new CFinale_SpecialDebris_C(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFinale_SpecialDebris_C"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFinale_SpecialDebris_C::Clone(void* pArg)
{
	CFinale_SpecialDebris_C* pInstance = new CFinale_SpecialDebris_C(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CFinale_SpecialDebris_C"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFinale_SpecialDebris_C::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
