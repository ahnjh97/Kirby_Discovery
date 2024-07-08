#include "stdafx.h"
#include "Baum.h"
#include "HitBox.h"
#include "Camera_Main.h"
#include "BaumPiece.h"
#include "FinaleRoad.h"

#include "Light.h"
#include "Particle.h"

#include "FinalePartical_Maker.h"

CBaum::CBaum(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPhysXObject{ pDevice, pContext }
{
}

CBaum::CBaum(const CBaum& rhs)
	: CPhysXObject{ rhs }
{
}

HRESULT CBaum::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBaum::Initialize(void* pArg)
{
	BAUMDESC Desc = {};

	if (pArg != nullptr)
		Desc = *(BAUMDESC*)pArg;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Desc.vPos);

	if (FAILED(Add_Components(Desc.wstrModelName)))
		return E_FAIL;

	m_bMotionBlur = true;
	m_bRimLight = true;

	m_fBaumSpeed = Desc.fBaumSpeed;
	m_vBaumMoveDir = Desc.vBaumMoveDir;

	_float4 vNewUp = m_vBaumMoveDir * -1.f;
	vNewUp.Normalize();
	_float4 vNewRight = { 0.f, 0.f, 0.f, 0.f };

	vNewUp == _float4(0.f, 1.f, 0.f, 0.f) ?
		vNewRight = XMVector3Cross(_float4(0.001f, 1.f, 0.f, 0.f), vNewUp) :
		vNewRight = XMVector3Cross(_float4(0.f, 1.f, 0.f, 0.f), vNewUp);

	vNewRight.Normalize();
	_float4 vNewLook = XMVector3Cross(vNewRight, vNewUp);
	vNewLook.Normalize();

	m_pTransformCom->Set_State(CTransform::STATE_UP, vNewUp);
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, vNewRight);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, vNewLook);

	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);


	if (m_eBaumType == BAUM_STARPIECE)
		m_fRimWidth = 0.5f;



	return S_OK;
}

_int CBaum::Tick(_float fTimeDelta)
{
	if (true == m_bDead)
		return Make_Partical();

	m_fTimeDelta = m_pGameInstance->Get_SecondTimer();

	if (m_pLight != nullptr)
		m_pLight->Update_LightPos(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

	// 지형에 박히지 않았을 때
	if (m_bOnTerrain == false)
	{
		m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_UP), m_fTimeDelta, 180.f);
		m_pControllerCom->Move_Dir(m_pTransformCom, m_vBaumMoveDir * m_fTimeDelta * m_fBaumSpeed, m_fTimeDelta);

		m_fScale += m_fTimeDelta * 4.f;
		if (m_fScale > 1.f)
			m_fScale = 1.f;
		m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

		if (m_pTransformCom->Get_State(CTransform::STATE_POSITION).y < -300.f)
		{
			return OBJ_DEAD;
		}

	}	
	else if (m_bOnTerrain == true)
	{
		// 지형에 붙어 따라가는 기능
		if (m_pMyRoad != nullptr)
		{
			_float4x4 NewWorldmatrix = m_HitWorld * m_pMyRoad->Get_WorldMatrix();
			m_pTransformCom->Set_WorldMatrix(NewWorldmatrix);
			_float4 NewPos = CUtils::Get_State_Vector_Matrix(NewWorldmatrix, CUtils::STATE_POSITION);
			m_pControllerCom->Set_Position(m_pTransformCom, NewPos);
		}
	}

	// 구현부
	if ((m_pControllerCom->Is_Terrain() == true || m_pControllerCom->RayCastToDynamicActor(m_vBaumMoveDir) < 2.f) && m_bOnTerrain == false)
	{
		CCamera_Main* pCamera = static_cast<CCamera_Main*>(m_pGameInstance->Get_CurCameraPtr());
		pCamera->Make_Shake();

		// 파티클을 만든다.
		Make_Partical();

		CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_FinalePartical_Maker")));
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		pMaker->Make_Partical(40, vPos, 5.f, 0.3f, 0.2f, _float4(0.f, 1.f, 0.f, 0.f), 180.f, CUtils::Make_RandomFloat(100.f, 150.f));


		m_bOnTerrain = true;
		Find_MyRoad();

		// 스타피스면 바로 죽는다.
		if (m_eBaumType == BAUM_STARPIECE)
		{
			m_bDead = true;
			return OBJ_NOEVENT;
		}
	}

	Compute_MotionBlur();

	return OBJ_NOEVENT;
}

void CBaum::Late_Tick(_float fTimeDelta)
{
	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 20.0f))
	{
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
	/*	if (m_eBaumType == BAUM_STARPIECE)
		{
			m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLOOM, this);
		}*/
	}
}

