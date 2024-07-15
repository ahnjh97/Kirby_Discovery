#include "stdafx.h"
#include "RoomGlass.h"
#include "EventCenter.h"
#include "Bone.h"

CRoomGlass::CRoomGlass(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CRoomGlass::CRoomGlass(const CRoomGlass& rhs)
	: CGameObject{ rhs }
{
}

void CRoomGlass::Activate(CGameObject* pObj)
{
	m_bHide = false;
	m_pModelCom->Set_Animation(0, 40.f, false, false);
}

HRESULT CRoomGlass::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRoomGlass::Initialize(void* pArg)
{
	GAMEOBJECT_DESC* desc = {};

	if (pArg != nullptr)
		desc = (GAMEOBJECT_DESC*)pArg;


	HRESULT hr;
	hr = __super::Initialize(desc);
	CHECK_FAILED(hr);

	if (desc != nullptr)
		Add_Components(desc->wstrModelName);

	m_bMotionBlur = false;
	m_bRimLight = false;
	m_bStencil = true;

	_uint iWrongMesh = m_pModelCom->Find_MeshIndex(string("MainGlass01M__GlassFrontC"));
	unordered_set<_uint> tempSet = { iWrongMesh };
	m_pModelCom->RemoveBlendMeshes(tempSet);

	//m_pModelCom->Set_Animation(0, 40.f, true, false);

	_float fScale = 1.45f;
	m_pTransformCom->Set_Scaled(fScale, fScale, 1);

	function<void(CGameObject*)> func = bind(&CRoomGlass::Activate, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_GLASSBREAK, this, func);

	m_bHide = true;
	
	m_pBone = m_pModelCom->Get_BonePtr("BreakGlass02MAllL");
	Safe_AddRef(m_pBone);

	return S_OK;
}

_int CRoomGlass::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (true == m_pModelCom->IsFinished())
		m_pModelCom->Set_Animation(1, 1.f, false);

	return OBJ_NOEVENT;
}

void CRoomGlass::Late_Tick(_float fTimeDelta)
{
	if (false == m_bHide) 
	{
		Compute_BoneViewZ(m_pBone);
		m_pModelCom->Play_Animation(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	}
}

HRESULT CRoomGlass::Render()
{
	HRESULT hr{};
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
		hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
		CHECK_FAILED(hr);

		hr = m_pShaderCom->Begin(ANIMMODEL_ALPHABLEND);
		CHECK_FAILED(hr);
		hr = m_pModelCom->Render(i);
		CHECK_FAILED(hr);
	}

	return S_OK;
}

HRESULT CRoomGlass::Add_Components(wstring& wstrModelName)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrModeltag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModeltag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT CRoomGlass::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &m_fWhiteColorDiffuse, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CRoomGlass* CRoomGlass::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRoomGlass* pInstance = new CRoomGlass(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRoomGlass"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CRoomGlass::Clone(void* pArg)
{
	CRoomGlass* pInstance = new CRoomGlass(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CRoomGlass"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CRoomGlass::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(KEVENT_SIMBA_GLASSBREAK, this);

	__super::Free();

	Safe_Release(m_pBone);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

