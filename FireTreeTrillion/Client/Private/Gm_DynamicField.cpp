#include "stdafx.h"
#include "Gm_DynamicField.h"

#include "HitBox.h"
#include "Kirby.h"
#include "Gm_ParkSolarPanelOnce.h"
#include "Gm_ParkSolarPanelCharge.h"
#include "SurprisedBoard.h"

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

	//모델 별 타입 지정
	if (TEXT("Gimmick_PkFunHouseDarkness01") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouseDarkness04") == wstrModelTag
		|| TEXT("Gimmick_PkFunHouseDarkness05") == wstrModelTag)
		m_eDFieldType = DFMOVE_UPDOWN;

	else if (TEXT("Gimmick_PkFunHouseDarkness02") == wstrModelTag || TEXT("Gimmick_PkFunHouseDarkness03") == wstrModelTag)
		m_eDFieldType = DFMOVE_LEFTRIGHT;

	else if (TEXT("Gimmick_PkFunHouse06") == wstrModelTag)
		m_eDFieldType = DFMOVE_FRONTBACK;

	else if (TEXT("Gimmick_PkFunHouse07") == wstrModelTag)
		m_eDFieldType = DFMOVE_NONE;

	m_IsInteraction = FALSE;

	//피직스 추가
	m_pDynamicActor = m_pModelCom->ReturnDynamicActor(m_pTransformCom->Get_WorldFloat4x4());
	m_pDynamicActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	return S_OK;
}

_int CGm_DynamicField::Tick(_float fTimeDelta)
{
	//if (TRUE == m_bDead)
	//	return OBJ_DEAD;

	if (nullptr == m_pSolarPanelOnce && nullptr == m_pSolarPanelCharge && nullptr == m_pSurpriseBoard)
		return OBJ_NOEVENT;

	if (nullptr == m_pSolarPanelOnce)
		return OBJ_NOEVENT;

	CGm_ParkSolarPanelOnce::PANELONCE_STATE eGimmickState = m_pSolarPanelOnce->Get_AnimState();

	if (CGm_ParkSolarPanelOnce::STATE_ONWAIT == eGimmickState)
	{
		_int a = 0;

		switch (m_eDFieldType)
		{
		case DFMOVE_UPDOWN:
		{
			_float3 vCurWorldPos = GET_POS;
			if (100.f <= vCurWorldPos.y) //특정 위치 도착할 경우
			{
				vCurWorldPos.y = 100.f;
				return OBJ_NOEVENT;
			}

			m_pTransformCom->Go_Up(fTimeDelta);

		}
		break;

		case DFMOVE_LEFTRIGHT: //SurprisedBoard 기믹
			break;

		case DFMOVE_FRONTBACK:
			break;
		case DFMOVE_NONE: break;
		}
	}

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
	case DFMOVE_LEFTRIGHT:		ImGui::Text(u8"DFMOVE_LEFTRIGHT"); break;
	case DFMOVE_FRONTBACK:		ImGui::Text(u8"DFMOVE_FRONTBACK"); break;
	case DFMOVE_NONE:	default: ImGui::Text(u8"DFMOVE_NONE"); break;
	}

	if (m_IsInteraction) ImGui::Text(u8"Gm_DynamicFiled :: IsInteraction : TRUE");
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
	if (nullptr != m_pSolarPanelOnce)
		m_pSolarPanelOnce->Collision(eContent, pObject);

	m_IsInteraction = TRUE;
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

	Safe_Release(m_pSolarPanelOnce);
	Safe_Release(m_pSolarPanelCharge);
	Safe_Release(m_pSurpriseBoard);
}
