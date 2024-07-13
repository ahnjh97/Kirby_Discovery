#include "stdafx.h"
#include "Gm_DynamicField.h"

#include "HitBox.h"
#include "Kirby.h"
//#include "BreakableRockParticle.h"

CGm_DynamicField::CGm_DynamicField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CGm_DynamicField::CGm_DynamicField(const CGm_DynamicField& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CGm_DynamicField::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGm_DynamicField::Initialize(void* pArg)
{
	GAMEOBJECT_DESC DynamicFieldDesc{};

	if (pArg != nullptr)
		DynamicFieldDesc = *(GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(&DynamicFieldDesc)))
		return E_FAIL;
	
	wstring wstrModelTag = DynamicFieldDesc.wstrModelName;
	if (FAILED(Add_Components(wstrModelTag))) //모델 별 컴포넌트 생성
		return E_FAIL;

	if (FAILED(SetUp_ShaderInfo(wstrModelTag))) //셰이더 정보 로드 및 세팅
		return E_FAIL;

	//m_eCurState = STATE_OFFWAIT;
	//m_pModelCom->Set_Animation(STATE_OFFWAIT, 30.f, FALSE, FALSE);

	m_IsInteraction = FALSE;

	//피직스 추가
	m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	//상호작용할 태양전지판 생성
	CGameObject::GAMEOBJECT_DESC tDesc{};
	_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();

	_float3 vWorldPos = GET_POS;
	_float3 vOffset = { 5.542f, 38.970f, -12.082f }; //5.542, 38.970, -12.082
	matWorld.Translation(/*vWorldPos + */vOffset);

	tDesc.matWorld = matWorld;
	m_pSolarPanel = dynamic_cast<CGm_ParkSolarPanelOnce*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Gm_ParkSolarPanelOnce"), &tDesc));
	if (nullptr == m_pSolarPanel)
		return E_FAIL;
	Safe_AddRef(m_pSolarPanel);

	//림라이트 OFF
	//m_bRimLight = FALSE;

	return S_OK;
}

_int CGm_DynamicField::Tick(_float fTimeDelta)
{
	//if (TRUE == m_bDead)
	//	return OBJ_DEAD;
	if (nullptr != m_pSolarPanel)
		m_pSolarPanel->Tick(fTimeDelta);
	/*
	switch (m_eCurState)
	{
	case STATE_OFFWAIT: break;//충전 전 대기
	case STATE_CHARGE: //충전 중
		if (TRUE == m_pModelCom->IsFinished()) //충전 중 애님 종료 시 충전 완료 상태 변경
		{
			m_pModelCom->Set_Animation(STATE_ONWAITSTART, 30.f, FALSE, TRUE);
			m_eCurState = STATE_ONWAITSTART;
		}
		break;

	case STATE_ONWAITSTART: //충전 시작
		if (TRUE == m_pModelCom->IsFinished())
		{
			m_pModelCom->Set_Animation(STATE_ONWAIT, 30.f, FALSE, TRUE);
			m_eCurState = STATE_ONWAIT;
		}
		break;
		
	case STATE_ONWAIT: break; //충전 완료
	case STATE_NONE:	default:	break;
	}
	*/

	return OBJ_NOEVENT;
}

void CGm_DynamicField::Late_Tick(_float fTimeDelta)
{
	//m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());
	if (nullptr != m_pSolarPanel)
		m_pSolarPanel->Late_Tick(fTimeDelta);

#pragma region FRUSTUM_CULLING

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

#pragma endregion

	//애니메이션 재생종료 시 Set_Dead
	//if (TRUE == m_pModelCom->IsFinished())
	//	Set_Dead();
}

