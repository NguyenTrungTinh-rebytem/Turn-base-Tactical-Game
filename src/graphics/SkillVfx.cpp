// src/graphics/SkillVfx.cpp
#include "SkillVfx.h"
#include "CombatantView.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
constexpr float NOMINAL_HEIGHT = 64.0f;   // chieu cao nhan vat "chuan" ma hieu ung duoc ve cho

std::string trim(const std::string& s) {
    const size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}  // namespace

SkillVfxPlayer::~SkillVfxPlayer() { unloadAll(); }

int SkillVfxPlayer::load(const std::string& dir) {
    m_dir = dir;
    m_instances.clear();   // Instance giu con tro vao m_skills
    m_skills.clear();
    std::ifstream file(dir + "/skill_vfx.csv");
    if (!file.is_open()) return 0;

    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line.compare(0, 6, "skill,") == 0) continue;

        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) f.push_back(trim(cell));
        if (f.size() < 15) continue;

        Clip c;
        c.role = f[1];
        c.effect = f[2];
        c.who = (f[3] == "caster") ? Who::CASTER : Who::TARGET;
        c.point = (f[4] == "head") ? Point::HEAD : (f[4] == "body") ? Point::BODY : Point::FEET;
        c.delayMs = std::atoi(f[5].c_str());
        c.frames = std::max(1, std::atoi(f[6].c_str()));
        c.frameMs = std::max(1, std::atoi(f[7].c_str()));
        c.w = std::atoi(f[8].c_str());
        c.h = std::atoi(f[9].c_str());
        c.pivotX = std::atoi(f[10].c_str());
        c.pivotY = std::atoi(f[11].c_str());
        c.loop = (std::atoi(f[12].c_str()) != 0);
        c.litPath = f[13];
        c.strictPath = f[14];
        if (c.w <= 0 || c.h <= 0) continue;

        m_skills[lower(f[0])].push_back(std::move(c));
        ++count;
    }
    return count;
}

void SkillVfxPlayer::clear() { m_instances.clear(); }

void SkillVfxPlayer::unloadAll() {
    m_instances.clear();
    if (IsWindowReady())   // sau CloseWindow() thi bo qua, khong crash
        for (auto& [path, texture] : m_textures)
            if (texture.id != 0) UnloadTexture(texture);
    m_textures.clear();
}

bool SkillVfxPlayer::hasSkill(const std::string& skillName) const {
    return m_skills.count(lower(trim(skillName))) > 0;
}

Texture2D SkillVfxPlayer::textureFor(const Clip& clip) {
    const std::string& rel = (m_tier == Tier::LIT) ? clip.litPath : clip.strictPath;
    const std::string path = m_dir + "/" + rel;
    auto it = m_textures.find(path);
    if (it != m_textures.end()) return it->second;

    Texture2D texture{};
    if (IsWindowReady() && FileExists(path.c_str())) {
        texture = LoadTexture(path.c_str());
        if (texture.id != 0) SetTextureFilter(texture, TEXTURE_FILTER_POINT);   // pixel art: khong lam mo
    } else {
        std::cerr << "[SkillVfx] missing sprite: " << path << '\n';
    }
    m_textures[path] = texture;   // luu ca khi loi de khong thu lai moi frame
    return texture;
}

Vector2 SkillVfxPlayer::anchorOf(const CombatantView& view, Point point) const {
    const BattleAnchor a = view.getAnchor();
    const Rectangle b = a.spriteBounds;
    switch (point) {
        case Point::HEAD: return Vector2{ b.x + b.width * 0.5f, b.y };
        case Point::BODY: return Vector2{ b.x + b.width * 0.5f, b.y + b.height * 0.5f };
        case Point::FEET: break;
    }
    return a.feet;
}

float SkillVfxPlayer::scaleFor(const CombatantView& view) const {
    const float k = view.getDrawBounds().height / NOMINAL_HEIGHT;   // boss to => hieu ung to theo
    return std::max(1.0f, std::min(4.0f, k)) * m_scaleMul;
}

const SkillVfxPlayer::Clip* SkillVfxPlayer::statusClip(const std::string& skillName) const {
    auto it = m_skills.find(lower(trim(skillName)));
    if (it == m_skills.end()) return nullptr;
    for (const Clip& c : it->second)
        if (c.role == "status") return &c;
    return nullptr;
}

