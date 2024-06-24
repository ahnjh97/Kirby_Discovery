#include "UIObject.h"
#include "GameInstance.h"

CUIObject::CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CUIObject::CUIObject(const CUIObject& rhs)
	: CGameObject(rhs)
	, m_ViewMatrix(rhs.m_ViewMatrix)
	, m_ProjMatrix(rhs.m_ProjMatrix)
	, m_bIsRender(rhs.m_bIsRender)
{
}

HRESULT CUIObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUIObject::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	return S_OK;
}

_int CUIObject::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	// 추후 다른 UI 클래스로 이사할 소스입니다. JYWI
	//_float fStandardTime(0.f); // 멤버변수로
	//_float fBeforeTime(0.f);   //ㅁㅂㅄ
	//fStandardTime += fTimeDelta;
	//if (fStandardTime - fBeforeTime >= 1.f)
	//{
		// 타임 숫자 텍스쳐 변경 함수
			// 타임이 20일 때, 점심시간 시작한다고 helper에게 알리기

			// 타임이 0일 때, 게임이 종료되었다고 helper에게 알리기

		//+) 타임바가 연두색에서 주황색으로 가는 조건은 bar 퍼센테이지가 60%이하일때이다. 그래서 타임이랑 상관 ㄴㄴ
	//}

	return OBJ_NOEVENT;
}

void CUIObject::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CUIObject::Render()
{
	return S_OK;
}

#ifdef _DEBUG
void CUIObject::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

void CUIObject::Free()
{
	__super::Free();

	if (!m_GroupUIs.empty())
	{
		for (auto& pGroupUI : m_GroupUIs)
		{
			for (auto& pUIObj : pGroupUI)
				Safe_Release(pUIObj);
		}
	}

	if (!m_LayerUIs.empty())
	{
		for (auto& pUIObj : m_LayerUIs)
			Safe_Release(pUIObj);

		m_LayerUIs.clear();
	}

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}

