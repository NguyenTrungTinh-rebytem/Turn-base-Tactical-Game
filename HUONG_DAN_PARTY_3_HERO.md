# Khac phuc Party 3 Hero (Core v3)

## Vi sao ban cu khong tao duoc 3 Hero?
- V2 dung phim T de tang so luong, tao dong doi tu dong trong GameEngine::createHero, chua cho nguoi choi chon tung Hero.
- Tab DOI HINH chi ve Hero dau tien va nhung o trong; khi tang so luong, so o duoc ve lai it di.
- EXE trong Game-Windows/ la ban build cu, khong tu cap nhat theo ma nguon.

## Tao 3 Hero o ban moi
1. Tu MENU -> BAT DAU -> chon Hero thu linh -> XAC NHAN.
2. Chon Wave -> vao man CHUAN BI TRAN DAU, tab DOI HINH.
3. Click o DONG DOI 2 hoac nhan F2; chon nhan vat trong man hinh CHON DONG DOI 2/3 -> XAC NHAN.
4. Click o DONG DOI 3 hoac nhan F3; chon nhan vat -> XAC NHAN.
5. O DOI HINH phai hien du 3 the co ten, chung toc, lop, he va chi so. Nhap ENTER hoac bam VAO TRAN.
6. Moi Hero da chon co luot va skill rieng (luot do TurnManager quan ly).

- F1: sua Thu linh; F2: sua dong doi 2; F3: sua dong doi 3.
- T: them dong doi tiep theo, khi du ba nguoi thi cho doi dong doi 3.
- Click o thu 3 khi chua tao o thu 2 se chuyen sang chon o thu 2; can tao lien tiep 1-2-3.
- ESC khi dang chon dong doi quay ve man CHUAN BI TRAN DAU.
- Neu khong them Hero thi van co the vao tran voi 1 Hero.

## Build voi MSYS2 UCRT64
cd /d/CODE/Turn-base-Tactical-Game
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --target game --parallel 4
./build-ucrt64/game.exe

Can chay build-ucrt64/game.exe da build MOI. Game-Windows/game.exe CU khong co tinh nang nay.

## Kiem thu
cmake --build build-ucrt64 --target test_core_integration test_entities --parallel 4
ctest --test-dir build-ucrt64 --output-on-failure

Do chua chay duoc cua so Windows/Raylib trong moi truong tao ban va, viec chay UI thuc te tren Windows can kiem tra.
