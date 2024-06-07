#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CHUD_KirbyStatus : public CUIObject
{
private:
	CHUD_KirbyStatus(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_KirbyStatus(const CHUD_KirbyStatus& rhs);
	virtual ~CHUD_KirbyStatus() = default;

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
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD_KirbyStatus* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void				Free() override;
};


END