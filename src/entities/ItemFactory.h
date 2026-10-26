#pragma once
#include <string>
#include "entities/ItemDatabase.h"
#include "entities/Item.h"

// Factory Method: tạo đúng lớp con của Item từ dữ liệu CSV.
class ItemFactory
{
public:
    // Ném std::invalid_argument nếu category và effect không khớp nhau.
    static ItemPtr createItem(const ItemData& data);

    // Tạo theo id trong items.csv. Ném std::out_of_range nếu id không tồn tại.
    static ItemPtr createItem(const ItemDatabase& db, const std::string& id);
};
