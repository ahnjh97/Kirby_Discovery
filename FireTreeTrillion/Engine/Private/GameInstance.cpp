#include "..\Public\GameInstance.h"

#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Object_Manager.h"
#include "Target_Manager.h"
#include "Level_Manager.h"
#include "Timer_Manager.h"
#include "Light_Manager.h"
#include "Font_Manager.h"
#include "SoundManager.h"

#include "Extractor.h"
#include "Renderer.h"
#include "Frustum.h"
#include "Picking.h"

#include "ImGUI_Manager.h"
#include "PhysX.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()	
{

}

HRESULT CGameInstance::Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext)
{
	/* 그래픽 디바이스를 초기화한다 .*/
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc, ppDevice, ppContext);
	if(nullptr == m_pGraphic_Device)
		return E_FAIL;	

	m_pInput_Device = CInput_Device::Create(hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pPhysx = CPhysX::Create();
	if (nullptr == m_pPhysx)
		return E_FAIL;

	/*m_pFont_Manager = CFont_Manager::Create(*ppGraphic_Device);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;
*/
	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pTarget_Manager = CTarget_Manager::Create(*ppDevice, *ppContext);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	/* 인풋 디바이스를 초기화한다 .*/

	/* 사운드 디바이스를 초기화한다 .*/
	m_pSound_Manager = CSound_Manager::Create();
	CHECK_NULLPTR(m_pSound_Manager);

	/* 오브젝트 매니져의 공간 예약을 한다. */
	m_pObject_Manager = CObject_Manager::Create(iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	/* 컴포넌트 매니져의 공간 예약을 한다. */
	m_pComponent_Manager = CComponent_Manager::Create(iNumLevels);
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create();
	if (nullptr == m_pFont_Manager)
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pExtractor = CExtractor::Create(*ppDevice, *ppContext);
	if (nullptr == m_pExtractor)
		return E_FAIL;
	
	/* IMGUI 매니저의 공간 예약을 한다. */
	m_pIMGUI_Manager = CImGUI_Manager::Create(EngineDesc.hWnd, *ppDevice, *ppContext);
	CHECK_NULLPTR(m_pIMGUI_Manager);

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;



	return S_OK;
}


void CGameInstance::Tick_Engine(_float fTimeDelta)
{
	if (nullptr == m_pLevel_Manager || 
		nullptr == m_pObject_Manager || 
		nullptr == m_pPipeLine)
		return;

	m_pInput_Device->Tick();

	m_pObject_Manager->Tick(fTimeDelta);	
	m_pPhysx->Tick(fTimeDelta);
	m_pPicking->Update();
	m_pPipeLine->Tick();

	m_pFrustum->Tick();
	m_pIMGUI_Manager->Late_Tick(fTimeDelta);

	m_pObject_Manager->Late_Tick(fTimeDelta);
	
	/* 반복적인 갱신이 필요한 객체들의 Tick함수를 호출한다. */
	m_pLevel_Manager->Tick(fTimeDelta);

}

HRESULT CGameInstance::Begin_Draw(const _float4 & vClearColor)
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();

	return S_OK;
}

HRESULT CGameInstance::End_Draw()
{
	return m_pGraphic_Device->Present();
}


HRESULT CGameInstance::Draw(_float fTimeDelta)
{
	if (nullptr == m_pGraphic_Device || 
		nullptr == m_pLevel_Manager)
		return E_FAIL;



	/* 화면에 그려져야할 객체들을 그리낟. == 오브젝트 매니져에 들어가있을꺼야 .*/
	/* 오브젝트 매니져에 렌더함수를 만들어서 호출하면 객체들을 다 그린다. */

	/* But. CRenderer객체의 렌더함수를 호출하여 객체를 그리낟. */
	m_pRenderer->Render(fTimeDelta);

	m_pLevel_Manager->Render();	

	m_pIMGUI_Manager->Render();
	m_pIMGUI_Manager->RenderUpdate();

	return S_OK;
}

HRESULT CGameInstance::Clear(_uint iClearLevelIndex)
{
	if (nullptr == m_pObject_Manager ||
		nullptr == m_pComponent_Manager ||
		nullptr == m_pPipeLine)
		return E_FAIL;

	/* 지정된 레벨용 자원(텍스쳐, 사운드, 객체등등) 을 삭제한다. */

	/* 사본 게임오브젝트. */
	m_pObject_Manager->Clear(iClearLevelIndex);

	/* 컴포넌트 원형 */
	m_pComponent_Manager->Clear(iClearLevelIndex);
	

	return S_OK;
}

