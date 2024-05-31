#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CBackGround final : public CUIObject
{
private:
	CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBackGround(const CBackGround& rhs);
	virtual ~CBackGround() = default;

public:
	virtual HRESULT Initialize_Prototype()		 override;
	virtual HRESULT Initialize(void* pArg)		 override;
	virtual _int	Tick(_float fTimeDelta)		 override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()					 override;
	virtual void	Render_IMGUI()				 override;

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources(CShader* _pShaderCom, _uint _iPassIndex, CTransform* _pTransCom, CTexture* _pTextureCom, _uint _iTexIndex);
	HRESULT Bind_VIBuffer(CVIBuffer_Rect* _pVIBufferCom);

public:
	static CBackGround*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

};
END