#pragma once
#include "Base.h"

BEGIN(Engine)
class CFSM_State;

/// <summary> 상태 제어가 필요한 객체에 대하여 상태 관리를 진행하는 클래스 </summary>
class ENGINE_DLL CFSM final : public CBase
{
	// FSM을 사용할 객체에서 CFSM_State를 모두 생성해서 맵으로전달
	// 초기 STATE값을 같이 전달할 것
	//typedef struct unordered_map<_uint, CFSM_State*>	MAP_FSM_STATE;
public:
	struct FSM_INFO
	{
		_uint				iState;
		class CModel*		pModel;
	};

private:
	CFSM() {};
	virtual ~CFSM() = default;

public:
	HRESULT				Initialize(void* pArg);
	
	void				Update(class CGameObject* pGameObject, _float fTimeDelta);
	_bool				ChangeState(_uint iState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);
	void				Add_State(_uint iState, CFSM_State* pInitState);
	CFSM_State*			Find_State(_uint iState);

	_uint				Get_State() { return m_iState; }

private:
	CFSM_State*			m_pCurrent_State = { nullptr };
	_uint				m_iState = { UINT_MAX };

	// FSM에서 들고 있을 Model
	class CModel*		m_pModel = { nullptr };

	// for Debug
	_uint				m_iPreState = 0; 
	
	unordered_map<_uint, CFSM_State*>	m_mapFSM_State;

public:
	static CFSM* Create();
	virtual void Free() override;

};

END