_bool CGameInstance::Get_KeyState(_ubyte byKeyID, KEYSTATE eState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIKeyState(byKeyID, eState);
}

_bool CGameInstance::Get_KeyState(MOUSEKEYSTATE eMouse, KEYSTATE eState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseState(eMouse, eState);
}

_bool CGameInstance::Get_DIKeyState(_ubyte byKeyID, KEYSTATE eState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIKeyState(byKeyID, eState);
}

_byte CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseState(eMouse);
}

_bool CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eMouse, KEYSTATE eState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseState(eMouse, eState);
}

_long CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

void CGameInstance::Set_WindowActive(_bool _bWindowActive)
{
	if (nullptr == m_pInput_Device)
		return;

	m_pInput_Device->Set_WindowActive(_bWindowActive);
}

_bool CGameInstance::Get_WindowActive()
{
	if (nullptr == m_pInput_Device)
		return true;

	return m_pInput_Device->Get_WindowActive();
}

HRESULT CGameInstance::Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, CGameObject * pRenderObject)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_RenderGroup(eRenderGroup, pRenderObject);	
}

void CGameInstance::Setting_RadialBlur(_fvector vWorldPos, _float fRadial, _float fSubtraction)
{
	if (nullptr == m_pRenderer)
		return;

	m_pRenderer->Setting_RadialBlur(vWorldPos, fRadial, fSubtraction);
}

void CGameInstance::Setting_RadialBlur(_float fRadial, _float fSubtraction)
{
	if (nullptr == m_pRenderer)
		return;

	m_pRenderer->Setting_RadialBlur(fRadial, fSubtraction);
}

HRESULT CGameInstance::Render_LightDepth_For_GameObject(CShader* pShader, CTransform* pTransform, CModel* pModel)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pRenderer->Render_LightDepth_For_GameObject(pShader, pTransform, pModel);

	return S_OK;
}

void CGameInstance::Update_LightShadow(_fvector vLightPos, _fvector vFocusPos)
{
	if (nullptr == m_pRenderer)
		return;

	m_pRenderer->Update_LightShadow(vLightPos, vFocusPos);
}

void CGameInstance::Update_DofFocus(_fvector vWorldPos)
{
	if (nullptr == m_pRenderer)
		return;

	m_pRenderer->Update_DofFocus(vWorldPos);

}

HRESULT CGameInstance::Add_DebugComponents(CComponent * pRenderComponent)
{

	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_DebugComponents(pRenderComponent);
}

HRESULT CGameInstance::Open_Level(_uint iNewLevelID, CLevel * pNewLevel)
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	return m_pLevel_Manager->Open_Level(iNewLevelID, pNewLevel);
}

HRESULT CGameInstance::Add_Prototype(const wstring & strPrototypeTag, CGameObject * pPrototype)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Prototype(strPrototypeTag, pPrototype);	
}

HRESULT CGameInstance::Add_Clone(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Clone(iLevelIndex, strLayerTag, strPrototypeTag, pArg);
}

CGameObject * CGameInstance::Clone_GameObject(const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Clone_GameObject(strPrototypeTag, pArg);
}

const CComponent * CGameInstance::Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComTag, _uint iIndex)
{
	CHECK_NULLPTR(m_pObject_Manager);
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComTag, iIndex);
}

list<CGameObject*>* CGameInstance::Get_List(_uint iLevelIndex, const wstring& strLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Get_List(iLevelIndex, strLayerTag);
}

void CGameInstance::Set_CurrentLevel(_int CurrentLevel)
{
	CHECK_NULLPTR(m_pObject_Manager);
	m_pObject_Manager->Set_CurrentLevel(CurrentLevel);
}

void CGameInstance::Clear_Layer(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return;

	m_pObject_Manager->Clear_Layer(iLevelIndex, wstrLayerTag);
}

_uint CGameInstance::Get_GameObject_Num(_uint _iLevelIndex, const wstring& _strLayerTag)
{
	CHECK_NULLPTR(m_pObject_Manager);
	return m_pObject_Manager->Get_GameObject_Num(_iLevelIndex, _strLayerTag);
}

CGameObject* CGameInstance::Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, _uint iIndex)
{
	CHECK_NULLPTR(m_pObject_Manager);
	return m_pObject_Manager->Get_GameObject(iLevelIndex, strLayerTag, iIndex);
}

CGameObject* CGameInstance::Get_GameObject(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	CHECK_NULLPTR(m_pObject_Manager);
	return m_pObject_Manager->Get_GameObject(iLevelIndex, wstrLayerTag);
}

