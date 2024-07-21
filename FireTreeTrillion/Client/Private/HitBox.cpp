#include "stdafx.h"
#include "HitBox.h"
#include "CollisionCenter.h"
#include "Bone.h"

CHitBox::CHitBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CHitBox::CHitBox(const CHitBox& rhs)
	: CGameObject( rhs )
{
}

HRESULT CHitBox::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CHitBox::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	HITBOX_DESC* pDesc = (HITBOX_DESC*)pArg;
	m_pOwner = pDesc->pOwner;
	Safe_AddRef(m_pOwner);
	m_pOwnerTransform = m_pOwner->Get_TransformCom();
	Safe_AddRef(m_pOwnerTransform);
	m_pOwnerCollisionDesc = pDesc->pDesc;
	m_pSocket = pDesc->pSocket;
	Safe_AddRef(m_pSocket);
	m_vBoneOffset = pDesc->vBoneOffset;
	m_vOffset = pDesc->vOffset;

	if (pDesc->matObjectPosition != _float4x4())
	{
		m_matFixed = pDesc->matObjectPosition;
		m_pTransformCom->Set_WorldMatrix(pDesc->matObjectPosition);
	}
	else if (nullptr != m_pSocket)
		m_pTransformCom->Set_WorldMatrix(m_pOwnerTransform->ComputeBoneWorldMatrix(m_pSocket, m_vBoneOffset));
	else if (_float3() != m_vOffset)
	{
		_float4x4 matWorld = m_pOwnerTransform->Get_WorldFloat4x4();
		_float4 vRight = XMVector3Normalize(XMVectorSet(matWorld._11, matWorld._12, matWorld._13, 0)) * m_vOffset.x;
		_float4 vUp = XMVector3Normalize(XMVectorSet(matWorld._21, matWorld._22, matWorld._23, 0)) * m_vOffset.y;
		_float4 vLook = XMVector3Normalize(XMVectorSet(matWorld._31, matWorld._32, matWorld._33, 0)) * m_vOffset.z;

		matWorld._41 += vRight.x + vUp.x + vLook.x;
		matWorld._42 += vRight.y + vUp.y + vLook.y;
		matWorld._43 += vRight.z + vUp.z + vLook.z;

		m_pTransformCom->Set_WorldMatrix(matWorld);
	}
	else
		m_pTransformCom->Set_WorldMatrix(m_pOwnerTransform->Get_WorldFloat4x4());

	m_eCollisionGroup = pDesc->pCollisionType;

	return S_OK;
}

_int CHitBox::Tick(_float fTimeDelta)
{
	if (m_pOwner == nullptr || m_pOwner->Get_Dead() == true)
		return OBJ_DEAD;

	_float4x4 pWorldMatrix = _float4x4();
	if (m_matFixed != _float4x4())
	{
		pWorldMatrix = m_matFixed;
		CUtils::Set_Scaled_Matrix(pWorldMatrix, 1.f, 1.f, 1.f);
	}
	else if (nullptr != m_pSocket) {
		pWorldMatrix = m_pOwnerTransform->Get_WorldFloat4x4();
		CUtils::Set_Scaled_Matrix(pWorldMatrix, 1.f, 1.f, 1.f);

		_float4 vBonePos = m_pOwnerTransform->ComputeBoneWorldPos(m_pSocket, m_vBoneOffset);
		vBonePos.y += m_pOwnerCollisionDesc->fOffSetY;
		memcpy(&(pWorldMatrix.m[3]), &vBonePos, sizeof(_float4));
	}
	else if (_float3() != m_vOffset)
	{
		pWorldMatrix = m_pOwnerTransform->Get_WorldFloat4x4();
		CUtils::Set_Scaled_Matrix(pWorldMatrix, 1.f, 1.f, 1.f);
		_float4 vRight = XMVectorSet(pWorldMatrix._11, pWorldMatrix._12, pWorldMatrix._13, 0) * m_vOffset.x;
		_float4 vUp = XMVectorSet(pWorldMatrix._21, pWorldMatrix._22, pWorldMatrix._23, 0) * m_vOffset.y;
		_float4 vLook = XMVectorSet(pWorldMatrix._31, pWorldMatrix._32, pWorldMatrix._33, 0) * m_vOffset.z;

		pWorldMatrix._41 += vRight.x + vUp.x + vLook.x;
		pWorldMatrix._42 += vRight.y + vUp.y + vLook.y;
		pWorldMatrix._43 += vRight.z + vUp.z + vLook.z;
	}
	else
	{
		pWorldMatrix = m_pOwnerTransform->Get_WorldFloat4x4();
		CUtils::Set_Scaled_Matrix(pWorldMatrix, 1.f, 1.f, 1.f);
		pWorldMatrix._42 += m_pOwnerCollisionDesc->fOffSetY;
	}
	m_pTransformCom->Set_WorldMatrix(pWorldMatrix);

	// 여기서 콜리전 센터에게 등록한다.
	Restore_Logic(fTimeDelta);

	return OBJ_NOEVENT;
}

