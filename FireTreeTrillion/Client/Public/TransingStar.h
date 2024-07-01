#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CTransingStar : public CUIObject
{
public:
	enum TYPE { OPEN, CLOSE, TYPE_END };

private:
	CTransingStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransingStar(const CTransingStar& rhs);
	virtual ~CTransingStar() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;

#ifdef _DEBUG
	virtual void				Render_IMGUI()								override;
#endif

public:
	void						Activate(TYPE _eActivateType);
	void						Deactivate();

	// CLOSE //////////////////////////////////////////
	void						Tick_AlphaStar(_float fTimeDelta);
	void						Tick_YeonDooStar(_float fTimeDelta);
	void						Tick_GreenStar(_float fTimeDelta);
	void						RenderClose();
	void						Set_NextLevel(LEVEL eNextLevel) { m_eNextLevel = eNextLevel; }

	// OPEN  ////////////////////////////////////////// 
	void						Tick_OpenAlphaStar(_float fTimeDelta);
	void						RenderOpen();


private:
	HRESULT						Add_Components();

private:
	array<CTexture*, 3>			m_arrTextures;
	array<_float4x4, 3>			m_arrayStarMatrix;

	// 사이즈 이동값에 대한 기준치들
	_float2						m_InitialSize = _float2(3000.f, 3000.f);
	_float2						m_MediumSize = _float2(400.f,400.f);
	
	// TransingStar의 쓸모
	TYPE						m_eActivateType = TYPE_END;
	// CLOSE TYPE일 경우, 이동될 LEVEL
	LEVEL						m_eNextLevel = LEVEL_END;

	// 알파 별 기준값
	_float						m_fAlphaTime = 1.f;

	// 연두 별 기준값
	_float						m_fYeonDooTime = 1.f;
	_float						m_fDecreaseValue = 0.f;
	_float						m_fDecreaseOffset = 2500.f;

	// 연두별 셰이더 처리
	_bool						m_bDeadYeonDoo = false;


public:
	static CTransingStar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*  Clone(void* pArg) override;
	virtual void		  Free() override;

};

END
