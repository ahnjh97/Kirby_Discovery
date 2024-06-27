#include "stdafx.h"
#include "BreakableRockPartical.h"
#include "ToppleableBridge.h"
#include "AnimBridge.h"
#include "HitBox.h"
#include "Kirby.h"

CToppleableBridge::CToppleableBridge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CToppleableBridge::CToppleableBridge(const CToppleableBridge& rhs)
	: CPhysXObject{ rhs }
{
}

void CToppleableBridge::OnCollision()
{
	if (true == m_bCollision)
		return;

	m_fHitTime = 0.f;
	m_bCollision = true;
	m_pModelCom->DisableActors();
	CAnimBridge* pAnimBridge = dynamic_cast<CAnimBridge*>(m_pAnimBridge);
	pAnimBridge->OnCollision();

	return;
}

HRESULT CToppleableBridge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CToppleableBridge::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = { nullptr };

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	Desc->fSpeedPerSec = 7.f;
	Desc->fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components(Desc->wstrModelName)))
		return E_FAIL;

	m_bMotionBlur = false;
	m_wstrModelName = Desc->wstrModelName;

	if (TEXT("BoardA") == m_wstrModelName || TEXT("BoardB") == m_wstrModelName) {
		if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_WorldFloat4x4())))
			return E_FAIL;
	}
	else if (TEXT("BoardC") == m_wstrModelName) {
		unordered_set<string> setIncludeMesh = { "FakeCollider" };
		if (FAILED(m_pModelCom->CreateStaticActors_Include(setIncludeMesh, m_pTransformCom->Get_WorldFloat4x4())))
			return E_FAIL;
	}

	vector<PxRigidActor*> vecActors = m_pModelCom->Get_Actors();

	CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	for (auto& actor : vecActors)
		pKirby->RegisterActorsToPlayer(actor, this);

	GAMEOBJECT_DESC tDesc{};
	tDesc.wstrModelName = Desc->wstrModelName + TEXT("_Anim");
	tDesc.matWorld = Desc->matWorld;
	
	m_pAnimBridge = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_AnimBridge"), &tDesc);

	return S_OK;
}

_int CToppleableBridge::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if(nullptr != m_pAnimBridge)
		m_pAnimBridge->Tick(fTimeDelta);

	if (m_bCollision)
		m_fHitTime += fTimeDelta;

	if (m_fHitTime > 0.f && m_fHitTime < 0.75f)
	{
		if(TEXT("BoardA") == m_wstrModelName || TEXT("BoardB") == m_wstrModelName)
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), m_pGameInstance->Get_SecondTimer());
	}
		
	return OBJ_NOEVENT;
}

void CToppleableBridge::Late_Tick(_float fTimeDelta)
{
	if (nullptr != m_pAnimBridge)
		m_pAnimBridge->Late_Tick(fTimeDelta);

	if (m_fHitTime > 0.75f && m_bActorCreated == false) {
		if (TEXT("BoardA") == m_wstrModelName || TEXT("BoardB") == m_wstrModelName) {
			if (FAILED(m_pModelCom->CreateStaticActor(m_pTransformCom->Get_WorldFloat4x4())))
				return;
			m_bActorCreated = true;
		}
		else if (TEXT("BoardC") == m_wstrModelName) {
			unordered_set<string> setExcludeMesh = { "FakeCollider" };
			if (FAILED(m_pModelCom->CreateStaticActors_Exclude(setExcludeMesh, m_pTransformCom->Get_WorldFloat4x4())))
				return;
			m_bActorCreated = true;
		}
	}
}

HRESULT CToppleableBridge::Render()
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
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CToppleableBridge::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CToppleableBridge::Render_IMGUI()
{
}
#endif

HRESULT CToppleableBridge::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelName;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CToppleableBridge::Bind_ShaderResources()
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
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
	//	return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

void CToppleableBridge::Make_Particles()
{
	for (_int i = 0; i < 9; ++i)
	{
		_float4x4 matrix = m_pTransformCom->Get_WorldFloat4x4();
		_float4 vDir = m_vDamegeDir;

		vDir = CUtils::Make_RandomAngle_Vector(120.f, vDir);
		vDir.Normalize();
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		vPos += vDir * 2.f;

		CUtils::Set_State_Matrix(matrix, CUtils::STATE_POSITION, vPos);
		CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		CUtils::Turn_OtherMatrix(matrix, _float4(1.f, 0.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
		_float fRandomscale = CUtils::Make_RandomFloat(0.6f, 1.6f);
		CUtils::Set_Scaled_Matrix(matrix, fRandomscale, fRandomscale, fRandomscale);

		CBreakableRockPartical::BREAKABLEPARTICALDESC desc = {};
		desc.matrix = matrix;
		vDir.y += 0.5f;
		desc.vMoveDir = (_float3)vDir;
		desc.fPower = m_fHitPower * 70.f;
		desc.wstrModelName = TEXT("RockPartical");

		if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_RockPartical"), TEXT("Prototype_GameObject_BreakableRockPartical"), &desc)))
			return;
	}
}

CToppleableBridge* CToppleableBridge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CToppleableBridge* pInstance = new CToppleableBridge(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CToppleableBridge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CToppleableBridge::Clone(void* pArg)
{
	CToppleableBridge* pInstance = new CToppleableBridge(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CToppleableBridge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CToppleableBridge::Free()
{
	__super::Free();

	Safe_Release(m_pAnimBridge);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
