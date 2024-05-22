#pragma once

#include "GameObject.h"

BEGIN(Engine)

// physX에서 controller사용하는 객체들을 character 산하에서 관리
class ENGINE_DLL CCharacter abstract : public CGameObject
{
public:
	struct CHARACTER_DESC : public GAMEOBJECT_DESC
	{
	};

protected:
	CCharacter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCharacter(const CCharacter& rhs);
	virtual ~CCharacter() = default;

public:
	virtual HRESULT Initialize_Prototype()			override;
	virtual HRESULT Initialize(void* pArg)			override;
	virtual _int	Tick(_float fTimeDelta)			override;
	virtual void	Late_Tick(_float fTimeDelta)	override;
	virtual HRESULT Render()						override;

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;

};

END