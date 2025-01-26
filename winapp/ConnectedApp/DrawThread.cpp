#include "DrawThread.h"
#include <stdexcept>

DrawThread::DrawThread(GameLogic& logic) : game_logic(logic) {
    // Create application window
    wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Wordle"), NULL };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindow(wc.lpszClassName, _T("Wordle"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 800, NULL, NULL, wc.hInstance, NULL);

    // Maximize the window
    ::ShowWindow(hwnd, SW_MAXIMIZE);
    ::UpdateWindow(hwnd);

    // Initialize Direct3D
    if (!CreateDeviceD3D()) {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        throw std::runtime_error("Failed to initialize Direct3D");
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    const char* fontPath = "../../assets/HelveticaNeue Bold.ttf";

    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 49.0f);
    IM_ASSERT(font != nullptr);


    // Setup style
    ImGui::StyleColorsDark();
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f); // background color  #121213
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);

    // Register error callback
    game_logic.setOnErrorOccurred([this](const std::string& error) {
        errorMessage = error;
        showError = true;
        });

    // Register invalid word callback
    game_logic.setOnInvalidWord([this]() {
        invalidWord = true;
        });

    // Register game state changed callback
    game_logic.setOnGameStateChanged([this]() {
        memset(inputBuffer, 0, sizeof(inputBuffer));
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

    // Get the size of the display
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Calculate the position and size for the game board
    ImVec2 boardSize(400, 480); // Adjusted the size to make room for the onscreen keyboard
    ImVec2 boardPos((displaySize.x - boardSize.x) * 0.5f, 50); // Center the board horizontally and position it vertically

    float rowSpacing = 4.f;
    // Draw the game board
    ImGui::SetNextWindowPos(boardPos);
    ImGui::SetNextWindowSize(boardSize);
    ImGui::Begin("##GameBoard", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

    // Game grid
    const auto& history = game_logic.getGuessHistory();
    for (const auto& guess : history) {
        for (const auto& letter : guess.letter_states) {
            ImVec4 color;
            if (letter.correct_position)
                color = ImVec4(83.0f / 255.0f, 141.0f / 255.0f, 78.0f / 255.0f, 1.0f); // Green
            else if (letter.in_word)
                color = ImVec4(181.0f / 255.0f, 159.0f / 255.0f, 59.0f / 255.0f, 1.0f); // Yellow
            else
                color = ImVec4(58.0f / 255.0f, 58.0f / 255.0f, 60.0f / 255.0f, 1.0f); // Gray

            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_Border, color); // colored cell border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            ImGui::Button(std::string(1, letter.letter).c_str(), ImVec2(60, 60)); 
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 8); 
        }
        ImGui::NewLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rowSpacing);
    }

    // Fill remaining rows with empty squares or current input
    size_t currentRow = history.size();
    if (!game_logic.isGameOver() && currentRow < 6) {
        for (size_t i = 0; i < 5; ++i) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent button (aka empty square)
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(58.0f / 255.0f, 58.0f / 255.0f, 60.0f / 255.0f, 1.0f)); // active cell border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            if (i < strlen(inputBuffer)) {
                ImGui::Button(std::string(1, inputBuffer[i]).c_str(), ImVec2(60, 60)); 
            }
            else {
                ImGui::Button(" ", ImVec2(60, 60)); 
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 8); 
        }
        ImGui::NewLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rowSpacing);
        currentRow++;
    }

    // Fill remaining rows with empty squares
    for (size_t i = currentRow; i < 6; i++) {
        for (int j = 0; j < 5; j++) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0)); // Transparent button
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(58.0f / 255.0f, 58.0f / 255.0f, 60.0f / 255.0f, 1.0f)); // empty cells border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            ImGui::Button(" ", ImVec2(60, 60)); // Adjust button size to fit the new board size
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 8); // Adjust the second parameter to change the spacing
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rowSpacing);
        ImGui::NewLine();
    }

    ImGui::End();

    // Input field
    if (!game_logic.isGameOver()) {
        ImGui::SetNextWindowPos(ImVec2((displaySize.x - 300) * 0.5f, boardPos.y + boardSize.y + 20));
        ImGui::SetNextWindowSize(ImVec2(300, 50));
        ImGui::Begin("##InputField", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
        ImGui::SetNextItemWidth(200);   // temporary input text field
        if (ImGui::InputText("##input", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (strlen(inputBuffer) == 5) {
                if (!game_logic.submitGuess(inputBuffer)) {
                    invalidWord = true;
                }
            }
        }
        ImGui::End();
    }

    // Game over message
    if (game_logic.isGameOver()) {
        ImGui::SetNextWindowPos(ImVec2((displaySize.x - 300) * 0.5f, boardPos.y + boardSize.y + 80));
        ImGui::SetNextWindowSize(ImVec2(300, 50));
        ImGui::Begin("##GameOver", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
        ImGui::TextColored(
            game_logic.hasWon() ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
            game_logic.hasWon() ? "Congratulations! You've won!" :
            ("Game Over! The word was: " + game_logic.getCurrentAnswer()).c_str()
        );
        ImGui::End();
    }

    // Error popup
    if (showError) {
        ImGui::OpenPopup("Error");
        showError = false;
    }

    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", errorMessage.c_str());
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Invalid word message
    if (invalidWord) {
        ImGui::OpenPopup("Invalid Word");
        invalidWord = false;
    }

    if (ImGui::BeginPopupModal("Invalid Word", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Invalid word. Please try again.");
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.07f, 0.07f, 0.08f, 1.00f }; // Background color #121213
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
