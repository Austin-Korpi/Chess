// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#define GL_SILENCE_DEPRECATION
#include <glfw3.h> // Will drag system OpenGL headers
#include <vector>

#include "MCTS.h"
#include "Pieces.h"
#include "Game.h"
#include "Engine.h"

#include <valgrind/callgrind.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(960, 960, "Chess, but its on linux", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //Load images
    const char* paths[] = { "Images/Chess_plt60.png",
                            "Images/Chess_nlt60.png",
                            "Images/Chess_blt60.png",
                            "Images/Chess_rlt60.png",
                            "Images/Chess_qlt60.png",
                            "Images/Chess_klt60.png",
                            "Images/Chess_pdt60.png",
                            "Images/Chess_ndt60.png",
                            "Images/Chess_bdt60.png",
                            "Images/Chess_rdt60.png",
                            "Images/Chess_qdt60.png",
                            "Images/Chess_kdt60.png",
                            "Images/blank.png",};

    GLuint pieceTextures[13]{};
    int my_image_width = 0;
    int my_image_height = 0;
    for (int i = 0; i < 13; i++) {
        LoadTextureFromFile(paths[i], &pieceTextures[i], &my_image_width, &my_image_height);
    }

     //Initialize Game Object
    Game game = Game();
    std::vector<Game> moveLog;
    moveLog.push_back(game);
    //State variable to hold the current selection
    Position selection{-1, -1};
    Position moves[27];
    std::string winner = "";
    int takeTurn = 5;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        style.WindowBorderSize = 0.0f;
        ImGui::Begin("Chess", nullptr, ImGuiWindowFlags_NoDecoration);
        
        
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));


        if (selection.x > 0) {
            for (int i = 0; i < 27; i++) {
                moves[i] = { -1, -1 };
            }
            // game.board[selection.y][selection.x]->find_valid_moves(game, moves);
        }
        else {
            moves[0] = { -1, -1 };
        }
        for (int y = 0; y < 8; y++)
            for (int x = 0; x < 8; x++)
            {
                if (x > 0)
                    ImGui::SameLine();
                ImGui::PushID(y * 8 + x);

                Piece* piece = game.board[y][x];

                //Coloring
                ImVec4 color;
                if (selection == Position{ x, y }) {
                    color = (ImVec4)ImColor::HSV(0.0f, 0.8f, 1.0f);
                }
                else{
                    color = (y + x) % 2 ? (ImVec4)ImColor::HSV(0.35f, 0.42f, 0.58f) :
                    (ImVec4)ImColor::HSV(0.24f, 0.12f, 0.93f);
                }
                for (int i = 0; moves[i].x != -1; i++) {
                    if (moves[i] == Position {x, y}) {
                        color = (ImVec4)ImColor::HSV(0.21f, .4f, 1.0f);
                        break;
                    }
                }
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

                // Picture selection
                int textureIndex = 0;
                if (piece == NULL) {
                    textureIndex = 12;
                }
                else {
                    textureIndex = piece->type;
                    if (!piece->white) {
                        textureIndex += 6;
                    }
                }

                // Button click
                if (ImGui::ImageButton("", (void*)(intptr_t)pieceTextures[textureIndex], ImVec2(120.0f, 120.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), color, ImVec4(1.0f, 1.0f, 1.0f, 1.0f))) {
                    if (piece != NULL && piece->white == game.turn) {
                        selection = { x, y };
                    }
                    // Move
                    else if (selection.x >= 0) {
                        bool ret = game.log_move({selection, {x,y}});
                        selection = { -1, -1 };
                        //Switch turns
                        if (ret) {
                            std::string message = game.switch_turns();
                            moveLog.push_back(game);
                            if (message != "") {
                                winner = message;
                            }
                        }
                    }
                    else {
                        selection = { -1, -1 };
                    }
                }
                ImGui::PopStyleColor();                
                ImGui::PopID();
            }

        // Game Over Pop Up
        if (winner != "") {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 150));
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
            ImGui::Begin("Game Over!", nullptr, window_flags);

            ImGui::Text("%s", winner.c_str());
            ImGui::End();
            ImGui::PopStyleVar();
        }

        ImGui::PopStyleVar(4);
        ImGui::End();


        //Undo
        if (ImGui::IsMouseReleased(1)) {
            if(moveLog.size() > 2){
                moveLog.pop_back();
                moveLog.pop_back();
                game = moveLog.back();
                selection = { -1, -1 };
                winner = "";
            }
        }

        //Engine Turn
        if (ImGui::IsMouseReleased(0)) {
            takeTurn = 5;
        }
        if (game.turn == false) {
            if (takeTurn > 0) {
                takeTurn--;
            }
            else if(winner == ""){
                // take_move(game);
                take_move_fast(game);
                // monte_carlo(game);
                winner = game.switch_turns();
                moveLog.push_back(game);
                takeTurn = false;
                // usleep(200000);
            }
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}