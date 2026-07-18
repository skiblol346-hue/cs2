#include "../shell/shell.hpp"
#include "../styling.hpp"
#include "gui/assets/fonts.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
	struct InputState
	{
		int   caret    = 0;
		int   anchor   = 0;
		float lastEdit = 0.f;
		bool  dragging = false;
	};

	struct Ghost
	{
		char  c   = 0;
		float x   = 0.f;
		float age = 0.f;
	};

	constexpr float kGhostRise = 12.f;
	constexpr float kGhostLife = 0.45f;
	constexpr float kCaretHalf = 0.53f;

	std::string                                         g_activeInputId;
	std::unordered_map<std::string, InputState>         g_states;
	std::unordered_map<std::string, std::vector<Ghost>> g_ghosts;

	float SubWidth(ImFont* font, float sz, const char* buf, int from, int to)
	{
		if (to <= from)
			return 0.f;
		return font->CalcTextSizeA(sz, FLT_MAX, 0.f, buf + from, buf + to).x;
	}

	int CaretFromX(ImFont* font, float sz, const char* buf, int len, float x)
	{
		float acc = 0.f;
		for (int i = 0; i < len; ++i)
		{
			const float cw = font->CalcTextSizeA(sz, FLT_MAX, 0.f, buf + i, buf + i + 1).x;
			if (x < acc + cw * 0.5f)
				return i;
			acc += cw;
		}
		return len;
	}

	void DeleteRange(char* buf, int a, int b)
	{
		const int len = static_cast<int>(std::strlen(buf));
		a = std::clamp(a, 0, len);
		b = std::clamp(b, 0, len);
		if (a >= b)
			return;
		std::memmove(buf + a, buf + b, static_cast<size_t>(len - b) + 1);
	}

	bool InsertChar(char* buf, int bufSize, int pos, char c)
	{
		const int len = static_cast<int>(std::strlen(buf));
		if (len + 1 >= bufSize)
			return false;
		pos = std::clamp(pos, 0, len);
		std::memmove(buf + pos + 1, buf + pos, static_cast<size_t>(len - pos) + 1);
		buf[pos] = c;
		return true;
	}
}

