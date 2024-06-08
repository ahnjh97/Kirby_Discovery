#pragma once
#include "Base.h"

BEGIN(Engine)
class CImGUI_Manager final : public CBase
{
public:
	enum STYLE { PINK, PURPLE, DARK, HYO, STYLE_END };
	enum FILE_MODE { FILE_SAVE, FILE_LOAD, FILE_NONE };

private:
	CImGUI_Manager() = default;
	virtual ~CImGUI_Manager() = default;

public:
	HRESULT Initialize(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext);
	void	Tick(_float fDeltaTime);
	void	Late_Tick(_float fDeltaTime);
	HRESULT Render();
	HRESULT	RenderUpdate();
	

public:
	void SetDockSpace();
	void RenderGrid();
	void EditTransform(_float4x4& matrix);
	CImGUI_Manager::FILE_MODE Set_FileDialog();

	// for Style
	void Set_IMGUIStyle(_uint uStyle);

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

public:
	static CImGUI_Manager* Create(HWND hWnd, ID3D11Device* pGraphic_Device, ID3D11DeviceContext* pContext);
	virtual void Free() override;

};

END

