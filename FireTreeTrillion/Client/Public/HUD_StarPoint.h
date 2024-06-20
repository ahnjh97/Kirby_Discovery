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
	
	void						Update_UIState(_float _fTimeDelta);
	void						Play_Animation(_float _fTimeDelta, STARPOINT_STATE _eSPstate);

public:
	static CHUD_StarPoint*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	class CKirby*				m_pKirby = { nullptr };

	_bool						m_IsMovingUP = { TRUE };
	_bool						m_IsLootTrigger = { FALSE };
	_bool						m_IsKirbyExist = { FALSE };

	STARPOINT_STATE				m_ePreState = { STARPOINT_NONE };
	STARPOINT_STATE				m_eCurState = { STARPOINT_NONE };
	
	_uint						m_iPreCoin = { 0 };

	_float4						m_vInitPos = { 0.f, 0.f, 0.f, 1.f };
	_float3						m_vInitSize = { 0.f, 0.f, 0.f };
	_float						m_fInitAlpha = { 0.f };

};


END