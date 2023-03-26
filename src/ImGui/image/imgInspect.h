// https://github.com/CedricGuillemet/imgInspect
//
// The MIT License(MIT)
//
// Copyright(c) 2019 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


/*

	Example

	Image pickerImage;
	ImGui::ImageButton(pickerImage.textureID, ImVec2(pickerImage.mWidth, pickerImage.mHeight));

	ImGuiIO& io = ImGui::GetIO();
	ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
	mouseUVCoord.y = 1.f - mouseUVCoord.y;


	if (io.KeyShift && io.MouseDown[0] && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
	{
			int width = pickerImage.mWidth;
			int height = pickerImage.mHeight;

			imageInspect(width, height, pickerImage.GetBits(), mouseUVCoord, displayedTextureSize);
	}

*/


//--

#pragma once

#include "ofMain.h"

#define IMGUI_DEFINE_MATH_OPERATORS // Access to math operators
#include "imgui_internal.h"
#include "ofxImGui.h"

namespace ImageInspect
{
	inline void histogram(const int width, const int height, const unsigned char* const bits)
	{
		unsigned int count[4][256] = { 0 };

		const unsigned char* ptrCols = bits;

		ImGui::InvisibleButton("histogram", ImVec2(512, 256));
		for (int l = 0; l < height * width; l++)
		{
			count[0][*ptrCols++]++;
			count[1][*ptrCols++]++;
			count[2][*ptrCols++]++;
			count[3][*ptrCols++]++;
		}

		unsigned int maxv = count[0][0];
		unsigned int* pCount = &count[0][0];
		for (int i = 0; i < 3 * 256; i++, pCount++)
		{
			maxv = (maxv > *pCount) ? maxv : *pCount;
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const ImVec2 rmin = ImGui::GetItemRectMin();
		const ImVec2 rmax = ImGui::GetItemRectMax();
		const ImVec2 size = ImGui::GetItemRectSize();
		const float hFactor = size.y / float(maxv);

		for (int i = 0; i <= 10; i++)
		{
			float ax = rmin.x + (size.x / 10.f) * float(i);
			float ay = rmin.y + (size.y / 10.f) * float(i);
			drawList->AddLine(ImVec2(rmin.x, ay), ImVec2(rmax.x, ay), 0x80808080);
			drawList->AddLine(ImVec2(ax, rmin.y), ImVec2(ax, rmax.y), 0x80808080);
		}

		const float barWidth = (size.x / 256.f);
		for (int j = 0; j < 256; j++)
		{
			// pixel count << 2 + color index(on 2 bits)
			uint32_t cols[3] = { (count[0][j] << 2), (count[1][j] << 2) + 1, (count[2][j] << 2) + 2 };
			if (cols[0] > cols[1])
				ImSwap(cols[0], cols[1]);
			if (cols[1] > cols[2])
				ImSwap(cols[1], cols[2]);
			if (cols[0] > cols[1])
				ImSwap(cols[0], cols[1]);
			float heights[3];
			uint32_t colors[3];
			uint32_t currentColor = 0xFFFFFFFF;
			for (int i = 0; i < 3; i++)
			{
				heights[i] = rmax.y - (cols[i] >> 2) * hFactor;
				colors[i] = currentColor;
				currentColor -= 0xFF << ((cols[i] & 3) * 8);
			}

			float currentHeight = rmax.y;
			const float left = rmin.x + barWidth * float(j);
			const float right = left + barWidth;
			for (int i = 0; i < 3; i++)
			{
				if (heights[i] >= currentHeight)
				{
					continue;
				}
				drawList->AddRectFilled(ImVec2(left, currentHeight), ImVec2(right, heights[i]), colors[i]);
				currentHeight = heights[i];
			}
		}
	}

	inline void drawNormal(ImDrawList* draw_list, const ImRect& rc, float x, float y, ofColor* c = nullptr)
	{
		draw_list->AddCircle(rc.GetCenter(), rc.GetWidth() / 2.f, 0x20AAAAAA, 24, 1.f);
		draw_list->AddCircle(rc.GetCenter(), rc.GetWidth() / 4.f, 0x20AAAAAA, 24, 1.f);

		draw_list->AddLine(rc.GetCenter(), rc.GetCenter() + ImVec2(x, y) * rc.GetWidth() / 2.f, 0xFF0000FF, 2.f);
		//fix
		//draw_list->AddLine(rc.GetCenter(), rc.GetCenter() + ImVec2(x * (rc.GetWidth() / 2.f), y * (rc.GetWidth() / 2.f)) , 0xFF0000FF, 2.f);
	}

	inline void inspect(const int width,
		const int height,
		const unsigned char* const bits,
		ImVec2 mouseUVCoord_,
		ImVec2 displayedTextureSize, ofColor* c = nullptr)
	{
		ImVec2 mouseUVCoord = ImVec2(
			ofClamp(mouseUVCoord_.x, 0, 1),
			ofClamp(mouseUVCoord_.y, 0, 1));

		ImGui::BeginTooltip();

		ImGui::BeginGroup();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		static const float zoomRectangleWidth = 160.f;

		// 1. Magnifier

		// bitmap zoom
		ImGui::InvisibleButton("AnotherInvisibleMan", ImVec2(zoomRectangleWidth, zoomRectangleWidth));

		const ImRect pickRc(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
		draw_list->AddRectFilled(pickRc.Min, pickRc.Max, 0xFF000000);
		static int zoomSize = 4;
		const float quadWidth = zoomRectangleWidth / float(zoomSize * 2 + 1);
		const ImVec2 quadSize(quadWidth, quadWidth);
		const int basex = ImClamp(int(mouseUVCoord.x * width), zoomSize, width - zoomSize);
		const int basey = ImClamp(int(mouseUVCoord.y * height), zoomSize, height - zoomSize);

		for (int y = -zoomSize; y <= zoomSize; y++)
		{
			for (int x = -zoomSize; x <= zoomSize; x++)
			{
				//fix
				//uint32_t texel = ((uint32_t*)bits)[(basey - y) * width + x + basex];
				size_t i = (basey - y) * width + x + basex;
				//size_t sz = ((uint32_t*)bits).size();
				size_t size = sizeof(((uint32_t*)bits)) / sizeof(((uint32_t*)bits)[0]);
				//size_t size = sizeof(((uint32_t*)bits)) / sizeof(((uint32_t*)bits)[0]);

				uint32_t texel = ((uint32_t*)bits)[i];
				ImVec2 pos = pickRc.Min + ImVec2(float(x + zoomSize), float(y + zoomSize)) * quadSize;
				draw_list->AddRectFilled(pos, pos + quadSize, texel);
			}
		}

		if (1)
		{
			ImGui::SameLine();

			// center quad
			const ImVec2 pos = pickRc.Min + ImVec2(float(zoomSize), float(zoomSize)) * quadSize;
			draw_list->AddRect(pos, pos + quadSize, 0xFF0000FF, 0.f, 15, 2.f);
		}

		if (0)
		{
			// normal direction
			ImGui::InvisibleButton("AndOneMore", ImVec2(zoomRectangleWidth, zoomRectangleWidth));
			ImRect normRc(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
			for (int y = -zoomSize; y <= zoomSize; y++)
			{
				for (int x = -zoomSize; x <= zoomSize; x++)
				{
					uint32_t texel = ((uint32_t*)bits)[(basey - y) * width + x + basex];
					const ImVec2 posQuad = normRc.Min + ImVec2(float(x + zoomSize), float(y + zoomSize)) * quadSize;
					//draw_list->AddRectFilled(pos, pos + quadSize, texel);
					const float nx = float(texel & 0xFF) / 128.f - 1.f;
					const float ny = float((texel & 0xFF00) >> 8) / 128.f - 1.f;
					const ImRect rc(posQuad, posQuad + quadSize);
					drawNormal(draw_list, rc, nx, ny);
				}
			}
		}

		ImGui::EndGroup();

		ImGui::SameLine();

		// 2. Labels

		ImGui::BeginGroup();

		//fix
		int i = (basey - zoomSize * 2 - 1) * width + basex;
		uint32_t texel = ((uint32_t*)bits)[i];
		//uint32_t texel = ((uint32_t*)bits)[(basey - zoomSize * 2 - 1) * width + basex];

		// size
		ImGui::Text("w,h %d,%d", int(displayedTextureSize.x), int(displayedTextureSize.y));

		// uv	
		ImGui::Text("U,V %1.3f,%1.3f", mouseUVCoord.x, mouseUVCoord.y);

		// coord
		ImGui::Text("x,y %d,%d", int(mouseUVCoord.x * width), int(mouseUVCoord.y * height));

		ImVec4 color = ImColor(texel);
		if (c != nullptr)
		{
			c->set(color);
		}

		// color
		if (0) {
			ImVec4 colHSV;
			ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, colHSV.x, colHSV.y, colHSV.z);
			//ImGui::Separator();
			//ImGui::Text("R 0x%02x  G 0x%02x  B 0x%02x", int(color.x * 255.f), int(color.y * 255.f), int(color.z * 255.f));
			ImGui::Text("R %1.3f G %1.3f B %1.3f", color.x, color.y, color.z);

			//ImGui::Separator();

			//ImGui::Text(
				//"H 0x%02x  S 0x%02x  V 0x%02x", int(colHSV.x * 255.f), int(colHSV.y * 255.f), int(colHSV.z * 255.f));
			ImGui::Text("H %1.3f S %1.3f V %1.3f", colHSV.x, colHSV.y, colHSV.z);

			//ImGui::Separator();

			//ImGui::Text("Alpha 0x%02x", int(color.w * 255.f));
			ImGui::Text("Alpha %1.3f", color.w);
		}

		//ImGui::Separator();

		ImGui::EndGroup();

		//histogram(width, height, bits);
		ImGui::EndTooltip();
	}
} // namespace ImageInspect
