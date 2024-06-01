#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

/// <summary> 
/// ABILITY 속성값을 부여하기 위한 상위 클래스입니다.
/// 1. physX 영향을 받는 대부분의 객체에 해당됩니다.
/// 2. 해당 클래스를 상속받지 않는 클래스는 다음과 같습니다.
///		- Effect, Camera, UI_Object
/// </summary>
class CPhysXObject abstract : public CGameObject
{
protected:
	CPhysXObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPhysXObject(const CPhysXObject& rhs);
	virtual ~CPhysXObject() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;
	virtual HRESULT Render_LightDepth()				override;

#ifdef _DEBUG
	virtual void	Render_IMGUI()					override;
#endif

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END