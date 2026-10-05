# ⚔️ [THE FIRST STEP] - 2.5D Pixel Turn-Based RPG

> **Môn học:** Lập trình Hướng đối tượng (OOP) & Cấu trúc Dữ liệu và Giải thuật (DSA)  
> **Đề tài:** ĐT10 - Động cơ Game Nhập vai Chiến thuật Turn-based RPG (Battle Engine)  
> **Ngôn ngữ:** C++17 | **Môi trường:** Visual Studio 2022 | **Thư viện đồ họa:** Raylib  

---

## 👨‍💻 Thông tin Nhóm & Phân công Nhiệm vụ

* **Giảng viên hướng dẫn:** 
* **Thành viên thực hiện:**

| STT | Họ và Tên | MSSV | Vai trò & Phân công Nhiệm vụ | Nhánh Git (Branch) |
| :-: | :--- | :-: | :--- | :--- |
| **1** | **Nguyễn Trung Tỉnh** | **6651071077** | **Trưởng nhóm (Director)**<br>• Quản lý dự án, thiết kế Character & Monster, làm Slide/Báo cáo.<br>• Code Module: Data Engine (`GameData`), Audio Engine (`SoundManager`), UI Menu. | `feature/member5-data-sound` |
| **2** | **Member 1** | *(Sửa MSSV)* | **Core Engine & FSM Specialist**<br>• Xây dựng Game Loop, Máy trạng thái FSM (`GameStateMachine`).<br>• Cài đặt **Command Pattern** (Đánh, Skill, Item & Cancel/Back nước đi). | `feature/member1-engine` |
| **3** | **Member 2** | *(Sửa MSSV)* | **OOP Entities & Factory Specialist**<br>• Cây kế thừa `Character` $\rightarrow$ `Player`/`Enemy`/`Boss`.<br>• Công thức Dame Ngũ hành $5 \times 5$, **Factory Method**, Template `Inventory<T>`. | `feature/member2-entities` |
| **4** | **Member 3** | *(Sửa MSSV)* | **DSA & Turn System Specialist**<br>• `TurnManager` dùng `std::priority_queue` xếp lượt theo `m_spd`.<br>• Quản lý 7 hiệu ứng Buff/Debuff, **Observer Pattern**, AI Boss. | `feature/member3-dsa` |
| **5** | **Member 4** | *(Sửa MSSV)* | **UI/UX & Graphics Specialist**<br>• Render Sprite Animation 2D (Idle/Attack), dựng bản đồ (Map).<br>• Vẽ HUD thanh máu HP/MP Bar, render danh sách Turn Order & UI. | `feature/member4-graphics` |

---

## 📌 Giới thiệu Dự án (Overview)

**[THE FIRST STEP]** là trò chơi nhập vai chiến thuật theo lượt (Turn-based RPG) góc nhìn 2.5D sử dụng đồ họa Pixel Art. Trò chơi trải qua 2 màn đấu chính (Stage 1: Trận chiến với Quái thường, Stage 2: Trận chiến sinh tồn với Boss) với cơ chế tính lượt theo Tốc độ (Speed) và Ma trận Khắc hệ Ngũ Hành.

Dự án được xây dựng nhằm ứng dụng các nguyên lý lập trình hướng đối tượng (OOP) nâng cao, các cấu trúc dữ liệu - giải thuật cốt lõi trong C++17 và áp dụng các Design Patterns thực tế trong thiết kế Game Engine.

---

## 💡 Kiến trúc OOP, DSA & Design Patterns Áp dụng

### 1. Lập trình Hướng đối tượng (OOP)
* **Encapsulation (Đóng gói):** Các thuộc tính chỉ số (`m_hp`, `m_maxHp`, `m_mp`, `m_spd`, `m_a`, `m_ma`, `m_d`, `m_md`) được bảo vệ bằng `private`/`protected`, chỉ truy cập qua Getter/Setter chuẩn.
* **Inheritance (Kế thừa):** Cây kế thừa mở rộng từ `Character` $\rightarrow$ `Player`, `Enemy`, `Boss`.
* **Polymorphism (Đa hình):** Ghi đè các phương thức ảo `virtual void takeDamage()` và `virtual void executeSkill()` để xử lý riêng biệt cho từng loại nhân vật và kỹ năng.
* **Operator Overloading:** Nạp chồng toán tử `+` cho struct `Stats` (cộng dồn chỉ số) và toán tử `<` hỗ trợ so sánh Tốc độ.
* **Template Class:** Xây dựng `Inventory<T>` quản lý túi đồ/trang bị linh hoạt.
* **Smart Pointers:** Quản lý vòng đời đối tượng an toàn bằng `std::unique_ptr` và `std::shared_ptr`, tuyệt đối chống thất thoát bộ nhớ (Memory Leak).

