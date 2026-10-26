#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "entities/ItemData.h"

// Lỗi đọc/kiểm tra items.csv. what() chứa tên file + dòng + nguyên nhân.
class ItemLoadError : public std::runtime_error {
public:
    explicit ItemLoadError(const std::string& message) : std::runtime_error(message) {}
};

// Đọc data/items.csv trực tiếp lúc chạy. Thêm/sửa vật phẩm = sửa CSV, KHÔNG cần biên dịch lại.
//
// Định dạng: dòng đầu (không phải '#') là HEADER, đọc theo TÊN cột nên đổi thứ tự cột hoặc
// thêm cột mới không làm hỏng. Cột bắt buộc: id, name, category, effect, value.
// Cột tùy chọn: nameVi, value2, duration, maxCarry, description (ô trống = 0 / rỗng).
// Hỗ trợ ô đặt trong "dấu nháy kép" (chứa dấu phẩy), file có BOM UTF-8 và xuống dòng CRLF
// (Excel lưu CSV trên Windows).
class ItemDatabase {
public:
    // Nạp <dataDir>/items.csv. Ném ItemLoadError nếu sai; khi lỗi, dữ liệu cũ được giữ nguyên.
    void load(const std::string& dataDir);

    // Giống load() nhưng KHÔNG ném: trả false và ghi lý do vào *error. Dùng khi muốn game
    // vẫn chạy (không có vật phẩm) thay vì dừng nếu items.csv hỏng.
    bool tryLoad(const std::string& dataDir, std::string* error = nullptr);

    // Nạp lại từ thư mục đã dùng lần trước (để thử số liệu mới mà không khởi động lại game).
    void reload();

    bool isLoaded() const { return m_loaded; }
    const std::string& getSourcePath() const { return m_sourcePath; }

    const std::vector<ItemData>& getItems() const { return m_items; }
    const ItemData* findItem(const std::string& id) const;   // nullptr nếu không có
    const ItemData& getItem(const std::string& id) const;    // ném std::out_of_range
    std::vector<ItemData> getItemsByCategory(ItemCategory category) const;

private:
    std::string m_dataDir;
    std::string m_sourcePath;
    std::vector<ItemData> m_items;
    bool m_loaded = false;
};
