#include "stdafx.h"
#include "AnimBridge.h"
#include "Bone.h"

#include "Camera_Main.h"

// 최대 회전 시간과 최대 회전 각도
#define M_PI 3.14159265358979323846f
const _float MAX_TIME = 3.3f;
const _float MAX_ANGLE = M_PI / 2.f;  // 90도 (라디안)

_float CalculateAngle(_float fTime) {

	// 0.f ~ 4.5f 의 타임값이 들어간다.
	// 0.f ~ 80.f 의 각도값이 출력되어야 한다.
	_float fAccTime = fTime / 3.8f;  // 0 ~ 1

	return pow(fAccTime, 5.f) * 80.f;
}


CAnimBridge::CAnimBridge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CAnimBridge::CAnimBridge(const CAnimBridge& rhs)
	: CPhysXObject{ rhs }
{
}

void CAnimBridge::OnCollision()
{
	m_bCollision = true;
	m_bSecondAnim = true;

	return;
}

HRESULT CAnimBridge::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimBridge::Initialize(void* pArg)
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
	m_bRimLight = false;
	m_wstrModelName = Desc->wstrModelName;

	if (TEXT("BoardC_Anim") == m_wstrModelName) {
		m_pModelCom->Set_Animation(1, 60.f, true, false);
		m_pBone = m_pModelCom->Get_BonePtr("joint2");
		if (nullptr == m_pBone)
			return E_FAIL;
		Safe_AddRef(m_pBone);
		m_pEditMatrix = m_pBone->Get_EditMatrixPtr();
	}
		
	return S_OK;
}

_int CAnimBridge::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	m_fTime += fTimeDelta;

	if (m_bCollision)
		m_fHitTime += fTimeDelta;

	if (TEXT("BoardA_Anim") == m_wstrModelName || TEXT("BoardB_Anim") == m_wstrModelName)
	{
		if (m_fHitTime > 0.f && m_fHitTime < 0.75f)
			m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), m_pGameInstance->Get_SecondTimer());

		if (m_fHitTime > 0.72f && true == m_bSecondAnim)
		{
			m_pModelCom->Set_Animation(1, 60.f, false, false);
			m_bSecondAnim = false;
			m_pGameInstance->PlaySound_Free(L"BridgeLand.wav", 0.5f);
		}
	}

	if (TEXT("BoardC_Anim") == m_wstrModelName)
	{
		if (m_pGameInstance->Get_DIKeyState(DIK_N, KEY_DOWN))
			m_fAngle += 1.f;
		else if (m_pGameInstance->Get_DIKeyState(DIK_M, KEY_DOWN))
			m_fAngle -= 1.f;

		if (3.6f < m_fHitTime && false == m_bSound)
		{
			m_bSound = true;
			m_pGameInstance->PlaySound_Free(L"BridgeToppleStart.wav", 0.5f);
		}

		if(m_bBoundTrigger == true && m_fHitTime >= 2.f)
		{
			_float4x4 RotationMatrix = _float4x4::Identity;
			m_fAngle = CalculateAngle(m_fHitTime - 2.f);
			if (m_fHitTime > 5.75f)
				m_fAngle = 80.f;

			CUtils::Turn_OtherMatrix(RotationMatrix, _float4(1, 0, 0, 0), -1.f, m_fAngle);
			*m_pEditMatrix = RotationMatrix;

			if (m_fHitTime > 5.8f)
			{
				m_bBoundTrigger = false;
				m_bBound = true;
				CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
				pCamera->Make_Shake(2.f);
			}
		}
		else if (m_bBound == true)
		{
			_float4x4 RotationMatrix = _float4x4::Identity;
			m_fBoundTime += fTimeDelta;
			m_fAngle = 80.f - (1.5f - (m_fBoundTime * 9.8f));

			CUtils::Turn_OtherMatrix(RotationMatrix, _float4(1, 0, 0, 0), -1.f, m_fAngle);
			*m_pEditMatrix = RotationMatrix;

			if (m_fAngle > 80.f)
			{
				m_bBound = false;
				m_fAngle = 80.f;
			}
		}
		else if (m_bBound == false && m_bBoundTrigger  == false && true == m_bSecondAnim)
		{
			*m_pEditMatrix = _float4x4::Identity;
			m_pModelCom->Set_Animation(0, 60.f, true, false);
			m_bSecondAnim = false;
			m_pGameInstance->PlaySound_Free(L"BridgeToppleEnd.wav", 0.55f); // 타이밍 잡아두기
		}
	}

	return OBJ_NOEVENT;
}

void CAnimBridge::Late_Tick(_float fTimeDelta)
{
	if (true == m_bCollision || TEXT("BoardC_Anim") == m_wstrModelName)
		m_pModelCom->Play_Animation(fTimeDelta);

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 150.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CAnimBridge::Render()
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
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Begin(ANIMMODEL_LINEAR_NORMAL_O)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}
	
	return S_OK;
}

HRESULT CAnimBridge::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CAnimBridge::Render_IMGUI()
{
	ImGui::Begin("debug");
	__super::Render_IMGUI();

	ImGui::Text("m_fAngle : %.2f", (_float)m_fAngle);
	ImGui::End();
}
#endif

HRESULT CAnimBridge::Add_Components(const wstring& _wstrModelName)
{
	HRESULT hr;

	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelName;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CAnimBridge::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CAnimBridge* CAnimBridge::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimBridge* pInstance = new CAnimBridge(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CAnimBridge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimBridge::Clone(void* pArg)
{
	CAnimBridge* pInstance = new CAnimBridge(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CAnimBridge"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimBridge::Free()
{
	__super::Free();

	Safe_Release(m_pBone);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
