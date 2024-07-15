#include "stdafx.h"
#include "QTE.h"
#include "QTE_Effect.h"
#include "FinaleKirby.h"

CQTE::CQTE(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CQTE::CQTE(const CQTE& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CQTE::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE::Initialize(void* pArg)
{
	QTEDESC		Desc{};

	if (pArg != nullptr)
		Desc = *(QTEDESC*)pArg;
	
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.0f);

	m_eQTEType = Desc.eType;
	m_vOffSet = Desc.vOffSet;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	if (FAILED(Add_Components()))
		return E_FAIL;


	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	for (_int i = TEXTURE_PLATE; i < TEXTURE_END; ++i)
	{
		XMStoreFloat4x4(&m_UIMatrix[i], XMMatrixIdentity());
	}
	CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_PLATE], m_fSizeY * 0.18f * m_fBaseScale, m_fSizeY * 0.18f * m_fBaseScale, 1.f);
	CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);
	CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE2], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);


	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(m_fSizeX, m_fSizeY, 0.f, 1.f));

	CGameObject* pFinaleKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
	_float4 vKirbyPos = pFinaleKirby->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);
	_float4 vUIPos = vKirbyPos + m_vOffSet;
	_matrix VPMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vViewportPos = XMVector3TransformCoord(vUIPos, VPMatrix);
	_float fViewX = XMVectorGetX(vViewportPos);
	_float fViewY = XMVectorGetY(vViewportPos);

	for (_int i = TEXTURE_PLATE; i < TEXTURE_END; ++i)
	{
		CUtils::Set_State_Matrix(m_UIMatrix[i], CUtils::STATE_POSITION, _float4(fViewX * m_fX, fViewY * m_fY, 1.f, 1.f));
	}

	return S_OK;
}

_int CQTE::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	if (m_iQTE_State == 0)
	{
		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_PLATE], m_fSizeY * 0.18f * m_fBaseScale, m_fSizeY * 0.18f * m_fBaseScale, 1.f);
		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);
		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE2], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);
		m_fBaseScale -= fTimeDelta * 30.f;
		m_fAlpha += fTimeDelta * 7.5f;

		if (m_fBaseScale < 1.f)
		{
			m_fBaseScale = 1.f;
			m_fAlpha = 1.f;
			m_iQTE_State = 1;
		}
	}
	else if (m_iQTE_State == 1)
	{
		QTE_Animation(fTimeDelta);
	}
	else if (m_iQTE_State == 2)
	{
		QTE_End(fTimeDelta);
	}



	Update_Pos(fTimeDelta);


	return OBJ_NOEVENT;
}

void CQTE::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CQTE::Render()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fRedRatio", &m_fRedRatio, sizeof(_float))))
		return E_FAIL;


	if (FAILED(m_pTextureCom[TEXTURE_PLATE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_UIMatrix[TEXTURE_PLATE])))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Begin(POSTEX_QTEPLATE)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	if (m_eQTEType == QTE_JOYSTICK)
	{
		if (FAILED(m_pTextureCom[m_eJoyStickControl]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_UIMatrix[m_eJoyStickControl])))
			return E_FAIL;
	}
	else
	{
		if (FAILED(m_pTextureCom[TEXTURE_BASE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_UIMatrix[TEXTURE_BASE])))
			return E_FAIL;

	}
	if (FAILED(m_pShaderCom->Begin(POSTEX_QTEBASE)))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CQTE::Add_Components()
{
	// UI 셰이더 전용
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	// Rect를 써야 한다.
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	wstring strTextureTag;
	if (m_eQTEType == QTE_A) strTextureTag = TEXT("QTE_A");
	else if (m_eQTEType == QTE_B) strTextureTag = TEXT("QTE_B");
	else if (m_eQTEType == QTE_JOYSTICK) strTextureTag = TEXT("QTE_JoyOff");


	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_QTE_Plate"),
		TEXT("Com_TexturePlate"), (CComponent**)&m_pTextureCom[TEXTURE_PLATE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_" + strTextureTag),
		TEXT("Com_TextureBase"), (CComponent**)&m_pTextureCom[TEXTURE_BASE])))
		return E_FAIL;

	if (m_eQTEType == QTE_JOYSTICK)
	{
		if (FAILED(__super::Add_Component(*m_pCurrentLevelID, TEXT("Prototype_Component_Texture_QTE_JoyOn"),
			TEXT("Com_TextureBase2"), (CComponent**)&m_pTextureCom[TEXTURE_BASE2])))
			return E_FAIL;
	}

	return S_OK;
}

