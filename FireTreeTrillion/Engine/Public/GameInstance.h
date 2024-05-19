#pragma once

/* 클라이언트개발자가 엔진의 기능을 사용하기위해서 항상 접근해야하는 클래스. */
#include "Renderer.h"
#include "Component_Manager.h"
#include "PipeLine.h"
#include "Utils.h"

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);
	void Tick_Engine(_float fTimeDelta);
	HRESULT Begin_Draw(const _float4 & vClearColor);
	HRESULT End_Draw();
	HRESULT Draw(_float fTimeDelta);
	HRESULT Clear(_uint iClearLevelIndex);

	_uint* Get_CurrentLevelID() { return &m_iCurrentLevelID; }
	void Set_CurrentLevelID(_uint iLevelIndex) { m_iCurrentLevelID = iLevelIndex; }


public: /* For.Input_Device */
	_bool	Get_DIKeyState(_ubyte byKeyID, KEYSTATE eState);
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse);
	_bool	Get_DIMouseState(MOUSEKEYSTATE eMouse, KEYSTATE eState);
	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouse);

public: /* For.Renderer */
	HRESULT Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	void Setting_RadialBlur(_fvector vWorldPos, _float fRadial, _float fSubtraction = 70.f);
	void Setting_RadialBlur(_float fRadial, _float fSubtraction = 70.f);
	HRESULT Render_LightDepth_For_GameObject(class CShader* pShader, class CTransform* pTransform, class CModel* pModel);


#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderComponent);
#endif

public: /* For.Level_Manager */
	HRESULT Open_Level(_uint iNewLevelID, class CLevel* pNewLevel);

public: /* For.Object_Manager */
	HRESULT Add_Prototype(const wstring& strPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_Clone(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag, void* pArg = nullptr);

	class CGameObject* Clone_GameObject(const wstring& strPrototypeTag, void* pArg = nullptr);
	const CComponent* Get_Component(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strComTag, _uint iIndex = 0);
	list<CGameObject*>* Get_List(_uint iLevelIndex, const wstring& strLayerTag);

	class CGameObject*	Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, _uint iIndex);
	class CGameObject*	Get_GameObject_ByTag(_uint iLevelIndex, const wstring& strLayerTag, wstring _tag);
	void				Set_CurrentLevel(_int CurrentLevel);

public: /* For.Component_Manager */
	HRESULT			  Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring& strPrototypeTag, void* pArg = nullptr);


public: /* For.Timer_Manager */
	HRESULT Add_Timer(const wstring& strTimerTag);
	_float Compute_TimeDelta(const wstring& strTimerTag);

public: /* For.PipeLine */
	void Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix);
	_matrix Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const;
	_matrix Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_vector Get_CamPosition_Vector() const;
	_float4 Get_CamPosition_Float4() const;

public: /* For.Light_Manager */
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
	void Clear_Light();
	class CLight* Get_LightLastAddress();


public: /* For.Font_Manager */
	HRESULT Add_Font(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strFontTag, const wstring& strFontFilePath);
	HRESULT Render_Font(const wstring& strFontTag, const wstring & strText, const _float2 & vPosition, _fvector vColor, _float fRadian);


public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring& strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const wstring& strMRTTag, const wstring& strRenderTargetTag);
	HRESULT Begin_MRT(const wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Bind_RTShaderResource(class CShader* pShader, const wstring& strRenderTargetTag, const _char* pConstantName);
	HRESULT Copy_Resource(const wstring& strRenderTargetTag, ID3D11Texture2D** ppTextureHub);

public: /* For.Frustum */
	_bool isInFrustum_WorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isInFrustum_LocalSpace(_fvector vLocalPos, _float fRange = 0.f);
	void TransformFrustum_LocalSpace(_fmatrix WorldMatrixInv);

public: /* For.Extractor */
	_vector Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset = 0);

public: /* For.PhysX */
	PxScene*	Get_Scene();
	PxPhysics*	Get_Physics();
	PxMaterial* Get_Material();
	void		AddActor(physx::PxActor& pActor);
	void		RemoveActor(physx::PxActor& pActor);

	void Test();
	_float4x4 Update(_fmatrix matrix);

#ifdef _DEBUG
	HRESULT Ready_RTVDebug(const wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Draw_RTVDebug(const wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

#endif

public: // Sound Manager
	_int SetVolume(CHANNELID eID, _float _vol);
	_int  VolumeUp(CHANNELID eID, _float _vol);
	_int  VolumeDown(CHANNELID eID, _float _vol);
	_int  BGMVolumeUp(_float _vol);
	_int  BGMVolumeDown(_float _vol);
	_int  Pause(CHANNELID eID);
	void PlayMySound(TCHAR* pSoundKey, CHANNELID eID, _float _vol);
	void PlayBGM(TCHAR* pSoundKey);
	void StopSound(CHANNELID eID);
	void StopAll();
	void ApplyLowPass(_bool bSet);
	void AddLowPass();
	_int  VolumeMin(CHANNELID eID);
	_int  VolumeRestore(CHANNELID eID);
	void PlaySound_Free(TCHAR* pSoundKey, _float _vol);

public: /* For.ImGui_Manager */
	void		ImGui_Render();
	void		EditTransform(/*const CCamera& camera,*/ _float4x4& _matrix);

public: /* For.PhysX */
	PxRigidDynamic* CreateDynamicActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial = nullptr);
	PxRigidStatic* CreateStaticActor(_float4 vPos, _float3* pVerticesPos, _uint iNumVertices, _uint* pIndices, _int iNumIndices, PxMaterial* pMaterial = nullptr);

private:
	class CGraphic_Device*			m_pGraphic_Device = { nullptr };
	class CInput_Device*			m_pInput_Device = { nullptr };
	class CLevel_Manager*			m_pLevel_Manager = { nullptr };
	class CObject_Manager*			m_pObject_Manager = { nullptr };
	class CComponent_Manager*		m_pComponent_Manager = { nullptr };
	class CRenderer*				m_pRenderer = { nullptr };
	class CTimer_Manager*			m_pTimer_Manager = { nullptr };	
	class CPipeLine*				m_pPipeLine = { nullptr };
	class CLight_Manager*			m_pLight_Manager = { nullptr };
	class CFont_Manager*			m_pFont_Manager = { nullptr };
	class CTarget_Manager*			m_pTarget_Manager = { nullptr };
	class CFrustum*					m_pFrustum = { nullptr };
	class CExtractor*				m_pExtractor = { nullptr };
	class CSound_Manager*			m_pSound_Manager = { nullptr };
	class CImGUI_Manager*			m_pIMGUI_Manager = { nullptr };
	class CPhysX*					m_pPhysx = { nullptr };

	_uint	m_iCurrentLevelID = { 0 };

public:		
	static void Release_Engine();
	virtual void Free() override;
};

END