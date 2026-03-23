
#include <array>

#include <imgui.h>
#include <raylib-cpp/raylib-cpp.hpp>
#include <rlImGui.h>

#include "calc2.hpp"
#include "raylib.h"
#include "util.hpp"

static constexpr int  screen_width  = 400;
static constexpr int  screen_height = 600;
static constexpr auto screen_title  = "Calc2";

auto main() -> int
{

   // constexpr auto input = ":sq . *;"
   //                        ": hypot sq ~ sq + sqrt ;"
   //                        "3 4 hypot ";

   // std::cout << run_calc(input);
   // Initialization
   //--------------------------------------------------------------------------------------
   //
   std::string            output_text {};
   std::array<char, 2048> input_buffer {};

   SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
   auto window = raylib::Window(screen_width, screen_height, screen_title);
   SetTargetFPS(30);

   rlImGuiSetup(true);
   scope_exit(rlImGuiShutdown());

   // Main game loop
   while (!window.ShouldClose()) // Detect window close button or ESC key
   {

      window.BeginDrawing();
      {
         window.ClearBackground(DARKGRAY);

         rlImGuiBegin();
         {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2((float)screen_width, (float)screen_height), ImGuiCond_Always);
            ImGui::Begin("Calculator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            // ========================= OUTPUT ====================================
            ImGui::BeginChild("Results", ImVec2(0, screen_height * 0.3819660112501453f), true);
            ImGui::Text("Output: ");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

            ImVec2 const out_avail = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("OutputText", ImVec2(out_avail.x, out_avail.y), false, ImGuiWindowFlags_HorizontalScrollbar);

            ImVec2 const inner_avail = ImGui::GetContentRegionAvail();

            ImGui::InputTextMultiline("##output", output_text.data(),
                                      static_cast<int>(output_text.size()) + 1, ImVec2(inner_avail.x, inner_avail.y),
                                      ImGuiInputTextFlags_ReadOnly);

            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::EndChild();

            // ====================== INPUT WINDOW ======================
            ImGui::BeginChild("Editor", ImVec2(0, 0), true);
            ImGui::Text("Input: ");
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

            ImVec2 const avail         = ImGui::GetContentRegionAvail();
            float const  right_margin  = 0.0f;
            float const  bottom_margin = 2.0f;

            if (ImGui::InputTextMultiline("##editor",
                                          input_buffer.data(),
                                          sizeof(input_buffer),
                                          ImVec2(avail.x - right_margin,
                                                 avail.y - bottom_margin),
                                          ImGuiInputTextFlags_AllowTabInput)) {

               output_text = run_calc(input_buffer.data(), input_buffer.size());
            }

            ImGui::PopStyleVar(2);
            ImGui::EndChild();
            ImGui::End();
         }
         rlImGuiEnd();
      }
      window.EndDrawing();
   }

   return 0;
}