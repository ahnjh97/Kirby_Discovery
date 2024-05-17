#pragma once
#pragma warning (disable : 4251)
#pragma warning (disable : 5208)
#pragma warning (disable : 4267)
//#pragma warning (disable : 4819)
//#pragma warning (disable : 4101)

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <DirectXCollision.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Engine
{
	enum KEYSTATE { KEY_FREE = 0, KEY_DOWN, KEY_PRESS, KEY_UP, KEY_END };
	enum MOUSEKEYSTATE { DIMKS_LBUTTON, DIMKS_RBUTTON, DIMKS_WHEEL, DIMKS_X, DIMKS_END };
	enum MOUSEMOVESTATE { DIMMS_X, DIMMS_Y, DIMMS_WHEEL, DIMMS_END };

	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };
}

// IMGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// SHADER - EFFECT
#include "Effects11/d3dx11effect.h"

// TEXTURE
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"

// COLLIDER : BOUNDING friends
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"

// FONT
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"

// RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"


using namespace DirectX;

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <array>
#include <set>
#include <queue>
#include <unordered_map>
#include <cassert>
#include <random>
#include <chrono>
#include <functional>
#include <fstream>
#include <cmath>
#include <tuple>
#include <numeric>
#include <limits>
#include <atlconv.h>
#include <iostream>
#include <utility>

using namespace std;
using namespace rapidjson;

namespace Engine
{
	const wstring g_strTransformTag = TEXT("Com_Transform");
	const wstring g_strColliderTag = TEXT("Com_Collider");
}

#include "Engine_Macro.h"
#include "Engine_Function.h"
#include "Engine_Typedef.h"
#include "Engine_Struct.h"


using namespace Engine;

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif

#endif // _DEBUG