namespace ksd
{
	void InputText(const char* id, char* buffer, int bufferSize) noexcept
	{
		if (!id || !buffer || bufferSize <= 0)
			return;

		ImDrawList*   dl    = ImGui::GetWindowDrawList();
		ImFont* const font  = fonts::Verdana();
		const float   fsize = fonts::kUiSize;
		const float   time  = static_cast<float>(ImGui::GetTime());

		const float  w       = ContentWidth();
		const ImVec2 drawPos = CursorPos() + ImVec2(20.f, 0.f);
		const ImRect bb(drawPos, drawPos + ImVec2(w, 20.f));
		const bool   hovered = bb.Contains(ImGui::GetMousePos());

		InputState& st  = g_states[id];
		int         len = static_cast<int>(std::strlen(buffer));

		constexpr float kXW   = 18.f;
		const bool   hasText   = len > 0;
		const float  textStart = bb.Min.x + 5.f;
		const float  textAreaW = hasText ? w - kXW : w;
		const float  textTop   = bb.Min.y + (bb.GetHeight() - font->CalcTextSizeA(fsize, FLT_MAX, 0.f, "Ag").y) * 0.5f;

		const ImRect xRect(ImVec2(bb.Max.x - kXW, bb.Min.y), bb.Max);
		const bool   overX = hasText && xRect.Contains(ImGui::GetMousePos());

		const bool wasActive = (g_activeInputId == id);
		if (hovered && !overX && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && PopUpsAreClosed())
		{
			g_activeInputId = id;
			st.caret    = CaretFromX(font, fsize, buffer, len, ImGui::GetMousePos().x - textStart);
			st.anchor   = st.caret;
			st.dragging = true;
			st.lastEdit = time;
		}
		else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !hovered && wasActive)
		{
			g_activeInputId.clear();
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			st.dragging = false;

		const bool active = (g_activeInputId == id);

		if (active && st.dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			st.caret = CaretFromX(font, fsize, buffer, len, ImGui::GetMousePos().x - textStart);

		if (active)
		{
			ImGuiIO& io = ImGui::GetIO();
			const int selMin = std::min(st.anchor, st.caret);
			const int selMax = std::max(st.anchor, st.caret);
			const bool hasSel = selMin != selMax;

			auto dropSelection = [&]()
			{
				DeleteRange(buffer, selMin, selMax);
				st.caret  = selMin;
				st.anchor = selMin;
				len = static_cast<int>(std::strlen(buffer));
			};

			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A, false))
			{
				st.anchor = 0;
				st.caret  = len;
				st.lastEdit = time;
			}

			for (ImWchar ch : io.InputQueueCharacters)
			{
				if (ch < 32 || ch >= 127 || io.KeyCtrl)
					continue;
				if (st.anchor != st.caret)
					dropSelection();
				if (InsertChar(buffer, bufferSize, st.caret, static_cast<char>(ch)))
				{
					st.caret++;
					st.anchor = st.caret;
				}
				len = static_cast<int>(std::strlen(buffer));
				st.lastEdit = time;
			}
			io.InputQueueCharacters.resize(0);

			if (ImGui::IsKeyPressed(ImGuiKey_Backspace, true))
			{
				if (hasSel)
				{
					dropSelection();
				}
				else if (st.caret > 0)
				{
					g_ghosts[id].push_back({ buffer[st.caret - 1],
						SubWidth(font, fsize, buffer, 0, st.caret - 1), 0.f });
					DeleteRange(buffer, st.caret - 1, st.caret);
					st.caret--;
					st.anchor = st.caret;
				}
				len = static_cast<int>(std::strlen(buffer));
				st.lastEdit = time;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Delete, true))
			{
				if (hasSel)
					dropSelection();
				else if (st.caret < len)
					DeleteRange(buffer, st.caret, st.caret + 1);
				len = static_cast<int>(std::strlen(buffer));
				st.lastEdit = time;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true))
			{
				if (st.caret > 0) st.caret--;
				if (!io.KeyShift) st.anchor = st.caret;
				st.lastEdit = time;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true))
			{
				if (st.caret < len) st.caret++;
				if (!io.KeyShift) st.anchor = st.caret;
				st.lastEdit = time;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_Home, false))
			{
				st.caret = 0;
				if (!io.KeyShift) st.anchor = 0;
				st.lastEdit = time;
			}
			if (ImGui::IsKeyPressed(ImGuiKey_End, false))
			{
				st.caret = len;
				if (!io.KeyShift) st.anchor = len;
				st.lastEdit = time;
			}
		}

		st.caret  = std::clamp(st.caret, 0, len);
		st.anchor = std::clamp(st.anchor, 0, len);

		dl->AddRect(bb.Min, bb.Max, style::col::outline());

		if (hasText)
		{
			const ImVec2 xSz = font->CalcTextSizeA(fsize, FLT_MAX, 0.f, "x");
			dl->AddText(font, fsize,
				ImVec2(bb.Max.x - kXW + (kXW - xSz.x) * 0.5f, textTop),
				overX ? style::col::text() : style::col::textDim(), "x");
			if (overX && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				buffer[0] = '\0';
				st.caret  = 0;
				st.anchor = 0;
				g_activeInputId.clear();
			}
		}

		dl->PushClipRect(ImVec2(textStart - 1.f, bb.Min.y), ImVec2(bb.Min.x + textAreaW - 2.f, bb.Max.y), true);

		if (active && st.anchor != st.caret)
		{
			const float x0 = textStart + SubWidth(font, fsize, buffer, 0, std::min(st.anchor, st.caret));
			const float x1 = textStart + SubWidth(font, fsize, buffer, 0, std::max(st.anchor, st.caret));
			const ImU32 selCol = IM_COL32(
				static_cast<int>(vars::colorAccent[0] * 255.f * 0.55f),
				static_cast<int>(vars::colorAccent[1] * 255.f * 0.55f),
				static_cast<int>(vars::colorAccent[2] * 255.f * 0.55f),
				std::min(140, style::menuAlpha));
			dl->AddRectFilled(ImVec2(x0, bb.Min.y + 2.f), ImVec2(x1, bb.Max.y - 2.f), selCol);
		}

		dl->AddText(font, fsize, ImVec2(textStart, textTop), style::col::text(), buffer);

		if (active)
		{
			const bool lit = std::fmod(time - st.lastEdit, kCaretHalf * 2.f) < kCaretHalf;
			if (lit)
			{
				const float cx = textStart + SubWidth(font, fsize, buffer, 0, st.caret);
				dl->AddLine(ImVec2(cx, bb.Min.y + 3.f), ImVec2(cx, bb.Max.y - 3.f), style::col::text());
			}
		}
		dl->PopClipRect();

		auto& ghosts = g_ghosts[id];
		if (!ghosts.empty())
		{
			const float dt = ImGui::GetIO().DeltaTime;
			dl->PushClipRect(ImVec2(textStart - 1.f, bb.Min.y - kGhostRise),
				ImVec2(bb.Min.x + textAreaW - 2.f, bb.Max.y), true);
			for (Ghost& gh : ghosts)
			{
				gh.age += dt;
				const float t = gh.age / kGhostLife;
				const int   a = std::min(style::menuAlpha, static_cast<int>(255.f * (1.f - t)));
				if (a <= 0)
					continue;
				const char s[2] = { gh.c, 0 };
				dl->AddText(font, fsize,
					ImVec2(textStart + gh.x, textTop - kGhostRise * t), IM_COL32(205, 205, 205, a), s);
			}
			dl->PopClipRect();
			ghosts.erase(std::remove_if(ghosts.begin(), ghosts.end(),
				[](const Ghost& g) { return g.age >= kGhostLife; }), ghosts.end());
		}

		AddItem(ImVec2(w + 20.f, 20.f), true);
	}
}
