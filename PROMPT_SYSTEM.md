==> " PROMT chung, nên điền cho AI PROMT này trước rồi PROMT cụ thể của nhiệm vụ sau "
[PROJECT CONTEXT & STRICT CODING STANDARDS] 
Dự án: Game 2.5D Turn-based RPG (Battle Engine - ĐT10) trên Visual Studio 2022.
Chuẩn C++: C++17. Thư viện đồ họa: Raylib.

1. RULES ĐẶT TÊN (NGHIÊM NGẶT 100%):
- Biến thành viên Class: BẮT BUỘC có tiền tố "m_" và viết camelCase. Ví dụ: m_hp, m_maxHp, m_mp, m_maxMp, m_a (Attack), m_ma (Magic Attack), m_d (Defend), m_md (Magic Defend), m_spd (Speed), m_skills, m_element.
- Class / Struct: PascalCase. Ví dụ: Character, Player, Enemy, Skill, TurnManager.
- Functions / Methods: camelCase. Ví dụ: takeDamage(), executeSkill(), getSpeed().
- Enums: UPPER_SNAKE_CASE.

2. CÁC KIỂU DỮ LIỆU CỐ ĐỊNH TẤT CẢ PHẢI DÙNG CHUNG:
- Enum Ngũ hành: enum class Element { KIM = 0, MOC, THUY, HOA, THO };
- Ma trận khắc hệ (float):
  ELEMENT_MATRIX[5][5] = {
    { 1.0f, 1.5f, 1.5f, 0.5f, 0.5f }, // KIM
    { 0.5f, 1.0f, 1.5f, 0.5f, 1.5f }, // MOC
    { 0.5f, 0.5f, 1.0f, 1.5f, 1.5f }, // THUY
    { 1.5f, 1.5f, 0.5f, 1.0f, 0.5f }, // HOA
    { 1.5f, 0.5f, 0.5f, 1.5f, 1.0f }  // THO
  };
- Struct Chỉ số (Stats):
  struct Stats {
      int hp, mp, a, ma, d, md, spd;
      Stats operator+(const Stats& other) const {
          return { hp + other.hp, mp + other.mp, a + other.a, ma + other.ma, d + other.d, md + other.md, spd + other.spd };
      }
  };
- Smart Pointers: Dùng std::unique_ptr và std::shared_ptr, KHÔNG dùng con trỏ thô để quản lý vòng đời object.
- Tách biệt rõ file .h (Header) và .cpp (Implementation). Dùng Forward Declaration khi cần.


*****[TASK: CORE ENGINE & COMMAND PATTERN]
Tôi là Member 1. Hãy viết C++17 cho các thành phần sau:
1. Class GameStateMachine quản lý 4 trạng thái: State::MENU, State::IN_BATTLE, State::VICTORY, State::GAME_OVER.
2. Cài đặt Command Pattern cho lượt đánh của Player:
   - Interface ICommand chứa virtual void execute() = 0 và virtual void undo() = 0.
   - Các class con: AttackCommand, UseSkillCommand, UseItemCommand.
   - Class CommandStack chứa std::vector<std::unique_ptr<ICommand>> để lưu vết hành động.
   - Cung cấp hàm cancelLastAction() (Undo/Cancel) để người chơi quay lại Menu chọn lệnh nếu chưa kết thúc lượt.
3. Tạo file GameEngine.h/.cpp điều phối Game Loop chính.

*****[TASK: OOP ENTITIES, FACTORY & TEMPLATE INVENTORY]
Tôi là Member 2. Hãy viết C++17 cho các thành phần sau:
1. Class cơ sở Character (Abstract Class) chứa các thuộc tính m_name, m_stats (Stats struct), m_element, m_currentHp, m_currentMp.
   - Các hàm ảo: virtual void takeDamage(int amount, Element atkElement, bool isMagic), virtual void executeSkill(Skill* skill, Character* target).
   - Nạp chồng toán tử so sánh < dựa trên m_stats.spd để hỗ trợ Priority Queue.
2. Lớp con Player và Enemy kế thừa từ Character.
3. Class CharacterFactory (Factory Method):
   - Hàm createHero(string name, RaceType race, ClassType pClass) tính m_stats = raceStats + classStats.
   - Random nạp đúng 4 skills (1 Race, 1 Class, 2 Element).
