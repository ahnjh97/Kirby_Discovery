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
	
	void						Activate(_float fTimeDelta);
	void						Deactivate();

	void						Set_FocusingPosition(_float4 vPos) { m_vFocusingPos = vPos; }

private:
	HRESULT						Add_Components();
	HRESULT						Bind_ShaderResources();
	HRESULT						Change_Size(_uint iNum);

	void						Update_Pos(_float4 _vPosition);

private:
	array<CTexture*, 3>			m_arrTextures;
	class CKirby*				m_pPlayer = nullptr;

	_float4						m_vFocusingPos = _float4();

	// 이미지 이동
	_float4						m_vStartPos = _float4();
	_float4						m_vFinPos = _float4();

	_float						m_fRatio = 1.f;
	_float						m_fRatioRemove = 1.f;
	_float2						m_InitialSize = _float2(2500.f, 2500.f);
	_float2						m_FinalSize = _float2(400.f, 400.f);
	_float2						m_DecSize = _float2(0.f, 0.f);
	_float2						m_DecGreenSize = _float2(0.f, 0.f);

	_float						m_fTimeDelta = _float();

	// 회전값 저장
	array<_float4x4, 3>			m_arrayStarMatrix;
	_float						m_fTurningTime = 0.f;

	_bool						m_bRemove = false;

public:
	static CLevelChangeStar*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	virtual void				Free() override;
};

END