### 2. Cấu trúc Dữ liệu & Giải thuật (DSA)
* **Priority Queue (`std::priority_queue`):** Quản lý thứ tự lượt đánh (Turn Order) tự động theo chỉ số Speed (`m_spd`). Nhân vật có tốc độ cao hơn sẽ được đi trước.
* **Finite State Machine (FSM):** Quản lý luồng trạng thái game (`State::MENU`, `State::IN_BATTLE`, `State::VICTORY`, `State::GAME_OVER`).
* **AI Decision Tree (Cây quyết định):** Xử lý lượt đi tự động cho Enemy/Boss (tự động chọn đánh thường, tung skill sát thương lớn hoặc hồi máu/buff khi HP xuống dưới 30%).
* **Vector (`std::vector`):** Quản lý danh sách Kẻ địch, danh sách Kỹ năng (`Skills`), Túi đồ và hàng đợi Command.

### 3. Design Patterns
* **Observer Pattern:** Class `SubjectCombat` phát sự kiện `onHealthChanged()` và `onCharacterDied()`. Modules UI và Sound đăng ký làm `ICombatObserver` để cập nhật thanh máu và phát hiệu ứng âm thanh.
* **Command Pattern:** Đóng gói hành động chiến đấu vào `ICommand` (`AttackCommand`, `UseSkillCommand`, `UseItemCommand`). Hỗ trợ danh sách `CommandStack` cho phép người chơi bấm **Cancel / Back (Undo chọn lại nước đi)** trước khi chốt lượt.
* **Factory Method:** Class `CharacterFactory` khởi tạo ngẫu nhiên Nhân vật/Quái vật dựa trên chỉ số gốc Race + Class + Element.

---

## 📐 Quy chuẩn Code & Đặt tên (Naming Conventions)

Để đảm bảo đồng bộ 100% giữa 5 thành viên khi merge code trên GitHub, toàn bộ dự án BẮT BUỘC tuân thủ:

* **Biến thành viên Class (Member variables):** Bắt buộc có tiền tố `m_` và dùng `camelCase` (Ví dụ: `m_hp`, `m_maxHp`, `m_mp`, `m_spd`, `m_element`, `m_skills`).
* **Tên Class / Struct / Enum:** Dùng `PascalCase` (Ví dụ: `Character`, `TurnManager`, `CombatObserver`, `CommandStack`).
* **Tên Hàm (Methods):** Dùng `camelCase` (Ví dụ: `takeDamage()`, `executeSkill()`, `getSpeed()`, `cancelLastAction()`).
* **Hằng số / Enum Values:** Dùng `UPPER_SNAKE_CASE` (Ví dụ: `KIM`, `MOC`, `THUY`, `HOA`, `THO`, `ATK_BOOST`, `STUN`).

---

## 🎮 Cơ chế Game (Game Mechanics)

### A. Công thức Chỉ số (Base Stats)
Chỉ số khởi tạo của Anh hùng dựa trên sự kết hợp giữa Chủng tộc và Lớp nhân vật:
$$\text{Base Stats} = \text{Race Stats} + \text{Class Stats}$$
* **6 Chủng tộc (Races):** Ogre, Human, Dwarf, Elf, Oni, Beastman.
* **6 Class:** Mage, Swordman, Assassin, Tanker, Enchanter, Paladin.

### B. Ma trận Khắc hệ Ngũ Hành ($5 \times 5$)
* **5 Hệ:** Kim (0), Mộc (1), Thủy (2), Hỏa (3), Thổ (4).
* **Công thức Dame:** 
* **Hệ số:** Tương khắc (+50% Dame $\rightarrow 1.5$), Bị khắc (-50% Dame $\rightarrow 0.5$), Bình hòa ($1.0$).

### C. Kỹ năng & Hiệu ứng
* **34 Skill:** Mỗi nhân vật khi khởi tạo được cấp 4 Skill ($1 \text{ Race Skill} + 1 \text{ Class Skill} + 2 \text{ Element Skills}$).
* **7 Hiệu ứng Buff/Debuff:** `ATK_BOOST`, `DEF_BOOST`, `REGEN`, `STUN` (bỏ lượt), `BURN` (mất HP theo turn), `SLOW` (giảm Speed), `WEAKNESS`.

---

## 📂 Cấu trúc Thư mục Project (Directory Structure)

```text
RPG_Game_DT10/
├── assets/                 # Tài nguyên game do Director quản lý
│   ├── images/             # Sprite sheets Pixel Art, Background, Icons
│   └── audio/              # Sound FX (SFX) & Music (BGM)
├── src/                    # Source Code C++
│   ├── core/               # Member 1: Game Loop, FSM, Command Pattern
│   ├── entities/           # Member 2: Character, Player, Enemy, Factory, Inventory<T>
│   ├── dsa/                # Member 3: TurnManager (Priority Queue), Status Effects, Observer
│   ├── graphics/           # Member 4 (Trần Văn B): Raylib Renderer, Sprite Animation, Map, UI
│   └── data/               # Member 5 (Nguyễn Trung Tỉnh): GameData, SoundManager, Start Menu
├── .gitignore              # Git Ignore file cho Visual Studio 2022
├── PROMPT_SYSTEM.md        # Bộ Prompt đồng bộ AI cho 5 thành viên
└── README.md               # Tài liệu dự án




