﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "hooks.hpp"

#include <tchar.h>
#include <iostream>
#include <d3d9.h>
#include <dinput.h>

#include "..\cheats\misc\logs.h"
#include "..\cheats\misc\misc.h"
#include "..\cheats\visuals\other_esp.h"
#include "..\cheats\visuals\radar.h"
#include "../ImGui/imgui_freetype.h"

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment(lib, "freetype.lib")

#include <shlobj.h>
#include <shlwapi.h>
#include <thread>
#include "..\cheats\menu_alpha.h"
#include "../Bytesa.h"

auto _visible = true;
static auto d3d_init = false;

namespace INIT
{
	HMODULE Dll;
	HWND Window;
	WNDPROC OldWindow;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks
{
	int rage_weapon = 0;
	int legit_weapon = 0;
	bool menu_open = false;
	bool input_shouldListen = false;

	ButtonCode_t* input_receivedKeyval;

	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static auto is_down = true;
		static auto is_clicked = false;

		if (GetAsyncKeyState(VK_INSERT))
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!GetAsyncKeyState(VK_INSERT) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked)
		{
			menu_open = !menu_open;

			if (menu_open && g_ctx.available())
			{
				if (g_ctx.globals.current_weapon != -1)
				{
					if (g_cfg.ragebot.enable)
						rage_weapon = g_ctx.globals.current_weapon;
					else if (g_cfg.legitbot.enabled)
						legit_weapon = g_ctx.globals.current_weapon;
				}
			}
		}

		auto pressed_buttons = false;
		auto pressed_menu_key = uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MOUSEWHEEL;

		if (g_ctx.local()->is_alive() && !pressed_menu_key && !g_ctx.globals.focused_on_input)
			pressed_buttons = true;

