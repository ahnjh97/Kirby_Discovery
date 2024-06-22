#pragma once

#include "Client_Defines.h"
#include "Character.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
class CPartObject;
END

BEGIN(Client)

class CWaddleDee abstract : public CCharacter
{
public:
	enum DEE_EYESTATE
	{
		EYE_IDLE, EYE_ANGER, EYE_CLOSE, EYE_SADNESS,
		EYE_PUPIL, EYE_BLINK, EYE_DOUBT, EYE_END
	};

	enum DEE_ANIM
	{
		DEEANIM_END
	};


	//enum DEE_STATE
	//{
	//	DEE_
	//	DEE_END
	//};

	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{

	}DEE_DESC;


protected:
	CWaddleDee(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee(const CWaddleDee& rhs);
	virtual ~CWaddleDee() = default;

public:
	virtual HRESULT Initialize_Prototype()	override;
	virtual HRESULT Initialize(void* pArg)	override;
	virtual _int	Tick(_float fTimeDelta) override;
	virtual void	Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;
	virtual HRESULT Render_LightDepth()		override;

#ifdef _DEBUG
	virtual void	Render_IMGUI() override;
#endif

	virtual void	Add_AnimEvent() override {}
	//virtual void	Collision(CCollisionCenter::CONTENT_TYPE eContent, CPhysXObject* pObject) override;
	//void			Change_State(DEE_STATE eState, _float _fAnimSpeed, _bool _bLoop, _bool _bInterpolation);


	//_bool			m_bRenderEye = { true };
	//_bool			m_bRenderBody = { true };

protected:
	CModel*			m_pModelCom = { nullptr };

	// FSM
	//void			SetUp_FSM();
	_bool			Custom_Face(_uint iMeshIndex);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;


};

END