HRESULT CBaum::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, TextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_NormalTexture", i, TextureType_NORMALS)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_MRATexture", i, TextureType_METALNESS)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bStencil", &m_bStencil, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bRimLight", &m_bRimLight, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("m_fRimWidth", &m_fRimWidth, sizeof(_float))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bMotionBlur", &m_bMotionBlur, sizeof(_bool))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vMotionVelocity", &m_vMotionVelocity, sizeof(_float4))))
			return E_FAIL;
		_float fWhiteColorDiffuse = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fWhiteColorDiffuse", &fWhiteColorDiffuse, sizeof(_float))))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(MODEL_NORMAL_O)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBaum::Render_LightDepth()
{
	if (FAILED(m_pGameInstance->Render_LightDepth_For_GameObject(m_pShaderCom, m_pTransformCom, m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

#ifdef _DEBUG
void CBaum::Render_IMGUI()
{

}
#endif

void CBaum::Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject)
{
	if (eContent == CCollisionCenter::CONTENT_BODY)
	{
		m_bDead = true;
	}
}

HRESULT CBaum::Add_Components(wstring wstrModelProtoTag)
{
	HRESULT hr;
	/* For.Com_Shader */
	hr = __super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom);
	CHECK_FAILED(hr);

	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_") + wstrModelProtoTag;
	hr = __super::Add_Component(wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	CHECK_FAILED(hr);

	m_eBaumType = wstrModelProtoTag == TEXT("Baum") ? BAUM_BAUM : BAUM_STARPIECE;

	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	CCharacterController::CONTROLLER_DESC desc{};
	desc.vInitialPos = vPos;
	desc.fOffset = 1.0f;
	desc.tCapsuleShape.fHeight = 1.f;// 1.f;
	desc.tCapsuleShape.fRadius = 0.5f;// 0.5f;
	hr = __super::Add_Component(TEXT("Prototype_Component_CharacterController"),
		TEXT("Com_Controller"), (CComponent**)&m_pControllerCom, &desc);


	CHitBox::HITBOX_DESC HitBox{};
	HitBox.pOwner = this;
	HitBox.pDesc = &m_tColliderDesc[BODY];
	HitBox.pCollisionType = FINALE_BAUM;
	if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_HitBox"), TEXT("Prototype_GameObject_HitBox"), &HitBox)))
		return E_FAIL;
	Set_BodyCollider(COLLIDER_SPHERE, 1.f, 2.f, 14.f);



	if (m_eBaumType == BAUM_STARPIECE)
	{
		LIGHT_DESC			LightDesc{};
		LightDesc.eType = LIGHT_DESC::TYPE_POINT;
		LightDesc.vPosition = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
		LightDesc.fRange = 30.f;
		LightDesc.vDiffuse = _float4(7.f, .5f, 0.f, 1.f);
		LightDesc.vAmbient = _float4(.5f, .5f, .5f, 1.f);
		LightDesc.vSpecular = _float4(0.f, 0.f, 0.0f, 1.f);
		if (FAILED(CGameInstance::Get_Instance()->Add_Light(LightDesc)))
			return E_FAIL;
		m_pLight = CGameInstance::Get_Instance()->Get_LightLastAddress();
		Safe_AddRef(m_pLight);

	}

	return S_OK;

}

HRESULT CBaum::Bind_ShaderResources()
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

void CBaum::Compute_MotionBlur()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_matrix ViewProjectionMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW) * m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vScreenPos = XMVector3TransformCoord(vPos, ViewProjectionMatrix);
	_float fScreenX = (XMVectorGetX(vScreenPos) + 1.f) * 0.5f;
	_float fScreenY = (XMVectorGetY(vScreenPos) + 1.f) * 0.5f;

	_float2 vCurScreenPos = _float2(fScreenX, 1.f - fScreenY);

	m_vMotionVelocity.x = (m_vPreScreenPos - vCurScreenPos).x;
	m_vMotionVelocity.y = (m_vPreScreenPos - vCurScreenPos).y;
	m_vMotionVelocity.z = m_ePhyXState != PO_NORMAL ? 1.f : 0.f;

	m_vPreScreenPos = vCurScreenPos;
}

void CBaum::Find_MyRoad()
{
	if (m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad")) == nullptr)
		return;
	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float	fMinDistance = { 1000.f };

	for (auto& pRoad : *m_pGameInstance->Get_List(*m_pCurrentLevelID, TEXT("Layer_FinaleRoad")))
	{
		_float4 vRoadPos = static_cast<CFinaleRoad*>(pRoad)->Get_WorldPos();

		_float fDistance = (vRoadPos - vPos).Length();

		if (fMinDistance > fDistance)
		{
			m_pMyRoad = static_cast<CFinaleRoad*>(pRoad);
			fMinDistance = fDistance;
		}
	}

	if (fMinDistance != 1000.f)
	{
		Safe_AddRef(m_pMyRoad);
		m_pMyRoad->Start_CollisionEvent();
		CFinalePartical_Maker* pMaker = static_cast<CFinalePartical_Maker*>(m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_FinalePartical_Maker")));
		_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		pMaker->Make_Partical(30, vPos, 13.f, 1.5f, 0.5f, _float4(0.f, -1.f, 0.f, 0.f), 180.f, 1.f);


		_float4x4 RoadInvMatrix = m_pMyRoad->Get_WorldMatrix().Invert();
		m_HitWorld = m_pTransformCom->Get_WorldMatrix() * RoadInvMatrix;
		m_HitWorld._42 -= 1.5f;
	}
}

_int CBaum::Make_Partical()
{
	if (m_eBaumType == BAUM_BAUM)
	{
		wstring wstrModelName[6] = {
			TEXT("BaumPieceA"),
			TEXT("BaumPieceB"),
			TEXT("BaumPieceC"),
			TEXT("BaumPieceD"),
			TEXT("BaumPieceC"),
			TEXT("BaumPieceD")
		};

		// 파티클을 만드는 함수
		for (_int i = 0; i < 6; ++i)
		{
			_float4x4 matrix = m_pTransformCom->Get_WorldFloat4x4();
			_float4 vDir = /*XMVector3Normalize(m_vBaumMoveDir * -1.f)*/ _float4(0.f, 1.f, 0.f, 0.f);

			vDir = CUtils::Make_RandomAngle_Vector(120.f, vDir);
			vDir.Normalize();
			CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			CUtils::Turn_OtherMatrix(matrix, _float4(1.f, 0.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			_float fRandomscale = CUtils::Make_RandomFloat(0.6f, 1.6f);
			CUtils::Set_Scaled_Matrix(matrix, fRandomscale, fRandomscale, fRandomscale);

			CBaumPiece::BAUMPIECEDESC desc = {};
			desc.matWorld = matrix;
			vDir.y += 0.5f;
			desc.wstrModelName = wstrModelName[i];
			desc.vParticalMoveDir = vDir;
			desc.fParticalSpeed = CUtils::Make_RandomFloat(100.f, 200.f);
			// Car Test
			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_BaumPiece"), TEXT("Prototype_GameObject_BaumPiece"), &desc)))
				return OBJ_DEAD;
		}

	}
	else if (m_eBaumType == BAUM_STARPIECE)
	{
		wstring wstrModelName[6] = {
		TEXT("BaumPieceE"),
		TEXT("BaumPieceF"),
		TEXT("BaumPieceE"),
		TEXT("BaumPieceF"),
		TEXT("BaumPieceE"),
		TEXT("BaumPieceF")
		};

		// 파티클을 만드는 함수
		for (_int i = 0; i < 6; ++i)
		{
			_float4x4 matrix = m_pTransformCom->Get_WorldFloat4x4();
			_float4 vDir = /*XMVector3Normalize(m_vBaumMoveDir * -1.f)*/ _float4(0.f, 1.f, 0.f, 0.f);

			vDir = CUtils::Make_RandomAngle_Vector(120.f, vDir);
			vDir.Normalize();
			CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 1.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			CUtils::Turn_OtherMatrix(matrix, _float4(1.f, 0.f, 0.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			CUtils::Turn_OtherMatrix(matrix, _float4(0.f, 0.f, 1.f, 0.f), 1.f, CUtils::Make_RandomFloat(0.f, 360.f));
			_float fRandomscale = CUtils::Make_RandomFloat(0.6f, 1.6f);
			CUtils::Set_Scaled_Matrix(matrix, fRandomscale, fRandomscale, fRandomscale);

			CBaumPiece::BAUMPIECEDESC desc = {};
			desc.matWorld = matrix;
			vDir.y += 0.5f;
			desc.wstrModelName = wstrModelName[i];
			desc.vParticalMoveDir = vDir;
			desc.fParticalSpeed = CUtils::Make_RandomFloat(100.f, 200.f);
			// Car Test
			if (FAILED(m_pGameInstance->Add_Clone(*m_pCurrentLevelID, TEXT("Layer_BaumPiece"), TEXT("Prototype_GameObject_BaumPiece"), &desc)))
				return OBJ_DEAD;
		}
	}

	//debris explode
	//CParticle::PARTICLE_DESC FXDesc{};
	//FXDesc.vInitPos = GET_POS;
	////FXDesc.vInitScale = { 25.f, 25.f, 25.f };

	//if (FAILED(m_pGameInstance->Add_Clone(*CGameInstance::Get_Instance()->Get_CurrentLevelID(), TEXT("Layer_Effect"), TEXT("Prototype_GameObject_debris explode"), &FXDesc)))
	//	return E_FAIL;

	return OBJ_DEAD;
}

CBaum* CBaum::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBaum* pInstance = new CBaum(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBaum"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBaum::Clone(void* pArg)
{
	CBaum* pInstance = new CBaum(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CBaum"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBaum::Free()
{
	__super::Free();

	Safe_Release(m_pControllerCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pMyRoad);

	if (m_pLight != nullptr)
		m_pLight->Set_DeadLight(true);

	Safe_Release(m_pLight);
}
