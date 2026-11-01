# ⚔️ [THE FIRST STEP] - 2.5D Pixel Turn-Based RPG

> **Môn học:** Lập trình Hướng đối tượng (OOP) & Cấu trúc Dữ liệu và Giải thuật (DSA)  
> **Đề tài:** ĐT10 - Động cơ Game Nhập vai Chiến thuật Turn-based RPG (Battle Engine)  
> **Ngôn ngữ:** C++17 | **Build:** CMake + Ninja (MinGW/MSYS2) hoặc Visual Studio 2022 | **Thư viện đồ họa:** Raylib  

---

## 👨‍💻 Thông tin Nhóm & Phân công Nhiệm vụ

* **Giảng viên hướng dẫn:** 
* **Thành viên thực hiện:**

| STT | Họ và Tên | MSSV | Vai trò & Phân công Nhiệm vụ | Nhánh Git (Branch) |
| :-: | :--- | :-: | :--- | :--- |
| **1** | **Nguyễn Trung Tỉnh** | **6651071077** | **Trưởng nhóm (Director)**<br>• Quản lý dự án, thiết kế Character & Monster, làm Slide/Báo cáo.<br>• Code Module: Data Engine (`GameData`), Audio Engine (`SoundManager`), UI Menu. | `feature/member5-data-sound` |
| **2** | **Võ Tấn Tiến**| **6651071074** | **Core Engine & FSM Specialist**<br>• Xây dựng Game Loop, Máy trạng thái FSM (`GameStateMachine`).<br>• Cài đặt **Command Pattern** (Đánh, Skill, Item & Cancel/Back nước đi). | `feature/member1-engine` |
| **3** | **Nguyễn Quang Nhân Kiệt** | *6651071035* | **OOP Entities & Factory Specialist**<br>• Cây kế thừa `Character` $\rightarrow$ `Player`/`Enemy`/`Boss`.<br>• Công thức Dame Ngũ hành $5 \times 5$, **Factory Method**, Template `Inventory<T>`. | `feature/member2-entities` |
| **4** | **Nguyễn Quốc Đạt** | *6651071014* | **DSA & Turn System Specialist**<br>• `TurnManager` dùng `std::priority_queue` xếp lượt theo `m_spd`.<br>• Quản lý 7 hiệu ứng Buff/Debuff, **Observer Pattern**, AI Boss. | `feature/member3-dsa` |
| **5** | **Huỳnh Quang Khải** | **6651071030** | **UI/UX & Graphics Specialist**<br>• Render Sprite Animation 2D (Idle/Attack), dựng bản đồ (Map).<br>• Vẽ HUD thanh máu HP/MP Bar, render danh sách Turn Order & UI. | `feature/member4-graphics` |

---

## 📌 Giới thiệu Dự án (Overview)

**[THE FIRST STEP]** là trò chơi nhập vai chiến thuật theo lượt (Turn-based RPG) góc nhìn 2.5D sử dụng đồ họa Pixel Art. Người chơi lập đội tối đa **3 anh hùng**, vượt qua **3 wave**: 2 wave Quái thường và 1 wave **Boss**. Có **Shop + Trang bị** mở ngay trước wave Boss. Trận đấu tính lượt theo Tốc độ (Speed) và Ma trận Khắc hệ Ngũ Hành.

Dự án ứng dụng các nguyên lý OOP nâng cao, cấu trúc dữ liệu - giải thuật cốt lõi trong C++17 và các Design Patterns thực tế trong thiết kế Game Engine.

### Vòng chơi (Game Flow)

```text
MENU -> CREATE_HERO (chọn / tạo đội) -> WAVE_SELECT -> PREP_MENU (chuẩn bị) -> IN_BATTLE
        Wave 1 (3 quái thường ngẫu nhiên) -> Wave 2 (3 quái tinh anh ngẫu nhiên)
        -> PREP_MENU + SHOP (nghỉ trước Boss) -> Wave 3 (Boss) -> VICTORY / GAME_OVER
```

