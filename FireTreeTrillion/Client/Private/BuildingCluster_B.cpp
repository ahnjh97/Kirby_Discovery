#include "stdafx.h"
#include "BuildingCluster_B.h"
#include "Bone.h"

#include "FinaleCut_ControlCenter.h"

CBuildingCluster_B::CBuildingCluster_B(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CBuildingCluster_B::CBuildingCluster_B(const CBuildingCluster_B& rhs)
	:CGameObject{ rhs }
{
}

HRESULT CBuildingCluster_B::Initialize_Prototype()
{
	ZeroMemory(m_pModelCom, sizeof(CModel*) * Z);
	return S_OK;
}

HRESULT CBuildingCluster_B::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* Desc = nullptr;

	if (pArg != nullptr)
		Desc = (GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(Desc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_bMotionBlur = false;
	m_bRimLight = true;
	//m_fRimWidth = .1f;

	m_bStencil = false;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(2550.f, 255.f, -136.f, 1.f));
	_float4 NewLook = _float4(1.f, 0.f, 0.f, 0.f);
	_float4 NewUp = _float4(0.f, 1.f, 0.f, 0.f);
	_float4 NewRight = XMVector3Cross(NewUp, NewLook);

	m_pTransformCom->Set_State(CTransform::STATE_LOOK, NewLook);
	m_pTransformCom->Set_State(CTransform::STATE_UP, NewUp);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, NewRight);


	// 초기화 해준다.
	for (auto& BuildingMatrix : m_BuildingMatrix)
	{
		XMStoreFloat4x4(&BuildingMatrix, XMMatrixIdentity());
	}

	return S_OK;
}

_int CBuildingCluster_B::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	m_fAccTime = m_pGameInstance->Get_SecondTimer();

	CFinaleCut_ControlCenter* pCenter =
		static_cast<CFinaleCut_ControlCenter*>(m_pGameInstance->Get_GameObject(LEVEL_FINALE, TEXT("Layer_FinaleCut_ControlCenter")));
	if (nullptr == pCenter)
		return OBJ_NOEVENT;

	_int iCutIndex = pCenter->Get_CutScene();

	if (iCutIndex == 12)
	{
		m_bRender = true;
		m_eCurCut = CUT12;
	}
	else if (iCutIndex == 13)
	{
		m_bRender = true;
	}
	else if (iCutIndex == 15)
	{
		m_bRender = true;
	}
	else if (iCutIndex == 6)
	{
		m_bRender = true;
		m_eCurCut = CUT6;
	}
	else if (iCutIndex == 7)
	{
		m_bRender = true;
		m_eCurCut = CUT7;
	}
	else if (iCutIndex == 8)
	{
		m_bRender = true;
	}
	else
	{
		m_bRender = false;
	}
	Set_Animation();

	return OBJ_NOEVENT;
}

