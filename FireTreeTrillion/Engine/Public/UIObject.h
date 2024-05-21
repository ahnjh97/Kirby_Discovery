#pragma once
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject
{
public:
	struct UIOBJ_DESC : public CGameObject::GAMEOBJECT_DESC
	{

	};

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& rhs);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;
	virtual void	Render_IMGUI()								override;

public:
	_float2			Get_pos2D() const { return m_position2D; }

	constexpr _bool	Get_IsRender() const noexcept { return m_bIsRender; }
	void			Set_IsRender(_bool _isRender) { m_bIsRender = _isRender; }

protected:
	// 2D UI 처리용
	_float2			m_size2D, m_position2D, m_WindowSize2D;
	_float2			m_Ratio2D;
	_float4x4		m_ViewMatrix, m_ProjMatrix;

	// 상황에 따른 Render 처리해주는 bool값
	_bool			m_bIsRender = false;

public:
	virtual CGameObject* Clone(_uint iLevelIndex, void* pArg) { return nullptr; }
	virtual void		 Free() override;

};

END

