# ORIGINAL UI AUDIT

Target: C++17 / Raylib UI in the original ZIP.
Screenshots mentioned in the request are not separately provided here; visual observations remain unverified from actual screenshots.

## API call sites, original files and lines

### src/core/GameEngine.cpp
  L792: if (IsKeyPressed(KEY_ESCAPE))
  L829: else if (IsKeyPressed(KEY_Z) && m_stateMachine.is(GameStateMachine::State::IN_BATTLE))
  L908: if (IsKeyPressed(KEY_T)) {
  L913: if (IsKeyPressed(KEY_G) && !m_gearIds.empty()) {
  L947: if (IsKeyPressed(KEY_ENTER))
  L1014: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
  L1021: if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_LEFT))
  L1024: if (IsKeyPressed(KEY_I)) openItemMenu();
  L1025: if (IsKeyPressed(KEY_S)) {
  L1038: if (IsKeyPressed(keys[i])) m_selectedItemIndex = static_cast<int>(i);
  L1039: if (IsKeyPressed(KEY_F1)) selectAlly(0);
  L1040: if (IsKeyPressed(KEY_F2)) selectAlly(1);
  L1041: if (IsKeyPressed(KEY_F3)) selectAlly(2);
  L1042: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) doUseItem();
  L1051: if (IsKeyPressed(keys[i]))
  L1055: if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
  L1066: if (IsKeyPressed(KEY_A))
  L1545: DrawText("VICTORY!", 500, 250, 50, GREEN);
  L1546: DrawText("Press ENTER to return to Menu", 420, 340, 22, DARKGRAY);
  L1550: DrawText("GAME OVER", 480, 250, 50, RED);
  L1551: DrawText("Press ENTER to return to Menu", 420, 340, 22, DARKGRAY);
  L1597: DrawText(label, ex, ey, 12, ORANGE);
  L1598: ex += MeasureText(label, 12) + 6;
  L1625: DrawText("TUI DO: [1-9] Chon  [F1-F3] Dong doi  [ENTER] Dung", 355, 177, 18, WHITE);
  L1626: if (m_selectedAlly) DrawText(TextFormat("Muc tieu: %s", m_selectedAlly->getName().c_str()), 355, 209, 18, YELLOW);
  L1628: DrawText(TextFormat("%d. %s", static_cast<int>(i + 1), items[i]->getName().c_str()),

### src/graphics/Battleui.cpp
  L65: DrawText(TextFormat("%s [%s]", c.getName().c_str(), elementName(c.getElement())),
  L71: DrawText(hpText, (int)(x + (BAR_W - MeasureText(hpText, 10)) / 2.0f), (int)(y + (HP_H - 10) / 2.0f), 10, WHITE);
  L80: DrawText("TURN ORDER", (int)x, (int)y, 16, GOLD);
  L94: DrawText(TextFormat("%d. %s", shown + 1, c->getName().c_str()), (int)x + 8, (int)y + 8, 14, textColor);
  L96: DrawText(spd, (int)(x + TURN_BOX_W - MeasureText(spd, 12) - 8), (int)y + 9, 12, alive ? SKYBLUE : DARKGRAY);
  L111: DrawText(TextFormat("MAN %d", stage), PANEL_X + PANEL_W - 100, BANNER_Y, 20, SKYBLUE);
  L112: DrawText("DEN LUOT:", PANEL_X + 20, BANNER_Y, 20, YELLOW);
  L115: DrawText(TextFormat("%s%s", current->getName().c_str(), waitingConfirm ? " (Cho xac nhan)" : ""),
  L118: DrawText(TextFormat("%s (Dang suy nghi...)", current->getName().c_str()), PANEL_X + 140, BANNER_Y, 20, RED);
  L126: DrawText(text, PANEL_X + 20, HELP_Y, 18, LIGHTGRAY);
  L131: DrawText(message.c_str(), PANEL_X + 20, LOG_Y, 20, ORANGE);
  L148: DrawText("KY NANG (1-4 chon, SPACE de dung, ESC dong)", boxX + 20, SKILL_BOX_Y + 15, 16, YELLOW);
  L158: DrawText(TextFormat("[%d] %s  (MP %d, %d%s)", i + 1, s.m_name.c_str(), s.m_mpCost, s.m_power, suffix),
  L162: DrawText(TextFormat("%d/%d", selectedIndex + 1, total), boxX + SKILL_BOX_W - 60, SKILL_BOX_Y + 15, 14, GRAY);
  L198: const int w = MeasureText(t.m_text.c_str(), size);
  L200: DrawText(t.m_text.c_str(), px + 2, py + 2, size, Color{ 0, 0, 0, c.a });
  L201: DrawText(t.m_text.c_str(), px, py, size, c);

### src/graphics/Herocreatescreen.cpp
  L44: DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
  L48: DrawText(label, static_cast<int>(x), static_cast<int>(y), 16, COL_LABEL);
  L54: DrawText(TextFormat("%d", value), static_cast<int>(bx + barW + 10.0f), static_cast<int>(y), 16, COL_TEXT);
  L201: if (IsKeyPressed(KEY_ESCAPE)) return HeroCreateAction::BACK;
  L202: if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) moveSelection(-1);
  L203: if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) moveSelection(+1);
  L204: if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) moveSelectionRow(-1);
  L205: if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) moveSelectionRow(+1);
  L206: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
  L209: const Vector2 mouse = GetMousePosition();
  L210: const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  L278: DrawText(p.m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 14.0f), 20, nameCol);
  L280: DrawText(m_races[p.m_raceIndex].m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 44.0f), 16, COL_LABEL);
  L281: DrawText(m_classes[p.m_classIndex].m_name.c_str(), static_cast<int>(tx), static_cast<int>(r.y + 66.0f), 16, COL_LABEL);
  L285: DrawText(ELEMENT_NAMES[e], static_cast<int>(tx + 16.0f), static_cast<int>(r.y + 92.0f), 16, COL_TEXT);
  L296: const int tw = MeasureText(text, 18);
  L298: DrawText(text, static_cast<int>(cx - tw / 2.0f), static_cast<int>(r.y + 269.0f), 18, COL_TEXT);
  L314: DrawText(TextFormat("CHI SO - %s", p.m_name.c_str()), static_cast<int>(panel.x + 24), static_cast<int>(panel.y + 14), 18, COL_TITLE);
  L326: DrawText("KY NANG", static_cast<int>(sx), static_cast<int>(panel.y + 14), 18, COL_TITLE);
  L332: DrawText(tag, static_cast<int>(sx), static_cast<int>(y), 20, tagColor);
  L333: DrawText(name.c_str(), static_cast<int>(sx + 40.0f), static_cast<int>(y), 20, COL_TEXT);

