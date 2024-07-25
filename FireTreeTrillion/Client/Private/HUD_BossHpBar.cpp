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
	Safe_AddRef(m_pMyMonster);

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fSizeX, m_fSizeY, 0.f, 1.f));

	for (_int i = UI_BARPLATE; i <= UI_BARHIGH; ++i)
	{
		XMStoreFloat4x4(&m_BarMatrix[i], XMMatrixIdentity());
		CUtils::Set_Scaled_Matrix(m_BarMatrix[i], 900.f, 55.f, 1.f);
		CUtils::Turn_OtherMatrix(m_BarMatrix[i], _float4(0.f, 0.f, 1.f, 0.f), 1.f, 3.5f);
	}
	XMStoreFloat4x4(&m_NameMatrix, XMMatrixIdentity());
	//CUtils::Set_Scaled_Matrix(m_NameMatrix, 240.f, 40.f, 1.f); //07.24) 보스네임 사이즈 조정
	CUtils::Set_Scaled_Matrix(m_NameMatrix, 300.f * 0.75f, 100.f * 0.75f, 1.f); //07.24) 보스네임 사이즈 조정
	CUtils::Turn_OtherMatrix(m_NameMatrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, -3.5f);

	CUtils::Set_State_Matrix(m_BarMatrix[UI_BARPLATE], CUtils::STATE_POSITION,
		_float4(m_fX - (m_fSizeX * 0.3f),
			-m_fY + (m_fSizeY * 0.1f), 
			1.f, 
			1.f));
	m_fOriginY[UI_BARPLATE] = m_BarMatrix[UI_BARPLATE]._42;

	CUtils::Set_State_Matrix(m_BarMatrix[UI_BARLOW], CUtils::STATE_POSITION,
		_float4(m_fX - (m_fSizeX * 0.3f),
			-m_fY + (m_fSizeY * 0.1f),
			1.f,
			1.f));
	m_fOriginY[UI_BARLOW] = m_BarMatrix[UI_BARLOW]._42;

	CUtils::Set_State_Matrix(m_BarMatrix[UI_BARMIDDLE], CUtils::STATE_POSITION,
		_float4(m_fX - (m_fSizeX * 0.3f),
			-m_fY + (m_fSizeY * 0.1f),
			1.f,
			1.f));
	m_fOriginY[UI_BARMIDDLE] = m_BarMatrix[UI_BARMIDDLE]._42;

	CUtils::Set_State_Matrix(m_BarMatrix[UI_BARHIGH], CUtils::STATE_POSITION,
		_float4(m_fX - (m_fSizeX * 0.3f),
			-m_fY + (m_fSizeY * 0.1f),
			1.f,
			1.f));
	m_fOriginY[UI_BARHIGH] = m_BarMatrix[UI_BARHIGH]._42;


	CUtils::Set_State_Matrix(m_NameMatrix, CUtils::STATE_POSITION,
		_float4(m_fX - (m_fSizeX * 0.08f),
			-m_fY + (m_fSizeY * 0.17f),
//		_float4(m_fX - (m_fSizeX * 0.1f),
//			-m_fY + (m_fSizeY * 0.175f),
			1.f,
			1.f));
	m_fNameOriginY = m_NameMatrix._42;



	m_vColor[UI_BARPLATE] = { 1.f, 1.f, 1.f };
	m_vColor[UI_BARLOW] = { 0.f, 0.f, 0.f };
	m_vColor[UI_BARMIDDLE] = { 255.f / 255.f, 128.f / 255.f, 64.f / 255.f };
	m_vColor[UI_BARHIGH] = { 138.f / 255.f, 20.f / 255.f, 174.f / 255.f };
	m_vNameColor = { 1.f, 1.f, 1.f };

	m_fAmplitude = 5.f;

	return S_OK;
}

