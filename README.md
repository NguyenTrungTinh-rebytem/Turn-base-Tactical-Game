# Turn-base-Tactical-Game
# ⚔️ Pixel Turn-Based RPG (C++ / DSA &amp; OOP Project)
# ⚔️️ [THE FIRST STEP] - 2.5D Turn-Based RPG

> **Môn học:** Lập trình Hướng đối tượng (OOP) & Cấu trúc Dữ liệu và Giải thuật (DSA)  
> **Ngôn ngữ:** C++17
> **Môi trường phát triển:** Visual Studio 2022  
> **Thư viện đồ họa:** Raylib 
---
## 👨‍💻 Thông tin Nhóm / Tác giả
* **Giảng viên hướng dẫn:** 
* **Thành viên thực hiện:**
  * Nguyễn Trung Tỉnh - MSSV:6651071077 (Trưởng nhóm, Character design, Thuyết trình, Director, Monster design)
  * Trần Văn B - MSSV: 20xxxxxx (UI/UX, Render Sprite Animation, Map)
  *
  *
  *
---
## 📌 Giới thiệu Dự án (Overview)
**[THE FIRST STEP]** là một trò chơi chiến thuật theo lượt (Turn-based RPG) góc nhìn 2.5D đơn giản. Trò chơi gồm 2 màn chơi (Stage 1: Quái thường, Stage 2: Trận chiến với Boss) với cơ chế chiến đấu tính lượt dựa trên chỉ số tốc độ của nhân vật.
Dự án được xây dựng nhằm ứng dụng các nguyên lý lập trình hướng đối tượng (OOP) cùng các cấu trúc dữ liệu và giải thuật cơ bản trong C++, kết hợp hiển thị đồ họa dạng Pixel Art Sprite Sheet.
---
## 💡 Các kiến thức OOP & DSA áp dụng
### 1. Lập trình Hướng đối tượng (OOP)
* **Encapsulation (Đóng gói):** Các thuộc tính chỉ số (`m_hp`, `m_maxHp`, `m_speed`, `m_baseDamage`) được ẩn giấu dạng `private`/`protected`, chỉ truy cập qua các phương thức Getter/Setter.
* **Inheritance (Kế thừa):**
  * Lớp cơ sở `Character` $\rightarrow$ các lớp con `Player`, `Enemy`, `Boss`.
  * Lớp cơ sở `Skill` $\rightarrow$ các lớp con `DamageSkill`, `HealSkill`.
* **Polymorphism (Đa hình):**
  * Ghi đè phương thức ảo `virtual void takeDamage()` và `virtual void useSkill()` để xử lý riêng biệt cho từng loại nhân vật/kỹ năng.
* **Smart Pointers:** Sử dụng `std::unique_ptr` và `std::shared_ptr` để quản lý bộ nhớ động an toàn, tránh Memory Leak.
### 2. Cấu trúc Dữ liệu & Giải thuật (DSA)
* **Priority Queue (Hàng chờ ưu tiên):** Quản lý thứ tự lượt đánh (Turn Order) trong trận đấu. Nhân vật nào có chỉ số `Speed` cao hơn sẽ được sắp xếp lên đầu hàng chờ để đi trước.
* **Vector (`std::vector`):** Quản lý danh sách Kẻ địch trong màn chơi, danh sách Kỹ năng (`Skills`), và Túi đồ (`Inventory`).
* **Finite State Machine (FSM):** Quản lý luồng trạng thái của Game (`GameState`: `MENU`, `PLAYING_TURN`, `PLAYER_CHOICE`, `ENEMY_TURN`, `VICTORY`, `GAME_OVER`).
* **AI Decision Tree (Cây quyết định đơn giản):** Xử lý lượt đi tự động của Enemy/Boss (tự chọn đánh thường, tung chiêu mạnh hoặc hồi máu khi HP dưới 30%).
---
## 📸 Demo Game
*(Chèn 1-2 ảnh screenshot hoặc GIF quay cảnh game đang chạy vào hàng dấu !)*
!
!
---
## 🛠️ Hướng dẫn Biên dịch & Chạy Dự án (Build & Run)




### Yêu cầu hệ thống
* Visual Studio 2022 (đã cài gói **Desktop development with C++**).
* Thư viện **Raylib** 

### Các bước mở và chạy trên Visual Studio 2022
1. 
