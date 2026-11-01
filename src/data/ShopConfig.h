#pragma once
// src/data/ShopConfig.h
// Gia vat pham (data/shop.csv) + cau hinh tien/roi tien (data/economy.csv). Sua CSV, KHONG can bien dich lai.
//
// shop.csv    :  id,price         (id = id trong items.csv; vat pham khong co trong file nay thi khong ban)
// economy.csv :  key,value        (cac khoa xem struct EconomyConfig; thieu khoa -> dung gia tri mac dinh)
// Dong bat dau bang '#' la ghi chu. Dong header (id,price / key,value) duoc bo qua.
#include <string>
#include <vector>

struct ShopEntry {
    std::string m_id;
    int         m_price = 0;
};

struct EconomyConfig {
    int m_startGold = 100;          // tien luc bat dau mot chuyen di
    int m_enemyBaseGold = 8;        // tien co ban moi quai thuong
    int m_enemyHpDivisor = 10;      // + maxHP / so nay (quai manh hon -> nhieu tien hon). 0 = tat
    int m_bossMultiplier = 5;       // boss nhan he so nay
    int m_waveClearBonus = 40;      // thuong them khi quet sach 1 wave
    int m_variancePercent = 20;     // dao dong +-% de moi lan roi hoi khac nhau
    int m_skipWaveGold = 60;        // tien tang them cho moi wave bi bo qua khi chon thang wave cao (vao thang boss)
};

class ShopConfig {
public:
    // Doc <dataDir>/shop.csv va <dataDir>/economy.csv. Thieu file -> giu mac dinh, tra ve false (khong nem loi).
    bool load(const std::string& dataDir);

    const std::vector<ShopEntry>& getEntries() const { return m_entries; }
    bool isSold(const std::string& itemId) const;
    int  getPrice(const std::string& itemId) const;          // 0 neu khong ban
    const EconomyConfig& getEconomy() const { return m_economy; }

    // Mac dinh khi khong co shop.csv: tu them cac id cho truoc voi gia mac dinh.
    void setEntries(std::vector<ShopEntry> entries) { m_entries = std::move(entries); }

private:
    std::vector<ShopEntry> m_entries;
    EconomyConfig m_economy;
};
