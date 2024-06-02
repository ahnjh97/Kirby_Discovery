#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
END

BEGIN(Client)

class CAnimToolHelper final : public CGameObject
{
public:
	//struct EVENT_INFO
	//{
	//	string	strEventName;
	//	_int	iStartFrame;
	//	_int	iEndFrame;
	//};

	//struct ANIM_INFO
	//{
	//	_float				fAnimSpeed;
	//	vector<EVENT_INFO>	vecEventInfo;
	//};

	using AnimToolMap = unordered_map< string, unordered_map< string, ANIM_INFO >>;

private:
	CAnimToolHelper(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimToolHelper(const CAnimToolHelper& rhs);
	virtual ~CAnimToolHelper() = default;

public:
	virtual HRESULT			Initialize_Prototype()				override;
	virtual HRESULT			Initialize(void* pArg = nullptr)	override;
	virtual _int			Tick(_float fTimeDelta)				override;
	virtual void			Late_Tick(_float fTimeDelta)		override;
	virtual HRESULT			Render()							override;
	//virtual void			Render_IMGUI()						override;

private:
	void					Ready_AnimModels();

	// RENDER
	void					Render_ObjectList();
	void					Render_AnimationList();
	void					Render_FrameLine(class CAnimation** pModel, const string& strAnimationTag);

	void					Save();
	void					Load(/*const string& FileName = ""*/);

private:
	vector<string>							m_vecAnimModels;
	class CAnimToolObject*					m_pAnimToolObj = nullptr;
	CModel*									m_pModel = nullptr;

	string									m_strModelName = string();
	string									m_strAnimationName = string();
	_float									m_fAnimationSpeed = _float();

	//vector<SEQUENCE_ITEM>					m_vecSequence;
	AnimToolMap								m_mapSequence;

public:
	static	CAnimToolHelper* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	 Clone(void* pArg) override;
	virtual void			 Free() override;

};

END

