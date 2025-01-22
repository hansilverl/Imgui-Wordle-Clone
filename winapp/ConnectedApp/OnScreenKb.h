// OnScreenKb.h
#ifndef ONSCREENKB_H
#define ONSCREENKB_H

#include <string>
#include "GameLogic.h"
#include "../../shared/ImGuiSrc/imgui.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_win32.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_dx11.h"

void RenderOnScreenKeyboard(float offsetX, float offsetY, float gridWidth, float gridHeight, float cellSize, float spacing,
    std::string& currentGuess, size_t& currentCol, size_t& currentRow, GameLogic& game_logic);

#endif // ONSCREENKB_H