void CHitBox::Late_Tick(_float fTimeDelta)
{
#ifdef _DEBUG
	if (m_pGameInstance->Get_HitBoxRender() == false)
		return;
#endif

	if (true == m_pGameInstance->isInFrustum_WorldSpace(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION), 2.0f))
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CHitBox::Render()
{

#ifdef _DEBUG

	if (m_pGameInstance->Get_HitBoxRender() == false)
		return S_OK;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	_float4x4 ViewMatrix, ProjMatrix;
	ViewMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_VIEW);
	ProjMatrix = m_pGameInstance->Get_Transform(CPipeLine::D3DTS_PROJ);
	_float4x4 VPMatrix = ViewMatrix * ProjMatrix;

	auto TransformToScreen = [&](XMVECTOR worldPos)
	{
		XMVECTOR screenPos = XMVector3TransformCoord(worldPos, VPMatrix);
		screenPos = XMVectorMultiplyAdd(screenPos, XMVectorSet(0.5f, -0.5f, 1.0f, 0.0f), XMVectorSet(0.5f, 0.5f, 0.0f, 0.0f));
		screenPos = XMVectorMultiply(screenPos, XMVectorSet(g_iWinSizeX, g_iWinSizeY, 1.f, 0.f));
		return ImVec2(XMVectorGetX(screenPos), XMVectorGetY(screenPos));
	};

	ImVec4 color = m_pOwnerCollisionDesc->bAlive == true ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f);


	if (m_pOwnerCollisionDesc->eHitbox == COLLIDER_CYLINDER)
	{
		_float3 vCenter = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		ImVec2 center = TransformToScreen(XMLoadFloat3(&vCenter));

		_float fBottomRadius = m_pOwnerCollisionDesc->fRadius;
		_float fTopRadius = m_pOwnerCollisionDesc->fRadius;
		_float fHeight = m_pOwnerCollisionDesc->fHeight;
		_int iSliceCnt = 8;

		std::vector<ImVec2> bottomCircle, topCircle;

		// 원기둥의 하단과 상단 점 계산
		for (int j = 0; j <= iSliceCnt; ++j)
		{
			float theta = j * 2.0f * DirectX::XM_PI / iSliceCnt;
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);

			_float3 bottomPoint = vCenter + _float3(fBottomRadius * cosTheta, -fHeight * 0.5f, fBottomRadius * sinTheta);
			_float3 topPoint = vCenter + _float3(fTopRadius * cosTheta, fHeight * 0.5f, fTopRadius * sinTheta);

			bottomCircle.push_back(TransformToScreen(XMLoadFloat3(&bottomPoint)));
			topCircle.push_back(TransformToScreen(XMLoadFloat3(&topPoint)));
		}

		// 원기둥 그리기
		for (int j = 0; j < iSliceCnt; ++j)
		{
			drawList->AddLine(bottomCircle[j], bottomCircle[j + 1], ImColor(color.x, color.y, color.z, color.w));
			drawList->AddLine(topCircle[j], topCircle[j + 1], ImColor(color.x, color.y, color.z, color.w));
			drawList->AddLine(bottomCircle[j], topCircle[j], ImColor(color.x, color.y, color.z, color.w));
		}
	}
	else if (m_pOwnerCollisionDesc->eHitbox == COLLIDER_SPHERE)
	{
		_float3 vCenter = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		ImVec2 center = TransformToScreen(XMLoadFloat3(&vCenter));
		_float fRadius = m_pOwnerCollisionDesc->fRadius;
		_int iSliceCnt = 8;

		vector<vector<ImVec2>> spherePoints;
		// 구의 표면을 이루는 점 계산
		for (int i = 0; i <= iSliceCnt; ++i)
		{
			float phi = DirectX::XM_PI * i / iSliceCnt;
			std::vector<ImVec2> stackPoints;

			for (int j = 0; j <= iSliceCnt; ++j)
			{
				float theta = 2.0f * DirectX::XM_PI * j / iSliceCnt;

				float x = fRadius * sinf(phi) * cosf(theta);
				float y = fRadius * cosf(phi);
				float z = fRadius * sinf(phi) * sinf(theta);

				_float3 point = vCenter + _float3(x, y, z);
				stackPoints.push_back(TransformToScreen(XMLoadFloat3(&point)));
			}
			spherePoints.push_back(stackPoints);
		}

		// 구 그리기
		for (int i = 0; i < iSliceCnt; ++i)
		{
			for (int j = 0; j < iSliceCnt; ++j)
			{
				drawList->AddLine(spherePoints[i][j], spherePoints[i][j + 1], ImColor(color.x, color.y, color.z, color.w));
				drawList->AddLine(spherePoints[i][j], spherePoints[i + 1][j], ImColor(color.x, color.y, color.z, color.w));
			}
		}
	}
	else if (m_pOwnerCollisionDesc->eHitbox == COLLIDER_FRUSTUM)
	{
		_float3 vCenter = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float3 vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		vCenter = vCenter + (vLook * m_pOwnerCollisionDesc->fRadius * 0.5f);
		ImVec2 center = TransformToScreen(XMLoadFloat3(&vCenter));
		_float fRadius = m_pOwnerCollisionDesc->fRadius * 0.5f;
		_int iSliceCnt = 8;

		vector<vector<ImVec2>> spherePoints;
		// 구의 표면을 이루는 점 계산
		for (int i = 0; i <= iSliceCnt; ++i)
		{
			float phi = DirectX::XM_PI * i / iSliceCnt;
			std::vector<ImVec2> stackPoints;

			for (int j = 0; j <= iSliceCnt; ++j)
			{
				float theta = 2.0f * DirectX::XM_PI * j / iSliceCnt;

				float x = fRadius * sinf(phi) * cosf(theta);
				float y = fRadius * cosf(phi);
				float z = fRadius * sinf(phi) * sinf(theta);

				_float3 point = vCenter + _float3(x, y, z);
				stackPoints.push_back(TransformToScreen(XMLoadFloat3(&point)));
			}
			spherePoints.push_back(stackPoints);
		}

		// 구 그리기
		for (int i = 0; i < iSliceCnt; ++i)
		{
			for (int j = 0; j < iSliceCnt; ++j)
			{
				drawList->AddLine(spherePoints[i][j], spherePoints[i][j + 1], ImColor(color.x, color.y, color.z, color.w));
				drawList->AddLine(spherePoints[i][j], spherePoints[i + 1][j], ImColor(color.x, color.y, color.z, color.w));
			}
		}

	}
