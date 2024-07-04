#include "stdafx.h"
#include "Radio.h"
#include "EventCenter.h"

CRadio::CRadio(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CRadio::CRadio(const CRadio& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CRadio::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CRadio::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;


	m_pModelCom->Set_Animation(RADIO_BEAT, 60.f, true, false);

	m_bStencil = true;
	m_bRimLight = true;
	m_bMotionBlur = false;


	//다리 뿌수기
	function<void(CGameObject*)> func = bind(&CRadio::JumpEvent, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_BREAK_RACINGMAP, this, func, 1);

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_fSaveY = vPos.y;

	m_pTransformCom->Turn(_float4(0.f, 1.f, 0.f, 0.f), 1.f, 71.f);

	return S_OK;
}

_int CRadio::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();



	if (m_bEventStart == true)
	{
		m_fEventTime += m_fTimeDelta;

		if (m_fEventTime > 5.8f)
		{
			m_pModelCom->Set_Animation(RADIO_JUMP, 60.f, false, false);
			m_bJump = true;
			m_bEventStart = false;
		}
	}

	if (m_bJump == true)
	{
		/*const _float fGravity = 4.5f;
		vPos.y += (40.f * m_fTimeDelta) - (fGravity * m_fJumpTime);*/

		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		m_fJumpTime += m_fTimeDelta * 2.f;

		if (m_iSwitchEaseIn == 0)
		{
			vPos.y = m_fSaveY + EASE_OUT(m_fJumpTime) * 1.5f;
			if (m_fJumpTime > 1.f)
			{
				m_iSwitchEaseIn = 1;
				m_fJumpTime -= 1.f;
				m_fSaveAfterY = vPos.y;
			}
		}
		else if (m_iSwitchEaseIn == 1)
		{
			vPos.y = m_fSaveAfterY - EASE_IN(m_fJumpTime) * 1.5f;
			if (m_fJumpTime > 1.f)
			{
				vPos.y = m_fSaveY;
				m_iSwitchEaseIn = 2;
				m_fJumpTime = 0.f;
			}
		}
		else if (m_iSwitchEaseIn == 2)
		{
			const _float fGravity = 4.5f;
			vPos.y += (20.f * m_fTimeDelta) - (fGravity * m_fJumpTime);

			if (m_fSaveY > vPos.y)
			{
				vPos.y = m_fSaveY;
				m_bJump = false;
				m_pModelCom->Set_Animation(RADIO_BEAT, 60.f, true, false);
			}
		}
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
	}

	return OBJ_NOEVENT;
}

void CRadio::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 3.0f))
	{
		if (Compute_OptimizationAnimation(m_fTimeDelta) == true)
			m_pModelCom->Play_Animation(m_fAccTime);

		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}

}

HRESULT CRadio::Render()
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
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CRadio::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CRadio::Render_IMGUI()
{
}
#endif

HRESULT CRadio::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Radio"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;

}

HRESULT CRadio::Bind_ShaderResources()
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

void CRadio::JumpEvent(CGameObject* pObj)
{
	m_bEventStart = { true };
}

CRadio* CRadio::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRadio* pInstance = new CRadio(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRadio"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRadio::Clone(void* pArg)
{
	CRadio* pInstance = new CRadio(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CRadio"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRadio::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	CEventCenter::Get_Instance()->Unsubscribe(this);


}
