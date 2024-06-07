#include "stdafx.h"
#include "WasteCan.h"

CWasteCan::CWasteCan(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CRigidObject{ pDevice, pContext }
{
}

CWasteCan::CWasteCan(const CWasteCan& rhs)
	: CRigidObject( rhs )
{
}

HRESULT CWasteCan::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWasteCan::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* pGameObjectDesc = nullptr;

	if (nullptr != pArg)
	{
		pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;
	}

	HRESULT  hr = __super::Initialize(pGameObjectDesc);
	CHECK_FAILED(hr);

	Add_Components();
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(rand() % 20, 15.f, -180.f, 1.f));

	hr = m_pModelCom->CreateDynamicActor(m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION));
	CHECK_FAILED(hr);
	
	return S_OK;
}

_int CWasteCan::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CWasteCan::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	m_pModelCom->Update_ActorTransform(m_pTransformCom);
	Overlap_Hitbox();
}

HRESULT CWasteCan::Render()
{
	HRESULT hr;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
		CHECK_FAILED(hr);
		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);
		
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CWasteCan::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CWasteCan::Render_IMGUI()
{
	if (ImGui::TreeNode("Guizmo"))
	{
		_float4x4 matWorld = m_pTransformCom->Get_WorldFloat4x4();
		m_pGameInstance->EditTransform(matWorld);
		m_pTransformCom->Set_WorldMatrix(matWorld);
		ImGui::Separator(); ImGui::NewLine();
		ImGui::TreePop();
	}
	ImGui::Separator(); ImGui::NewLine();

	__super::Render_IMGUI();
}
#endif

void CWasteCan::Overlap_Hitbox()
{
	// 히트박스 기하학적 모양 정의 (박스 형태)
	PxBoxGeometry hitboxGeometry(PxVec3(5.0f, 5.0f, 5.0f)); // 히트박스 크기

	// 컨트롤러의 위치 가져오기
	_vector pos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	PxVec3 controllerPosition = CUtils::To_PxVec3(pos);

	// 히트박스 위치 설정 (플레이어 앞)
	PxVec3 hitboxPosition = controllerPosition + PxVec3(0.5f, -0.5f, 0.0f);

	// 회전은 기본값으로 설정 (항등 쿼터니언)
	PxQuat rotation = PxQuat(PxIdentity);

	// 히트박스의 변환 생성
	PxTransform hitboxPose(hitboxPosition, rotation);

	// Overlap 테스트 실행
	PxOverlapBuffer hitBuffer; // 충돌 정보를 저장할 버퍼
	PxScene* myScene = m_pGameInstance->Get_Scene();//m_pController->getActor()->getScene();
	_bool status = myScene->overlap(hitboxGeometry, hitboxPose, hitBuffer);

	if (status)
	{
		cout << "Hitbox overlap detected with " << hitBuffer.getNbAnyHits() << " objects." << endl;
		for (PxU32 i = 0; i < hitBuffer.getNbAnyHits(); i++)
		{
			const PxOverlapHit& hit = hitBuffer.getAnyHit(i);
			PxActor* actor = hit.actor;
			if (actor)
			{
				const char* name = actor->getName();
				cout << "Hit object: " << (name ? name : "Unnamed Actor") << endl;
				// 몬스터와의 충돌 처리
			}
		}
	}
	else
	{
		//No hitbox overlap detected.
	}
}

HRESULT CWasteCan::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_WasteCanYellow"),
		TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	/* For.Com_RigidMesh */
	//hr = __super::Add_Component(TEXT("Prototype_Component_RigidMesh"),
	//	TEXT("Com_RigidMesh"), (CComponent**)&m_pModelCom);
	//CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CWasteCan::Bind_ShaderResources()
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

CWasteCan* CWasteCan::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWasteCan* pInstance = new CWasteCan(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CWasteCan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWasteCan::Clone(void* pArg)
{
	CWasteCan* pInstance = new CWasteCan(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CWasteCan"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWasteCan::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