void CQTE::Update_Pos(_float fTimeDelta)
{
	CGameObject* pFinaleKirby = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
	if (pFinaleKirby == nullptr)
		return;

	_float4 vKirbyPos = static_cast<CFinaleKirby*>(pFinaleKirby)->m_vBonePos;
	_float4 vUIPos = vKirbyPos + m_vOffSet;
	_matrix VPMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vViewportPos = XMVector3TransformCoord(vUIPos, VPMatrix);
	_float fViewX = XMVectorGetX(vViewportPos);
	_float fViewY = XMVectorGetY(vViewportPos);
	for (_int i = TEXTURE_PLATE; i < TEXTURE_END; ++i)
	{
		CUtils::Set_State_Matrix(m_UIMatrix[i], CUtils::STATE_POSITION, _float4(fViewX * m_fX, fViewY * m_fY, 1.f, 1.f));
	}
}

void CQTE::QTE_Animation(_float fTimeDelta)
{
	if (m_eQTEType == QTE_A || m_eQTEType == QTE_B)
	{
		if ( (m_eQTEType == QTE_B  && m_pGameInstance->Get_DIKeyState(DIK_X, KEY_DOWN)) ||
			(m_eQTEType == QTE_A && m_pGameInstance->Get_DIKeyState(DIK_C, KEY_DOWN)))
		{
			m_fRedRatio = 0.7f;
			m_fBaseScale = 0.5f;
			m_fPlateScale = 1.5f;

			m_bClick = !m_bClick;

			CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
			QTEEffectdesc.vOffSet = m_vOffSet;
			QTEEffectdesc.vColor = m_bClick == true ?
				_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
				_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
				return;
		}
		m_fRedRatio -= fTimeDelta * 5.f;
		if (m_fRedRatio < 0.f)
			m_fRedRatio = 0.f;
		m_fPlateScale -= fTimeDelta * 4.f;
		if (m_fPlateScale < 1.f)
			m_fPlateScale = 1.f;
		m_fBaseScale += fTimeDelta * 5.f;
		if (m_fBaseScale > 1.f)
			m_fBaseScale = 1.f;

		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_PLATE], m_fSizeY * 0.18f * m_fPlateScale, m_fSizeY * 0.18f * m_fPlateScale, 1.f);
		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);
	}
	else if (m_eQTEType == QTE_JOYSTICK)
	{
		m_fJoyStickControlTime += fTimeDelta;

		if (m_pGameInstance->Get_DIKeyState(DIK_UP, KEY_DOWN))
		{
			m_bClick = !m_bClick;

			CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
			QTEEffectdesc.vOffSet = m_vOffSet;
			QTEEffectdesc.vColor = m_bClick == true ?
				_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
				_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
				return;

			m_eJoyStickControl = TEXTURE_BASE2;
			m_fRedRatio = 0.7f;
			m_fPlateScale = 1.5f;
			m_fJoyStickControlTime = 0.f;

			CUtils::Rotation(m_UIMatrix[TEXTURE_BASE2], CUtils::Get_State_Vector_Matrix(m_UIMatrix[TEXTURE_BASE2], CUtils::STATE_LOOK), ToRadian(0.f));

		}
		
		if (m_pGameInstance->Get_DIKeyState(DIK_DOWN, KEY_DOWN))
		{
			m_bClick = !m_bClick;

			CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
			QTEEffectdesc.vOffSet = m_vOffSet;
			QTEEffectdesc.vColor = m_bClick == true ?
				_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
				_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
				return;

			m_eJoyStickControl = TEXTURE_BASE2;
			m_fRedRatio = 0.7f;
			m_fPlateScale = 1.5f;
			m_fJoyStickControlTime = 0.f;

			CUtils::Rotation(m_UIMatrix[TEXTURE_BASE2], CUtils::Get_State_Vector_Matrix(m_UIMatrix[TEXTURE_BASE2], CUtils::STATE_LOOK), ToRadian(180.f));

		}
		
		if (m_pGameInstance->Get_DIKeyState(DIK_RIGHT, KEY_DOWN))
		{
			m_bClick = !m_bClick;

			CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
			QTEEffectdesc.vOffSet = m_vOffSet;
			QTEEffectdesc.vColor = m_bClick == true ?
				_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
				_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
				return;

			m_eJoyStickControl = TEXTURE_BASE2;
			m_fRedRatio = 0.7f;
			m_fPlateScale = 1.5f;
			m_fJoyStickControlTime = 0.f;

			CUtils::Rotation(m_UIMatrix[TEXTURE_BASE2], CUtils::Get_State_Vector_Matrix(m_UIMatrix[TEXTURE_BASE2], CUtils::STATE_LOOK), ToRadian(90.f));

		}
		
		if (m_pGameInstance->Get_DIKeyState(DIK_LEFT, KEY_DOWN))
		{
			m_bClick = !m_bClick;

			CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
			QTEEffectdesc.vOffSet = m_vOffSet;
			QTEEffectdesc.vColor = m_bClick == true ?
				_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
				_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
			if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
				return;

			m_eJoyStickControl = TEXTURE_BASE2;
			m_fRedRatio = 0.7f;
			m_fPlateScale = 1.5f;
			m_fJoyStickControlTime = 0.f;

			CUtils::Rotation(m_UIMatrix[TEXTURE_BASE2], CUtils::Get_State_Vector_Matrix(m_UIMatrix[TEXTURE_BASE2], CUtils::STATE_LOOK), ToRadian(270.f));
		}

		if (m_fJoyStickControlTime > 0.15f)
			m_eJoyStickControl = TEXTURE_BASE;

		m_fRedRatio -= fTimeDelta * 5.f;
		if (m_fRedRatio < 0.f)
			m_fRedRatio = 0.f;
		m_fPlateScale -= fTimeDelta * 4.f;
		if (m_fPlateScale < 1.f)
			m_fPlateScale = 1.f;

		CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_PLATE], m_fSizeY * 0.18f * m_fPlateScale, m_fSizeY * 0.18f * m_fPlateScale, 1.f);

	}
}

