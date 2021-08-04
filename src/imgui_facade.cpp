#include "imgui_facade.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace {

void setup_style_colors(void) {
    ImGui::StyleColorsDark();

#if 1
    // Reference: https://github.com/ocornut/imgui/issues/707
    // clang-format off

    ImVec4* colors = ImGui::GetStyle().Colors;

    #define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
    #define XHI(v)  ImVec4(0.802f, 0.075f, 0.256f, v)
    #define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
    #define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
    #define BKGD(v) ImVec4(0.200f, 0.220f, 0.270f, v)
    #define TEXT(v) ImVec4(0.860f, 0.930f, 0.890f, v)

    colors[ImGuiCol_Tab                 ] = HI(1.0f);
    colors[ImGuiCol_TabActive           ] = XHI(0.50f);
    colors[ImGuiCol_TabHovered          ] = XHI(1.0f);
    colors[ImGuiCol_TabUnfocused        ] = LOW(1.00f);
    colors[ImGuiCol_TabUnfocusedActive  ] = MED(0.60f);
    colors[ImGuiCol_DockingPreview      ] = HI(1.00f);
    colors[ImGuiCol_DragDropTarget      ] = BKGD(1.00f);
    colors[ImGuiCol_DockingEmptyBg      ] = BKGD(1.00f);
    colors[ImGuiCol_Text                ] = TEXT(0.78f);
    colors[ImGuiCol_TextDisabled        ] = TEXT(0.28f);
    colors[ImGuiCol_WindowBg            ] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg             ] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg             ] = BKGD(0.9f);
    colors[ImGuiCol_Border              ] = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
    colors[ImGuiCol_BorderShadow        ] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg             ] = BKGD(1.00f);
    colors[ImGuiCol_FrameBgHovered      ] = MED(0.78f);
    colors[ImGuiCol_FrameBgActive       ] = MED(1.00f);
    colors[ImGuiCol_TitleBg             ] = LOW(1.00f);
    colors[ImGuiCol_TitleBgActive       ] = HI(1.00f);
    colors[ImGuiCol_TitleBgCollapsed    ] = BKGD(0.75f);
    colors[ImGuiCol_MenuBarBg           ] = BKGD(0.47f);
    colors[ImGuiCol_ScrollbarBg         ] = BKGD(1.00f);
    colors[ImGuiCol_ScrollbarGrab       ] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = MED(0.78f);
    colors[ImGuiCol_ScrollbarGrabActive ] = MED(1.00f);
    colors[ImGuiCol_CheckMark           ] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_SliderGrab          ] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    colors[ImGuiCol_SliderGrabActive    ] = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_Button              ] = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
    colors[ImGuiCol_ButtonHovered       ] = MED(0.86f);
    colors[ImGuiCol_ButtonActive        ] = MED(1.00f);
    colors[ImGuiCol_Header              ] = MED(0.76f);
    colors[ImGuiCol_HeaderHovered       ] = MED(0.86f);
    colors[ImGuiCol_HeaderActive        ] = HI(1.00f);
    colors[ImGuiCol_ResizeGrip          ] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
    colors[ImGuiCol_ResizeGripHovered   ] = MED(0.78f);
    colors[ImGuiCol_ResizeGripActive    ] = MED(1.00f);
    colors[ImGuiCol_PlotLines           ] = TEXT(0.63f);
    colors[ImGuiCol_PlotLinesHovered    ] = MED(1.00f);
    colors[ImGuiCol_PlotHistogram       ] = TEXT(0.63f);
    colors[ImGuiCol_PlotHistogramHovered] = MED(1.00f);
    colors[ImGuiCol_TextSelectedBg      ] = MED(0.43f);
    colors[ImGuiCol_Border              ] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);

    #undef TEXT
    #undef BKGD
    #undef LOW
    #undef MED
    #undef XHI
    #undef HI

    // clang-format on
#endif
}

} // anonymous namespace

void init_imgui(GLFWwindow* window) {
    assert(window != nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    setup_style_colors();

    ImGui_ImplGlfw_InitForOpenGL(window, /*install_callbacks*/ true);
    ImGui_ImplOpenGL3_Init();
}

void deinit_imgui(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void begin_imgui_frame(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void end_imgui_frame(void) {
    ImGui::Render();
    // int width, height;
    // glfwGetFramebufferSize(window, &width, &height);
    // glViewport(0, 0, width, height);
    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void show_imgui_demo_window(void) {
    ImGui::ShowDemoWindow();
}

void imgui_config_mouse(bool should_capture) {
    ImGuiIO& io = ImGui::GetIO();
    if (should_capture) {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    } else {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
}