_int CHUD_BossHpBar::Tick(_float fTimeDelta)
{
	if (m_pMyMonster == nullptr || m_pMyMonster->Get_Dead() || m_bDead == true)
		return OBJ_DEAD;

	if (m_uInitializeBar == 0)
	{
		// 처음 등장하는 등장 바 애니메이션. 알파값이 전부 차면서, 시작된다. 그 이후로 절대 발동안함.
		InitializeBar(fTimeDelta);
	}
	else if (m_uInitializeBar == 1)
	{
		// 보스의 움직임을 실시간으로 받아오고, 움직임이 구현되어있다.
		Compute_MyBossHp(fTimeDelta);
	}
	else if (m_uInitializeBar == 2)
	{
		ReleaseBar(fTimeDelta);
	}

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
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", UI_MASK)))
		return E_FAIL;

	// 바의 밑 바닥부터 상위 바 까지 그린다.
	for (_int i = UI_BARPLATE; i <= UI_BARHIGH; ++i)
	{ 
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_BarMatrix[i])))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i)))
			return E_FAIL;

		if (i == UI_BARPLATE)
		{
			const _float BlackRatio = 1.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskRatio", &BlackRatio, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor[i], sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Begin(POSTEX_BOSS_BARPASS_DEFAULT)))
				return E_FAIL;

		}
		else if (i == UI_BARLOW)
		{
			const _float BlackRatio = 1.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskRatio", &BlackRatio, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor[i], sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Begin(POSTEX_BOSS_BARPASS_DEFAULT)))
				return E_FAIL;

		}
		else if (i == UI_BARMIDDLE)
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fBossSlowHpBar, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor[i], sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Begin(POSTEX_BOSS_BARPASS_DEFAULT)))
				return E_FAIL;

		}
		else if (i == UI_BARHIGH)
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMaskRatio", &m_fBossHpBar, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vColor[i], sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Begin(POSTEX_BOSS_BARPASS_DEFAULT)))
				return E_FAIL;
		}
		if (FAILED(m_pVIBufferCom->Bind_Buffers()))
			return E_FAIL;
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_NameMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vRColor", &m_vNameColor, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pTextureNameCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	//07.23) PASS값 수정 POSTEX_QTEEFFECT > POSTEX_ALPHABLEND_NOTEST
	if (FAILED(m_pShaderCom->Begin(POSTEX_ALPHABLEND_NOTEST))) 
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

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

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_BossBar"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	wstring wstrTextureTag;
	if (*m_pCurrentLevelID == LEVEL_DEEDEEDEE) wstrTextureTag = TEXT("DeeDeeDee");
	else if (*m_pCurrentLevelID == LEVEL_SIMBA) wstrTextureTag = TEXT("Leongar");
	else if (*m_pCurrentLevelID == LEVEL_FINALBOSS) wstrTextureTag = TEXT("Elfilis");

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_HUD_BossName_") + wstrTextureTag,
		TEXT("Com_NameTexture"), (CComponent**)&m_pTextureNameCom)))
		return E_FAIL;

	return S_OK;
}

void CHUD_BossHpBar::InitializeBar(_float fTimeDelta)
{
	// 0.5초만에 알파가 1이 된다.
	if (m_fAlpha < 1.f)
		m_fAlpha += fTimeDelta * 2.f;
	else if (m_fAlpha > 1.f)
		m_fAlpha = 1.f;

	m_fCurBossHpRatio = m_pMyMonster->Get_Hp() / m_pMyMonster->Get_MaxHp();

	// 1초만에 피가 찬다.
	m_fBossHpBar += fTimeDelta;

	
	if (m_fBossHpBar >= m_fCurBossHpRatio)
	{
		m_fBossSlowHpBar = m_fBossHpBar = m_fCurBossHpRatio;
		m_uInitializeBar = 1;
		m_fAlpha = 1.f;
	}
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
		m_bComputeRatioDelta = true;

		m_bShaking = true;
		m_fShakingTime = 0.f;
		m_fAmplitude = 5.f;

		m_fBossHpBar = m_fCurBossHpRatio;

		if (m_fBossHpBar <= 0.f)
			m_uInitializeBar = 2;
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
		_float fCycle = 75.f;

		m_fShakingTime += fTimeDelta;
		m_fShakingAcc += fTimeDelta * fCycle;
		m_fMoveY = sin(m_fShakingAcc) * m_fAmplitude;

		for (_int i = UI_BARPLATE; i <= UI_BARHIGH; ++i)
		{
			_float4 vUIPos = CUtils::Get_State_Vector_Matrix(m_BarMatrix[i], CUtils::STATE_POSITION);
			vUIPos.y = m_fOriginY[i] + m_fMoveY;
			CUtils::Set_State_Matrix(m_BarMatrix[i], CUtils::STATE_POSITION, vUIPos);
		}

		_float4 vNameUIPos = CUtils::Get_State_Vector_Matrix(m_NameMatrix, CUtils::STATE_POSITION);
		vNameUIPos.y = m_fNameOriginY + m_fMoveY;
		CUtils::Set_State_Matrix(m_NameMatrix, CUtils::STATE_POSITION, vNameUIPos);


		m_fAmplitude -= fTimeDelta * 20.f;

		if (m_fShakingTime > 0.25f)
		{
			m_fShakingTime = 0.f;
			m_bShaking = false;
			m_fAmplitude = 5.f;
			m_fMoveY = 0.f;
			for (_int i = UI_BARPLATE; i <= UI_BARHIGH; ++i)
			{
				_float4 vUIPos = CUtils::Get_State_Vector_Matrix(m_BarMatrix[i], CUtils::STATE_POSITION);
				vUIPos.y = m_fOriginY[i];
				CUtils::Set_State_Matrix(m_BarMatrix[i], CUtils::STATE_POSITION, vUIPos);
			}

			vNameUIPos = CUtils::Get_State_Vector_Matrix(m_NameMatrix, CUtils::STATE_POSITION);
			vNameUIPos.y = m_fNameOriginY;
			CUtils::Set_State_Matrix(m_NameMatrix, CUtils::STATE_POSITION, vNameUIPos);
		}
	}

	m_fPreBossHpRatio = m_fCurBossHpRatio;

	// 미리 자리 잡아두고 흔들어 보자.
	// 구조 조금 바꿔볼 것.
}

void CHUD_BossHpBar::ReleaseBar(_float fTimeDelta)
{
	m_fBossSlowHpBar -= fTimeDelta;

	if (m_fBossSlowHpBar < 0.f)
		m_fBossSlowHpBar = 0.f;

	m_fAlpha -= fTimeDelta * 2.f;


	if (m_fAlpha <= 0.f)
		m_bDead = true;
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
	Safe_Release(m_pTextureNameCom);
}
