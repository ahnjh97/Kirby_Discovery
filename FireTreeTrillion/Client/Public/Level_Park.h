#pragma once
#include "Client_Defines.h"
#include "Level.h"
#include "Monster.h"

BEGIN(Client)

class CLevel_Park final : public CLevel
{
private:
	CLevel_Park(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Park() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Map();
	HRESULT Ready_Triggers();
	HRESULT Ready_Monsters();
	HRESULT Ready_Items();
	HRESULT Ready_Kickables();
	HRESULT Ready_Objects();
	HRESULT Ready_UI();

	void	Teleport_Player();
	void	Change_Levels();
	void	SummonMonsters(_uint iTriggerIndex);

	HRESULT Add_EnvMap();
	enum TEXTURETYPE { TYPE_ENV, TYPE_LUT, TYPE_NORMAL, TYPE_END };
	CTexture* m_pEnvTexture[TYPE_END] = { nullptr, nullptr, nullptr };

	vector<CMonster::MONSTER_DESC> m_vecMonsterDescs[10];
	unordered_set<_uint> m_setActivatedMonsterTriggers;

public:
	static CLevel_Park* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END