### src/graphics/PrepMenuScreen.cpp
  L65: DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
  L68: DrawText(text, static_cast<int>(right - MeasureText(text, size)), static_cast<int>(y), size, c);
  L89: DrawText(label, static_cast<int>(x), static_cast<int>(y), 18, COL_LABEL);
  L152: if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_Q)) selectTab(-1);
  L153: if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_TAB)) selectTab(+1);
  L154: if (IsKeyPressed(KEY_ONE))   m_tab = TAB_PARTY;
  L155: if (IsKeyPressed(KEY_TWO))   m_tab = TAB_SKILLS;
  L156: if (IsKeyPressed(KEY_THREE)) m_tab = TAB_ITEMS;
  L157: if (IsKeyPressed(KEY_FOUR))  m_tab = TAB_GATE;
  L159: if (IsKeyPressed(KEY_F1)) return PrepMenuAction::EDIT_MEMBER_1;
  L160: if (IsKeyPressed(KEY_F2)) return PrepMenuAction::EDIT_MEMBER_2;
  L161: if (IsKeyPressed(KEY_F3)) return PrepMenuAction::EDIT_MEMBER_3;
  L163: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) return PrepMenuAction::START;
  L166: const Vector2 mouse = GetMousePosition();
  L174: if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  L224: GetMousePosition(), &m_info.m_hero.m_stats);
  L258: const bool hovered = CheckCollisionPointRec(GetMousePosition(), r);
  L266: DrawText(TextFormat("He %s", elementName(h->m_element)), static_cast<int>(cx - 16), static_cast<int>(r.y + 97), 18, elementColor(h->m_element));
  L292: DrawText("KY NANG CUA ANH HUNG", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
  L306: const bool rowHover = CheckCollisionPointRec(GetMousePosition(), r);
  L312: DrawText(s.m_name.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 10), 22, COL_TEXT);
  L315: DrawText(sub.c_str(), static_cast<int>(r.x + 24), static_cast<int>(r.y + 38), 16, COL_LABEL);
  L317: DrawText(elementName(s.m_element), static_cast<int>(r.x + r.width - 410), static_cast<int>(r.y + 20), 18, elementColor(s.m_element));
  L326: DrawText("TUI DO MANG VAO TRAN", static_cast<int>(area.x + 28), static_cast<int>(area.y + 20), 20, COL_TITLE);
  L360: DrawText(g.m_title.empty() ? "CONG" : g.m_title.c_str(), static_cast<int>(leftX), static_cast<int>(area.y + 22), 34, COL_TITLE);
  L363: DrawText(line.c_str(), static_cast<int>(leftX), static_cast<int>(y), 18, COL_LABEL);
  L372: DrawText("THOI HAN", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 16), 16, COL_DIM);
  L374: DrawText(TextFormat("%d", g.m_deadlineTurns), static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 44), 56, COL_TITLE);
  L375: DrawText("luot de ha gac cong", static_cast<int>(clockBox.x + 100 + (g.m_deadlineTurns >= 100 ? 36 : 0)),
  L378: DrawText("Khong gioi han", static_cast<int>(clockBox.x + 20), static_cast<int>(clockBox.y + 52), 30, COL_OK);
  L385: DrawText("VONG KHAC HE", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 14), 16, COL_DIM);
  L390: DrawText(elementName(ring[i]), static_cast<int>(hx + 20), static_cast<int>(hintBox.y + 49), 16, elementColor(ring[i]));
  L393: DrawText("Khac he: x1.5 sat thuong", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 84), 16, COL_OK);
  L394: DrawText("Bi khac he: x0.5 sat thuong", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 108), 16, COL_ERR);
  L395: DrawText("(Kim > Moc > Tho > Thuy > Hoa > Kim)", static_cast<int>(hintBox.x + 20), static_cast<int>(hintBox.y + 128), 14, COL_DIM);
  L405: DrawText(g.m_bossName.c_str(), static_cast<int>(boss.x + 100), static_cast<int>(boss.y + 12), 28, COL_TEXT);
  L407: DrawText(TextFormat("He %s", elementName(g.m_bossElement)), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 66), 18, elementColor(g.m_bossElement));
  L408: DrawText(TextFormat("HP %d", g.m_bossHp), static_cast<int>(boss.x + 46), static_cast<int>(boss.y + 94), 18, COL_TEXT);
  L412: DrawText(g.m_hasBoss ? "QUAN HO TONG" : "QUAI CO THE GAP", static_cast<int>(rightX), static_cast<int>(ry), 16, COL_TITLE);
  L418: DrawText(e.m_name.c_str(), static_cast<int>(rightX + 28), static_cast<int>(ry), 20, COL_TEXT);
  L424: DrawText("Khong co du lieu", static_cast<int>(rightX), static_cast<int>(ry), 18, COL_DIM);
  L426: DrawText(TextFormat("+%d quai khac", static_cast<int>(g.m_enemies.size() - shown)), static_cast<int>(rightX + 28), static_cast<int>(ry), 14, COL_DIM);

