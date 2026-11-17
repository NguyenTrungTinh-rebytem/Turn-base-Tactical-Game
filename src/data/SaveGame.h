#pragma once
// src/data/SaveGame.h
// FILE LUU TIEN TRINH (savegame.dat, dang key=value de de doc/debug).
// Luu tai MOC ON DINH: ngay sau khi qua wave (chua vao wave ke tiep) -> khong can luu hang doi luot / hieu ung giua tran.
// Module nay KHONG biet ve GameEngine: Engine dien SaveData roi goi save(); load xong Engine tu dung lai doi hinh.
#include <string>
#include <utility>
#include <vector>

struct SaveHero {
    std::string m_name;
    std::string m_race;
    std::string m_class;
    int m_element = 0;     // static_cast<int>(Element)
    int m_hp = 1;          // 0 = da guc
    int m_mp = 0;
};

struct SaveData {
    int m_version = 1;
    int m_nextWave = 1;                                   // wave se choi tiep khi tai
    int m_gold = 0;
    std::string m_savedAt;                                // "dd/mm/yyyy HH:MM"
    std::vector<SaveHero> m_heroes;                       // theo thu tu trong doi hinh (toi da 3)
    std::vector<std::pair<std::string, int>> m_bag;       // id vat pham trong tui + so luong
    std::string m_gear[3];                                // id trang bi dang deo cua tung hero; rong = khong deo
};

namespace SaveGame {
    bool exists(const std::string& path);
    bool save(const std::string& path, SaveData data);    // tu dong ghi m_savedAt. false = khong ghi duoc
    bool load(const std::string& path, SaveData& out);    // false = khong co file / file hong
    bool remove(const std::string& path);
}
