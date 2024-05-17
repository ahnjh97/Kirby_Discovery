#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CKey_Manager final : public CBase
{
public:
	enum KeyState
	{
		KEY_FREE = 0,
		KEY_DOWN,
		KEY_PRESS,
		KEY_UP,
	};

private:
	CKey_Manager();
	virtual ~CKey_Manager() = default;

public:
	_bool		GetKey_State(_uint _State, _ubyte _Key);
	// »ç¿ë ·Ê1. if( GetKey_State(KEY_DOWN, 'K')) ~
	// »ç¿ë ·Ê2. if( GetKey_State(KEY_DOWN, 'K') && GetKey_State(KEY_PRESS, 'W')) ~
	
private:
	_ubyte			m_KeyState[VK_MAX] = { 0, };

public:
	static CKey_Manager* Create();
	virtual void Free() override;

};


END

