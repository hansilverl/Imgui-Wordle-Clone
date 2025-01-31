#include "DrawThread.h"
#include <stdexcept>
#include "Colors.h"

DrawThread::DrawThread(GameLogic& logic) : game_logic(logic), scoreBoard("scores.txt") {
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

    // Medium font for Keyboard
    io.Fonts->AddFontFromFileTTF(fontPath, 25.0f);
    // Add a smaller font for the "Enter" button
    io.Fonts->AddFontFromFileTTF(fontPath, 14.0f);
    // Backspace font
    io.Fonts->AddFontFromFileTTF("../../assets/CustomFont.ttf", 23.0f);
    // High Score font
    io.Fonts->AddFontFromFileTTF("../../assets/CustomFont.ttf", 49.0f);

    // Setup style
    ImGui::StyleColorsDark();
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = BACKGROUND_COLOR;

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
        if (game_logic.hasWon()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            showNamePopup = true;
        }
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
    ImVec2 boardPos((displaySize.x - boardSize.x) * 0.5f, 80); // Center the board horizontally and position it vertically

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
                color = GREEN_COLOR; // Use the defined green color
            else if (letter.in_word)
                color = YELLOW_COLOR; // Use the defined yellow color
            else
                color = WRONG_COLOR; // Use the defined gray color

            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color); // Make hover the same color
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color); // Make active the same color
            ImGui::PushStyleColor(ImGuiCol_Border, color); // Colored cell border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            ImGui::Button(std::string(1, letter.letter).c_str(), ImVec2(60, 60));
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
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
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent button hover
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0)); // Transparent button active
            ImGui::PushStyleColor(ImGuiCol_Border, WRONG_COLOR); // Active cell border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            if (i < strlen(inputBuffer)) {
                ImGui::Button(std::string(1, inputBuffer[i]).c_str(), ImVec2(60, 60));
            }
            else {
                ImGui::Button(" ", ImVec2(60, 60));
            }
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
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
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0)); // Transparent button hover
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0)); // Transparent button active
            ImGui::PushStyleColor(ImGuiCol_Border, WRONG_COLOR); // Empty cells border
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
            ImGui::Button(" ", ImVec2(60, 60)); // Adjust button size to fit the new board size
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);
            ImGui::SameLine(0, 8); // Adjust the second parameter to change the spacing
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rowSpacing);
        ImGui::NewLine();
    }

    ImGui::End();

    // Handle keyboard input
    if (!game_logic.isGameOver()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && strlen(inputBuffer) > 0) {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Enter) && strlen(inputBuffer) == 5) {
            if (!game_logic.submitGuess(inputBuffer)) {
                invalidWord = true;
            }
        }
        for (int i = ImGuiKey_A; i <= ImGuiKey_Z; ++i) {
            if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(i)) && strlen(inputBuffer) < 5) {
                inputBuffer[strlen(inputBuffer)] = static_cast<char>(i - ImGuiKey_A + 'A');
            }
        }
    }

    // Draw the on-screen keyboard
    onScreenKb.Render(inputBuffer, invalidWord, game_logic, boardSize);

    // Game over message
    if (game_logic.isGameOver()) {
        ImGui::SetNextWindowPos(ImVec2((displaySize.x - 600) * 0.5f, boardPos.y + boardSize.y + 180));
        ImGui::Begin("##GameOver", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
        ImGui::TextColored(
            game_logic.hasWon() ? GREEN_COLOR : ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
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
        ImGui::Text("Please try again.");
        ImGui::Dummy(ImVec2(10.0f, 10.0f)); // Add dummy space
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("OK").x) * 0.5f);
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Ask for the user's name if the game is won

    if (showNamePopup) {
        ImGui::OpenPopup("Enter Name");
        showNamePopup = false;
    }

    if (ImGui::BeginPopupModal("Enter Name", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText(" ", userName, IM_ARRAYSIZE(userName));
        ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Add dummy space
        if (ImGui::Button("OK")) {
            scoreBoard.addScore(std::string(userName), static_cast<int>(history.size()));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Render high scores button
    scoreBoard.renderHighScoresButton(displaySize);

    // Render high scores popup
    scoreBoard.renderHighScoresPopup();

    // Rendering
    ImGui::Render();
    const float clear_color_with_alpha[4] = { BACKGROUND_COLOR.x, BACKGROUND_COLOR.y, BACKGROUND_COLOR.z, BACKGROUND_COLOR.w }; // Background color
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
