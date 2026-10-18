#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/BattleMap.h  (phien ban 2D nhin ngang)
#include "raylib.h"
#include <string>

enum class StageId { STAGE_1 = 0, STAGE_2 };

// Vi tri dung cua 1 nhan vat tren san dau 2D
struct BattleSlot {
    Vector2 m_feetPos{};      // diem cham dat (giua day chan)
    float   m_scale = 1.0f;
    bool    m_flipX = false;  // quai o ben phai quay mat sang trai
};

class BattleMap {
public:
    BattleMap() = default;
    ~BattleMap();
    BattleMap(const BattleMap&) = delete;
    BattleMap& operator=(const BattleMap&) = delete;

    // Doi stage. Co anh nen thi nap, khong co thi ve nen thu tuc (placeholder).
    void setStage(StageId stage, const std::string& backgroundPath = "");
    StageId getStage() const { return m_stage; }
    bool hasBackground() const { return m_hasBackground; }

    void draw() const;                                        // goi dau tien trong BeginDrawing()
    void drawShadow(const BattleSlot& slot, float frameWidth) const;

    // index 0 = dung gan giua man hinh nhat (tuyen dau). scaleMul > 1 cho Boss.
    BattleSlot getPlayerSlot(int index, int count, float scaleMul = 1.0f) const;
    BattleSlot getEnemySlot(int index, int count, float scaleMul = 1.0f) const;

    static Vector2 toTopLeft(const BattleSlot& slot, int frameW, int frameH);
    static Vector2 getHeadPos(const BattleSlot& slot, int frameH);   // cho BattleUI::setAnchor()

private:
    BattleSlot makeSlot(int index, int count, bool isPlayer, float scaleMul) const;
    float groundY() const;
    void drawProceduralBackground() const;

    StageId   m_stage = StageId::STAGE_1;
    Texture2D m_background{};
    bool      m_hasBackground = false;
};