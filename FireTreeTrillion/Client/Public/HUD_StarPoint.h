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
	void						Play_Animation(_float _fTimeDelta, HUD_STARPOINT _eSPstate);

public:
	static CHUD_StarPoint*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;

private:
	class CKirby*				m_pKirby = { nullptr };

	_bool						m_IsMovingUP = { TRUE };
	_bool						m_IsLootTrigger = { FALSE };

	HUD_STARPOINT				m_ePreState = { STARPOINT_NONE };
	HUD_STARPOINT				m_eCurState = { STARPOINT_NONE };
	
	_uint						m_iPreCoin = { 0 };

};


END