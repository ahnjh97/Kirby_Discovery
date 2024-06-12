#include "..\Public\GameObject.h"
#include "GameInstance.h"



CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	, m_pGameInstance(rhs.m_pGameInstance)
	, m_wstrPrototypeTag(rhs.m_wstrPrototypeTag)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CComponent * CGameObject::Get_Component(const wstring & strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if (iter == m_Components.end())
		return nullptr;

	return iter->second;	
}

void CGameObject::Set_ShaderVars(_uint iShaderVars)
{
	m_iShaderVars = iShaderVars;
	m_bStencil = (iShaderVars >> 2) & 1;
	m_bRimLight = (iShaderVars >> 1) & 1;
	m_bMotionBlur = iShaderVars & 1;
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

/* 실제 게임내엣 사용되는 객체가 호출하는 함수다. */
HRESULT CGameObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if(nullptr != pArg)
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;		

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

	m_Components.emplace(g_strTransformTag, m_pTransformCom);

	Safe_AddRef(m_pTransformCom);

	if (nullptr != pArg) {
		m_pTransformCom->Set_WorldMatrix(pGameObjectDesc->matWorld);
		m_iShaderVars = pGameObjectDesc->iShaderVars;
		m_fRimWidth = pGameObjectDesc->fRimWidth;
		m_bStencil = (m_iShaderVars >> 2) & 1;
		m_bRimLight = (m_iShaderVars >> 1) & 1;
		m_bMotionBlur = m_iShaderVars & 1;
	}
		
	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();

	return S_OK;
}

_int CGameObject::Tick(_float fTimeDelta)
{


	return OBJ_NOEVENT;
}

void CGameObject::Late_Tick(_float fTimeDelta)
{

}

HRESULT CGameObject::Render()
{
	return S_OK;
}

#ifdef _DEBUG
void CGameObject::Render_IMGUI()
{
	string strTag = CUtils::WstrToStr(m_wstrPrototypeTag);
	string strWindowName = "Component Window : " + strTag;

	ImGui::BeginChild(strWindowName.c_str());
	for (auto& com : m_Components)
	{
		_char szName[256];
		CUtils::WCharToChar(com.first.c_str(), szName);

		if (ImGui::CollapsingHeader(szName))
			com.second->Render_IMGUI();
	}
	ImGui::EndChild();
	
#pragma region Stencil RimLight MotionBlur
	ImGui::Checkbox("Stencil", &m_bStencil);
	ImGui::SameLine();
	ImGui::Checkbox("RimLight", &m_bRimLight);
	ImGui::SameLine();
	ImGui::Checkbox("MotionBlur", &m_bMotionBlur);

	float windowWidth = ImGui::GetContentRegionAvail().x;
	float inputFloatWidth = 100; // 설정된 너비
	float thicknessTextWidth = ImGui::CalcTextSize("Thickness ").x;
	float totalWidth = thicknessTextWidth + inputFloatWidth + ImGui::GetStyle().ItemSpacing.x;
	float offset = (windowWidth - totalWidth) * 0.5f;
	if (offset > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

	ImGui::Text("Thickness ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	ImGui::InputFloat("##fRimWidth", &m_fRimWidth, 0.01f, 1.0f, "%.3f");
#pragma endregion

}
#endif



HRESULT CGameObject::Add_Component(_uint iLevelIndex, const wstring & strPrototypeTag, const wstring & strComponentTag, CComponent** ppOut, void * pArg)
{
	CComponent*		pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
	if (nullptr == pComponent)
		return E_FAIL;

	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	m_Components.emplace(strComponentTag, pComponent);
	
	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

/// <summary> LEVEL_STATIC이 아닌 경우, GameObject 내부에서 LEVEL을 처리한다. </summary>
HRESULT CGameObject::Add_Component(const wstring& strPrototypeTag, const wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
	CComponent* pComponent = m_pGameInstance->Clone_Component(*m_pCurrentLevelID, strPrototypeTag, pArg);
	if (nullptr == pComponent)
		return E_FAIL;

	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	m_Components.emplace(strComponentTag, pComponent);

	*ppOut = pComponent;

	Safe_AddRef(pComponent);

	return S_OK;
}

void CGameObject::Delete_Component(const wstring& strPrototypeTag, const wstring& strComponentTag, CComponent** ppOut)
{
	auto	iter = m_Components.find(strComponentTag);
	if (iter == m_Components.end())
		ALARM_FAIL("없는 컴포넌트를 찾으려고 했습니다.");
	else
	{
		Safe_Release(iter->second);
		m_Components.erase(iter);
	}
}

// fViewZ와 ViewPos를 업데이트 한다.
HRESULT CGameObject::Compute_ViewZ()
{
	_vector	vPosition = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	vPosition = XMVector3TransformCoord(vPosition, m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW));

	m_fViewZ = XMVectorGetZ(vPosition);
	XMStoreFloat3(&m_vViewPos, vPosition);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);

	m_Components.clear();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

}
