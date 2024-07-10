#include "stdafx.h"
#include "Crumble.h"

#include "HitBox.h"
#include "Kirby.h"

CCrumble::CCrumble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CCrumble::CCrumble(const CCrumble& rhs)
	: CPhysXObject(rhs)
{
}

HRESULT CCrumble::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCrumble::Initialize(void* pArg)
{
	HRESULT hr;
	CRUMBLE_DESC* desc = {};
	if (pArg != nullptr)
		desc = (CRUMBLE_DESC*)pArg;

	hr = __super::Initialize(desc);
	CHECK_FAILED(hr);

	if (desc != nullptr)
		Add_Components(desc->wstrModelName);

	m_bMotionBlur = false;
	m_bRimLight = true;
	m_bStencil = true;

	m_pDynamicActor = m_pNonAnimModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	if (pKirby != nullptr)
		pKirby->RegisterActorsToPlayer_ForBox(m_pDynamicActor, this);

	m_vOriginPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_uint uAnim = desc->uInitialState;
	m_pModelCom->Set_Animation(uAnim, 30.f, true, false);

	return S_OK;
}

_int CCrumble::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;
	
	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();
	
	Change_State();

	_float4 vPos = GET_POS;
	m_pDynamicActor->setKinematicTarget(PxTransform{ vPos.x, vPos.y, vPos.z });

	return OBJ_NOEVENT;
}

void CCrumble::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(m_fTimeDelta);
	
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CCrumble::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(ANIMMODEL_NORMAL_O);
		CHECK_FAILED(hr);

		m_pModelCom->Render(i);
	}
	
	return S_OK;
}

HRESULT CCrumble::Render_LightDepth()
{
	HRESULT hr = m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

#ifdef _DEBUG
void CCrumble::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}
#endif

void CCrumble::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{

}

void CCrumble::Change_State()
{
	if (m_pModelCom->IsFinished())
	{
		switch (m_pModelCom->Get_CurAnimIndex())
		{
		case APPEAR:
		{
			m_pModelCom->Set_Animation(WAIT, 30.f, true, false);
			m_bOnce = false;
		}
		break;
		case DISAPPEAR:
		{
			m_fAccTimeDisappear += m_fTimeDelta;
			if (m_fAccTimeDisappear >= 3.f)
			{
				m_pModelCom->Set_Animation(APPEAR, 30.f, false, false);
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vOriginPosition);
				m_fAccTimeDisappear = 0.f;
			}
		}
		break;
		}
	}
	else
	{
		switch (m_pModelCom->Get_CurAnimIndex())
		{
		case PREDISAPPEAR:
		{
			m_fAccTimePreDisappear += m_fTimeDelta;
			if (m_fAccTimePreDisappear >= 2.f)
			{
				m_pModelCom->Set_Animation(DISAPPEAR, 30.f, false, false);
				m_fAccTimePreDisappear = 0.f;
			}
		}
		break;
		case DISAPPEAR:
		{
			m_pTransformCom->Go_Down(m_fTimeDelta);
		}
		break;
		}
	}
}

void CCrumble::Break_Crumble()
{
	if (false == m_bOnce)
	{
		m_pModelCom->Set_Animation(PREDISAPPEAR, 30.f, true, false);
		m_bOnce = true;
	}
}

void CCrumble::Add_Components(wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Crumble"), TEXT("Com_Model_Anim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_NonAnim_Crumble"), TEXT("Com_Model_NonAnim"), (CComponent**)&m_pNonAnimModelCom);
	CHECK_FAILED(hr);
}

HRESULT CCrumble::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
		return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
	//	return E_FAIL;

	return S_OK;
}

CCrumble* CCrumble::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCrumble* pInstance = new CCrumble(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCrumble"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCrumble::Clone(void* pArg)
{
	CCrumble* pInstance = new CCrumble(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCrumble"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCrumble::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pNonAnimModelCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