#endif


	return S_OK;
}

HRESULT CHitBox::Render_LightDepth()
{

	return S_OK;
}

#ifdef _DEBUG
void CHitBox::Render_IMGUI()
{


}

#endif

void CHitBox::Restore_Logic(_float fTimeDelta)
{
	// true라면 검사를 하지않는것이다.
	_bool bDontAddCollisionCenter = { false };

	CGameObject* pPlayer = m_pGameInstance->Get_GameObject(*m_pCurrentLevelID, TEXT("Layer_Player"));
	if (pPlayer == nullptr)
		return;

	_float4 vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4 vPlayerPos = pPlayer->Get_TransformCom()->Get_State(CTransform::STATE_POSITION);

	_float fLength = { 0.f };

	if (LEVEL_FINALBOSS == *m_pCurrentLevelID)
		fLength = 200.f;
	else
		fLength = 70.f;

	if ((vPlayerPos - vPos).Length() > fLength)
		bDontAddCollisionCenter = true;

	// 몸통 전용 콜라이더 일 경우. (내가 가지고 있는 구조체 벨류 값)
	if (m_pOwnerCollisionDesc->eValue == BODY)
	{
		if (m_pOwnerCollisionDesc->bAlive == false)
		{
			m_fCollisionTime += fTimeDelta;
			if (m_fCollisionTime > 0.1f)
			{
				m_pOwnerCollisionDesc->bAlive = true;
				m_fCollisionTime = 0.f;
			}
		}

		if (m_pOwnerCollisionDesc->bAlive == true)
		{
			if (bDontAddCollisionCenter == false)
				CCollisionCenter::Get_Instance()->Add_Collision((COLLISION_TYPE)m_eCollisionGroup, this);
		}
	}
	// 공격 전용 콜라이더 일 경우. (내가 가지고 있는 구조체 벨류 값)
	else if (m_pOwnerCollisionDesc->eValue == ATTACK 
		|| ATTACK2 == m_pOwnerCollisionDesc->eValue
		|| ATTACK3 == m_pOwnerCollisionDesc->eValue)
	{
		if (m_pOwnerCollisionDesc->bAlive == false)
		{
			m_fCollisionTime = 0.f;
		}

		if (m_pOwnerCollisionDesc->bAlive == true)
		{
			m_fCollisionTime += fTimeDelta;

			if (m_fCollisionTime > 0.05f)
			{
				m_fCollisionTime = 0.f;
				m_pOwnerCollisionDesc->bAlive = false;
			}
			else
			{
				if (bDontAddCollisionCenter == false)
					CCollisionCenter::Get_Instance()->Add_Collision((COLLISION_TYPE)m_eCollisionGroup, this);
			}
		}
	}
}

HRESULT CHitBox::Add_Components()
{

	return S_OK;
}

HRESULT CHitBox::Bind_ShaderResources()
{
	//if (nullptr == m_pShaderCom)
	//	return E_FAIL;

	//if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
	//	return E_FAIL;

	return S_OK;
}

CHitBox* CHitBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHitBox* pInstance = new CHitBox(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CHitBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHitBox::Clone(void* pArg)
{
	CHitBox* pInstance = new CHitBox(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CHitBox"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHitBox::Free()
{
	__super::Free();

	Safe_Release(m_pSocket);
	Safe_Release(m_pOwner);
	Safe_Release(m_pOwnerTransform);
}

