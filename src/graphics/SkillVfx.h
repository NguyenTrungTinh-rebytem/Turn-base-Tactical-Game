#pragma once
// src/graphics/SkillVfx.h
// Phat hieu ung ky nang (sprite sheet tu goi PVFX Foundry, CC0) len chien truong.
// SkillVfxPlayer CHI VE: khong chua logic game, khong include GameEngine.
// Du lieu: <dir>/skill_vfx.csv (sinh boi tools/vfx_import.py). Thieu file / thieu anh => bo qua, khong crash.
//
// Cach dung (GameEngine):
//   load()                      sau InitWindow
//   playSkill()                 khi cast skill           -> hieu ung 1 lan (main / support)
//   startStatus() + syncStatus  khi skill gay hieu ung   -> hieu ung lap, tu tat khi het status
//   update(dt) / draw()         moi frame (draw trong canh de an theo SceneShader)
//   clear()                     khi vao man moi (truoc khi xoa CombatantView)
//   unloadAll()                 truoc CloseWindow
#include "raylib.h"
#include "dsa/StatusEffect.h"    // StatusType, StatusEffectManager
#include <string>
#include <unordered_map>
#include <vector>

class CombatantView;

class SkillVfxPlayer {
public:
    enum class Tier { LIT, STRICT };   // LIT = co anh sang; STRICT = palette han che

    SkillVfxPlayer() = default;
    ~SkillVfxPlayer();
    SkillVfxPlayer(const SkillVfxPlayer&) = delete;
    SkillVfxPlayer& operator=(const SkillVfxPlayer&) = delete;

    // Doc <dir>/skill_vfx.csv. Tra ve so clip doc duoc (0 = thieu file -> moi ham khac thanh no-op).
    int  load(const std::string& dir);
    void unloadAll();                  // giai phong texture + xoa moi hieu ung. An toan khi goi nhieu lan
    void clear();                      // xoa moi hieu ung dang chay (giu texture da nap)

    void setTier(Tier tier) { m_tier = tier; }
    Tier getTier() const { return m_tier; }
    void setScaleMultiplier(float k) { m_scaleMul = k; }
    bool hasSkill(const std::string& skillName) const;

    // Cast: caster/target la CombatantView (non-owning). target == caster => skill tu buff.
    // statusWillApply = true => bo qua clip 'status' (se do startStatus lo, vi no bam theo nhan vat).
    void playSkill(const std::string& skillName, const CombatantView* caster,
                   const CombatantView* target, bool statusWillApply);

    // Hieu ung lap bam theo nhan vat cho den khi syncStatus thay status da het.
    void startStatus(const std::string& skillName, const CombatantView* owner, StatusType type);
    void syncStatus(const CombatantView* owner, const StatusEffectManager* effects);

    void update(float deltaTime);
    void draw() const;
    bool isPlayingOneShot() const;     // con hieu ung 1 lan dang chay / cho (de Core co the doi)

private:
    enum class Who   { TARGET, CASTER };
    enum class Point { FEET, BODY, HEAD };

    struct Clip {
        std::string role, effect;
        Who   who = Who::TARGET;
        Point point = Point::FEET;
        int   delayMs = 0, frames = 1, frameMs = 70, w = 0, h = 0, pivotX = 0, pivotY = 0;
        bool  loop = false;
        std::string litPath, strictPath;
    };

    struct Instance {
        const Clip* clip = nullptr;
        Texture2D   texture{};
        Vector2     pos{};                       // chi dung cho hieu ung 1 lan
        bool        flip = false;
        float       scale = 1.0f;
        float       delay = 0.0f;                // giay con phai cho truoc khi bat dau
        float       time = 0.0f;
        bool        persistent = false;
        const CombatantView* owner = nullptr;    // chi dung cho persistent
        StatusType  status = StatusType::ATK_BOOST;
    };

    Tier        m_tier = Tier::LIT;
    float       m_scaleMul = 1.0f;
    std::string m_dir;
    std::unordered_map<std::string, std::vector<Clip>> m_skills;   // khoa: ten skill viet thuong
    std::unordered_map<std::string, Texture2D>         m_textures; // khoa: duong dan; id = 0 => nap loi (khong thu lai)
    std::vector<Instance> m_instances;
    std::vector<std::string> m_warned;                             // ten skill da bao thieu hieu ung (chi log 1 lan)

    Texture2D   textureFor(const Clip& clip);
    Vector2     anchorOf(const CombatantView& view, Point point) const;
    float       scaleFor(const CombatantView& view) const;
    int         frameOf(const Instance& inst) const;     // -1 = da xong
    const Clip* statusClip(const std::string& skillName) const;
};