CGameObject* CGameInstance::Get_GameObject_ByTag(_uint iLevelIndex, const wstring& strLayerTag, wstring _tag)
{
	CHECK_NULLPTR(m_pObject_Manager);
	return m_pObject_Manager->Get_GameObject_ByTag(iLevelIndex, strLayerTag, _tag);
}

HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag, CComponent * pPrototype)
{
	CHECK_NULLPTR(m_pComponent_Manager);
	return m_pComponent_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CComponent * CGameInstance::Clone_Component(_uint iLevelIndex, const wstring & strPrototypeTag, void * pArg)
{
	CHECK_NULLPTR(m_pComponent_Manager);
	return m_pComponent_Manager->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
}

CComponent_Manager::PROTOTYPES* CGameInstance::Get_ComMap(_uint iLevelIdx)
{
	return m_pComponent_Manager->Get_ComMap(iLevelIdx);
}

HRESULT CGameInstance::Add_Timer(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	return m_pTimer_Manager->Add_Timer(strTimerTag);	
}

_float CGameInstance::Compute_TimeDelta(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return 0.0f;

	return m_pTimer_Manager->Compute_TimeDelta(strTimerTag);
}

void CGameInstance::Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	if (nullptr == m_pPipeLine)
		return;

	m_pPipeLine->Set_Transform(eState, TransformMatrix);
}

_float4x4 CGameInstance::Get_Transform(CPipeLine::TRANSFORMSTATE _eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4::Identity;

	return m_pPipeLine->Get_Transform(_eState);
}

_float4x4 CGameInstance::Get_Transform_Inv(CPipeLine::TRANSFORMSTATE _eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4::Identity;

	return m_pPipeLine->Get_Transform_Inv(_eState);
}

_matrix CGameInstance::Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform(eState);
}

_matrix CGameInstance::Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform_Matrix_Inverse(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform_Float4x4_Inverse(eState);
}

//_vector CGameInstance::Get_CamPosition_Vector() const
//{
//	if (nullptr == m_pPipeLine)
//		return XMVectorZero();
//
//	return m_pPipeLine->Get_CamPosition_Vector();
//}

_float4 CGameInstance::Get_CamPosition() const
{
	if (nullptr == m_pPipeLine)
		return _float4();

	return m_pPipeLine->Get_CamPosition();
}

HRESULT CGameInstance::Add_Camera(CCamera* pCamera)
{
	return m_pPipeLine->Add_Camera(pCamera);
}

HRESULT CGameInstance::Switch_CurCamera(_int iIdx)
{
	return m_pPipeLine->Switch_CurCamera(iIdx);
}

void CGameInstance::Clear_Camera()
{
	return m_pPipeLine->Clear_Camera();
}

CCamera* CGameInstance::Get_CurCameraPtr()
{
	return m_pPipeLine->Get_CurCameraPtr();
}

const LIGHT_DESC * CGameInstance::Get_LightDesc(_uint iIndex)
{
	if (m_pLight_Manager == nullptr)
		return nullptr;

	return m_pLight_Manager->Get_LightDesc(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC & LightDesc)
{
	if (m_pLight_Manager == nullptr)
		return E_FAIL;

	return m_pLight_Manager->Add_Light(LightDesc);
}

HRESULT CGameInstance::Render_Lights(CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	if (m_pLight_Manager == nullptr)
		return E_FAIL;

	return m_pLight_Manager->Render(pShader, pVIBuffer);
}

void CGameInstance::Clear_Light()
{
	if (m_pLight_Manager == nullptr)
		return;

	m_pLight_Manager->Clear_Light();
}

CLight* CGameInstance::Get_LightLastAddress()
{
	if (m_pLight_Manager == nullptr)
		return nullptr;

	return m_pLight_Manager->Get_LightLastAddress();
}

HRESULT CGameInstance::Add_Font(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strFontTag, const wstring & strFontFilePath)
{
	if (m_pFont_Manager == nullptr)
		return E_FAIL;

	return m_pFont_Manager->Add_Font(pDevice, pContext, strFontTag, strFontFilePath);	
}

HRESULT CGameInstance::Render_Font(const wstring & strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fRadian)
{
	if (m_pFont_Manager == nullptr)
		return E_FAIL;

	return m_pFont_Manager->Render(strFontTag, strText, vPosition, vColor, fRadian);
}

HRESULT CGameInstance::Add_RenderTarget(const wstring & strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4 & vClearColor)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Add_RenderTarget(strRenderTargetTag, iSizeX, iSizeY, ePixelFormat, vClearColor);
}

