#pragma once
// ============================================================================
//  LayeredSprite - ve chong 2 lop: Than (theo Race) + Giap/Vu khi (theo Class)
//  Hai lop dung CHUNG mot bo dem frame nen luon dong bo.
//
//  Quy uoc file (moi animation = 1 dai ngang, o 128x128, chan xuong cung 1 muc):
//    assets/images/body_<race>_<anim>.png     vd: body_elf_attack.png
//    assets/images/armor_<class>_<anim>.png   vd: armor_mage_idle.png
//  <anim> = idle | attack | hurt | death | victory
//  Ca 2 lop cua cung 1 animation PHAI co cung so frame.
// ============================================================================
#include <array>
#include <map>
#include <memory>
#include <string>
#include "raylib.h"
#include "CharacterRules.h"

enum class LayerAnim { IDLE = 0, ATTACK, HURT, DEATH, VICTORY, COUNT };

struct LayerAnimInfo {
    int   frames;   // so frame trong dai
    float fps;      // toc do phat
    bool  loop;     // false = dung o frame cuoi (vd. DEATH)
};

constexpr int LAYER_ANIM_COUNT = static_cast<int>(LayerAnim::COUNT);
constexpr int SPRITE_FRAME_SIZE = 128;

const LayerAnimInfo& getLayerAnimInfo(LayerAnim anim);
const char* layerAnimToString(LayerAnim anim);

// Cache texture dung chung de doi race/class khong phai nap lai file.
// Goi LayerTextureCache::clear() TRUOC CloseWindow().
class LayerTextureCache {
public:
    static std::shared_ptr<Texture2D> get(const std::string& path);
    static void clear();
private:
    static std::map<std::string, std::shared_ptr<Texture2D>>& storage();
};

class LayeredSprite {
public:
    // Doi race/class. Tra ve false (va GIU NGUYEN lua chon cu) neu to hop bi cam.
    bool setLoadout(RaceType race, ClassType pClass);

    // Chi doi than (giu nguyen giap) / chi doi giap (giu nguyen than).
    // Van kiem tra to hop hop le; tra ve false neu bi cam.
    bool setRace(RaceType race);
    bool setClass(ClassType pClass);

    // Chuyen animation. restart = false: neu dang phat animation do thi bo qua.
    void play(LayerAnim anim, bool restart = false);

    void update(float deltaTime);

    // pos = goc tren-trai cua o 128x128 (sau khi nhan scale).
    void draw(Vector2 pos, bool flipX = false, float scale = 1.0f, Color tint = WHITE) const;

    bool      isFinished() const;   // animation khong lap da toi frame cuoi
    int       getFrame() const    { return m_frame; }
    LayerAnim getAnim() const     { return m_anim; }
    RaceType  getRace() const     { return m_race; }
    ClassType getClass() const    { return m_class; }
    bool      isReady() const     { return m_hasLoadout; }
    bool      hasTextures() const;   // co it nhat 1 anh (than hoac giap) da nap duoc
    void      unload();              // tha tham chieu texture (goi truoc CloseWindow)

private:
    using TexPtr = std::shared_ptr<Texture2D>;

    void reloadBody();
    void reloadArmor();
    int  frameCountFor(LayerAnim anim) const;   // an toan neu anh co it frame hon du kien
    static void drawLayer(const TexPtr& tex, int frame, Vector2 pos,
                          bool flipX, float scale, Color tint);

    RaceType  m_race  = RaceType::HUMAN;
    ClassType m_class = ClassType::SWORDMAN;
    bool      m_hasLoadout = false;

    std::array<TexPtr, LAYER_ANIM_COUNT> m_bodyTex{};
    std::array<TexPtr, LAYER_ANIM_COUNT> m_armorTex{};

    LayerAnim m_anim  = LayerAnim::IDLE;
    int       m_frame = 0;
    float     m_timer = 0.0f;
};
