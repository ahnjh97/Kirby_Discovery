#include "stdafx.h"
#include "Gm_DynamicField.h"

#include "HitBox.h"
#include "Kirby.h"
#include "Gm_ParkSolarPanelOnce.h"
#include "Gm_ParkSolarPanelCharge.h"
#include "SurprisedBoard.h"
#include "Kirby.h"

void CGm_DynamicField::Set_SolarPanelOnce(CGm_ParkSolarPanelOnce* _pSolarPanel)
{
	m_pSolarPanelOnce = _pSolarPanel; 
	Safe_AddRef(m_pSolarPanelOnce);
}

void CGm_DynamicField::Set_SolarPanelCharge(CGm_ParkSolarPanelCharge* _pSolarPanel)
{
	m_pSolarPanelCharge = _pSolarPanel;  
	Safe_AddRef(m_pSolarPanelCharge);
}

void CGm_DynamicField::Set_SurpriseBoard(CSurprisedBoard* _pSurpriseBoard)
{
	m_pSurpriseBoard = _pSurpriseBoard; 
	Safe_AddRef(m_pSurpriseBoard);
}

void CGm_DynamicField::RegisterToActorToKirby()
{
	 CKirby* pKirby = dynamic_cast<CKirby*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player")));
	 if (nullptr == pKirby || nullptr == m_pDynamicActor)
		 return;

	 pKirby->RegisterActorToPlayer_ForDynamicField(m_pDynamicActor, this);
}

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

	DynamicFieldDesc.fSpeedPerSec = 10.f;
	DynamicFieldDesc.fRotationPerSec = 90.f;
	m_iGimmickIndex = DynamicFieldDesc.iShaderVars;
	DynamicFieldDesc.iShaderVars = 6;

	if (FAILED(__super::Initialize(&DynamicFieldDesc)))
		return E_FAIL;

	wstring wstrModelTag = DynamicFieldDesc.wstrModelName;
	if (FAILED(Add_Components(wstrModelTag))) //모델 별 컴포넌트 생성
		return E_FAIL;

	if (FAILED(SetUp_ShaderInfo(wstrModelTag))) //셰이더 정보 로드 및 세팅
		return E_FAIL;

#pragma region SET DYNAMICFIELD_TYPE

	//모델 별 타입 지정
	if (TEXT("Gimmick_PkFunHouseDarkness01") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouseDarkness04") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouseDarkness05") == wstrModelTag)
	{
		m_eDFieldType = DFMOVE_UPDOWN;
		m_eGimmickType = GIMMICK_SPONCE;
	}

	if (TEXT("Gimmick_PkFunHouseDarkness02") == wstrModelTag)
	{
		m_eDFieldType = DFMOVE_RIGHT;
		m_eGimmickType = GIMMICK_SURPRISE;
	}

	if (TEXT("Gimmick_PkFunHouseDarkness03") == wstrModelTag)
	{
		m_eDFieldType = DFMOVE_LEFT;
		m_eGimmickType = GIMMICK_SURPRISE;
	}

	//if (TEXT("Gimmick_PkFunHouse06") == wstrModelTag)
	if (TEXT("Gimmick_PkFunHouse06A") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouse06B") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouse06C") == wstrModelTag)
	{
		m_eDFieldType = DFMOVE_FRONTBACK;
		m_eGimmickType = GIMMICK_SPCHARGE;
	}

	if (TEXT("Gimmick_PkFunHouse07") == wstrModelTag)
	{
		m_eDFieldType = DFMOVE_NONE;
		m_pStaticActor = m_pModelCom->ReturnStaticActor(m_pTransformCom->Get_WorldFloat4x4());
	}

#pragma endregion
	
	else
	{
		m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
		m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	}

	m_bIsInteraction = FALSE;

	return S_OK;
}

_int CGm_DynamicField::Tick(_float fTimeDelta)
{
	if (TRUE == m_bDead)
		return OBJ_DEAD;

	switch (m_eGimmickType)
	{
	case GIMMICK_SPONCE:
		if (-1 == m_eSPOnceState)
			return OBJ_NOEVENT;

		m_eSPOnceState = m_pSolarPanelOnce->Get_CurState();
		break;

	case GIMMICK_SPCHARGE:
		if (-1 == m_eSPChargeState)
			return OBJ_NOEVENT;

		m_eSPChargeState = m_pSolarPanelCharge->Get_CurState();
		break;

	case GIMMICK_SURPRISE:
		break;

	case GIMMICK_NONE:
	default:
		break;
	}

	Movement_Field(fTimeDelta);

	if (nullptr != m_pDynamicActor) // 트랜스폼 월드 행렬에 맞춰서 다이나믹 액터도 같이 움직이도록 
		m_pDynamicActor->setGlobalPose(CUtils::TransformToPxTransform(m_pTransformCom));

	return OBJ_NOEVENT;
}

void CGm_DynamicField::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	//애니메이션 재생종료 시 Set_Dead
	//if (TRUE == m_pModelCom->IsFinished())
	//	Set_Dead();
}

