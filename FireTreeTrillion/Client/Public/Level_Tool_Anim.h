#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

/// <summary>
/// 1. 모든 ANIM MODEL를 사용하는 객체들을 LIST로 불러온다.
/// 2. 해당 LIST에서 객체 하나를 선택했을 경우, 해당 객체에 해당하는 애니메이션을 불러온다.
/// 3. 애니메이션을 선택하여 키프레임을 확인할 수 있다.
/// 4. 해당 키프레임을 선택할 시, COLLISION, EFFECT, SOUND창이 뜬다.
///	5. 해당 창들의 이벤트들을 먼저 선택한 키프레임에 넣을 수 있다.
/// </summary>
class CLevel_Tool_Anim final : public CLevel
{
private:
	CLevel_Tool_Anim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Tool_Anim() = default;

public:
	virtual HRESULT Initialize()			override;
	virtual void	Tick(_float fTimeDelta) override;
	virtual HRESULT Render()				override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_Ground(const wstring& strLayerTag);
	HRESULT Ready_Layer_Character(const wstring& strLayerTag);
	HRESULT Ready_Layer_IMGUI(const wstring& strLayerTag);
	
public:
	static CLevel_Tool_Anim* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};

END