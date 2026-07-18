#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <unordered_map>

namespace ksd
{
	struct GridRect
	{
		float x = 0.f;
		float y = 0.f;
		float w = 3.f;
		float h = 10.f;
	};

	struct ChildState
	{
		std::string name;
		std::string displayName;
		float xPos = 0.f;
		float yPos = 0.f;
		float xSize = 3.f;
		float ySize = 10.f;
		bool init = false;
		bool dragging = false;
		bool resizing = false;
		ImVec2 dragClickOffset = {};
		float scrollRatio = -1.f;
		ImVec2 cursor = {};
		ImVec2 prevCursor = {};
		ImVec2 pos = {};
		ImVec2 size = {};
		float contentHeight = 0.f;
		float contentBottomY = 0.f;
		float appliedScroll = 0.f;
	};

	void BeginMain() noexcept;
	void EndMain() noexcept;
	void BlurRegion(const ImVec2& min, const ImVec2& max) noexcept;
	void ResetChildScrollStates() noexcept;
	bool TabButton(const char* label, int* selected, int num, int total, bool legitTab = false, float slotWidthPx = 0.f) noexcept;
	bool BeginChild(const char* name, GridRect grid = {}, bool legitChild = false) noexcept;
	void EndChild() noexcept;

	ImVec2 CursorPos() noexcept;
	void AddItem(const ImVec2& size, bool comboSpacing = false) noexcept;
	float ContentWidth() noexcept;
	ImVec2 WindowPos() noexcept;
	ImVec2 WindowSize() noexcept;
	float  PanelRightX() noexcept;

	bool IsPopupBlocking() noexcept;
	bool ItemActive(const char* id) noexcept;
	void SetItemActive(const char* id, bool active) noexcept;
	const char* SelectedItem() noexcept;
	void SetSelectedItem(const char* id) noexcept;
	bool PopUpsAreClosed() noexcept;

	bool ButtonBehavior(const char* id, ImRect bb, bool returnOnRelease = false) noexcept;

	int& CurrentTab() noexcept;

	bool Button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f)) noexcept;
	bool ListBox(const char* id, int* selected, const char* const* items, int count) noexcept;
	void InputText(const char* id, char* buffer, int bufferSize) noexcept;
}

#include "../checkbox/checkbox.hpp"
#include "../sliders/sliders.hpp"
#include "../combo/combo.hpp"
#include "../keybind/keybind.hpp"
