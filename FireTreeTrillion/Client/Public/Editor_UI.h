#pragma once
#include "Client_Defines.h"
#include "UIObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CEditor_UI : public CUIObject
{
private:
	CEditor_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEditor_UI(const CEditor_UI& rhs);
	virtual ~CEditor_UI() = default;

public:
	virtual HRESULT Initialize_Prototype()						override;
	virtual HRESULT Initialize(void* pArg)						override;
	virtual _int	Tick(_float fTimeDelta)						override;
	virtual void	Late_Tick(_float fTimeDelta)				override;
	virtual HRESULT Render()									override;

#ifdef _DEBUG
	virtual void	Render_IMGUI()								override;
#endif

private:
	//IMGUI UI/UX
	_bool			Set_DockSpace();
	_bool			Window_Directories();
	_bool			Tab_LayerList();
	_bool			Tab_GroupList();

	_bool			Window_Textures();
	_bool			Window_Properties();
	_bool			Window_Tools();

	_bool			Set_OrthoProj(); //투영스페이스 직교 
	_bool			Set_GizmoGrid(); //기즈모 위젯 그리드 세팅
	_bool			Set_GizmoSync(CUIObject* _pUIObj); //기즈모 위젯 동기화 및 세팅

	_bool			Edit_Transform(CUIObject* _pUIObj); //변환
	_bool			Edit_RGBAColor(); //색상 편집
	_bool			Edit_Text(CUIObject* _pUIObj); //텍스트 편집

	_bool			Create_UIObject(LAYER_TYPE _eLayerType, UI_TYPE _eUIType);
	_bool			Delete_UIObject(LAYER_TYPE _eLayerType);
	_bool			Grouping_UIObject(UI_GROUP _eUIGroup);

public:
	_bool			Save_Texture(const string& _strFilePath, ID3D11RenderTargetView* _pRTV);
	_bool			Save_FileData(const string& _strFilePath);
	_bool			Load_FileData(const string& _strFilePath);	

public:
	static CEditor_UI*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;
	virtual void			Free() override;

private:
	wstring					m_wstrInputText = {};
};


END