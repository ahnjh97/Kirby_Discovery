#pragma once
#include "Client_Defines.h"
#include "HUD.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CHUD_Kirby : public CUIObject
{
private:
	CHUD_Kirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHUD_Kirby(const CHUD_Kirby& rhs);
	virtual ~CHUD_Kirby() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;
	virtual void				Render_IMGUI()								override;

private:
	HRESULT						Add_Components();
	//HRESULT					Bind_ShaderResources();
	HRESULT						Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT						Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CHUD_Kirby*			Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};


END