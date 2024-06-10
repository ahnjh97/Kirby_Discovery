#include "stdafx.h"
#include "MapToolObject.h"

CMapToolObject::CMapToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMapToolObject::CMapToolObject(const CMapToolObject& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CMapToolObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMapToolObject::Initialize(void* pArg)
{
	MAPTOOLOBJECT_DESC GameObjectDesc = {};
	if (pArg)
		GameObjectDesc = *(MAPTOOLOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_iTriggerIndex = GameObjectDesc.iTriggerIndex;
	m_iTriggerType = GameObjectDesc.iTriggerType;
	m_iCamType = GameObjectDesc.iCamType;
	m_fRadius = GameObjectDesc.fRadius;
	m_RallyPoints = GameObjectDesc.RallyPoints;
	m_strConnectedMonster = GameObjectDesc.strConnectedMonster;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelName)))
		return E_FAIL;
	
	if (GameObjectDesc.wstrModelName == TEXT("Dummy"))
	{
		if (FAILED(Add_PartObject()))
			return E_FAIL;

		m_pOrbitingCamera->Set_OrbitingCameraPos(XMVectorSet(0, 0, -m_fRadius, 0));
	}

	return S_OK;
}

_int CMapToolObject::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (nullptr != m_pOrbitingCamera) {
		m_pOrbitingCamera->Set_Radius(m_fRadius);
		m_pOrbitingCamera->Tick(fTimeDelta);
	}
		
	return OBJ_NOEVENT;
}

void CMapToolObject::Late_Tick(_float fTimeDelta)
{
	if (nullptr != m_pOrbitingCamera)
		m_pOrbitingCamera->Late_Tick(fTimeDelta);

	if(false == m_bHide)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CMapToolObject::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	string strModelName = m_pModelCom->Get_ModelInfo().strModelName;

	_uint iPassIndex{};
	if ("Trigger" == strModelName)
		iPassIndex = MODEL_TRIGGER;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if(FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

		if (FAILED(m_pShaderCom->Begin(iPassIndex)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CMapToolObject::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelTag;
	if (FAILED(__super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CMapToolObject::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (m_pModelCom->Get_ModelInfo().strModelName == "Trigger") {
		if (FAILED(m_pShaderCom->Bind_RawValue("g_iTriggerType", &m_iTriggerType, sizeof(_uint))))
			return E_FAIL;
	}
	m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
	m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
	if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
		return E_FAIL;
	m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));

	return S_OK;
}

HRESULT CMapToolObject::Add_PartObject()
{
	CPartObject* pOrbitingCamera = { nullptr };
	CPartObject::PARTOBJECT_DESC tPartObjectDesc{};

	tPartObjectDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	tPartObjectDesc.wstrModelName = TEXT("Camera");
	tPartObjectDesc.matWorld._11 = 0.5f;
	tPartObjectDesc.matWorld._22 = 0.5f;
	tPartObjectDesc.matWorld._33 = 0.5f;
	tPartObjectDesc.matWorld._43 = -2;
	
	m_pOrbitingCamera = dynamic_cast<COrbitingCamera*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_OrbitingCamera"), &tPartObjectDesc));

	return S_OK;
}

CMapToolObject* CMapToolObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMapToolObject* pInstance = new CMapToolObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMapToolObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMapToolObject::Clone(void* pArg)
{
	CMapToolObject* pInstance = new CMapToolObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CMapToolObject"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMapToolObject::Free()
{
	__super::Free();

	Safe_Release(m_pOrbitingCamera);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
