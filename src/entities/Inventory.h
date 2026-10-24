#pragma once
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

// Túi đồ dạng template. Dùng với T = ItemPtr (std::shared_ptr<Item>) để chứa
// các loại vật phẩm khác nhau (đa hình) trong cùng một túi.
template <typename T>
class Inventory {
private:
    std::vector<T> m_items;

public:
    void addItem(const T& item) {
        m_items.push_back(item);
    }

    bool removeItem(const T& item) {
        auto it = std::find(m_items.begin(), m_items.end(), item);
        if (it != m_items.end()) {
            m_items.erase(it);
            return true;
        }
        return false;
    }

    // Xóa theo vị trí. Trả về false nếu index không hợp lệ.
    bool removeAt(std::size_t index) {
        if (index >= m_items.size()) return false;
        m_items.erase(m_items.begin() + static_cast<std::ptrdiff_t>(index));
        return true;
    }

    // Lấy vật phẩm theo vị trí. Ném std::out_of_range nếu index sai.
    const T& getItem(std::size_t index) const {
        if (index >= m_items.size()) throw std::out_of_range("Inventory::getItem: index ngoai pham vi");
        return m_items[index];
    }

    const std::vector<T>& getItems() const {
        return m_items;
    }

    std::size_t size() const { return m_items.size(); }
    bool isEmpty() const { return m_items.empty(); }
    void clear() { m_items.clear(); }

    // Vị trí của phần tử đầu tiên thỏa pred, -1 nếu không có.
    template <typename Pred>
    int findIndex(Pred pred) const {
        for (std::size_t i = 0; i < m_items.size(); ++i) {
            if (pred(m_items[i])) return static_cast<int>(i);
        }
        return -1;
    }

    template <typename Pred>
    std::size_t countIf(Pred pred) const {
        return static_cast<std::size_t>(std::count_if(m_items.begin(), m_items.end(), pred));
    }
};
