#include "svg.hpp"
#include "guiicons.hpp"
#include "imgui.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d11.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
	ID3D11Device* g_device = nullptr;

	struct Icon
	{
		ImTextureID tex = 0;
		int w = 0;
		int h = 0;
	};
	std::unordered_map<std::string, Icon> g_cache;

	bool LoadSvgText(const char* name, std::string& out)
	{
		return guiicons::Find(name, out);
	}

	float ReadAttr(const std::string& s, const char* attr, float fallback)
	{
		const std::string key = std::string(attr) + "=\"";
		const auto pos = s.find(key);
		if (pos == std::string::npos)
			return fallback;
		return static_cast<float>(std::atof(s.c_str() + pos + key.size()));
	}

	const char* ParseFloat(const char* s, float& out)
	{
		while (*s == ' ' || *s == ',' || *s == '\t' || *s == '\n' || *s == '\r')
			++s;
		char* end = nullptr;
		out = std::strtof(s, &end);
		return (end && end != s) ? end : s + 1;
	}

	struct Sub { std::vector<ImVec2> pts; };

	void ParsePath(const std::string& d, std::vector<Sub>& subs)
	{
		const char* s = d.c_str();
		float cx = 0.f, cy = 0.f, sx = 0.f, sy = 0.f;
		char  cmd = 0;
		Sub*  cur = nullptr;

		while (*s)
		{
			while (*s == ' ' || *s == ',' || *s == '\t' || *s == '\n' || *s == '\r')
				++s;
			if (!*s)
				break;

			if (std::isalpha(static_cast<unsigned char>(*s)))
			{
				cmd = *s;
				++s;
				if ((cmd == 'z' || cmd == 'Z') && cur)
				{
					cur->pts.push_back(ImVec2(sx, sy));
					cx = sx;
					cy = sy;
				}
				continue;
			}

			const bool rel = (cmd >= 'a' && cmd <= 'z');
			const char C   = static_cast<char>(std::toupper(static_cast<unsigned char>(cmd)));

			if (C == 'M')
			{
				float x, y;
				s = ParseFloat(s, x);
				s = ParseFloat(s, y);
				if (rel) { x += cx; y += cy; }
				subs.push_back(Sub());
				cur = &subs.back();
				cur->pts.push_back(ImVec2(x, y));
				cx = sx = x;
				cy = sy = y;
				cmd = rel ? 'l' : 'L';
			}
			else if (C == 'L' && cur)
			{
				float x, y;
				s = ParseFloat(s, x);
				s = ParseFloat(s, y);
				if (rel) { x += cx; y += cy; }
				cur->pts.push_back(ImVec2(x, y));
				cx = x; cy = y;
			}
			else if (C == 'H' && cur)
			{
				float x;
				s = ParseFloat(s, x);
				if (rel) x += cx;
				cx = x;
				cur->pts.push_back(ImVec2(cx, cy));
			}
			else if (C == 'V' && cur)
			{
				float y;
				s = ParseFloat(s, y);
				if (rel) y += cy;
				cy = y;
				cur->pts.push_back(ImVec2(cx, cy));
			}
			else if (C == 'C' && cur)
			{
				float x1, y1, x2, y2, x, y;
				s = ParseFloat(s, x1); s = ParseFloat(s, y1);
				s = ParseFloat(s, x2); s = ParseFloat(s, y2);
				s = ParseFloat(s, x);  s = ParseFloat(s, y);
				if (rel) { x1 += cx; y1 += cy; x2 += cx; y2 += cy; x += cx; y += cy; }
				const int kSteps = 18;
				for (int i = 1; i <= kSteps; ++i)
				{
					const float t = static_cast<float>(i) / kSteps;
					const float u = 1.f - t;
					const float bx = u*u*u*cx + 3*u*u*t*x1 + 3*u*t*t*x2 + t*t*t*x;
					const float by = u*u*u*cy + 3*u*u*t*y1 + 3*u*t*t*y2 + t*t*t*y;
					cur->pts.push_back(ImVec2(bx, by));
				}
				cx = x; cy = y;
			}
			else
			{
				float tmp;
				s = ParseFloat(s, tmp);
			}
		}

		for (Sub& sp : subs)
		{
			if (sp.pts.size() >= 2)
			{
				const ImVec2 a = sp.pts.front();
				const ImVec2 b = sp.pts.back();
				if (a.x != b.x || a.y != b.y)
					sp.pts.push_back(a);
			}
		}
	}

	ImTextureID Upload(const std::vector<unsigned char>& rgba, int w, int h)
	{
		if (!g_device)
			return 0;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width            = w;
		desc.Height           = h;
		desc.MipLevels        = 1;
		desc.ArraySize        = 1;
		desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage            = D3D11_USAGE_DEFAULT;
		desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA sub = {};
		sub.pSysMem     = rgba.data();
		sub.SysMemPitch = w * 4;

		ID3D11Texture2D* tex = nullptr;
		if (FAILED(g_device->CreateTexture2D(&desc, &sub, &tex)) || !tex)
			return 0;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
		srvd.Format                    = desc.Format;
		srvd.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels       = 1;

		ID3D11ShaderResourceView* srv = nullptr;
		const HRESULT hr = g_device->CreateShaderResourceView(tex, &srvd, &srv);
		tex->Release();
		if (FAILED(hr) || !srv)
			return 0;

		return reinterpret_cast<ImTextureID>(srv);
	}

	Icon Rasterize(const char* name, int heightPx)
	{
		Icon icon;

		std::string svgText;
		if (!LoadSvgText(name, svgText))
			return icon;

		float vbx = 0.f, vby = 0.f, vbw = 0.f, vbh = 0.f;
		const auto vbPos = svgText.find("viewBox=\"");
		if (vbPos != std::string::npos)
		{
			const char* p = svgText.c_str() + vbPos + 9;
			p = ParseFloat(p, vbx);
			p = ParseFloat(p, vby);
			p = ParseFloat(p, vbw);
			p = ParseFloat(p, vbh);
		}
		if (vbw <= 0.f || vbh <= 0.f)
		{
			vbw = ReadAttr(svgText, "width", 32.f);
			vbh = ReadAttr(svgText, "height", 32.f);
		}
		if (vbw <= 0.f || vbh <= 0.f)
			return icon;

		const auto dPos = svgText.find(" d=\"");
		if (dPos == std::string::npos)
			return icon;
		const auto dStart = dPos + 4;
		const auto dEnd   = svgText.find('"', dStart);
		if (dEnd == std::string::npos)
			return icon;
		const std::string d = svgText.substr(dStart, dEnd - dStart);

		std::vector<Sub> subs;
		ParsePath(d, subs);
		if (subs.empty())
			return icon;

		const int H  = std::max(heightPx, 4);
		const int W  = std::max(static_cast<int>(std::lround(vbw / vbh * H)), 4);
		const int ss = 4;
		const int SW = W * ss;
		const int SH = H * ss;

		for (Sub& sp : subs)
			for (ImVec2& pt : sp.pts)
			{
				pt.x = (pt.x - vbx) / vbw * SW;
				pt.y = (pt.y - vby) / vbh * SH;
			}

		std::vector<unsigned char> cover(static_cast<size_t>(SW) * SH, 0);
		std::vector<float> xs;
		for (int r = 0; r < SH; ++r)
		{
			const float yline = r + 0.5f;
			xs.clear();
			for (const Sub& sp : subs)
			{
				const size_t n = sp.pts.size();
				for (size_t i = 0; i + 1 < n; ++i)
				{
					const ImVec2 a = sp.pts[i];
					const ImVec2 b = sp.pts[i + 1];
					if ((a.y <= yline && b.y > yline) || (b.y <= yline && a.y > yline))
					{
						const float t = (yline - a.y) / (b.y - a.y);
						xs.push_back(a.x + t * (b.x - a.x));
					}
				}
			}
			std::sort(xs.begin(), xs.end());
			for (size_t i = 0; i + 1 < xs.size(); i += 2)
			{
				int xa = static_cast<int>(std::ceil(xs[i] - 0.5f));
				int xb = static_cast<int>(std::ceil(xs[i + 1] - 0.5f));
				xa = std::max(xa, 0);
				xb = std::min(xb, SW);
				for (int x = xa; x < xb; ++x)
					cover[static_cast<size_t>(r) * SW + x] = 1;
			}
		}

		std::vector<unsigned char> rgba(static_cast<size_t>(W) * H * 4, 0);
		const float inv = 1.f / (ss * ss);
		for (int y = 0; y < H; ++y)
			for (int x = 0; x < W; ++x)
			{
				int sum = 0;
				for (int dy = 0; dy < ss; ++dy)
					for (int dx = 0; dx < ss; ++dx)
						sum += cover[static_cast<size_t>(y * ss + dy) * SW + (x * ss + dx)];
				const unsigned char a = static_cast<unsigned char>(sum * inv * 255.f + 0.5f);
				unsigned char* px = &rgba[(static_cast<size_t>(y) * W + x) * 4];
				px[0] = 255; px[1] = 255; px[2] = 255; px[3] = a;
			}

		icon.tex = Upload(rgba, W, H);
		icon.w   = W;
		icon.h   = H;
		return icon;
	}
}

namespace svg
{
	void Init(ID3D11Device* device) noexcept
	{
		g_device = device;
	}

	ImTextureID Get(const char* name, int heightPx, int& outW, int& outH) noexcept
	{
		char key[128];
		std::snprintf(key, sizeof(key), "%s@%d", name, heightPx);

		const auto it = g_cache.find(key);
		if (it != g_cache.end())
		{
			outW = it->second.w;
			outH = it->second.h;
			return it->second.tex;
		}

		const Icon icon = Rasterize(name, heightPx);
		g_cache[key] = icon;
		outW = icon.w;
		outH = icon.h;
		return icon.tex;
	}
}
