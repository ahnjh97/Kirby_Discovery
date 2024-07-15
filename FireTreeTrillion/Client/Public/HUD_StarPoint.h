#pragma once
#include "Client_Defines.h"
#include "UIObject.h"
#include "HUD.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CHUD_StarPoint : public CHUD
{
public:
	enum STARPOINT_STATE {
		STARPOINT_IDLE, STARPOINT_WAIT,
		STARPOINT_HIDE, STARPOINT_SHOW, STARPOINT_LOOT, STARPOINT_DROP, STARPOINT_NONE
	};

private:
	CHUD_StarPoint(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_StarPoint(const CHUD_StarPoint& rhs);
	virtual ~CHUD_StarPoint() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	//virtual void				Render_IMGUI()								override;
#endif

private:
	HRESULT						Add_Components();
	HRESULT						Render_BindSet(CShader* _pShaderCom, CTransform* _pTransCom);
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);
	
	// 코인의 정보를 받고 어떤 숫자가 나와야 하는지, 현재 상황이 어떤지만 채킹한다.
	void						Compute_Coin(_float _fTimeDelta);
	void						Movement_CoinUI(_float _fTimeDelta);
	void						Disappear_CoinUI(_float fTimeDelta);

public:
	static CHUD_StarPoint*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	//_bool						m_IsMovingUP = { TRUE };
	//_bool						m_IsLootTrigger = { FALSE };
	//_bool						m_IsKirbyExist = { FALSE };

	//STARPOINT_STATE				m_ePreState = { STARPOINT_NONE };
	STARPOINT_STATE				m_eCurState = { STARPOINT_NONE };
	//
	//_uint						m_iPreCoin = { 0 };

	_uint						m_iCurCoin = { 0 };
	_uint						m_iPreCoin = { 0 };
	_bool						m_bInitializeCoin = { true };

	// 코인 아이콘이 밝아지며, 프레임 재생이 되는 불 값이다. 또한 이걸로 숫자도 통 튀게 한다.
	_bool						m_bPlusCoin = { false };
	_uint						m_iHun = { 0 };
	_uint						m_iTen = { 0 };
	_uint						m_iOne = { 0 };

	// 숫자가 위로 올라가는 수치
	_float						m_fUpNumY = { 0.f };
	// 아이콘이 밝아진다.
	_float						m_fWhiteColor = { 0.f };
	// 반응이 없을때 옆으로 들어간다.
	_float						m_fIdleTime = { 0.f };
	_float						m_fAlpha = { 0.f };

	_float						m_fFrame = { 3.f };

	_float4						m_vInitPos = { 0.f, 0.f, 0.f, 1.f };
	_float3						m_vInitSize = { 0.f, 0.f, 0.f };
	_float						m_fInitAlpha = { 0.f };

	_float						m_fSaveMyX = { 0.f };
	_bool						m_bRender = { false };
	_float						m_fFontX = { 0.f };
};


END