HRESULT CGameInstance::Add_MRT(const wstring & strMRTTag, const wstring & strRenderTargetTag)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Add_MRT(strMRTTag, strRenderTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const wstring & strMRTTag, ID3D11DepthStencilView* pDSV)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV);
}

HRESULT CGameInstance::End_MRT()
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Bind_RTShaderResource(CShader * pShader, const wstring & strRenderTargetTag, const _char * pConstantName)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Bind_ShaderResource(pShader, strRenderTargetTag, pConstantName);
}

HRESULT CGameInstance::Copy_Resource(const wstring & strRenderTargetTag, ID3D11Texture2D ** ppTextureHub)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Copy_Resource(strRenderTargetTag, ppTextureHub);
}

_bool CGameInstance::isInFrustum_WorldSpace(_fvector vWorldPos, _float fRange)
{
	if (m_pFrustum == nullptr)
		return false;

	return m_pFrustum->isIn_WorldSpace(vWorldPos, fRange);
}

_bool CGameInstance::isInFrustum_LocalSpace(_fvector vLocalPos, _float fRange)
{
	if (m_pFrustum == nullptr)
		return false;

	return m_pFrustum->isIn_LocalSpace(vLocalPos, fRange);
}

void CGameInstance::TransformFrustum_LocalSpace(_fmatrix WorldMatrixInv)
{
	if (m_pFrustum == nullptr)
		return;

	m_pFrustum->Transform_LocalSpace(WorldMatrixInv);
}

_vector CGameInstance::Compute_WorldPos(const _float2 & vViewportPos, const wstring & strZRenderTargetTag, _uint iOffset)
{
	if (m_pExtractor == nullptr)
		return XMVectorZero();

	return m_pExtractor->Compute_WorldPos(vViewportPos, strZRenderTargetTag, iOffset);
}

PxPhysics* CGameInstance::Get_Physics()
{
	return m_pPhysx->Get_Physics();
}

PxScene* CGameInstance::Get_Scene()
{
	return m_pPhysx->Get_Scene();
}

PxMaterial* CGameInstance::Get_Material()
{
	return m_pPhysx->Get_Material();
}

PxControllerManager* CGameInstance::Get_ControllerManager()
{
	return m_pPhysx->Get_ControllerManager();
}

void CGameInstance::AddActor(physx::PxActor& pActor)
{
	m_pPhysx->AddActor(pActor);
}

void CGameInstance::RemoveActor(physx::PxActor& pActor)
{
	m_pPhysx->RemoveActor(pActor);
}

void CGameInstance::Test()
{
	m_pPhysx->Test();
}

_float4x4 CGameInstance::Update(_fmatrix matrix)
{
	return m_pPhysx->Update(matrix);
}

_uint CGameInstance::Get_CollisionContent(COLLISION_TYPE eMeType, COLLISION_TYPE eOtherType)
{
	return m_pPhysx->Get_CollisionContent(eMeType, eOtherType);
}

#ifdef _DEBUG
HRESULT CGameInstance::Ready_RTVDebug(const wstring & strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Ready_Debug(strRenderTargetTag, fX, fY, fSizeX, fSizeY);
}
HRESULT CGameInstance::Draw_RTVDebug(const wstring& strMRTTag, CShader * pShader, CVIBuffer_Rect * pVIBuffer)
{
	if (m_pTarget_Manager == nullptr)
		return E_FAIL;

	return m_pTarget_Manager->Render_Debug(strMRTTag, pShader, pVIBuffer);
}
#endif


