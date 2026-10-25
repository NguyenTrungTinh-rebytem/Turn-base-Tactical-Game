#ifdef GRAPHICS_STANDALONE_TEST
// File test doc lap. Build chinh KHONG dinh nghia GRAPHICS_STANDALONE_TEST nen file nay rong.
// Test: g++ -std=c++17 -DGRAPHICS_STANDALONE_TEST src/graphics/main_sprite_test.cpp src/graphics/SpriteAnimation.cpp -o test.exe -lraylib -lopengl32 -lgdi32 -lwinmm
// Member 4 - Huynh Quang Khai | Sprint 1: Window + Sprite Sheet + Idle Animation
// File test doc lap: chi can SpriteAnimation + Raylib. KHONG la main.cpp cua game.
#include "raylib.h"
#include "SpriteAnimation.h"
#include <memory>

int test_sprite_main() {
    constexpr int SCREEN_W = 1280;
    constexpr int SCREEN_H = 720;
    InitWindow(SCREEN_W, SCREEN_H, "[THE FIRST STEP] - Sprint 1: Sprite Test");
    SetTargetFPS(60);

    // ---- Cau hinh sprite sheet (sua cho khop asset cua Director) ----
    const std::string SHEET_PATH = "assets/images/test_sheet.png";
    constexpr int FRAME_W = 64;
    constexpr int FRAME_H = 64;
    constexpr float SCALE = 5.0f;

    auto hero = std::make_unique<SpriteAnimation>();
    bool loaded = hero->load(SHEET_PATH, FRAME_W, FRAME_H);
    // Neu sheet cua Director khac mac dinh, chinh tai day:
    // hero->setClip(AnimState::IDLE, { /*row*/0, /*frames*/4, /*sec*/0.18f, /*loop*/true });
    hero->play(AnimState::IDLE);

    bool flipX = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Phim test: 1/2/3 doi trang thai, F lat ngang
        if (IsKeyPressed(KEY_ONE))   hero->play(AnimState::IDLE);
        if (IsKeyPressed(KEY_TWO))   hero->play(AnimState::ATTACK, true);
        if (IsKeyPressed(KEY_THREE)) hero->play(AnimState::TAKE_DAMAGE, true);
        if (IsKeyPressed(KEY_F))     flipX = !flipX;

        hero->update(dt);

        BeginDrawing();
        ClearBackground(Color{ 24, 26, 40, 255 });

        // Nen san dau don gian
        DrawRectangle(0, 480, SCREEN_W, SCREEN_H - 480, Color{ 50, 60, 70, 255 });
        DrawLine(0, 480, SCREEN_W, 480, GRAY);

        // Sprite chinh (bam day chan xuong mat dat)
        float w = FRAME_W * SCALE, h = FRAME_H * SCALE;
        hero->drawFrame({ 300.0f, 480.0f - h }, flipX, SCALE);

        // Xem truoc ca sprite sheet + khung danh dau frame hien tai
        if (hero->isLoaded()) {
            const Texture2D& tex = hero->getTexture();
            Vector2 origin = { 760.0f, 60.0f };
            DrawText("SPRITE SHEET", (int)origin.x, (int)origin.y - 24, 18, GOLD);
            DrawRectangle((int)origin.x - 4, (int)origin.y - 4, tex.width + 8, tex.height + 8, Color{ 10, 10, 10, 255 });
            DrawTextureV(tex, origin, WHITE);
            DrawRectangleLinesEx(
                { origin.x + hero->getCurrentFrame() * FRAME_W, origin.y + hero->getCurrentRow() * FRAME_H,
                  (float)FRAME_W, (float)FRAME_H }, 2.0f, RED);
        }

        // Thong tin debug
        DrawText(loaded ? "Sprite sheet: LOADED" : "Sprite sheet: NOT FOUND (dang ve placeholder)",
                 20, 20, 20, loaded ? GREEN : ORANGE);
        DrawText(TextFormat("Frame: %d | Row: %d | FPS: %d", hero->getCurrentFrame(), hero->getCurrentRow(), GetFPS()),
                 20, 50, 20, RAYWHITE);
        DrawText("[1] IDLE  [2] ATTACK  [3] TAKE_DAMAGE  [F] Flip  [ESC] Thoat", 20, SCREEN_H - 35, 18, LIGHTGRAY);

        EndDrawing();
    }

    hero.reset();      // giai phong texture TRUOC khi dong cua so
    CloseWindow();
    return 0;
}

int main() { return test_sprite_main(); }
#endif // GRAPHICS_STANDALONE_TEST