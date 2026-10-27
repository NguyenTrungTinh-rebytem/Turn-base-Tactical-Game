#include "LayeredSprite.h"
#include <algorithm>

namespace {

constexpr LayerAnimInfo ANIM_INFO[LAYER_ANIM_COUNT] = {
    /* IDLE    */ { 4, 6.0f,  true  },
    /* ATTACK  */ { 6, 12.0f, false },
    /* HURT    */ { 3, 10.0f, false },
    /* DEATH   */ { 6, 8.0f,  false },
    /* VICTORY */ { 4, 6.0f,  true  },
};

constexpr const char* ANIM_NAMES[LAYER_ANIM_COUNT] = {
    "idle", "attack", "hurt", "death", "victory"
};

std::string makePath(const char* layer, const char* who, LayerAnim anim)
{
    return std::string("assets/images/") + layer + "_" + who + "_" + layerAnimToString(anim) + ".png";
}

} // namespace

const LayerAnimInfo& getLayerAnimInfo(LayerAnim anim)
{
    return ANIM_INFO[static_cast<int>(anim)];
}

const char* layerAnimToString(LayerAnim anim)
{
    return ANIM_NAMES[static_cast<int>(anim)];
}

// ---------------------------------------------------------------- cache ----
std::map<std::string, std::shared_ptr<Texture2D>>& LayerTextureCache::storage()
{
    static std::map<std::string, std::shared_ptr<Texture2D>> s_cache;
    return s_cache;
}

std::shared_ptr<Texture2D> LayerTextureCache::get(const std::string& path)
{
    auto& cache = storage();
    auto it = cache.find(path);
    if (it != cache.end()) return it->second;

    Texture2D tex = LoadTexture(path.c_str());
    std::shared_ptr<Texture2D> ptr;
    if (tex.id != 0) {
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);   // giu pixel art sac net
        ptr = std::shared_ptr<Texture2D>(new Texture2D(tex), [](Texture2D* t) {
            if (IsWindowReady()) UnloadTexture(*t);   // sau CloseWindow() thi bo qua, khong crash
            delete t;
        });
    }
    cache[path] = ptr;   // luu ca nullptr de khong thu nap lai file thieu moi frame
    return ptr;
}

void LayerTextureCache::clear()
{
    storage().clear();
}

// -------------------------------------------------------------- sprite -----
void LayeredSprite::reloadBody()
{
    for (int a = 0; a < LAYER_ANIM_COUNT; ++a)
        m_bodyTex[a] = LayerTextureCache::get(makePath("body", raceToString(m_race), static_cast<LayerAnim>(a)));
}

void LayeredSprite::reloadArmor()
{
    for (int a = 0; a < LAYER_ANIM_COUNT; ++a)
        m_armorTex[a] = LayerTextureCache::get(makePath("armor", classToString(m_class), static_cast<LayerAnim>(a)));
}

bool LayeredSprite::setLoadout(RaceType race, ClassType pClass)
{
    if (!isValidCombination(race, pClass)) return false;
    const bool raceChanged  = !m_hasLoadout || race   != m_race;
    const bool classChanged = !m_hasLoadout || pClass != m_class;
    m_race = race;
    m_class = pClass;
    m_hasLoadout = true;
    if (raceChanged)  reloadBody();     // doi race: chi thay than
    if (classChanged) reloadArmor();    // doi class: chi thay giap
    return true;
}

bool LayeredSprite::hasTextures() const
{
    const int idle = static_cast<int>(LayerAnim::IDLE);
    return m_hasLoadout && (m_bodyTex[idle] || m_armorTex[idle]);
}

void LayeredSprite::unload()
{
    for (auto& t : m_bodyTex)  t.reset();
    for (auto& t : m_armorTex) t.reset();
    m_hasLoadout = false;
    m_frame = 0;
    m_timer = 0.0f;
}

bool LayeredSprite::setRace(RaceType race)   { return setLoadout(race, m_class); }
bool LayeredSprite::setClass(ClassType pClass) { return setLoadout(m_race, pClass); }

void LayeredSprite::play(LayerAnim anim, bool restart)
{
    if (!restart && anim == m_anim) return;
    m_anim = anim;
    m_frame = 0;
    m_timer = 0.0f;
}

int LayeredSprite::frameCountFor(LayerAnim anim) const
{
    const int a = static_cast<int>(anim);
    int frames = ANIM_INFO[a].frames;
    // Neu anh thuc te co it frame hon du kien thi cat lai, tranh doc ngoai anh.
    if (m_bodyTex[a])  frames = std::min(frames, m_bodyTex[a]->width  / SPRITE_FRAME_SIZE);
    if (m_armorTex[a]) frames = std::min(frames, m_armorTex[a]->width / SPRITE_FRAME_SIZE);
    return std::max(frames, 1);
}

void LayeredSprite::update(float deltaTime)
{
    if (!m_hasLoadout) return;
    const LayerAnimInfo& info = getLayerAnimInfo(m_anim);
    const int frames = frameCountFor(m_anim);
    const float frameTime = 1.0f / info.fps;

    m_timer += deltaTime;
    while (m_timer >= frameTime) {
        m_timer -= frameTime;
        if (m_frame + 1 < frames)      ++m_frame;
        else if (info.loop)            m_frame = 0;
        else { m_timer = 0.0f; break; }   // giu o frame cuoi
    }
}

bool LayeredSprite::isFinished() const
{
    if (getLayerAnimInfo(m_anim).loop) return false;
    return m_frame >= frameCountFor(m_anim) - 1;
}

void LayeredSprite::drawLayer(const TexPtr& tex, int frame, Vector2 pos,
                              bool flipX, float scale, Color tint)
{
    if (!tex) return;   // thieu file -> bo qua lop nay, khong crash
    const float size = static_cast<float>(SPRITE_FRAME_SIZE);
    Rectangle src  = { frame * size, 0.0f, flipX ? -size : size, size };
    Rectangle dest = { pos.x, pos.y, size * scale, size * scale };
    DrawTexturePro(*tex, src, dest, Vector2{ 0.0f, 0.0f }, 0.0f, tint);
}

void LayeredSprite::draw(Vector2 pos, bool flipX, float scale, Color tint) const
{
    if (!m_hasLoadout) return;
    const int a = static_cast<int>(m_anim);
    // Lop 1 (than) truoc, lop 2 (giap + vu khi) de len tren, CUNG m_frame.
    drawLayer(m_bodyTex[a],  m_frame, pos, flipX, scale, tint);
    drawLayer(m_armorTex[a], m_frame, pos, flipX, scale, tint);
}
