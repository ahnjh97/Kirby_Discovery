#include "stdafx.h"
#include "Monster.h"
#include "MultiEffect.h"

CMonster::CMonster(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCharacter{ pDevice , pContext }
{
}

CMonster::CMonster(const CMonster& rhs)
	: CCharacter{ rhs }
{
}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void* pArg)
{
	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	m_fY = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION).m128_f32[1];

	return S_OK;
}

_int CMonster::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	// 무적시간을 담당한다.
	Damage_Delay(fTimeDelta);

	if (m_fY - 300.f > m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION).m128_f32[1])
		m_bDead = true;

	return OBJ_NOEVENT;
}

void CMonster::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

}

HRESULT CMonster::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CMonster::Render_LightDepth()
{
	return S_OK;
}

void CMonster::Damage_Delay(_float fTimeDelta)
{
	if (m_fPreHp > m_fHp)
	{
		m_bMonsterOverPower = true;
	}

	if (m_bMonsterOverPower == true)
	{
		m_fMonsterOverPowerTime += fTimeDelta;

		if (m_fMonsterOverPowerTime > m_fMonsterOverPowerMaxTime)
		{
			m_bMonsterOverPower = false;
			m_fMonsterOverPowerTime = 0.f;
		}
	}

	m_fPreHp = m_fHp;
}

#ifdef _DEBUG
void CMonster::Render_IMGUI()
{
	__super::Render_IMGUI();
}

#endif

void CMonster::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}
