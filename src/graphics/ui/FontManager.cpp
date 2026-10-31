#include "FontManager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
namespace {
std::vector<int> vietnameseCodepoints() {
    std::vector<int> glyphs;
    auto add=[&glyphs](int a,int b){for(int c=a;c<=b;++c)glyphs.push_back(c);};
    add(0x20,0x7E);
    add(0xA0,0x1B0);            // Includes Đ, đ, ơ/ư & Latin Extended-A
    add(0x0300,0x036F);        // Combining marks (precomposed preferred)
    add(0x1E00,0x1EF9);        // All Vietnamese tone-mark codepoints
    for(int c : {0x2013,0x2014,0x2018,0x2019,0x201C,0x201D,
                 0x2022,0x2026,0x00D7,0x00B7}) glyphs.push_back(c);
    std::sort(glyphs.begin(),glyphs.end());
    glyphs.erase(std::unique(glyphs.begin(),glyphs.end()),glyphs.end());
    return glyphs;
}
void audit(Font font,const char* role) {
    const int fallback=GetGlyphIndex(font,'?');
    // Required characters for on-screen Vietnamese; triangles are drawn as
    // primitives because the bundled Noto text fonts have no U+25B6/25C0.
    constexpr int required[]={0x0110,0x0111,0x01A0,0x01A1,0x01AF,0x01B0,
        0x1EA0,0x1EA1,0x1EBF,0x1EC7,0x1ECD,0x1ED3,0x1ED9,
        0x1EE3,0x1EE9,0x1EEF,0x1EF1,0x1EF3,0x1EF5,0x1EF7,0x1EF9,
        0x2013,0x2014,0x2026,0x00D7};
    for(int codepoint:required) if(GetGlyphIndex(font,codepoint)==fallback)
        TraceLog(LOG_WARNING,"FontManager: missing U+%04X in %s",codepoint,role);
}
}
FontManager& FontManager::instance(){static FontManager inst;return inst;}
void FontManager::initialize(const std::string& directory){
    shutdown();m_assetsDirectory=directory;m_scale=1.f;load();
}
void FontManager::setRenderScale(float windowScale){
    const float scaled=std::max(0.7f,std::min(3.0f,windowScale));
    if(std::abs(scaled-m_scale)<0.075f) return;
    m_scale=scaled;
    // Raylib textures must be alive while rendering; called before BeginDrawing.
    load();
}
void FontManager::load(){
    shutdown();
    if(m_assetsDirectory.empty()||!IsWindowReady())return;
    auto glyphs=vietnameseCodepoints();
    struct Family{Role role;const char* file;const char* alias;};
    const Family families[]={
        {Role::BODY,"NotoSans-Medium.ttf","BODY"},
        {Role::HUD,"NotoSans-CondensedSemiBold.ttf","HUD"},
        {Role::DISPLAY,"NotoSerifDisplay-SemiBold.ttf","DISPLAY"}
    };
    for(const auto& family:families){
        const std::string path=m_assetsDirectory+"/fonts/"+family.file;
        if(!FileExists(path.c_str())) {TraceLog(LOG_WARNING,"Missing font: %s",path.c_str());continue;}
        // Shared logical size list ensures nearest-size selection never produces
        // a tiny atlas that is subsequently blown up at 1080p.
        for(int s : {16,18,20,22,24,26,28,32,38,48,76}){
            if(family.role==Role::DISPLAY && s!=38 && s!=48 && s!=76)continue;
            if(family.role==Role::BODY && s!=16 && s!=18 && s!=20)continue;
            if(family.role==Role::HUD && s!=20 && s!=22 && s!=26 && s!=28)continue;
            const int pixelSize=std::max(16,static_cast<int>(std::lround(s*m_scale)));
            Font loaded=LoadFontEx(path.c_str(),pixelSize,glyphs.data(),static_cast<int>(glyphs.size()));
            if(!loaded.texture.id){TraceLog(LOG_ERROR,"Font load failed: %s",path.c_str());continue;}
            SetTextureFilter(loaded.texture,TEXTURE_FILTER_BILINEAR);
            if(s==20 || (family.role==Role::DISPLAY&&s==48))audit(loaded,family.alias);
            m_entries.push_back({family.role,s,loaded});
        }
    }
    m_loaded=!m_entries.empty();
}
Font FontManager::font(Role role,int logicalSize)const {
    const Entry* match=nullptr;
    for(const Entry& entry:m_entries){
        if(entry.m_role!=role)continue;
        if(!match || std::abs(entry.m_logicalSize-logicalSize)<std::abs(match->m_logicalSize-logicalSize))match=&entry;
    }
    if(!match && role!=Role::BODY)return font(Role::BODY,logicalSize);
    return match?match->m_font:GetFontDefault();
}
void FontManager::shutdown(){
    if(IsWindowReady())for(const Entry& entry:m_entries)UnloadFont(entry.m_font);
    m_entries.clear();m_loaded=false;
}