| Wave | Nội dung | Ghi chú |
| :-: | :--- | :--- |
| 1 | Ngẫu nhiên 3 trong 5 quái thường (Goblin, Wolf, Slime, Imp, Golem) | Nền `bg_wave1` |
| 2 | Ngẫu nhiên 3 trong 5 quái tinh anh (mạnh hơn khoảng 1.6 lần) | Sau wave này mở Shop |
| 3 | Boss Ember Dragon | Nền `bg_wave2`, shader tông Boss |

* **Hồi phục sau mỗi wave:** mọi nhân vật còn sống hồi **20% HP và 20% MP** tối đa. Nhân vật đã gục phải dùng vật phẩm hồi sinh.
* **Số wave tự suy ra từ dữ liệu:** wave cuối là `stage` lớn nhất trong `data/monsters.csv`; wave có Boss được nhận biết bằng cột `isBoss`. Muốn thêm wave chỉ cần thêm dòng quái vào CSV.

---

## 💡 Kiến trúc OOP, DSA & Design Patterns Áp dụng

### 1. Lập trình Hướng đối tượng (OOP)
* **Encapsulation (Đóng gói):** Chỉ số (`m_hp`, `m_mp`, `m_spd`, `m_a`, `m_ma`, `m_d`, `m_md`) được bảo vệ bằng `private`/`protected`, truy cập qua Getter/Setter.
* **Inheritance (Kế thừa):** `Character` $\rightarrow$ `Player`, `Enemy`, `Boss`; `Item` $\rightarrow$ `Potion`, `ManaPotion`, `ReviveItem`, `BoosterItem`, `HeldGear`.
* **Polymorphism (Đa hình):** Ghi đè `virtual takeDamage()`, `virtual executeSkill()`, và hiệu ứng sử dụng vật phẩm (`Item`).
* **Operator Overloading:** Toán tử `+` cho `Stats` và toán tử `<` so sánh Tốc độ.
* **Template Class:** `Inventory<T>` quản lý túi đồ dùng chung cho cả đội (giới hạn số lượng mang theo theo từng vật phẩm).
* **Smart Pointers:** `std::unique_ptr`, `std::shared_ptr` (`ItemPtr`) quản lý vòng đời đối tượng.

### 2. Cấu trúc Dữ liệu & Giải thuật (DSA)
* **Priority Queue:** `TurnManager` xếp thứ tự lượt theo `m_spd`.
* **Finite State Machine (FSM):** `GameStateMachine` với các trạng thái `MENU`, `CREATE_HERO`, `WAVE_SELECT`, `PREP_MENU`, `IN_BATTLE`, `VICTORY`, `GAME_OVER`. Shop là lớp phủ (overlay) trong `PREP_MENU`, không phải trạng thái riêng.
* **AI Decision Tree:** Enemy/Boss tự chọn đánh thường, skill mạnh hoặc hồi máu/buff khi HP dưới 30%.
* **Hash Map (`std::unordered_map`):** Tra cứu vật phẩm, giá Shop, sprite quái theo id.
* **Vector (`std::vector`):** Danh sách kẻ địch, kỹ năng, túi đồ, hàng đợi Command.
* **Random shuffle (`std::shuffle`):** Chọn ngẫu nhiên 3 quái cho mỗi wave thường.

### 3. Design Patterns
* **Observer:** `SubjectCombat` phát `onHealthChanged()` / `onCharacterDied()`; UI và Âm thanh đăng ký làm `ICombatObserver`.
* **Command:** `AttackCommand`, `UseSkillCommand`, `UseItemCommand` trong `CommandStack` cho phép **Cancel / Back (Undo)** trước khi chốt lượt (Undo khôi phục cả HP, MP, hiệu ứng; vàng chỉ được cộng khi chốt lượt nên Undo không bị lợi dụng).
* **Factory Method:** `CharacterFactory` (nhân vật/quái theo Race + Class + Element) và `ItemFactory` (tạo vật phẩm từ `data/items.csv`).
* **Data-Driven Design:** Chỉ số quái, vật phẩm, giá Shop, kinh tế, anh hùng dựng sẵn đều đọc từ CSV, chỉnh cân bằng không cần biên dịch lại.

