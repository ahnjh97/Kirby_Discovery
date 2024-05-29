#pragma once


#include "GameInstance.h"

namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, 
                 LEVEL_LOGO, 
                 LEVEL_GAMEPLAY, 

                //============= TOOL
                 LEVEL_TOOL_FX,
                 LEVEL_TOOL_UI,
                 LEVEL_TOOL_ANIM,
                 LEVEL_TOOL_MAP,

                 LEVEL_END };

    const unsigned int	g_iWinSizeX = 1600; //1280;
    const unsigned int	g_iWinSizeY = 900; //720;
}


extern HWND g_hWnd;
extern HINSTANCE g_hInst;

using namespace std;
using namespace Client;

#define ADD_GAMEOBJECT_PROTOTYPE(tag, className) \
    do { \
        wstring wstrResult = L"Prototype_GameObject_" + wstring(tag); \
        if (FAILED(m_pGameInstance->Add_Prototype(wstring(wstrResult), className::Create(m_pDevice, m_pContext)))) \
            return E_FAIL; \
    } while (0)


enum PASS_POSTEX {
    POSTEX_DEFAULT, POSTEX_ALPHABLEND, POSTEX_WHITE, POSTEX_BLOOM, POSTEX_END
};

enum PASS_MODEL {
    MODEL_NORMAL_O, MODEL_NORMAL_X, MODEL_SHADOW, MODEL_SKY, MODEL_BLOOM, MODEL_BLEND, MODEL_TRIGGER, MODEL_END
};

enum PASS_ANIMMODEL {
    ANIMMODEL_NORMAL_O, ANIMMODEL_NORMAL_X, ANIMMODEL_SHADOW, ANIMMODEL_KIRBYMOUTH, ANIMMODEL_KIRBYEYE, ANIMMODEL_BLOOM, ANIMMODEL_BLEND, ANIMMODEL_END
};

enum PASS_INSTANCEPOINT {
    INSTANCEPOINT_DEFAULT, INSTANCEPOINT_WHITE, INSTANCEPOINT_END
};