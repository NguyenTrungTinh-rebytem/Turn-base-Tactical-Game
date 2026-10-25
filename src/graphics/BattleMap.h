#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/BattleMap.h  (2D nhin ngang)
#include "raylib.h"
#include <string>
#include <vector>

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

    // Doi stage. Co anh nen thi nap, khong co thi ve nen thu tuc (mau khac nhau theo Stage).
    void setStage(StageId stage, const std::string& backgroundPath = "");
    StageId getStage() const { return m_stage; }
    bool hasBackground() const { return m_hasBackground; }

    // Giai phong texture nen. Core goi trong shutdown(), TRUOC CloseWindow(). An toan khi goi nhieu lan.
    void unloadAll();

    void draw() const;                                        // goi dau tien trong BeginDrawing()
    void drawShadow(const BattleSlot& slot, float frameWidth, float alpha = 1.0f) const;

    // ---- Bo cuc ca doi hinh (KHUYEN DUNG): 1-4 don vi/phe, tu dong khong de nhau ----
    // scaleMuls[i] = he so to cua don vi i (Boss = 1.3f). Ket qua cung thu tu.
    // Index 0 = tuyen dau (gan giua man hinh nhat). Dong qua se thu nho dong deu de vua man hinh.
    std::vector<BattleSlot> layoutPlayers(const std::vector<float>& scaleMuls) const;
    std::vector<BattleSlot> layoutEnemies(const std::vector<float>& scaleMuls) const;

    // ---- API cu (van dung duoc): chi biet scale cua chinh no, cac don vi khac coi nhu 1.0 ----
    BattleSlot getPlayerSlot(int index, int count, float scaleMul = 1.0f) const;
    BattleSlot getEnemySlot(int index, int count, float scaleMul = 1.0f) const;

    static Vector2 toTopLeft(const BattleSlot& slot, int frameW, int frameH);
    static Vector2 getHeadPos(const BattleSlot& slot, int frameH);

private:
    std::vector<BattleSlot> layoutSide(const std::vector<float>& scaleMuls, bool isPlayer) const;
    BattleSlot pickSlot(int index, int count, bool isPlayer, float scaleMul) const;
    float groundY() const;
    void drawProceduralBackground() const;

    StageId   m_stage = StageId::STAGE_1;
    Texture2D m_background{};
    bool      m_hasBackground = false;
};