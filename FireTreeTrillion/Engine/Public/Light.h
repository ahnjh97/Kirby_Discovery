#pragma once

#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CLight final : public CBase
{
private:
	CLight();
	virtual ~CLight() = default;

public:
	const LIGHT_DESC* Get_LightDesc() const {
		return &m_LightDesc;
	}

public:
	HRESULT Initialize(const LIGHT_DESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer, _bool bForTool);

	// 빛이 죽을 운명인지 Mgr에서 확인한다. (그리고 죽임)
	_bool	Get_DeadLight() { return m_bDead; }
	// 빛을 죽을 운명으로 만들어버린다.
	void	Set_DeadLight(_bool bDead) { m_bDead = bDead; }

	// 따라다니게 하는 빛을 만든다.
	void	Update_LightPos(_fvector vPos);
	// 빛 컬링
	_bool	Compute_RenderCull();


private:
	LIGHT_DESC			m_LightDesc = {};
	_bool				m_bDead = { false };
	_int				m_iLifeTime = { 0 };

public:
	static CLight* Create(const LIGHT_DESC& LightDesc);
	virtual void Free() override;
};

END