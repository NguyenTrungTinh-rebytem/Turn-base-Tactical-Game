#pragma once
#include <vector>
#include <algorithm>

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

    const std::vector<T>& getItems() const {
        return m_items;
    }
};