_int CGameInstance::SetVolume(CHANNELID eID, _float _vol)
{
	return m_pSound_Manager->SetVolume(eID, _vol);
}
_int CGameInstance::VolumeUp(CHANNELID eID, _float _vol)
{
	return m_pSound_Manager->VolumeUp(eID, _vol);
}
_int CGameInstance::VolumeDown(CHANNELID eID, _float _vol)
{
	return m_pSound_Manager->VolumeDown(eID, _vol);
}
_int CGameInstance::BGMVolumeUp(_float _vol)
{
	return m_pSound_Manager->BGMVolumeUp(_vol);
}
_int CGameInstance::BGMVolumeDown(_float _vol)
{
	return m_pSound_Manager->BGMVolumeDown(_vol);
}
_int CGameInstance::Pause(CHANNELID eID)
{
	return m_pSound_Manager->Pause(eID);
}
void CGameInstance::PlayMySound(TCHAR* pSoundKey, CHANNELID eID, _float _vol)
{
	m_pSound_Manager->PlayMySound(pSoundKey, eID, _vol);
}
void CGameInstance::PlayBGM(TCHAR* pSoundKey)
{
	m_pSound_Manager->PlayBGM(pSoundKey);
}
void CGameInstance::StopSound(CHANNELID eID)
{
	m_pSound_Manager->StopSound(eID);
}
void CGameInstance::StopAll()
{
	m_pSound_Manager->StopAll();
}
void CGameInstance::ApplyLowPass(_bool bSet)
{
	m_pSound_Manager->ApplyLowPass(bSet);
}
void CGameInstance::AddLowPass()
{
	m_pSound_Manager->AddLowPass();
}
_int CGameInstance::VolumeMin(CHANNELID eID)
{
	return m_pSound_Manager->VolumeMin(eID);
}
_int CGameInstance::VolumeRestore(CHANNELID eID)
{
	return m_pSound_Manager->VolumeRestore(eID);
}

void CGameInstance::PlaySound_Free(TCHAR* pSoundKey, _float _vol)
{
	m_pSound_Manager->PlaySound_Free(pSoundKey, _vol);
}

void CGameInstance::ImGui_Render()
{
	CHECK_NULLPTR(m_pIMGUI_Manager);
	m_pIMGUI_Manager->Render();
}


void CGameInstance::EditTransform(_float4x4& _matrix)
{
	CHECK_NULLPTR(m_pIMGUI_Manager);
	m_pIMGUI_Manager->EditTransform(_matrix);
}

void CGameInstance::RenderGrid()
{
	CHECK_NULLPTR(m_pIMGUI_Manager);
	m_pIMGUI_Manager->RenderGrid();
}

CImGUI_Manager::FILE_MODE CGameInstance::Set_FileDialog()
{
	CHECK_NULLPTR(m_pIMGUI_Manager);
	return m_pIMGUI_Manager->Set_FileDialog();
}

PxRigidDynamic* CGameInstance::CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
{
	if (nullptr == m_pPhysx)
		return nullptr;

	return m_pPhysx->CreateDynamicActor(vPos, pVerticesPos, iNumVertices, pIndices, iNumIndices, pMaterial);
}

PxRigidStatic* CGameInstance::CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial)
{
	if (nullptr == m_pPhysx)
		return nullptr;

	return m_pPhysx->CreateStaticActor(vPos, pVerticesPos, iNumVertices, pIndices, iNumIndices, pMaterial);
}

void CGameInstance::Register_Player(PxActor* pPlayerActor)
{
	if (nullptr != m_pPhysx)
		m_pPhysx->Register_Player(pPlayerActor);
}

void CGameInstance::Register_Trigger(PxActor* pTriggerActor, _int iTriggerType, _int iTriggerIndex)
{
	if (nullptr != m_pPhysx)
		m_pPhysx->Register_Trigger(pTriggerActor, iTriggerType, iTriggerIndex);
}

void CGameInstance::SetUp_TriggerFunc(_int iTriggerType, function<void(_int)> func)
{
	if (nullptr != m_pPhysx)
		m_pPhysx->SetUp_TriggerFunc(iTriggerType, func);
}

void CGameInstance::Clear_EventCallBack()
{
	if (nullptr != m_pPhysx)
		m_pPhysx->Clear_EventCallBack();
}

void CGameInstance::Transform_PickingToLocalSpace(const CTransform* pTransform, _float3* pRayDir, _float3* pRayPos)
{
	if (nullptr == m_pPicking)
		return;

	m_pPicking->Transform_PickingToLocalSpace(pTransform, pRayDir, pRayPos);
}

_float2 CGameInstance::Get_MouseViewPortPos()
{
	if (nullptr == m_pPicking)
		return _float2();

	return m_pPicking->Get_MouseViewPortPos();
}

void CGameInstance::Release_Engine()
{
	CGameInstance::Get_Instance()->Free();
	Destroy_Instance();
}

void CGameInstance::Free()
{
	Safe_Release(m_pFrustum);
	Safe_Release(m_pExtractor);
	Safe_Release(m_pTarget_Manager);
	Safe_Release(m_pIMGUI_Manager);
	Safe_Release(m_pFont_Manager);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pRenderer);	
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pComponent_Manager);
	Safe_Release(m_pPicking);
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pSound_Manager);
	Safe_Release(m_pPhysx);
	Safe_Release(m_pGraphic_Device);
}
