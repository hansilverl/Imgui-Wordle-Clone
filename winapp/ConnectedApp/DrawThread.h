#pragma once
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include "../../shared/ImGuiSrc/imgui.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_win32.h"
#include "../../shared/ImGuiSrc/backends/imgui_impl_dx11.h"
#include "GameLogic.h"
#include "ScoreBoard.h"
#include "OnScreenKb.h"
#include <memory>


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class DrawThread {
public:
    DrawThread(GameLogic& logic);
    ~DrawThread();
    void operator()();

private:
    GameLogic& game_logic;
    WNDCLASSEX wc;
    HWND hwnd;
    ID3D11Device* g_pd3dDevice;
    ID3D11DeviceContext* g_pd3dDeviceContext;
    IDXGISwapChain* g_pSwapChain;
    ID3D11RenderTargetView* g_mainRenderTargetView;
    OnScreenKeyboard onScreenKb;
    ScoreBoard scoreBoard;

    bool CreateDeviceD3D();
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    void RenderFrame();

    // Game UI elements
    char inputBuffer[6] = "";
    std::string errorMessage;
    bool showError = false;
    bool invalidWord = false;
    char userName[50] = "";
    bool showNamePopup = false;

    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool showScores = false;

};