		if (!pressed_buttons && d3d_init && menu_open && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam) && !input_shouldListen)
			return true;

		if (menu_open && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE) && !input_shouldListen)
			return false;

		return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
	}

	long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		static auto original_fn = directx_hook->get_func_address <EndSceneFn>(42);
		return original_fn(pDevice);
	}

	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(INIT::Window);
		ImGui_ImplDX9_Init(pDevice);

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		style.WindowMinSize = ImVec2(10, 10);

		ImFontConfig m_config;
		m_config.OversampleH = m_config.OversampleV = 3;
		m_config.PixelSnapH = false;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x0400, 0x044F, // Cyrillic
			0,
		};

		char windows_directory[MAX_PATH];
		GetWindowsDirectory(windows_directory, MAX_PATH);

		auto verdana_directory = (std::string)windows_directory + "\\Fonts\\Verdana.ttf";

		// Menu fonts
		c_menu::get().futura = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 15.f, &m_config, ranges);
		c_menu::get().futura_small = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 13.f, &m_config, ranges);
		c_menu::get().futura_large = io.Fonts->AddFontFromMemoryTTF((void*)neverpuk, sizeof(neverpuk), 20.f, &m_config, ranges);

		c_menu::get().super_ico = io.Fonts->AddFontFromMemoryTTF((void*)super_ico, sizeof(super_ico), 20.f, &m_config, ranges);
		c_menu::get().gotham = io.Fonts->AddFontFromMemoryTTF((void*)GothamPro, sizeof(GothamPro), 13.f, &m_config, ranges);
		c_menu::get().interfaces = io.Fonts->AddFontFromMemoryTTF((void*)interfaces, sizeof(interfaces), 15.f * 1.25, &m_config, ranges);
		c_menu::get().font = io.Fonts->AddFontFromMemoryTTF((void*)font, sizeof(font), 13.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());

		ImGui_ImplDX9_CreateDeviceObjects();
		d3d_init = true;
	}

	long __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region)
	{
		static auto original = directx_hook->get_func_address <PresentFn>(17);
		g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

		if (!d3d_init)
			GUI_Init(device);

		IDirect3DVertexDeclaration9* vertex_dec;
		device->GetVertexDeclaration(&vertex_dec);

		IDirect3DVertexShader9* vertex_shader;
		device->GetVertexShader(&vertex_shader);

		c_menu::get().device = device;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		c_menu::get().draw(menu_open);
		g_Misc->watermark();
		g_Misc->spectators_list();
		g_Misc->key_binds();

		if (g_ctx.globals.should_update_radar)
			Radar::get().OnMapLoad(m_engine()->GetLevelNameShort(), device);
		else
			Radar::get().Render();

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		device->SetVertexShader(vertex_shader);
		device->SetVertexDeclaration(vertex_dec);

		return original(device, src_rect, dest_rect, dest_wnd_override, dirty_region);
	}

	long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto ofunc = directx_hook->get_func_address<EndSceneResetFn>(16);

		if (!d3d_init)
			return ofunc(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = ofunc(pDevice, pPresentationParameters);

		if (SUCCEEDED(hr))
			ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}

	DWORD original_getforeignfallbackfontname;
	DWORD original_setupbones;
	DWORD original_doextrabonesprocessing;
	DWORD original_standardblendingrules;
	DWORD original_updateclientsideanimation;
	DWORD original_physicssimulate;
	DWORD original_oClampBonesInBBox;
	DWORD original_modifyeyeposition;
	DWORD original_calcviewmodelbob;
	DWORD original_processinterpolatedlist;
	DWORD original_o_InterpolateServerEntities = (DWORD)DetourFunction((byte*)util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56 57")), (byte*)hooks::hkInterpolateServerEntities);
	DWORD original_clmove = (DWORD)DetourFunction((byte*)util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 83 EC 10 53 8A D9 F3 0F 11 45 FC 8B")), (byte*)hooks::Hooked_CLMove);
	
	//

	
	/*inline PBYTE get_relative_address(uint64_t addr, uint64_t size) {

		if (!addr) return 0;

		PBYTE Address = (PBYTE)((UINT_PTR)(addr));
		PBYTE Pointer = Address + *(PINT)((UINT_PTR)addr + (size - 4));
		return (addr + size + Pointer);
	}
	uint64_t hostinput_clmove_sig = (uint64_t)get_relative_address(util::FindSignature(crypt_str("engine.dll"), crypt_str("E8 ? ? ? ? F3 0F 10 45 ? F2 0F 10 4D ?")), 0x1);
	DWORD original_clmove = (DWORD)DetourFunction((byte*)hostinput_clmove_sig, (byte*)hooks::Hooked_CLMove);*/
	/*DWORD og1 = (DWORD)DetourFunction((byte*)util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 56 8B F1 33 C0 57 8B 7D 08")), (byte*)hooks::sub_10797090);
	DWORD og2 = (DWORD)DetourFunction((byte*)util::FindSignature(crypt_str("engine.dll"), crypt_str("55 8B EC 56 8B F1 33 C0 57 8B 7D 08")), (byte*)hooks::sub_102A83C0);
	DWORD og3 = (DWORD)DetourFunction((byte*)util::FindSignature(crypt_str("studiorender.dll"), crypt_str("55 8B EC 56 8B F1 33 C0 57 8B 7D 08")), (byte*)hooks::sub_10057020);*/



	vmthook* directx_hook;
	vmthook* client_hook;
	vmthook* clientstate_hook;
	vmthook* engine_hook;
	vmthook* player_hook;
	vmthook* clientmode_hook;
	vmthook* inputinternal_hook;
	vmthook* renderview_hook;
	vmthook* panel_hook;
	vmthook* modelcache_hook;
	vmthook* materialsys_hook;
	vmthook* modelrender_hook;
	vmthook* surface_hook;
	vmthook* bspquery_hook;
	vmthook* prediction_hook;
	vmthook* trace_hook;
	vmthook* filesystem_hook;
	vmthook* game_movement_hook;
	//vmthook* netchannel_hook;

	C_HookedEvents hooked_events;
}

void __fastcall hooks::hooked_setkeycodestate(void* thisptr, void* edx, ButtonCode_t code, bool bDown)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetKeyCodeState_t>(91);

	if (input_shouldListen && bDown)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, bDown);
}

void __fastcall hooks::hooked_setmousecodestate(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state)
{
	static auto original_fn = inputinternal_hook->get_func_address <SetMouseCodeState_t>(92);

	if (input_shouldListen && state == BUTTON_PRESSED)
	{
		input_shouldListen = false;

		if (input_receivedKeyval)
			*input_receivedKeyval = code;
	}

	return original_fn(thisptr, code, state);
}