---

## 📐 Quy chuẩn Code & Đặt tên (Naming Conventions)

Toàn bộ dự án BẮT BUỘC tuân thủ:

* **Biến thành viên:** tiền tố `m_` + `camelCase` (`m_hp`, `m_maxHp`, `m_gold`, `m_shopOpen`).
* **Class / Struct / Enum:** `PascalCase` (`Character`, `TurnManager`, `ShopScreen`).
* **Hàm:** `camelCase` (`takeDamage()`, `recoverPartyAfterWave()`).
* **Hằng số / Enum values:** `UPPER_SNAKE_CASE` (`KIM`, `ATK_BOOST`, `WAVE_CLEAR_RECOVER_RATE`).
* **Chuỗi hiển thị trong game:** tiếng Việt **không dấu** (font mặc định của Raylib không có dấu tiếng Việt).

---

## 🎮 Cơ chế Game (Game Mechanics)

### A. Công thức Chỉ số (Base Stats)
$$\text{Base Stats} = \text{Race Stats} + \text{Class Stats}$$
* **6 Chủng tộc:** Ogre, Human, Dwarf, Elf, Oni, Beastman.
* **6 Class:** Mage, Swordman, Assassin, Tanker, Enchanter, Paladin.
* Luật Race x Class (được phép / không) nằm trong `data/CharacterRules`.

### B. Ma trận Khắc hệ Ngũ Hành ($5 \times 5$)
* **5 Hệ:** Kim, Mộc, Thủy, Hỏa, Thổ.
* **Hệ số:** Tương khắc $\rightarrow 1.5$, Bị khắc $\rightarrow 0.5$, Bình hòa $\rightarrow 1.0$.
* Nguyên tố của từng skill cấu hình trong `data/skill_elements.csv`.

### C. Kỹ năng & Hiệu ứng
* **34 Skill:** mỗi nhân vật có 4 Skill ($1$ Race + $1$ Class + $2$ Element).
* **7 Hiệu ứng:** `ATK_BOOST`, `DEF_BOOST`, `REGEN`, `STUN`, `BURN`, `SLOW`, `WEAKNESS`.

### D. Đội hình & Anh hùng dựng sẵn
* Đội tối đa **3 anh hùng**, chọn ở màn hình tạo đội / chuẩn bị.
* **10 anh hùng dựng sẵn** (`data/heroes.csv`): Lyra, Kael, Sylwen, Kurogane, Fenrir, Grumm, Thorin, Aldric, Roland, Brokk. Mỗi người có sprite chiến đấu và bộ animation riêng (idle / attack / hurt / death).

### E. Kinh tế, Vật phẩm, Shop & Trang bị
* **Vàng:** quái chết rơi vàng (dựa trên HP, có dao động ngẫu nhiên; Boss nhân hệ số lớn) + thưởng qua màn. Thông số trong `data/economy.csv` (`start_gold`, `enemy_base_gold`, `enemy_hp_divisor`, `variance_percent`, `boss_multiplier`, `wave_clear_bonus`, `skip_wave_gold`).
* **16 vật phẩm** (`data/items.csv`, ảnh `assets/images/items/ITM_*.png`):
  * Tiêu hao: HP Minor/Major, Energy, Phoenix (hồi sinh).
  * Buff trận: Power, Iron, Haste, Precision.
  * Trang bị mang theo (`HeldGear`, mỗi người 1 món): Leftover, Vampiric, Survival Sash, Guardian, Berserk, Executioner, Heavy, Counter.
* **Shop** (`data/shop.csv` - id, giá): mở ở màn Chuẩn bị trước wave Boss (nút Shop hoặc phím **B**). Hai tab: **Cửa hàng** và **Kho đồ / Trang bị**.
* Mỗi vật phẩm có giới hạn mang theo (`maxCarry`); túi đồ dùng chung cả đội.

