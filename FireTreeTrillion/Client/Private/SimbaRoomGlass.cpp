#include "stdafx.h"
#include "SimbaRoomGlass.h"
#include "EventCenter.h"

CSimbaRoomGlass::CSimbaRoomGlass(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSimbaRoomGlass::CSimbaRoomGlass(const CSimbaRoomGlass& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSimbaRoomGlass::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSimbaRoomGlass::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {};
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
		return E_FAIL;

	m_bRimLight = false;

	function<void(CGameObject*)> func = bind(&CSimbaRoomGlass::Set_Dead, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_GLASSBREAK, this, func);

	return S_OK;
}

_int CSimbaRoomGlass::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CSimbaRoomGlass::Late_Tick(_float fTimeDelta)
{
	Compute_ViewZ();

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSimbaRoomGlass::Render()
{
	HRESULT hr{};
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

		hr = m_pShaderCom->Begin(MODEL_ALPHABLEND);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CSimbaRoomGlass::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelTag;
	if (FAILED(__super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSimbaRoomGlass::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CSimbaRoomGlass* CSimbaRoomGlass::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSimbaRoomGlass* pInstance = new CSimbaRoomGlass(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSimbaRoomGlass"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSimbaRoomGlass::Clone(void* pArg)
{
	CSimbaRoomGlass* pInstance = new CSimbaRoomGlass(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CSimbaRoomGlass"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSimbaRoomGlass::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(KEVENT_SIMBA_GLASSBREAK, this);

	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
