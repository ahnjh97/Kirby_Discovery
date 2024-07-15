#include "stdafx.h"
#include "OriginCage.h"
#include "EventCenter.h"
#include "Bone.h"

COriginCage::COriginCage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

COriginCage::COriginCage(const COriginCage& rhs)
	: CGameObject{ rhs }
{
}

void COriginCage::Activate(CGameObject* pObj)
{
	m_pModelCom->Set_Animation(CAGE_BREAK, 40.f, false, false);
}

HRESULT COriginCage::Initialize_Prototype()
{
	return S_OK;
}

HRESULT COriginCage::Initialize(void* pArg)
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
	m_eState = CAGE_STATE_BEFORE;

	SetUpMeshIndices();

	m_pModelCom->Set_Animation(CAGE_WAIT, 0.001f, true, false);

	function<void(CGameObject*)> func = bind(&COriginCage::Activate, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_CAGEBREAK, this, func);

	m_pBone = m_pModelCom->Get_BonePtr("Glass1L");

	return S_OK;
}

_int COriginCage::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (true == m_pModelCom->IsFinished())
		m_pModelCom->Set_Animation(CAGE_BREAKAFTER, 1.f, false);

	return OBJ_NOEVENT;
}

void COriginCage::Late_Tick(_float fTimeDelta)
{
	if (false == m_bHide)
	{
		Compute_BoneViewZ(m_pBone);
		m_pModelCom->Play_Animation(fTimeDelta);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	}
}

HRESULT COriginCage::Render()
{
	HRESULT hr{};
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	RenderMeshes(m_vecDefaultMeshes);
	if(CAGE_STATE_BEFORE == m_eState)
		RenderMeshes(m_vecBeforeMeshes);
	if (CAGE_STATE_CRACK == m_eState)
	{
		RenderMeshes(m_vecBeforeMeshes);
		RenderMesh(m_iGlassCrackMesh);
	}
	else if(CAGE_STATE_AFTER == m_eState)
		RenderMeshes(m_vecAfterMeshes);

	return S_OK;
}

HRESULT COriginCage::Add_Components(wstring& wstrModelName)
{
	HRESULT hr{};
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrModeltag = TEXT("Prototype_Component_Model_") + wstrModelName;
	hr = __super::Add_Component(wstrModeltag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	return S_OK;
}

HRESULT COriginCage::Bind_ShaderResources()
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

void COriginCage::SetUpMeshIndices()
{
	for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); i++)
	{
		string strMeshName = m_pModelCom->Get_MeshName(i);
		if ("GlassBreakCrackM__GlassCrackC" == strMeshName)
			m_iGlassCrackMesh = i;
		else if ("GlassBack1M__GlassFrontC" == strMeshName || "GlassFront1M__GlassFrontC" == strMeshName
			|| "GlassFront2M__GlassBreakFrontC" == strMeshName || "GlassInside1M__GlassInsideC" == strMeshName)
			m_vecBeforeMeshes.emplace_back(i);
		else if ("GlassBreak__GlassBreakFrontC" == strMeshName || "GlassBack2M__GlassBreakBackC" == strMeshName)
			m_vecAfterMeshes.emplace_back(i);
		else
			m_vecDefaultMeshes.emplace_back(i);
	}
}

void COriginCage::RenderMesh(_uint iMeshIdex)
{
	HRESULT hr{};
	hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", iMeshIdex, TextureType_DIFFUSE);
	CHECK_FAILED(hr);
	hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", iMeshIdex, TextureType_NORMALS);
	CHECK_FAILED(hr);
	hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", iMeshIdex, TextureType_METALNESS);
	CHECK_FAILED(hr);
	hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", iMeshIdex);
	CHECK_FAILED(hr);

	hr = m_pShaderCom->Begin(ANIMMODEL_ALPHABLEND);
	CHECK_FAILED(hr);
	hr = m_pModelCom->Render(iMeshIdex);
	CHECK_FAILED(hr);
}

void COriginCage::RenderMeshes(const vector<_uint>& _vecMeshIndices)
{
	HRESULT hr{};

	for (auto& i : _vecMeshIndices)
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
}

COriginCage* COriginCage::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COriginCage* pInstance = new COriginCage(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : COriginCage"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* COriginCage::Clone(void* pArg)
{
	COriginCage* pInstance = new COriginCage(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : COriginCage"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void COriginCage::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(this);

	__super::Free();

	Safe_Release(m_pBone);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}

