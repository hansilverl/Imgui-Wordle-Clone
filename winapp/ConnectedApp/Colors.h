// Colors.h
#ifndef COLORS_H
#define COLORS_H

#include "../../shared/ImGuiSrc/imgui.h"

// Default color for unused keys
const ImVec4 DEFAULT_COLOR = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
// Color for letters not in word
const ImVec4 WRONG_COLOR = ImVec4(58.0f / 255.0f, 58.0f / 255.0f, 60.0f / 255.0f, 1.0f);
// Color for letters in wrong position
const ImVec4 YELLOW_COLOR = ImVec4(181.0f / 255.0f, 159.0f / 255.0f, 59.0f / 255.0f, 1.0f);
// Color for letters in correct position
const ImVec4 GREEN_COLOR = ImVec4(83.0f / 255.0f, 141.0f / 255.0f, 78.0f / 255.0f, 1.0f);
// Background color
const ImVec4 BACKGROUND_COLOR = ImVec4(0.07f, 0.07f, 0.08f, 1.00f); // #121213

#endif // COLORS_H