HRESULT CGm_DynamicField::Render()
{
	if (nullptr != m_pSolarPanel)
		m_pSolarPanel->Render();


	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	HRESULT hr{};

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", i, TextureType_EMISSIVE)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fSamplingFactor", &m_vecSamplingFactors[i], sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(m_vecPassIndices[i]))) //셰이더 PASS 정보
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CGm_DynamicField::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CGm_DynamicField::Render_IMGUI()
{
	if (nullptr != m_pSolarPanel)
		m_pSolarPanel->Render_IMGUI();

	/*
	switch (m_eCurState)
	{
	case STATE_OFFWAIT:			ImGui::Text(u8"STATE_OFFWAIT"); break;
	case STATE_CHARGE:			ImGui::Text(u8"STATE_CHARGE"); break;
	case STATE_ONWAITSTART:	ImGui::Text(u8"STATE_ONWAITSTART"); break;
	case STATE_ONWAIT:		ImGui::Text(u8"STATE_ONWAIT"); break;
	case STATE_NONE:	default: ImGui::Text(u8"STATE_NONE"); break;
	}
	
	if (m_IsInteraction) ImGui::Text(u8"IsInteraction : TRUE");
	else ImGui::Text(u8"IsInteraction : FALSE");
	*/

#pragma region IMGUI GIZMO

	if (ImGui::TreeNode("Gizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}

	ImGui::Separator(); ImGui::NewLine();
	__super::Render_IMGUI();

#pragma endregion
}
#endif

void CGm_DynamicField::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (nullptr != m_pSolarPanel)
		m_pSolarPanel->Collision(eContent, pObject);


	m_IsInteraction = TRUE;
	
	//충전 대기 상태에서 키꾹 > 충전 시작
	/*
	if (m_pGameInstance->Get_DIKeyState(DIK_A, KEY_DOWN) && STATE_OFFWAIT == m_eCurState)
	{
		m_pModelCom->Set_Animation(STATE_CHARGE, 30.f, FALSE, TRUE);
		m_eCurState = STATE_CHARGE;
	}
	*/

#pragma region KEY_FRAME CUSTOM 1 SCOOP

	/*
	else //키꾹 해제 시 충전 해제 (현재 사용x)
	{
		if (STATE_CHARGE == m_pModelCom->Get_CurAnimIndex())
		{
			_float fDuration = m_pModelCom->Get_Duration(); //전체 재생길이에서 현재 재생시점을 체크
			_float fTrackPos = m_pModelCom->Get_Trackposition();
			_float fSubTrackPos = fDuration - fTrackPos; //감산하여 충전 해제 애니메이션 자연스럽게 보정

			m_pModelCom->Set_Animation(STATE_DECREASES, 60.f, FALSE, FALSE);
			m_pModelCom->Set_TrackPosition(fSubTrackPos);
			m_eCurState = STATE_DECREASES;
		}
	}
	*/

#pragma endregion

}

HRESULT CGm_DynamicField::Add_Components(const wstring& _wstrModelTag)
{
	HRESULT hr;
	
	//맵 셰이더
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel_Map"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);

	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + _wstrModelTag;
	hr = __super::Add_Component((wstrModelTag), TEXT("Com_Model_NonAnim"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CGm_DynamicField::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CGm_DynamicField::SetUp_ShaderInfo(const wstring& _wstrModelTag)
{
	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	m_vecPassIndices.resize(iNumMeshes);
	m_vecSamplingFactors.resize(iNumMeshes);
	fill(m_vecSamplingFactors.begin(), m_vecSamplingFactors.end(), 1.f);

	string strFilePath = "../../../objects_txt/" + CUtils::WstrToStr(_wstrModelTag) + "_ShaderInfo.txt";

	fstream fileStream(strFilePath, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		wstring wstrError = TEXT("Failed to Open: ") + _wstrModelTag + L"_ShaderInfo.txt";
		return E_FAIL;
	}

	_uint iPassIndex{};
	_float fSamplingFactor{};
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		fileStream.read(reinterpret_cast<char*>(&iPassIndex), sizeof(iPassIndex));
		fileStream.read(reinterpret_cast<char*>(&fSamplingFactor), sizeof(fSamplingFactor));
		if (fileStream.eof()) {
			fileStream.close();
			return E_FAIL;
		}

		m_vecPassIndices[i] = iPassIndex;
		m_vecSamplingFactors[i] = fSamplingFactor;
	}

	fileStream.close();
	return S_OK;
}

CGm_DynamicField* CGm_DynamicField::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGm_DynamicField* pInstance = new CGm_DynamicField(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGm_DynamicField"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGm_DynamicField::Clone(void* pArg)
{
	CGm_DynamicField* pInstance = new CGm_DynamicField(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CGm_DynamicField"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGm_DynamicField::Free()
{
	__super::Free();

	m_pGameInstance->ReleaseActor(m_pDynamicActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pSolarPanel);
}
