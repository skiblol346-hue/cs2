#pragma once

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <dxgi.h>
#include <d3d11.h>

#ifdef _DEBUG
#define WINCALL(func) func
#else
#define WINCALL(func) LI_FN(func).cached()
#endif

#include "lazy_importer.hpp"
#include "xorstr.hpp"
#include "platform/Hash.hpp"
#include "runtime/ServiceAccess.hpp"
#include "MinHook.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "platform/Console.hpp"
#include "math/Vector.hpp"
#include "memory/VTable.hpp"
