#include "stdafx.h"
#include "StarBlock.h"
#include "StarBlockPiece.h"

CStarBlock::CStarBlock(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CMapObject{ pDevice, pContext }
{
}

CStarBlock::CStarBlock(const CStarBlock& rhs)
	: CMapObject( rhs )
{
}

HRESULT CStarBlock::Initialize_Prototype()
{
	m_eCollisionGroup = OBJECT;
	return S_OK;
}

HRESULT CStarBlock::Initialize(void* pArg)
{
	STARBLOCK_DESC tStarBlcokDesc{};
	if (nullptr != pArg) {
		tStarBlcokDesc = *(STARBLOCK_DESC*)pArg;
		m_eSize = tStarBlcokDesc.eSize;
	}

	HRESULT hr;
	hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(-3.f, 7.f, -188.f, 1.f));
	Add_Components();

	return S_OK;
}

_int CStarBlock::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_bDead)
		return OBJ_DEAD;

	m_pControllerCom->FreeFall(m_pTransformCom, fTimeDelta, 0.5f);
	return OBJ_NOEVENT;
}

void CStarBlock::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CStarBlock::Render()
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
		hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(MODEL_NORMAL_O);
		CHECK_FAILED(hr);
		
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CStarBlock::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CStarBlock::Render_IMGUI()
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

void CStarBlock::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	m_iHP -= 5;
	if (m_iHP <= 0)
	{
		// 여기서 StarBlockPiece 뿌리기
		HRESULT hr = S_OK;
		CStarBlockPiece::PIECE_DESC desc{};
		_float4 vInitialpos = GET_POS;
		desc.vInitialPos = _float4(vInitialpos.x + 1.f, vInitialpos.y + 1.5f, vInitialpos.z + 1.f, 1.f);
		//desc.vDir =k
		hr = m_pGameInstance->Add_Clone(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_MapObject"), TEXT("Prototype_GameObject_StarBlockPiece"), &desc);
		CHECK_FAILED(hr);

		m_bDead = true;
	}
}

HRESULT CStarBlock::Add_Components()
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	switch (m_eSize)
	{
	case SMALL:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockS"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	case MEDIUM:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockM"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	case LARGE:
	{
		hr = __super::Add_Component(TEXT("Prototype_Component_Model_StarBlockL"),
			TEXT("Com_Model"), (CComponent**)&m_pModelCom);
		CHECK_FAILED(hr);
	}
	break;
	}

	/* For.Com_CharacterController */
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = GET_POS;
	desc.fOffset = 1.1f;
	desc.uCollisionType = m_eCollisionGroup;
	desc.eType = CCharacterController::BOX;
	desc.tBoxShape.fHalfForwardExtent = 1.f;
	desc.tBoxShape.fHalfHeight = 1.f;
	desc.tBoxShape.fHalfSideExtent = 1.f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);
	CHECK_FAILED(hr);
	//m_pControllerCom->Set_Object(this);

	return S_OK;
}

HRESULT CStarBlock::Bind_ShaderResources()
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

CStarBlock* CStarBlock::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStarBlock* pInstance = new CStarBlock(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CStarBlock::Clone(void* pArg)
{
	CStarBlock* pInstance = new CStarBlock(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CStarBlock"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CStarBlock::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pControllerCom);
}