4. Template Class Inventory<T> quản lý danh sách vật phẩm std::vector<T> có các hàm addItem(), removeItem(), getItem().

*****[TASK: DSA TURN MANAGER & OBSERVER PATTERN]
Tôi là Member 3. Hãy viết C++17 cho các thành phần sau:
1. Class TurnManager:
   - Dùng std::priority_queue<Character*, std::vector<Character*>, CharacterCompare> để xếp thứ tự lượt đánh dựa vào m_stats.spd của Character.
   - Hàm getNextTurn() trả về Character* đi tiếp theo, tự động cập nhật lại queue khi hết vòng.
2. Hệ thống Buff/Debuff (Status Effects):
   - Enum Class StatusType { ATK_BOOST, DEF_BOOST, REGEN, STUN, BURN, SLOW, WEAKNESS }.
   - Struct StatusEffect { StatusType type; int durationTurns; float value; }.
   - Hàm updateEffects(Character* target) trừ số lượt hiệu ứng sau mỗi Turn và áp dụng logic (Stun bỏ lượt, Burn trừ HP, Slow giảm spd trong Priority Queue).
3. Observer Pattern:
   - Interface ICombatObserver với hàm virtual void onHealthChanged(Character* target, int damage), virtual void onCharacterDied(Character* target).
   - Class SubjectCombat để TurnManager/Character gọi thông báo cho Observer.

*****[[TASK: RAYLIB GRAPHICS & BATTLE RENDERER]
Tôi là Member 4. Hãy viết C++17 bằng thư viện Raylib cho các thành phần sau:
1. Class SpriteAnimation quản lý Sprite Sheet 2D:
   - Load Texture2D, hàm update(float deltaTime), và DrawFrame(Vector2 pos, bool flipX).
   - Quản lý các trạng thái hoạt ảnh: IDLE, ATTACK, TAKE_DAMAGE.
2. Class BattleUI:
   - Hàm drawHealthBar(Vector2 pos, int currentHp, int maxHp): Vẽ thanh máu HP đỏ/xanh.
   - Hàm drawManaBar(Vector2 pos, int currentMp, int maxMp): Vẽ thanh MP xanh dương.
   - Hàm drawTurnOrderBar(const std::vector<Character*>& turnList): Vẽ danh sách lượt đánh ở góc màn hình.
3. Kế thừa ICombatObserver từ Member 3 để khi sự kiện onHealthChanged() kích hoạt, UI tự động tạo hiệu ứng nhấp nháy hoặc nổi số Damage (Floating Text).

*****[TASK: GAME DATA CONFIG, SOUND ENGINE & MENU SYSTEM]
Tôi là Member 5 (Director). Hãy viết C++17 bằng thư viện Raylib cho các thành phần sau:

1. Class GameData (Data Config Specialist):
   - Chứa các hàm static trả về chỉ số gốc (Stats struct) cho 6 Chủng tộc (Ogre, Human, Dwarf, Elf, Oni, Beastman) và 6 Class (Mage, Swordman, Assassin, Tanker, Enchanter, Paladin).
   - Chứa mảng dữ liệu tĩnh cấu hình thông số cho 34 Skill (gồm m_skillId, m_skillName, m_manaCost, m_damageMultiplier, m_element, m_effectType, m_effectChance).
   - Hàm static Stats getRaceStats(RaceType race) và static Stats getClassStats(ClassType pClass).

2. Class SoundManager (Audio Engine):
   - Sử dụng Raylib Audio để quản lý âm thanh.
   - Hàm loadSounds() để nạp nhạc nền (BGM) và các hiệu ứng âm thanh (SFX: Click button, Attack, SkillCast, Victory).
   - Các hàm playBGM(), stopBGM(), playSFX(SoundType type) và unloadSounds().

3. UI State System (Start Menu & Victory/GameOver Screens):
   - Hàm drawStartMenu(Vector2 mousePos, bool isClicked): Vẽ tiêu đề Game, nút "Start Game" và "Exit". Trả về trạng thái được chọn.
   - Hàm drawEndGameScreen(bool isVictory): Vẽ bảng thông báo Thắng/Thua cùng nút "Play Again" hoặc "Main Menu".
