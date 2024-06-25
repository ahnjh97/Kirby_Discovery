#include "stdafx.h"
#include "PartTimeFood.h"

#include "Hitbox.h"

CPartTimeFood::CPartTimeFood(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CItemObject{ pDevice, pContext }
{
}

CPartTimeFood::CPartTimeFood(const CPartTimeFood& rhs)
	: CItemObject{ rhs }
{
}

HRESULT CPartTimeFood::Initialize_Prototype()
{
	//m_eCollisionGroup = ITEM;

	return S_OK;
}

HRESULT CPartTimeFood::Initialize(void* pArg)
{
	FOOD_DESC* pFoodDesc = (FOOD_DESC*)pArg;
	m_pBoneMatrix =  pFoodDesc->pBoneMatrix;
	m_bRender =  pFoodDesc->bRender;
	m_eItem =  (PARTTIME_ITEM)pFoodDesc->uItem;
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	fill(m_arrModelCom.begin(), m_arrModelCom.end(), nullptr);
	if (FAILED(Add_Components()))
		return E_FAIL;

	m_vOriginPos = GET_POS;
	return S_OK;
}

_int CPartTimeFood::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return OBJ_DEAD;

	return OBJ_NOEVENT;
}

void CPartTimeFood::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	}
}

HRESULT CPartTimeFood::Render()
{
	if (!m_bRender) return S_OK;

	_int itemNum = (_int)m_eItem;
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_arrModelCom[itemNum]->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_arrModelCom[itemNum]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[itemNum]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_arrModelCom[itemNum]->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;

		m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool));
		m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool));
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool));

		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_arrModelCom[itemNum]->Render(i);
	}

	return S_OK;
}

HRESULT CPartTimeFood::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_arrModelCom[(_int)m_eItem])))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CPartTimeFood::Render_IMGUI()
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

void CPartTimeFood::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
}

void CPartTimeFood::Update_Position(_float4 vPos)
{
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
}

HRESULT CPartTimeFood::Add_Components()
{
	HRESULT hr = S_OK;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);
	
	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FoodCake"),
		TEXT("Com_Model_Cake"), (CComponent**)&m_arrModelCom[(_int)PARTTIME_ITEM::CAKE]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FoodTomato"),
		TEXT("Com_Model_Tomato"), (CComponent**)&m_arrModelCom[(_int)PARTTIME_ITEM::TOMATO]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_Item_EnergyDrink"),
		TEXT("Com_Model_Drink"), (CComponent**)&m_arrModelCom[(_int)PARTTIME_ITEM::DRINK]);
	CHECK_FAILED(hr);

	hr = __super::Add_Component(TEXT("Prototype_Component_Model_FoodBurger"),
		TEXT("Com_Model_Burger"), (CComponent**)&m_arrModelCom[(_int)PARTTIME_ITEM::BURGER]);
	CHECK_FAILED(hr);

	/*CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = ITEM;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 0.5f, 0.f, 0.7f);*/


	return S_OK;
}

HRESULT CPartTimeFood::Bind_ShaderResources()
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

CPartTimeFood* CPartTimeFood::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPartTimeFood* pInstance = new CPartTimeFood(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPartTimeFood"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPartTimeFood::Clone(void* pArg)
{
	CPartTimeFood* pInstance = new CPartTimeFood(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPartTimeFood"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPartTimeFood::Free()
{
	__super::Free();

	for(auto& item: m_arrModelCom)
		Safe_Release(item);

	Safe_Release(m_pShaderCom);
}

