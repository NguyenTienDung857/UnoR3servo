TÀI LIỆU CẬP NHẬT THEO FIRMWARE HIỆN TẠI (Arduino UNO R3)

Nguồn đối chiếu: src/main.cpp và include/config.h

1) SƠ ĐỒ CHÂN PHẦN CỨNG ĐANG DÙNG

- D7: RelayBplus
	- Mặc định khởi động: OFF (RELAY_OFF)
	- Lệnh COM "1": ON
	- Lệnh COM "2": OFF

- D2: ServoFontCam
	- Lệnh COM "3": quay về HOME 0 độ
	- Lệnh COM "4": quay đến 45 độ

- D5: ServoSDcard
	- State machine non-blocking bằng millis()
	- Giới hạn góc tối đa 93 độ
	- Lệnh COM "7": chạy 1 chu kỳ

- D3: ServoOled
	- Lệnh COM "8": quay nhanh 90 độ, chờ 0.5s, quay về 0 độ

- SDA/SCL: OLED 1.3 inch SH1106 I2C
	- Cấu hình xoay ngược màn hình: U8G2_R2 (180 độ)



2) BẢNG LỆNH COM ĐANG HỖ TRỢ

- Chuỗi 1 ký tự:
	- "1": RelayBplus ON
	- "2": RelayBplus OFF
	- "3": ServoFontCam về 0 độ
	- "4": ServoFontCam tới 45 độ
	- "7": bắt đầu chu kỳ ServoSDcard
	- "8": bắt đầu chu kỳ ServoOled

- Chuỗi 2 ký tự:
	- "44": xóa màn hình OLED

- Chuỗi 5 ký tự (tất cả phải là số):
	- Dạng "ABCDE"
	- P1 = A
	- P2 = BC (bỏ số 0 đầu nếu có)
	- P3 = DE (bỏ số 0 đầu nếu có)
	- Hiển thị OLED:
		- Dòng 1: P1 sát trái, P2 sát phải
		- Dòng 2: P3 căn giữa

4) OLED (LUỒNG HIỂN THỊ) 1,3 inch I2C

- Khởi động: hiện "YURA" to ở giữa màn hình.
- Nhận "44": xóa màn hình (clear).
- Nhận 5 chữ số hợp lệ: tách P1/P2/P3 và vẽ đúng bố cục 2 dòng.
- Font đang dùng: nhóm logisoso (ưu tiên chữ số lớn để dễ đọc).

5) SERVO SDCARD (D5) 



Thông số:
- SERVO_SDCARD_STEP_MS = 32ms/1 độ (~3 giây để đi 0 -> 93)
- SERVO_SDCARD_WAIT_MS = 1000ms

6) LOG DEBUG ĐANG CÓ SẴN

Có thể bật/tắt bằng macro:
- SERIAL_DEBUG_ENABLE

Nhóm log:
- [RX-BYTE]: từng byte nhận từ COM
- [RX-CMD]: gói lệnh đã chốt (delimiter/timeout)
- [PROCESS]: phân luồng lệnh
- [ACT]: hành động đã thực thi
- [WARN]: lệnh lỗi, lệnh bị bỏ qua
- [OLED-COM]: chi tiết lệnh OLED được nhận/chấp nhận/từ chối
- [OLED-RENDER]: thông tin render OLED theo mode/nội dung

7) TRÌNH TỰ KHỞI ĐỘNG (setup)

- Serial.begin(9600)
- Relay D7 -> OFF
- Attach 3 servo (D5, D3, D2)
- Đưa servo về vị trí khởi động
- Wire.begin + tăng I2C clock
- Hiện "YURA"

