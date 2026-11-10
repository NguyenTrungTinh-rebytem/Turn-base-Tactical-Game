// src/graphics/AttackLunge.cpp
#include "graphics/AttackLunge.h"
#include <algorithm>
#include <cmath>

namespace AttackLunge {
namespace {

constexpr float T_WIND = 0.07f;    // lui nhe lay da
constexpr float T_IN   = 0.16f;    // lao toi
constexpr float T_HIT  = 0.34f;    // dung lai danh (thoi gian animation ATTACK)
constexpr float T_BACK = 0.30f;    // luot ve
constexpr float T_TOTAL = T_WIND + T_IN + T_HIT + T_BACK;
constexpr float WIND_DIST = 12.0f;
constexpr float HOP_HEIGHT = 10.0f;
constexpr float PI_F = 3.14159265f;

struct Lunge {
    CombatantView* view = nullptr;
    BattleSlot     home{};
    Vector2        stop{};         // diem dung sat muc tieu
    float          dir = 1.0f;     // +1: lao sang phai, -1: sang trai
    float          t = 0.0f;
    bool           hitFired = false;
    std::function<void()> onHit;
};

std::vector<Lunge> g_lunges;

float easeInQuad(float u)    { return u * u; }
float easeInOutQuad(float u) { return u < 0.5f ? 2.0f * u * u : 1.0f - std::pow(-2.0f * u + 2.0f, 2.0f) / 2.0f; }
float easeOutCubic(float u)  { return 1.0f - std::pow(1.0f - u, 3.0f); }
float clamp01(float v)       { return std::max(0.0f, std::min(1.0f, v)); }

Vector2 lerp(Vector2 a, Vector2 b, float u) { return Vector2{ a.x + (b.x - a.x) * u, a.y + (b.y - a.y) * u }; }

// Vi tri chan tai thoi diem t cua luot toi; moving = dang di chuyen (de ve bong mo)
Vector2 posAt(const Lunge& l, float t, bool* moving = nullptr)
{
    const Vector2 home = l.home.m_feetPos;
    const Vector2 wind{ home.x - l.dir * WIND_DIST, home.y };
    if (moving) *moving = false;
    t = std::max(0.0f, t);

    if (t < T_WIND)
        return lerp(home, wind, easeOutCubic(t / T_WIND));
    t -= T_WIND;
    if (t < T_IN) {
        if (moving) *moving = true;
        const float u = clamp01(t / T_IN);
        Vector2 p = lerp(wind, l.stop, easeInQuad(u));
        p.y -= std::sin(u * PI_F) * HOP_HEIGHT;          // nhay nhe khi lao
        return p;
    }
    t -= T_IN;
    if (t < T_HIT) {
        const float u = clamp01(t / T_HIT);
        // giat nhe ve phia truoc luc danh
        return Vector2{ l.stop.x + l.dir * std::sin(u * PI_F) * 6.0f, l.stop.y };
    }
    t -= T_HIT;
    if (t < T_BACK) {
        if (moving) *moving = true;
        const float u = clamp01(t / T_BACK);
        Vector2 p = lerp(l.stop, home, easeInOutQuad(u));
        p.y -= std::sin(u * PI_F) * HOP_HEIGHT * 0.6f;
        return p;
    }
    return home;
}

void applyPos(Lunge& l, Vector2 p)
{
    BattleSlot s = l.home;
    s.m_feetPos = p;
    l.view->setSlot(s);
}

} // namespace

bool start(CombatantView* attacker, const CombatantView* target, std::function<void()> onHit)
{
    if (!attacker || !target || attacker == target) return false;

    // Dang lao roi -> giu nguyen vi tri goc, bat dau lai tu dau
    BattleSlot home = attacker->getSlot();
    for (auto it = g_lunges.begin(); it != g_lunges.end();) {
        if (it->view == attacker) { home = it->home; it = g_lunges.erase(it); }
        else ++it;
    }

    const BattleSlot ts = target->getSlot();
    const float dx = ts.m_feetPos.x - home.m_feetPos.x;
    if (std::fabs(dx) < 4.0f) return false;

    Lunge l;
    l.view = attacker;
    l.home = home;
    l.dir = (dx > 0.0f) ? 1.0f : -1.0f;
    // Khoang cach dung = ti le theo kich thuoc hai ben (boss to -> dung xa hon)
    const float gap = std::max(50.0f, std::min(230.0f, 22.0f * (home.m_scale + ts.m_scale) + 10.0f));
    l.stop = Vector2{ ts.m_feetPos.x - l.dir * gap, ts.m_feetPos.y + 2.0f };   // +2: ve sau muc tieu
    l.onHit = std::move(onHit);
    g_lunges.push_back(std::move(l));
    return true;
}

void update(float dt, const ViewList& views)
{
    for (auto it = g_lunges.begin(); it != g_lunges.end();) {
        Lunge& l = *it;

        // Con tro con hop le khong? (view co the da bi xoa khi doi wave)
        const bool valid = std::any_of(views.begin(), views.end(),
            [&](const std::unique_ptr<CombatantView>& v) { return v.get() == l.view; });
        if (!valid) { it = g_lunges.erase(it); continue; }

        l.t += std::min(dt, 0.05f);
        if (!l.hitFired && l.t >= T_WIND + T_IN) {
            l.hitFired = true;
            if (l.onHit) { auto cb = std::move(l.onHit); l.onHit = nullptr; cb(); }
        }
        if (l.t >= T_TOTAL) {
            l.view->setSlot(l.home);
            it = g_lunges.erase(it);
            continue;
        }
        applyPos(l, posAt(l, l.t));
        ++it;
    }
}

void drawGhosts(const BattleMap& map, const ViewList& views, const std::function<Color(size_t)>& tintOf)
{
    for (Lunge& l : g_lunges) {
        bool moving = false;
        posAt(l, l.t, &moving);
        if (!moving) continue;
        size_t idx = views.size();
        for (size_t i = 0; i < views.size(); ++i)
            if (views[i].get() == l.view) { idx = i; break; }
        if (idx == views.size()) continue;

        const Color base = tintOf ? tintOf(idx) : WHITE;
        const BattleSlot current = l.view->getSlot();
        for (int k = 3; k >= 1; --k) {                          // xa nhat ve truoc
            const Vector2 p = posAt(l, l.t - 0.035f * static_cast<float>(k));
            BattleSlot s = l.home;
            s.m_feetPos = p;
            l.view->setSlot(s);
            const unsigned char a = static_cast<unsigned char>(95 - 25 * k + 25);   // 95, 70, 45
            l.view->draw(map, Color{ base.r, base.g, base.b, static_cast<unsigned char>(std::min<int>(a, base.a)) });
        }
        l.view->setSlot(current);
    }
}

bool isActive(const CombatantView* view)
{
    return std::any_of(g_lunges.begin(), g_lunges.end(), [&](const Lunge& l) { return l.view == view; });
}

void clear()
{
    // Tra cac view ve cho goc truoc khi bo (neu con song)
    for (Lunge& l : g_lunges) {
        // view co the da bi xoa: GameEngine goi clear() TRUOC khi xoa views nen an toan
        if (l.view) l.view->setSlot(l.home);
    }
    g_lunges.clear();
}

} // namespace AttackLunge
