#pragma once

#include "Base.h"

BEGIN(Engine)

class CLight_Manager final : public CBase
{
private:
	CLight_Manager();
	virtual ~CLight_Manager() = default;

public:
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);

public:
	HRESULT Initialize();
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

	// 방금 추가한 놈의 주소를 뜯어온다. ( 빛을 따라오게 하는 용도 )
	class CLight* Get_LightLastAddress() {
		return m_Lights.back();
	}

	// 빛을 초기화 한다.
	void	Clear_Light();


private:
	list<class CLight*>				m_Lights;

public:
	static CLight_Manager* Create();
	virtual void Free() override;
};

END