void CBuildingCluster_B::Late_Tick(_float fTimeDelta)
{
	if (m_bRender == false)
		return;


	m_pBuildingCluster->Play_Animation(m_fAccTime);
	Update_BuildingMatrix();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

void CBuildingCluster_B::Update_BuildingMatrix()
{
	string strBoneName[Z] = {
		"A01L", "A02L", "B01L", "C01L", "C02L", "C03L", "C04L", "D01L", "D02L"
	};

	CBone* pBone = { nullptr };
	_float4x4 pBoneLocalMatrix;
	_float4x4 pBoneWorldMatrix;

	for (_int i = A1; i < Z; ++i)
	{
		pBone = m_pBuildingCluster->Get_BonePtr(strBoneName[i].c_str());
		pBoneLocalMatrix = *pBone->Get_CombinedTransformationMatrix();
		pBoneWorldMatrix = pBoneLocalMatrix * m_pTransformCom->Get_WorldFloat4x4();
		m_BuildingMatrix[i] = pBoneWorldMatrix;

		if (i == C3)
		{
			m_BuildingMatrix[i]._42 -= 30.f;
		}
		else if (i == C1)
		{
			m_BuildingMatrix[i]._42 -= 26.5f;
			m_BuildingMatrix[i]._41 += 20.f;
		}
		else if (i == A2)
		{
			m_BuildingMatrix[i]._42 -= 30.f;
		}
		else if (i == C4)
		{
			m_BuildingMatrix[i]._41 -= 5.f;
			m_BuildingMatrix[i]._42 -= 6.f;
		}
		else if (i == C2)
		{
			m_BuildingMatrix[i]._42 -= 6.f;
		}
		else if (i == A1)
		{
			m_BuildingMatrix[i]._41 += 6.f;
			m_BuildingMatrix[i]._42 -= 33.f;
			m_BuildingMatrix[i]._43 += 30.f;
		}
		else if (i == D1)
		{
			m_BuildingMatrix[i]._42 -= 29.f;
		}
		else if (i == B1)
		{
			m_BuildingMatrix[i]._42 -= 80.f;
		}

	}
}

HRESULT CBuildingCluster_B::Render()
{
	for (_int i = A1; i < Z; ++i)
	{
		Rendering_Building((BUILDING)i);
	}
	return S_OK;
}

HRESULT CBuildingCluster_B::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderModelCom);
	CHECK_FAILED(hr);


	// 메인 뼈대. 이것이 움직임으로서 다른 빌딩들이 움직일 수 있다.
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_BuildingCluster02"),
		TEXT("Com_Model"), (CComponent**)&m_pBuildingCluster);
	CHECK_FAILED(hr);


	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingA"),
		TEXT("Com_ModelA1"), (CComponent**)&m_pModelCom[A1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelA2"), (CComponent**)&m_pModelCom[A2]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelB1"), (CComponent**)&m_pModelCom[B1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelC1"), (CComponent**)&m_pModelCom[C1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelC2"), (CComponent**)&m_pModelCom[C2]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelC3"), (CComponent**)&m_pModelCom[C3]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelC4"), (CComponent**)&m_pModelCom[C4]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelD1"), (CComponent**)&m_pModelCom[D1]);
	CHECK_FAILED(hr);
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_MovableBuildingC"),
		TEXT("Com_ModelD2"), (CComponent**)&m_pModelCom[D2]);
	CHECK_FAILED(hr);

	return S_OK;
}

void CBuildingCluster_B::Set_Animation()
{
	if (m_eCurCut == m_ePreCut)
		return;

	switch (m_eCurCut)
	{
	case CUT6:
		m_pBuildingCluster->Set_Animation(CUT6, 70.f, false, false);
		break;
	case CUT7:
		m_pBuildingCluster->Set_Animation(CUT7, 50.f, false, false);
		break;
	case CUT12:
		m_pBuildingCluster->Set_Animation(CUT12, 50.f, false, false);
		break;
	default:
		break;
	}

	m_ePreCut = m_eCurCut;
}


HRESULT CBuildingCluster_B::Rendering_Building(BUILDING eType)
{
	if (nullptr == m_pShaderModelCom)
		return E_FAIL;
	if (FAILED(m_pShaderModelCom->Bind_Matrix("g_WorldMatrix", &m_BuildingMatrix[eType])))
		return E_FAIL;
	if (FAILED(m_pShaderModelCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderModelCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom[eType]->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (i == 0)
			continue;

		if (FAILED(m_pModelCom[eType]->Bind_ShaderResource(m_pShaderModelCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom[eType]->Bind_ShaderResource(m_pShaderModelCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom[eType]->Bind_ShaderResource(m_pShaderModelCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pShaderModelCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderModelCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderModelCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderModelCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderModelCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;
		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderModelCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;
		m_pModelCom[eType]->Render(i);
	}
	return S_OK;
}


CBuildingCluster_B* CBuildingCluster_B::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBuildingCluster_B* pInstance = new CBuildingCluster_B(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBuildingCluster_B"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBuildingCluster_B::Clone(void* pArg)
{
	CBuildingCluster_B* pInstance = new CBuildingCluster_B(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBuildingCluster_B"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBuildingCluster_B::Free()
{
	__super::Free();

	for (auto& pModel : m_pModelCom)
		Safe_Release(pModel);

	Safe_Release(m_pShaderModelCom);
	Safe_Release(m_pBuildingCluster);
}
