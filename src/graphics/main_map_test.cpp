// Member 4 - Huynh Quang Khai | Test Map + Slot 2D (doc lap, khong can Character)
// Phim: 1 = Stage 1 (3 vs 3) | 2 = Stage 2 (3 vs Boss) | F1 = hien diem slot | ESC = thoat
#include "raylib.h"
#include "BattleMap.h"
#include "SpriteAnimation.h"
#include <memory>
#include <vector>

struct Actor {
    std::unique_ptr<SpriteAnimation> m_anim;
    BattleSlot m_slot;
    Color m_tint = WHITE;
};

static std::vector<Actor> buildStage(const BattleMap& map, StageId stage) {
    std::vector<Actor> actors;
    auto add = [&](const BattleSlot& slot, Color tint) {
        Actor a;
        a.m_anim = std::make_unique<SpriteAnimation>();
        a.m_anim->load("assets/images/test_sheet.png", 64, 64);
        a.m_anim->play(AnimState::IDLE);
        a.m_slot = slot;
        a.m_tint = tint;
        actors.push_back(std::move(a));
    };
    const Color heroTints[3] = { WHITE, Color{ 180, 255, 180, 255 }, Color{ 255, 220, 150, 255 } };
    for (int i = 0; i < 3; ++i) add(map.getPlayerSlot(i, 3), heroTints[i]);

    if (stage == StageId::STAGE_1) {
        for (int i = 0; i < 3; ++i) add(map.getEnemySlot(i, 3), Color{ 255, 150, 150, 255 });
    } else {
        add(map.getEnemySlot(0, 1, 1.8f), Color{ 200, 90, 220, 255 });   // Boss to gap 1.8 lan
    }
    return actors;
}

int test_map_main(){
    InitWindow(1280, 720, "[THE FIRST STEP] - Map Test");
    SetTargetFPS(60);

    BattleMap map;
    map.setStage(StageId::STAGE_1, "assets/images/bg_stage1.png");
    std::vector<Actor> actors = buildStage(map, StageId::STAGE_1);
    bool showSlots = false;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ONE)) { map.setStage(StageId::STAGE_1, "assets/images/bg_stage1.png"); actors = buildStage(map, StageId::STAGE_1); }
        if (IsKeyPressed(KEY_TWO)) { map.setStage(StageId::STAGE_2, "assets/images/bg_stage2.png"); actors = buildStage(map, StageId::STAGE_2); }
        if (IsKeyPressed(KEY_F1)) showSlots = !showSlots;

        for (auto& a : actors) a.m_anim->update(GetFrameTime());

        BeginDrawing();
        ClearBackground(BLACK);
        map.draw();
        for (Actor& actor : actors) {
            Actor* a = &actor;
            map.drawShadow(a->m_slot, 64.0f);
            a->m_anim->drawFrame(BattleMap::toTopLeft(a->m_slot, 64, 64), a->m_slot.m_flipX, a->m_slot.m_scale, a->m_tint);
            if (showSlots) {
                DrawCircleV(a->m_slot.m_feetPos, 4, RED);
                Vector2 h = BattleMap::getHeadPos(a->m_slot, 64);
                DrawCircleV(h, 4, YELLOW);
            }
        }
        DrawText(map.hasBackground() ? "Background: IMAGE" : "Background: PROCEDURAL (chua co anh)", 20, 20, 20, RAYWHITE);
        DrawText("[1] Stage 1  [2] Stage 2 (Boss)  [F1] Slot  [ESC] Thoat", 20, 690, 18, LIGHTGRAY);
        EndDrawing();
    }
    actors.clear();   // giai phong texture truoc CloseWindow
    CloseWindow();
    return 0;
}
