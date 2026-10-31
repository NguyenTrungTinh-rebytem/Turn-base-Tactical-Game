#include "Strings.h"
#include <cstddef>
namespace {
Strings::Language currentLanguage=Strings::Language::VI;
struct Line {const char* vi;const char* en;};
constexpr Line table[]={
 {"Bắt đầu","Start"},{"Thoát","Exit"},{"Ở lại","Stay"},{"Rời khỏi hành trình?","Leave the journey?"},
 {"Xác nhận","Confirm"},{"Quay lại","Back"},{"Đội hình","Squad"},{"Kỹ năng","Skills"},
 {"Túi đồ","Inventory"},{"Thông tin công","Wave info"},{"Màn","Wave"},{"Lượt","Turn"},
 {"Tấn công","Attack"},{"Kỹ năng","Skill"},{"Vật phẩm","Item"},{"Hủy","Cancel"},
 {"Menu","Menu"},{"Tiếp tục","Resume"},{"Về menu chính","Main menu"},
 {"Chiến thắng","Victory"},{"Thất bại","Defeat"},{"Thử lại","Retry"},
 {"Nhật ký","Battle log"},{"Nhấp vào mục tiêu để thực hiện hành động","Click a target"},
 {"Chọn hành động bằng chuột","Choose a command"},{"Thứ tự lượt","Turn order"},
 {"Thủ lĩnh","Leader"},{"Chỉ số","Stats"},{"Chọn thủ lĩnh","Choose leader"},
 {"Không có vật phẩm","No available items"},{"Thêm nhân vật","Add hero"},
 {"HP","HP"},{"MP","MP"},{"Địch đang hành động","Enemy turn"},
 {"Kết thúc lượt","End turn"},{"Hoàn tác","Undo"},{"Kiểm tra font tiếng Việt","Vietnamese font test"}
};
}
namespace Strings {
void setLanguage(Language lang){currentLanguage=lang;}
Language language(){return currentLanguage;}
const char* get(Key key){
 const auto i=static_cast<std::size_t>(key);
 return i<sizeof(table)/sizeof(Line)?(currentLanguage==Language::VI?table[i].vi:table[i].en):"?";
}
}
