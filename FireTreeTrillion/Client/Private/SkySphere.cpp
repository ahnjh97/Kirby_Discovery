#include "stdafx.h"
#include "GameInstance.h"
#include "SkySphere.h"
#include "Particle.h"

CSkySphere::CSkySphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{ pDevice, pContext }
{
}

CSkySphere::CSkySphere(const CSkySphere& rhs)
	:CGameObject{ rhs }
{
}

HRESULT CSkySphere::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkySphere::Initialize(void* pArg)
{
	HRESULT hr = __super::Initialize(pArg);
	CHECK_FAILED(hr);
	
	SKYSPHERE_DESC SkySphereDesc{};
	if (nullptr != pArg)
		SkySphereDesc = *(SKYSPHERE_DESC*)pArg;

	m_strModelTag = SkySphereDesc.strModelTag;
	m_strTextureTag = SkySphereDesc.strTextureTag;

	//레벨 별 상태 변경을 위한 값 저장
	m_pCurrentLevelID = m_pGameInstance->Get_CurrentLevelID();
	
	hr = Add_Components();
	CHECK_FAILED(hr);

	if (LEVEL_FINALBOSS == *m_pCurrentLevelID)
		m_pTransformCom->Set_Scaled(_float3{ 0.1f, 0.1f, 0.1f });
	else if (LEVEL_FINALE == *m_pCurrentLevelID)
	{
		m_pTransformCom->Set_Scaled(_float3{ .6f, .6f, .6f });

	}
	else
		m_pTransformCom->Set_Scaled(_float3{ .6f, .6f, .6f });


	
	return S_OK;
}

_int CSkySphere::Tick(_float fTimeDelta)
{
	return OBJ_NOEVENT;
}

void CSkySphere::Late_Tick(_float fTimeDelta)
{
	_float4 vCamPos = m_pGameInstance->Get_CamPosition();
	vCamPos.y -= 100.f;
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vCamPos);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_PRIORITY, this);
}

HRESULT CSkySphere::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	HRESULT hr;
	
	if (LEVEL_FINALBOSS > *m_pCurrentLevelID)
	{
		for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		{
			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DepthTexture", i, TextureType_HEIGHT);
			CHECK_FAILED(hr);

			hr = m_pShaderCom->Begin(MODEL_SKY);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}

#pragma region LEVEL_FINALBOSS & LEVEL_FINALE ::TEXTURE SWAP 

	//1) 현재 해당 레벨 진입 시에 임시로 설정. 추후 FIELD/1PASE/2PASE 시점에 스왑하는 방식으로 변경 필요
	// ex) 에피리스 HP 45% 일 경우, 2페이즈 시작 (SKY_LAB_2PASE)
	//2) 추후 일렁일렁 움직이는 효과 셰이더로 세팅 필요
	SKY_TYPE eSkyType = { SKY_LAB_1PASE };
	if (LEVEL_FINALBOSS <= *m_pCurrentLevelID)
	{
		if (LEVEL_FINALE == *m_pCurrentLevelID) //피날레 레벨에서는 스피어 텍스처 교체
			eSkyType = SKY_LAB_2PASE;

		for (_uint i = 0; i < m_pModelCom->Get_NumMeshes(); ++i)
		{
			hr = m_pTextureCom[TEX_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", eSkyType);
			CHECK_FAILED(hr);

			//hr = m_pTextureCom[TEX_NORMAL]->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", SKY_LAB_2PASE);
			//CHECK_FAILED(hr);

			////TEX_EMISSIVE :: 방출 옵션
			//hr = m_pTextureCom[TEX_EMISSIVE]->Bind_ShaderResource(m_pShaderCom, "g_EmissiveTexture", SKY_LAB_2PASE);
			//CHECK_FAILED(hr);

			//hr = m_pTextureCom[TEX_HEIGHT]->Bind_ShaderResource(m_pShaderCom, "g_DepthTexture", SKY_LAB_2PASE);
			//CHECK_FAILED(hr);

			hr = m_pShaderCom->Begin(MODEL_SKY);
			CHECK_FAILED(hr);

			hr = m_pModelCom->Render(i);
			CHECK_FAILED(hr);
		}
	}

#pragma endregion


	return S_OK;
}

HRESULT CSkySphere::Add_Components()
{
	HRESULT hr;

	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"), 
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	wstring wstrProtoTagMod = TEXT("Prototype_Component_Model_") + CUtils::StrToWstr(m_strModelTag);
	hr = __super::Add_Component(LEVEL_STATIC, wstrProtoTagMod, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

#pragma region LAB_DISCOVERA
	
	//LEVEL_FINALBOSS & LEVEL_FINALE일 경우 해당 텍스처를 세팅
	if (LEVEL_FINALBOSS <= *m_pCurrentLevelID)
	{
		wstring wstrProtoTagTex = TEXT("Prototype_Component_Texture_") + CUtils::StrToWstr(m_strTextureTag);
		hr = __super::Add_Component(wstrProtoTagTex,
			TEXT("Com_Tex_Lab_Diffuse"), (CComponent**)&m_pTextureCom[TEX_DIFFUSE]);
		CHECK_FAILED(hr);

		hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SkySphere_Lab_CloudNoize"),
			TEXT("Com_Tex_Lab_CloudNoize"), (CComponent**)&m_pTextureCom[TEX_MRA]);
		CHECK_FAILED(hr);

		hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SkySphere_LabBoss_2Pase_Normal"),
			TEXT("Com_Tex_LabBoss_Normal"), (CComponent**)&m_pTextureCom[TEX_NORMAL]);
		CHECK_FAILED(hr);

		hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SkySphere_LabBoss_2Pase_Emissive"),
			TEXT("Com_Tex_LabBoss_Emissive"), (CComponent**)&m_pTextureCom[TEX_EMISSIVE]);
		CHECK_FAILED(hr);

		hr = __super::Add_Component(TEXT("Prototype_Component_Texture_SkySphere_LabBoss_2Pase_Height"),
			TEXT("Com_Tex_LabBoss_Height"), (CComponent**)&m_pTextureCom[TEX_HEIGHT]);
		CHECK_FAILED(hr);
	}

#pragma endregion

	return S_OK;
}

HRESULT CSkySphere::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CSkySphere* CSkySphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSkySphere* pInstance = new CSkySphere(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSkySphere"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSkySphere::Clone(void* pArg)
{
	CSkySphere* pInstance = new CSkySphere(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Clone : CSkySphere"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkySphere::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	for(auto& iTex : m_pTextureCom)
		Safe_Release(iTex);

	__super::Free();
}