HRESULT CGm_DynamicField::Render()
{
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
	switch (m_eDFieldType)
	{
	case DFMOVE_UPDOWN:			ImGui::Text(u8"DFMOVE_UPDOWN"); break;
	case DFMOVE_LEFT:		ImGui::Text(u8"DFMOVE_LEFT"); break;
	case DFMOVE_RIGHT:		ImGui::Text(u8"DFMOVE_RIGHT"); break;
	case DFMOVE_FRONTBACK:		ImGui::Text(u8"DFMOVE_FRONTBACK"); break;
	case DFMOVE_NONE:	default: ImGui::Text(u8"DFMOVE_NONE"); break;
	}

	if (m_bIsInteraction) ImGui::Text(u8"Gm_DynamicFiled :: IsInteraction : TRUE");
	else ImGui::Text(u8"Gm_DynamicFiled :: IsInteraction : FALSE");

	string strGimmickIndex = "Index :" + to_string(m_iGimmickIndex);
	ImGui::Text(strGimmickIndex.c_str());

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

void CGm_DynamicField::Apply_Quake(_float _fTimeDelta, _float _fQuakeDuration, _float _fShakeIntensity)
{
	if (m_bIsQuake)
	{
		m_fQuakeTime += _fTimeDelta;
		if (m_fQuakeTime <= _fQuakeDuration) //지진 지속시간 체크
		{
			_float fShakeValue = _fShakeIntensity * std::sin(m_fQuakeTime * 70.0f); //지진 강도(진폭) 설정
			if (fShakeValue > 0)
				m_pTransformCom->Go_Up(fShakeValue * _fTimeDelta);

			else
				m_pTransformCom->Go_Down(-fShakeValue * _fTimeDelta);
		}
		else
			m_bIsQuake = FALSE;
	}
	else
	{
		m_fQuakeTime = 0.f;
	}
}

_int CGm_DynamicField::Movement_Field(_float _fTimeDelta)
{
	_float3 vCurPos;
	switch (m_eDFieldType)
	{
	case DFMOVE_UPDOWN:
		vCurPos = GET_POS;

		if (CGm_ParkSolarPanelOnce::STATE_ONWAIT == m_eSPOnceState) //충전 완료
		{
			if (56.963f <= vCurPos.y) //56.963 > 36.963 
			{
				vCurPos.y = 56.963f; //위치 보정
				m_bIsQuake = TRUE;

				Apply_Quake(_fTimeDelta, 1.f, 0.1f);

			}

			else
				m_pTransformCom->Go_Up(_fTimeDelta * 0.3f);
		}
		break;

		//깜놀보드 충돌거리 조건에 따라 DFMOVE_LEFT, DFMOVE_RIGHT를 체크
	case DFMOVE_LEFT:
		vCurPos = GET_POS;

		if (m_bIsInteraction) //RayCast 상호작용 검사
		{
			if (19.460f >= vCurPos.x) //24.460 > 14.460
			{
				vCurPos.x = 19.460f;
				m_bIsQuake = TRUE;

				if (m_bIsQuake)
					Apply_Quake(_fTimeDelta, 1.f, 0.1f);
			}

			else
				m_pTransformCom->Go_Left(_fTimeDelta * 0.5f);
		}


		break;
	case DFMOVE_RIGHT:
		vCurPos = GET_POS;

		if (m_bIsInteraction) //RayCast 상호작용 검사
		{
			Apply_Quake(_fTimeDelta, 1.f, 0.1f);
			if (29.851f <= vCurPos.x) //24.851 > 34.851
			{
				vCurPos.x = 29.851f;
				m_bIsQuake = TRUE;
			}

			//원작의 경우는 0.25f 속도인데 좀 더 빠르게 변경
			else
				m_pTransformCom->Go_Right(_fTimeDelta * 0.5f); //깜놀보드 애님 상태가 종료될 경우, 해당 움직임을 수행
		}
		//특정 애님 상태일 경우, 필드도 비활성 움직임 처리
		if (m_bIsReturnMove)
		{
			if (24.851f >= vCurPos.x)
			{
				vCurPos.x = 24.851f;
				m_bIsQuake = TRUE;

				if (m_bIsQuake)
					Apply_Quake(_fTimeDelta, 1.f, 0.1f);
			}

			else
				m_pTransformCom->Go_Left(_fTimeDelta * 0.75f);
		}
		break;

	case DFMOVE_FRONTBACK:
		vCurPos = GET_POS;

		//충전 완료 시에 빠르게 활성화
		if (CGm_ParkSolarPanelCharge::STATE_CHARGEDWAIT == m_eSPChargeState)
		{
			if (-77.289f >= vCurPos.z) //-67.289 > -77.289
			{
				vCurPos.z = -77.289f;
				m_bIsQuake = TRUE;

				if (m_bIsQuake)
					Apply_Quake(_fTimeDelta, 1.f, 0.1f);
			}

			else
				m_pTransformCom->Go_Backward(_fTimeDelta * 2.f);
		}
		//충전 해제 상태동안은 천천히 이동 후 비활성화 처리
		if (CGm_ParkSolarPanelCharge::STATE_DECREASES == m_eSPChargeState) //충전 해제
		{
			if (-67.289f <= vCurPos.z)
			{
				vCurPos.z = -67.289f;
				return OBJ_NOEVENT;
			}

			else
				m_pTransformCom->Go_Straight(_fTimeDelta * 0.2f); //난이도 너프함..
		}
		break;

	case DFMOVE_NONE:
	default:
		break;
	}

	return OBJ_NOEVENT;
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
	m_pGameInstance->ReleaseActor(m_pStaticActor);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pSolarPanelOnce);
	Safe_Release(m_pSolarPanelCharge);
	Safe_Release(m_pSurpriseBoard);
}
