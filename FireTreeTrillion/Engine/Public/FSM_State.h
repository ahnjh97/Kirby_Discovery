#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CFSM_State abstract : public CBase
{
protected:
	CFSM_State();
	virtual ~CFSM_State() = default;

public:
	/// <summary> 상태 진입했을 때 처음만 호출하는 함수 </summary>
	/// <param name="_pModel"> 모델 </param>
	/// <param name="_fAnimSpeed"> 해당 상태일 때의 애니메이션 속도 </param>
	/// <param name="_bLoop"> 루프 유무 </param>
	/// <param name="_bInterpolation"> 보간 유무 </param>
	virtual void OnStateEnter(class CModel* _pModel, _uint _iAnimIndex, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);

	/// <summary> 상태 진입되어 있는 상태에서 매 tick마다 호출 </summary>
	/// <param name="pGameObject"> 상태 제어할 객체(this로 넘겨받는다.) </param>
	virtual void OnStateUpdate(class CGameObject* pGameObject, _float fTimeDelta) {}

	/// <summary> 상태가 변경될 때 호출 </summary>
	virtual void OnStateExit() {}

protected:
	class CGameInstance* m_pGameInstance = { nullptr };
	   
public:
	virtual void Free() override;

};

END

