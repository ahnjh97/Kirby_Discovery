#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

// 0. 이친구를 클라로 옮겨주세요.
// 1. 스크립트 내용을 저장한다. //스크립트 내용은 파싱할 것입니다.
// 2. 어떻게 RENDER ON-OFF할 것인지
// 3. 어떻게 스크립트 내용물이 다 끝났다고 판단할 것인지
	// [EX] 벡터에 있는 내용물들을 다 사용하고 empty()인지 확인하거나 벡터에 있는 내용물들을 다 순회해서 사용했는 지 판단하기
BEGIN(Client)
class CDialog : public CUIObject
{
public:
	typedef struct DialogMessage
	{
		_uint	uLevel = LEVEL_END;
		wstring wstrNPC = { TEXT("") };
		string strPath = "";

	}DIALOG_DESC;

private:
	CDialog(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext);
	CDialog(const CDialog& _rhs);
	virtual ~CDialog() = default;

public:
	virtual HRESULT				Initialize_Prototype()						override;
	virtual HRESULT				Initialize(void* pArg)						override;
	virtual _int				Tick(_float fTimeDelta)						override;
	virtual void				Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT				Render()									override;


public:
	string  utf8_encode(const wstring& wstr);
	wstring utf8_decode(const string& str);

	void	Save();
	void	Load(string strPath);

private:

public:
	static CDialog* Create(ID3D11Device* _pDevice, ID3D11DeviceContext* _pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

private:
	DIALOG_DESC			m_tMessage_Desc{};

};
END