### src/graphics/Skilltooltip.cpp
  L139: int w = std::max(MIN_W, MeasureText(info.m_name.c_str(), TITLE_SIZE) + 2 * PAD);
  L140: for (const TipLine& l : lines) w = std::max(w, MeasureText(l.m_text.c_str(), TEXT_SIZE) + 2 * PAD);
  L158: DrawText(info.m_name.c_str(), x + PAD, y + PAD, TITLE_SIZE, accent);
  L163: DrawText(l.m_text.c_str(), x + PAD, ty, TEXT_SIZE, l.m_color);

### src/graphics/SpriteAnimation.cpp
  L78: DrawText(TextFormat("f%d", m_currentFrame), (int)dest.x + 4, (int)dest.y + 4, 10, BLACK);

### src/graphics/StartMenu.cpp
  L40: const int w = MeasureText(text, size);
  L41: DrawText(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), size, color);
  L95: if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) m_selected = (m_selected + 1) % count;
  L96: if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) m_selected = (m_selected + count - 1) % count;
  L97: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE))
  L101: const Vector2 mouse = GetMousePosition();
  L106: if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  L224: DrawText(m_statusText.c_str(), 20, static_cast<int>(H) - 30, 16, m_statusOk ? COL_OK : COL_ERR);
  L226: DrawText(credit, static_cast<int>(W) - MeasureText(credit, 14) - 20, static_cast<int>(H) - 28, 14, Color{ 160, 155, 190, 200 });

