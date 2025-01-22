// DrawThread.cpp
#include "DrawThread.h"
#include "OnScreenKb.h"
#include <stdexcept>

DrawThread::DrawThread(GameLogic& logic) : game_logic(logic) {
    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Calculate center position for the window
    int windowWidth = 800;
    int windowHeight = 800;
    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    // Create application window
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Wordle"), NULL };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, _T("Wordle"), WS_OVERLAPPEDWINDOW, xPos, yPos, windowWidth, windowHeight, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D()) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        throw std::runtime_error("Failed to initialize Direct3D");
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Setup style
    ImGui::StyleColorsDark();

    // Register error callback
    game_logic.setOnErrorOccurred([this](const std::string& error) {
        errorMessage = error;
        showError = true;
        });
}

DrawThread::~DrawThread() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void DrawThread::operator()() {
    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }
        RenderFrame();
    }
}

void DrawThread::RenderFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Create main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Wordle", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings);

    // Define sizes
    const float cellSize = 40.0f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const float gridWidth = (5 * cellSize) + (4 * spacing);
    const float gridHeight = (6 * cellSize) + (5 * spacing);

    const ImVec2 windowSize = ImGui::GetIO().DisplaySize;
    const float offsetX = (windowSize.x - gridWidth) / 2.0f;
    const float offsetY = (windowSize.y - gridHeight) / 3.0f;

    // Center the grid
    ImGui::SetCursorPos(ImVec2(offsetX, offsetY));

    // Variables to track current input state
    static size_t currentRow = game_logic.getGuessHistory().size(); // Active row
    static size_t currentCol = 0;                                  // Current column in active row
    static std::string currentGuess(5, ' ');                       // Temporary guess for active row

    const auto& history = game_logic.getGuessHistory();

    // Render all 6 rows
    for (size_t i = 0; i < 6; i++) {
        ImGui::SetCursorPosX(offsetX); // Reset X position for each row

        if (i < history.size()) {
            // Render submitted guesses
            const auto& guess = history[i];
            for (size_t j = 0; j < 5; j++) {
                const auto& letter = guess.letter_states[j];
                ImVec4 color;
                if (letter.correct_position)
                    color = ImVec4(108.0f / 255.0f, 169.0f / 255.0f, 101.0f / 255.0f, 1.0f);
                else if (letter.in_word)
                    color = ImVec4(200.0f / 255.0f, 182.0f / 255.0f, 83.0f / 255.0f, 1.0f); // Yellow
                else
                    color = ImVec4(120.0f / 255.0f, 124.0f / 255.0f, 127.0f / 255.0f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui::Button(std::string(1, letter.letter).c_str(), ImVec2(cellSize, cellSize));
                ImGui::PopStyleColor(2);

                if (j < 4) ImGui::SameLine();
            }
        }
        else if (i == currentRow) {
            // Render current row (active input)
            for (size_t j = 0; j < 5; j++) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui::Button(currentGuess[j] == ' ' ? " " : std::string(1, currentGuess[j]).c_str(), ImVec2(cellSize, cellSize));
                ImGui::PopStyleColor(2);

                if (j < 4) ImGui::SameLine();
            }
        }
        else {
            // Render empty rows
            for (size_t j = 0; j < 5; j++) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
                ImGui::Button(" ", ImVec2(cellSize, cellSize));
                ImGui::PopStyleColor(2);

                if (j < 4) ImGui::SameLine();
            }
        }
        ImGui::NewLine();
    }

    // Handle keyboard input
    ImGuiIO& io = ImGui::GetIO();
    for (int key = ImGuiKey_A; key <= ImGuiKey_Z; key++) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key)) && currentCol < 5) {
            currentGuess[currentCol] = static_cast<char>('A' + (key - ImGuiKey_A));
            currentCol++;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && currentCol > 0) {
        currentCol--;
        currentGuess[currentCol] = ' ';
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter) && currentCol == 5) {
        if (game_logic.submitGuess(currentGuess)) {
            currentGuess = std::string(5, ' ');
            currentCol = 0;
            currentRow++;
        }
    }

    // Render the virtual keyboard
    RenderOnScreenKeyboard(offsetX, offsetY, gridWidth, gridHeight, cellSize, spacing, currentGuess, currentCol, currentRow, game_logic);

    // Game over message in a popup
    if (game_logic.isGameOver()) {
        // Open the popup
        ImGui::OpenPopup("Game Over");

        // Center the popup
        ImGui::SetNextWindowPos(ImVec2((windowSize.x - 300.0f) / 2.0f, (windowSize.y - 100.0f) / 2.0f), ImGuiCond_Always);

        if (ImGui::BeginPopupModal("Game Over", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Display the message
            ImGui::TextColored(
                game_logic.hasWon() ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
                game_logic.hasWon() ? "Congratulations! You've won!" :
                ("Game Over!"));

            // Add a button to close the popup
            if (ImGui::Button("OK", ImVec2(120, 40))) {
                ImGui::CloseCurrentPopup();
                PostQuitMessage(0);
            }

            ImGui::EndPopup();
        }
    }

    ImGui::End();

    // Render everything
    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}

// Implementation of D3D initialization methods
bool DrawThread::CreateDeviceD3D() {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void DrawThread::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void DrawThread::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void DrawThread::CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

LRESULT WINAPI DrawThread::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
    {
        DrawThread* pThis = reinterpret_cast<DrawThread*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis && pThis->g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            pThis->CleanupRenderTarget();
            pThis->g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            pThis->CreateRenderTarget();
        }
    }
    return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
