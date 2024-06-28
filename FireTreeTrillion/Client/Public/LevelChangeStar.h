#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CLevelChangeStar : public CUIObject
{
private:
	CLevelChangeStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelChangeStar(const CLevelChangeStar& rhs);
	virtual ~CLevelChangeStar() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif
	void						Set_Position(_float3 vPos) { m_vPlayerPos = vPos; }

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources();
	void						Update_Pos(_float3 _vPosition);

private:
	array<CTexture*, 3>			m_arrTextures;
	class CKirby*				m_pPlayer = nullptr;
	_float3						m_vPlayerPos = _float3();
	_float4						m_vFinPos = _float4();

public:
	static CLevelChangeStar*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

END