### src/graphics/Turnorderbar.cpp
  L102: DrawText("TURN ORDER", static_cast<int>(x0), static_cast<int>(MARGIN), 16, GOLD);
  L117: DrawText(TextFormat("%d. %s", r.m_rank, r.m_name.c_str()),
  L120: DrawText(spd, static_cast<int>(box.x + BOX_W - MeasureText(spd, 12) - 8), static_cast<int>(box.y) + 9, 12,

### src/graphics/Waveselectscreen.cpp
  L48: DrawText(text, static_cast<int>(cx - MeasureText(text, size) / 2.0f), static_cast<int>(y), size, c);
  L146: if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) select(-1);
  L147: if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) select(+1);
  L150: if (IsKeyPressed(digitKeys[i])) m_selected = i;
  L151: if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
  L158: const Vector2 mouse = GetMousePosition();
  L171: if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
  L254: DrawText("QUAI CO THE GAP", static_cast<int>(r.x + 20), static_cast<int>(y), 15, COL_TITLE);
  L259: DrawText(e.m_name.c_str(), static_cast<int>(r.x + 46), static_cast<int>(y), 18, COL_TEXT);
  L261: DrawText(el, static_cast<int>(r.x + r.width - 20 - MeasureText(el, 14)), static_cast<int>(y + 3), 14, elementColor(e.m_element));
  L265: DrawText(TextFormat("+%d quai khac", static_cast<int>(w.m_enemies.size() - MAX_ENEMY_LINES)),

### src/graphics/main_map_test.cpp
  L56: if (IsKeyPressed(KEY_ONE)) { map.setStage(StageId::STAGE_1, "assets/images/bg_stage1.png"); actors = buildStage(map, StageId::STAGE_1); }
  L57: if (IsKeyPressed(KEY_TWO)) { map.setStage(StageId::STAGE_2, "assets/images/bg_stage2.png"); actors = buildStage(map, StageId::STAGE_2); }
  L58: if (IsKeyPressed(KEY_F1)) showSlots = !showSlots;
  L77: DrawText(map.hasBackground() ? "Background: IMAGE" : "Background: PROCEDURAL (chua co anh)", 20, 20, 20, RAYWHITE);
  L78: DrawText("[1] Stage 1  [2] Stage 2 (Boss)  [F1] Slot  [ESC] Thoat", 20, 690, 18, LIGHTGRAY);

### src/graphics/main_sprite_test.cpp
  L34: if (IsKeyPressed(KEY_ONE))   hero->play(AnimState::IDLE);
  L35: if (IsKeyPressed(KEY_TWO))   hero->play(AnimState::ATTACK, true);
  L36: if (IsKeyPressed(KEY_THREE)) hero->play(AnimState::TAKE_DAMAGE, true);
  L37: if (IsKeyPressed(KEY_F))     flipX = !flipX;
  L56: DrawText("SPRITE SHEET", (int)origin.x, (int)origin.y - 24, 18, GOLD);
  L65: DrawText(loaded ? "Sprite sheet: LOADED" : "Sprite sheet: NOT FOUND (dang ve placeholder)",
  L67: DrawText(TextFormat("Frame: %d | Row: %d | FPS: %d", hero->getCurrentFrame(), hero->getCurrentRow(), GetFPS()),
  L69: DrawText("[1] IDLE  [2] ATTACK  [3] TAKE_DAMAGE  [F] Flip  [ESC] Thoat", 20, SCREEN_H - 35, 18, LIGHTGRAY);

## Summary / plan

- Default Raylib text path and ASCII labels lack Vietnamese glyph support; skill and item views were predominantly keyboard driven.
- Inventory cards used a constant red circle. HUD overhead labels had no overlap resolution; the battle log was a single fixed line.
- Implementation: FontManager + Unicode UiText; letterboxed logical mouse UiInput; procedural Ember & Rune UI elements; mouse commands; native tooltips; collision-avoiding bar placement.

## Known limitations

- The Raylib runtime and VS2022 were not available here, so screens and FPS need on-device verification.
- Full English locale, mouse drag scrollbars, and Settings menu are deferred.
