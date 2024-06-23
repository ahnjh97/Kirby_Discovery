#include "stdafx.h"
#include "WaddleDee.h"
#include "FSM.h"

CWaddleDee::CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CCharacter{ pDevice, pContext }
{
}

CWaddleDee::CWaddleDee(const CWaddleDee& rhs)
	:CCharacter{ rhs }
{
}



HRESULT CWaddleDee::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWaddleDee::Initialize(void* pArg)
{
	HRESULT hr;

	hr = __super::Initialize(pArg);
	CHECK_FAILED_MSG(hr, "와들디 생성 망했어");

	m_eEyeState = DEEEYE_IDLE;

	return S_OK;
}

_int CWaddleDee::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	return OBJ_NOEVENT;
}

void CWaddleDee::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CWaddleDee::Render()
{
	__super::Render();
	return S_OK;
}

HRESULT CWaddleDee::Render_LightDepth()
{
	return S_OK;
}

void CWaddleDee::Change_State(DEE_ANIM eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation)
{
	m_pFSM->ChangeState((_uint)eState, _fAnimSpeed, _bLoop, _bInterpolation);
}

#ifdef _DEBUG
void CWaddleDee::Render_IMGUI()
{
	__super::Render_IMGUI();
}
#endif

_bool CWaddleDee::Custom_Face(_uint iMeshIndex)
{
	return _bool();
}

CGameObject* CWaddleDee::Clone(void* pArg)
{
	return nullptr;
}

void CWaddleDee::Dee_SystemTick(_float fTimeDelta)
{
	m_bIsKirbyInZone = false;

	if (0.f != m_fResetHiTime)
	{
		m_fResetHiTime -= fTimeDelta;

		if (m_fResetHiTime <= 0.f)
		{
			m_fResetHiTime = 0.f;
			Set_DeeEyeState(DEEEYE_IDLE);
			SetHiToKirby(false);
		}
	}
}

void CWaddleDee::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	//Safe_Release(m_pEyeTextureCom);
	//Safe_Release(m_pTestTextureCom);
}