### F. Âm thanh & Hiệu ứng hình ảnh
* **AudioManager** (`src/audio`): nhạc nền theo trạng thái (menu, thám hiểm, chiến đấu, game over), tiếng click, tiếng đánh/bị đánh/chết riêng cho anh hùng nam, nữ, quái thường và Rồng.
* **SceneShader** (`src/graphics`): hậu kỳ cho cảnh chiến đấu - tối viền (vignette), bloom nhẹ, mờ tilt-shift, chỉnh màu, tia nắng xuyên tán lá chuyển động và hạt bụi/tàn lửa; tông màu khác nhau cho wave thường và Boss. UI vẽ sau shader nên vẫn sắc nét. Nếu GPU không hỗ trợ, game tự chạy không shader. Bóng đổ dưới chân nhân vật vẽ riêng trong `GameEngine`.

### G. Điều khiển chính
| Phím | Chức năng |
| :-: | :--- |
| **A** | Tấn công thường |
| **S** | Mở menu Kỹ năng |
| **I** | Mở túi Vật phẩm trong trận |
| **TAB** | Đổi mục tiêu |
| **F1 - F3** | Chọn đồng đội |
| **ESC** | Quay lại / Menu |
| **B** | Mở / đóng Shop (màn Chuẩn bị trước Boss) |
| **F9** | Bật / tắt shader cảnh |

---

## 🛠️ Hướng dẫn Build & Chạy

**Yêu cầu:** CMake, Ninja, trình biên dịch C++17 (MinGW-w64 / MSYS2 ucrt64 hoặc MSVC), Raylib 5.5.

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
.\build\game.exe
```

* CMake dùng `file(GLOB_RECURSE ...)` nên **khi thêm file `.cpp` mới phải xóa thư mục `build` và cấu hình lại**:
  `Remove-Item -Recurse -Force build` rồi chạy lại lệnh cmake ở trên.
* Game tìm thư mục `assets/` và `data/` theo đường dẫn tương đối; thiếu ảnh vật phẩm sẽ hiện ô chữ cái thay thế, thiếu âm thanh game chạy im lặng.

---

## 📂 Cấu trúc Thư mục Project (Directory Structure)

```text
RPG_Game_DT10/
├── assets/
│   ├── images/             # Sprite anh hùng (hero_<id>_idle/attack/hurt/death/battle.png),
│   │   │                   # sprite quái (*_sheet.png), background wave
│   │   └── items/          # Icon vật phẩm ITM_*.png
│   └── audio/              # Nhạc nền (mp3) & SFX (wav/ogg)
├── data/                   # Dữ liệu CSV: heroes, monsters, items, shop, economy,
│                           #   races, classes, skills, skill_elements
├── src/
│   ├── core/               # Member 1 (Võ Tấn Tiến): GameEngine, FSM, Command Pattern
│   ├── entities/           # Member 2 (Nguyễn Quang Nhân Kiệt): Character, Player, Enemy, Boss,
│   │                       #   Factory, Inventory<T>, Item, ItemFactory, Party
│   ├── dsa/                # Member 3 (Nguyễn Quốc Đạt): TurnManager, Status Effects, Observer, AI
│   ├── graphics/           # Member 4 (Huỳnh Quang Khải): Sprite Animation, Map, BattleUI,
│   │                       #   PrepMenuScreen, ShopScreen, ItemIcons, SceneShader
│   ├── audio/              # AudioManager (nhạc nền + SFX)
│   └── data/               # Member 5 (Nguyễn Trung Tỉnh): GameData, ItemDatabase, ShopConfig,
│                           #   HeroPresetConfig, CharacterRules, Start Menu
├── CMakeLists.txt          # Build bằng CMake + Ninja
├── .gitignore
├── PROMPT_SYSTEM.md        # Bộ Prompt đồng bộ AI cho 5 thành viên
└── README.md               # Tài liệu dự án
```
