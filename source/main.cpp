
#include <array>

#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

#include "calc2.hpp"
#include "util.hpp"

#include "embedded_font.inc"

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
   InitWindow(screen_width, screen_height, screen_title);
   scope_exit(CloseWindow());
   SetTargetFPS(30);

   rlImGuiSetup(true);
   scope_exit(rlImGuiShutdown());

   ImGuiIO& io = ImGui::GetIO();
   ImGui::StyleColorsDark();

   ImFontConfig cfg {};
   cfg.OversampleH          = 2;
   cfg.OversampleV          = 1;
   cfg.PixelSnapH           = true;
   cfg.FontDataOwnedByAtlas = false;
   ImFont* mono             = io.Fonts->AddFontFromMemoryTTF(
     static_cast<void*>(JetBrainsMono_Regular_ttf),
     static_cast<int>(JetBrainsMono_Regular_ttf_len),
     18.0f, &cfg);

   while (!WindowShouldClose()) {

      BeginDrawing();
      scope_exit(EndDrawing());
      ClearBackground(DARKGRAY);
      {
         rlImGuiBegin();
         scope_exit(rlImGuiEnd());

         ImGui::PushFont(mono);
         scope_exit(ImGui::PopFont());

         ImGui::SetNextWindowPos(ImVec2 { 0, 0 }, ImGuiCond_Always);
         ImGui::SetNextWindowSize(ImVec2 { static_cast<float>(screen_width), static_cast<float>(screen_height) }, ImGuiCond_Always);
         ImGui::Begin("Calculator", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
         scope_exit(ImGui::End());

         // ========================= OUTPUT =========================
         {
            ImGui::BeginChild("Results", ImVec2 { 0, screen_height * 0.3819660112501453f }, ImGuiChildFlags_None);
            scope_exit(ImGui::EndChild());

            ImGui::Text("Output: ");
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4 { 0, 0, 0, 0 });
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 0, 0 });
            scope_exit(ImGui::PopStyleColor());
            scope_exit(ImGui::PopStyleVar());

            ImVec2 const out_avail = ImGui::GetContentRegionAvail();

            ImGui::BeginChild("OutputText", ImVec2 { out_avail.x, out_avail.y }, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
            scope_exit(ImGui::EndChild());

            ImVec2 const inner_avail = ImGui::GetContentRegionAvail();

            ImGui::InputTextMultiline("##output", output_text.data(),
                                      static_cast<int>(output_text.size()),
                                      ImVec2 { inner_avail.x, inner_avail.y },
                                      ImGuiInputTextFlags_ReadOnly);
         }

         // ====================== INPUT WINDOW ======================
         {
            ImGui::BeginChild("Editor", ImVec2 { 0, 0 }, ImGuiChildFlags_None);
            scope_exit(ImGui::EndChild());

            ImGui::Text("Input: ");
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2 { 4, 4 });
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 4, 4 });
            scope_exit(ImGui::PopStyleVar(2));

            ImVec2 const avail         = ImGui::GetContentRegionAvail();
            float const  right_margin  = 0.0f;
            float const  bottom_margin = 2.0f;

            if (ImGui::InputTextMultiline("##editor",
                                          input_buffer.data(),
                                          static_cast<int>(input_buffer.size()),
                                          ImVec2 { avail.x - right_margin, avail.y - bottom_margin },
                                          ImGuiInputTextFlags_AllowTabInput)) {

               output_text = run_calc(input_buffer.data(), input_buffer.size());
            }
         }
      }
   }

   return 0;
}