void CQTE::QTE_End(_float fTimeDelta)
{
	if (m_bEndEffectTrigger == true)
	{
		CQTE_Effect::QTEEFFECTDESC QTEEffectdesc = {};
		QTEEffectdesc.vOffSet = m_vOffSet;
		QTEEffectdesc.vColor = m_bClick == true ?
			_float3(226.f / 255.f, 226.f / 255.f, 97.f / 255.f) :
			_float3(155.f / 255.f, 231.f / 255.f, 231.f / 255.f);
		if (FAILED(CGameInstance::Get_Instance()->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_QTE_Effect"), TEXT("Prototype_GameObject_QTE_Effect"), &QTEEffectdesc)))
			return;

		m_bEndEffectTrigger = false;
	}

	m_fPlateScale += fTimeDelta * 10.f;
	m_fBaseScale += fTimeDelta * 10.f;
	m_fAlpha -= fTimeDelta * 5.f;
	CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_PLATE], m_fSizeY * 0.18f * m_fPlateScale, m_fSizeY * 0.18f * m_fPlateScale, 1.f);
	CUtils::Set_Scaled_Matrix(m_UIMatrix[TEXTURE_BASE], m_fSizeY * 0.1f * m_fBaseScale, m_fSizeY * 0.1f * m_fBaseScale, 1.f);


	if (m_fAlpha < 0.f)
	{
		m_fAlpha = 0.f;
		m_bDead = true;
	}
}

CQTE* CQTE::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE* pInstance = new CQTE(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CQTE"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE::Clone(void* pArg)
{
	CQTE* pInstance = new CQTE(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CQTE"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CQTE::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);

	for (auto& pTexture : m_pTextureCom)
		Safe_Release(pTexture);
}
