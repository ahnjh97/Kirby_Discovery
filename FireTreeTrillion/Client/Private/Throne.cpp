#include "stdafx.h"
#include "Throne.h"
#include "EventCenter.h"
#include "Bone.h"

CThrone::CThrone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CThrone::CThrone(const CThrone& rhs)
	: CGameObject{ rhs }
{
}

void CThrone::Break(CGameObject* pObj)
{
	 m_bPlayAnimation = true;
	 m_pGameInstance->PlaySound_Free(L"ThroneBreak.wav", 0.5f);
}

HRESULT CThrone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CThrone::Initialize(void* pArg)
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

	CBone* pPieceManager = m_pModelCom->Get_BonePtr("PieceAllL");
	_float4x4* pEditMatrix = pPieceManager->Get_EditMatrixPtr();
	_float4x4 matTemp = _float4x4::Identity;

	_float fScale = 0.685f;
	matTemp._11 = matTemp._22 = fScale;
	matTemp._33 = 0.9f;
	matTemp._42 = 2.0f;
	matTemp._43 = 0.24f;

	*pEditMatrix = matTemp;

	m_pModelCom->Set_Animation(0, 50.f, false, false);
	m_pModelCom->Play_Animation(m_pGameInstance->Get_FirstTimer());

	function<void(CGameObject*)> func = bind(&CThrone::Break, this, placeholders::_1);
	CEventCenter::Get_Instance()->Subscribe(KEVENT_SIMBA_THRONEBREAK, this, func);

	for (_uint i = 1; i < 9; i++)
	{
		string strPieceName = "Piece" + to_string(i) + "__ThroneC";
		m_setPieces.emplace(m_pModelCom->Find_MeshIndex(strPieceName));
	}

	m_setPieces.emplace(m_pModelCom->Find_MeshIndex(string("CvConcretePoleIronPartsBM2__CvConcretePoleC")));

	return S_OK;
}

_int CThrone::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return OBJ_DEAD;

	if (m_pModelCom->Get_AnimRatio() > 0.55f && false == m_bMeshesRemoved)
	{
		m_bMeshesRemoved = true;

		unordered_set<_uint> tempSet;
		tempSet.emplace(m_pModelCom->Find_MeshIndex(string("MainM__ThroneC")));
		tempSet.emplace(m_pModelCom->Find_MeshIndex(string("CvConcretePoleIronPartsBM1__CvConcretePoleC")));

		m_pModelCom->RemoveBlendMeshes(tempSet);
	}

	if (m_pModelCom->Get_AnimRatio() > 0.59f && false == m_bGlassBreak)
	{
		m_bGlassBreak = true;
		CEventCenter::Get_Instance()->Notify(KEVENT_SIMBA_GLASSBREAK);
	}

	if (m_pModelCom->Get_CurAnimIndex() == 0 && true == m_pModelCom->IsFinished())
		m_bPlayAnimation = false;

	return OBJ_NOEVENT;
}

void CThrone::Late_Tick(_float fTimeDelta)
{
	if(true == m_bPlayAnimation)
		m_pModelCom->Play_Animation(m_pGameInstance->Get_SecondTimer());

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CThrone::Render()
{
	HRESULT hr{};
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	if (true == m_bPlayAnimation && m_pModelCom->Get_AnimRatio() > 0.55f)
	{
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

			hr = m_pShaderCom->Begin(ANIMMODEL_NEARCLIP);
			CHECK_FAILED(hr);
			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}
	else
	{
		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (m_setPieces.end() != m_setPieces.find(i))
				continue;

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
			CHECK_FAILED(hr);
			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
			CHECK_FAILED(hr);
			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
			CHECK_FAILED(hr);
			hr = m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i);
			CHECK_FAILED(hr);

			hr = m_pShaderCom->Begin(ANIMMODEL_NEARCLIP);
			CHECK_FAILED(hr);
			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}

	return S_OK;
}

HRESULT CThrone::Add_Components(wstring& wstrModelName)
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

HRESULT CThrone::Bind_ShaderResources()
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

CThrone* CThrone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CThrone* pInstance = new CThrone(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CThrone"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CThrone::Clone(void* pArg)
{
	CThrone* pInstance = new CThrone(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CThrone"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CThrone::Free()
{
	CEventCenter::Get_Instance()->Unsubscribe(KEVENT_SIMBA_THRONEBREAK, this);

	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}