void SkillVfxPlayer::playSkill(const std::string& skillName, const CombatantView* caster,
                               const CombatantView* target, bool statusWillApply) {
    auto it = m_skills.find(lower(trim(skillName)));
    if (it == m_skills.end()) {
        if (!m_skills.empty() && std::find(m_warned.begin(), m_warned.end(), skillName) == m_warned.end()) {
            m_warned.push_back(skillName);
            std::cerr << "[SkillVfx] khong co hieu ung cho skill \"" << skillName
                      << "\" (ten phai khop cot 'skill' trong skill_vfx.csv)\n";
        }
        return;
    }
    if (!caster && !target) return;
    if (!caster) caster = target;
    if (!target) target = caster;

    // Dich o ben trai nguoi cast => lat ngang hieu ung (hieu ung ve huong sang phai)
    const bool flip = (target != caster) &&
                      (target->getAnchor().feet.x < caster->getAnchor().feet.x);

    for (const Clip& clip : it->second) {
        if (clip.role == "alt") continue;                    // ban thay the: khong phat
        if (clip.role == "status" && statusWillApply) continue;   // do startStatus lo

        const CombatantView* view = (clip.who == Who::CASTER) ? caster : target;
        const Texture2D texture = textureFor(clip);
        if (texture.id == 0 || texture.width < clip.w) continue;

        Instance inst;
        inst.clip = &clip;
        inst.texture = texture;
        inst.pos = anchorOf(*view, clip.point);
        inst.flip = flip;
        inst.scale = scaleFor(*view);
        inst.delay = static_cast<float>(clip.delayMs) / 1000.0f;
        m_instances.push_back(inst);
    }
}

void SkillVfxPlayer::startStatus(const std::string& skillName, const CombatantView* owner, StatusType type) {
    if (!owner) return;
    const Clip* clip = statusClip(skillName);
    if (!clip) return;
    for (const Instance& i : m_instances)
        if (i.persistent && i.owner == owner && i.status == type) return;   // da co

    const Texture2D texture = textureFor(*clip);
    if (texture.id == 0 || texture.width < clip->w) return;

    Instance inst;
    inst.clip = clip;
    inst.texture = texture;
    inst.persistent = true;
    inst.owner = owner;
    inst.status = type;
    m_instances.push_back(inst);
}

void SkillVfxPlayer::syncStatus(const CombatantView* owner, const StatusEffectManager* effects) {
    m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(),
        [&](const Instance& i) {
            return i.persistent && i.owner == owner && (!effects || !effects->hasEffect(i.status));
        }), m_instances.end());
}

int SkillVfxPlayer::frameOf(const Instance& inst) const {
    const int frame = static_cast<int>(inst.time * 1000.0f / static_cast<float>(inst.clip->frameMs));
    if (inst.persistent) return frame % inst.clip->frames;
    return (frame < inst.clip->frames) ? frame : -1;
}

void SkillVfxPlayer::update(float deltaTime) {
    for (Instance& inst : m_instances) {
        if (inst.delay > 0.0f) { inst.delay -= deltaTime; continue; }
        inst.time += deltaTime;
    }
    m_instances.erase(std::remove_if(m_instances.begin(), m_instances.end(),
        [&](const Instance& i) { return !i.persistent && i.delay <= 0.0f && frameOf(i) < 0; }),
        m_instances.end());
}

bool SkillVfxPlayer::isPlayingOneShot() const {
    return std::any_of(m_instances.begin(), m_instances.end(),
                       [](const Instance& i) { return !i.persistent; });
}

void SkillVfxPlayer::draw() const {
    for (const Instance& inst : m_instances) {
        if (inst.delay > 0.0f) continue;
        const Clip& c = *inst.clip;
        const int frame = frameOf(inst);
        if (frame < 0) continue;

        Vector2 pos = inst.pos;
        float scale = inst.scale;
        bool flip = inst.flip;
        if (inst.persistent) {                      // bam theo nhan vat (slot co the doi khi bo tri lai)
            if (!inst.owner || !inst.owner->isVisible()) continue;   // nhan vat da chet => an
            pos = anchorOf(*inst.owner, c.point);
            scale = scaleFor(*inst.owner);
            flip = false;
        }

        const int cols = std::max(1, inst.texture.width / c.w);
        const Rectangle src{ static_cast<float>((frame % cols) * c.w), static_cast<float>((frame / cols) * c.h),
                             flip ? -static_cast<float>(c.w) : static_cast<float>(c.w), static_cast<float>(c.h) };
        const Rectangle dst{ std::round(pos.x), std::round(pos.y),
                             static_cast<float>(c.w) * scale, static_cast<float>(c.h) * scale };
        // origin = pivot (da nhan scale) -> diem pivot cua frame roi dung vao pos. Lat ngang => pivot lat theo.
        const float ox = static_cast<float>(flip ? (c.w - c.pivotX) : c.pivotX) * scale;
        const float oy = static_cast<float>(c.pivotY) * scale;
        DrawTexturePro(inst.texture, src, dst, Vector2{ ox, oy }, 0.0f, WHITE);
    }
}
