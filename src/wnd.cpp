#include <wnd.h>
#include <raylib.h>
#include <rlImGui.h>
#include <ui.h>

void wnd::Run()
{
    InitWindow(800, 600, "ImDivert");
    SetTargetFPS(60);

    rlImGuiSetup(true);
    ui::Init();
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();
        ui::Render();
        rlImGuiEnd();
        EndDrawing();
    }
    ui::Shutdown();
    CloseWindow();    
}