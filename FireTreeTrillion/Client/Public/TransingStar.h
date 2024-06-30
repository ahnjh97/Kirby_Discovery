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
	void						Set_Activate(_bool _bActivate);// { m_bActivate = _bActivate; }

private:
	HRESULT						Add_Components();

private:
	array<CTexture*, 3>			m_arrTextures;
	array<_float4x4, 3>			m_arrayStarMatrix;

	// 사이즈 이동값에 대한 기준치들
	_float2						m_InitialSize = _float2(4000.f, 4000.f);
	_float2						m_MediumSize = _float2(500.f, 500.f);
	
	// 
	_bool						m_bActivate = false;
	_float						m_fTurningTimeRemains = 1.f;
	_float						m_fAlphaTimeRemains = 1.f;

	_bool						m_bDeadYeonDoo = false;
	_float						m_fDecreaseValue = 0.f;

public:
	static CTransingStar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*  Clone(void* pArg) override;
	virtual void		  Free() override;

};

END
