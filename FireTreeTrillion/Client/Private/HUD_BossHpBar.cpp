#include "stdafx.h"
#include "HUD_BossHpBar.h"
#include "Monster.h"

CHUD_BossHpBar::CHUD_BossHpBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CHUD_BossHpBar::CHUD_BossHpBar(const CHUD_BossHpBar& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CHUD_BossHpBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHUD_BossHpBar::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;
	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pMyMonster = (CMonster*)pArg;

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fSizeX, m_fSizeY, 0.f, 1.f));

	for (_int i = UI_BARPLATE; i < UI_END; ++i)
		XMStoreFloat4x4(&m_BarMatrix[i], XMMatrixIdentity());

	m_vColor[UI_BARLOW] = { 0.f, 0.f, 0.f };
	m_vColor[UI_BARMIDDLE] = { 183.f / 255.f, 106.f / 255.f, 16.f / 255.f };
	m_vColor[UI_BARHIGH] = { 138.f / 255.f, 20.f / 255.f, 174.f / 255.f };

	m_fAmplitude = 30.f;

	return S_OK;
}

_int CHUD_BossHpBar::Tick(_float fTimeDelta)
{
	if (m_pMyMonster == nullptr || m_pMyMonster->Get_Dead())
		return OBJ_DEAD;

	// 보스의 움직임을 실시간으로 받아옴.
	Compute_MyBossHp(fTimeDelta);

	// 처음 등장하는 등장 바 애니메이션. 알파값이 전부 차면서, 시작된다. 그 이후로 절대 발동안함.
	InitializeBar(fTimeDelta);

	// 바의 움직임을 제어한다.
	Bar_Animation(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHUD_BossHpBar::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHUD_BossHpBar::Render()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	// 바의 밑 바닥부터 상위 바 까지 그린다.
	for (_int i = UI_BARPLATE; i <= UI_BARHIGH; ++i)
	{ 
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_BarMatrix[i])))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(9)))
			return E_FAIL;
		if (FAILED(m_pVIBufferCom->Bind_Buffers()))
			return E_FAIL;
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CHUD_BossHpBar::Add_Components()
{
	// UI 셰이더 전용
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	// Rect를 써야 한다.
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_HUD_BossBar"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	return S_OK;
}

void CHUD_BossHpBar::InitializeBar(_float fTimeDelta)
{
}

void CHUD_BossHpBar::Compute_MyBossHp(_float fTimeDelta)
{
	m_fCurBossHpRatio = m_pMyMonster->Get_Hp() / m_pMyMonster->Get_MaxHp();


	if (m_bInitializeRatioSet == true)
	{
		m_fBossSlowHpBar = m_fBossHpBar = m_fPreBossHpRatio = m_fCurBossHpRatio;
		m_bInitializeRatioSet = false;
	}


	// 만약, 현재 HP 가 과거 HP 보다 낮아졌다면, 피통이 닳은것이다. (1틱 체킹)
	if (m_fCurBossHpRatio < m_fPreBossHpRatio)
	{
		m_bDamage = true;
		m_fSlowMovingTime = 0.f;
		m_fRatioDelta = 0.f;

		m_bShaking = true;
		m_fShakingTime = 0.f;
		m_fAmplitude = 30.f;
	}

	
	if (m_bDamage == true)
	{
		m_fSlowMovingTime += fTimeDelta;

		if (m_fSlowMovingTime > 1.2f)
		{
			// 피가 단지 0.8초가 지났다면 처음에 한번 델타 값을 구한 뒤, 닳게한다.
			if (m_bComputeRatioDelta == true)
			{
				m_fRatioDelta = m_fBossSlowHpBar - m_fBossHpBar;
				m_bComputeRatioDelta = false;
			}

			m_fBossSlowHpBar -= m_fRatioDelta * fTimeDelta * 4.f;

			// 만약, 도달했다면 다음에 받을 준비를 한다.
			if (m_fBossSlowHpBar < m_fBossHpBar)
			{
				m_bComputeRatioDelta = true;
				m_bDamage = false;
				m_fSlowMovingTime = 0.f;
				m_fRatioDelta = 0.f;
				m_fBossSlowHpBar = m_fBossHpBar;
			}
		}
	}


	if (m_bShaking == true)
	{
		// 진동 주기
		_float fCycle = 50.f;

		m_fShakingTime += fTimeDelta;
		m_fShakingAcc += fTimeDelta * fCycle;
		m_fY = sin(m_fShakingAcc) * m_fAmplitude;

		m_fAmplitude -= fTimeDelta * 60.f;

		if (m_fShakingTime > 0.5f)
		{
			m_fShakingTime = 0.f;
			m_bShaking = false;
			m_fAmplitude = 30.f;
			m_fY = 0.f;
		}
	}

	m_fPreBossHpRatio = m_fCurBossHpRatio;

	// 미리 자리 잡아두고 흔들어 보자.
	// 구조 조금 바꿔볼 것.
}

void CHUD_BossHpBar::Bar_Animation(_float fTimeDelta)
{
}

CHUD_BossHpBar* CHUD_BossHpBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHUD_BossHpBar* pInstance = new CHUD_BossHpBar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CHUD_BossHpBar"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHUD_BossHpBar::Clone(void* pArg)
{
	CHUD_BossHpBar* pInstance = new CHUD_BossHpBar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHUD_BossHpBar"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CHUD_BossHpBar::Free()
{
	__super::Free();
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pMyMonster);
}
