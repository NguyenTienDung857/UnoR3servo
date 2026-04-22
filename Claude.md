tôi có 1 con arduino  uno R3
chân D2 tôi đang nối với RelayBplus , set mặc định chân này là giá trị low, nếu uno R3 nhận được giá trị 1 qua cổng com thì bật chân này lên giá trị high , nếu nhận giá trị 2 qua cổng com thì tắt chân này về low 

Chân D7 và D8 tôi nối với RelayCamFont_1 và RelayCamFont_2, set mặc định chân này là giá trị low, nếu uno R3 nhận được giá trị 3 qua cổng com thì bật hai chân này lên giá trị high , nếu nhận giá trị 4 qua cổng com thì tắt hai chân này về low 

Chân D10 và D12 tôi nối với RelayCamRear_1 và RelayCamRear_2, set mặc định chân này là giá trị low, nếu uno R3 nhận được giá trị 5 qua cổng com thì bật hai chân này lên giá trị high , nếu nhận giá trị 6 qua cổng com thì tắt hai chân này về low 

Chân D5 tôi nối vào ServoSDcard, mặc định khi UNO R3 khởi độngn nó sẽ quay về về gốc tọa độ 0 rất là chậm,  khi mà Uno R3 nhận được giá 7 qua cổng COM nó sẽ quay servo này một góc 93 độ rất là chậm ̣̣( quay 93 độ thời gian tầm 3s cho chậm), rồi đợi ở góc 93 độ này tầm 1 giây rồi quay lại vị trí gốc home cũng chậm như thế là xong 1 chu kì. Lưu ý cái servo tên là ServoSDcard  này phải quay thật là chậm, trong mọi trường hợp không được quay quá 93 độ nhé 

chân D3 tôi nói với cái servo , tên là servoOled, mặc định khi khởi động servo phải quay về ở  góc 0 , tức là gốc home đấy, khi UNO nhận được giá trị là 8 thì  sẽ quay cái ServoOled này một góc là 90 độ, quay nhanh ấy, rồi đợi 0.5 giây rồi quay lại vị trí gốc 0 là ok  xong 1 chu kì 

Chân SDA và SLC của UNO R3 tôi đang nối với một cái màn hình OLED 1.3 inch , giao tiếp qua I2C, bạn code mặc định khi khởi động nó sẽ in chữ YURA ra màn hình, in chữ to vào nhé , và đợi, khi nhận được giá trị ở cổng COM là một chữ có 5 chữ số thì in ra màn hình với quay tắc sau : Ví dụ nhận được 10203 thì in ra màn hình thật là to , in to nhất có thể nhé ,vì số 02 và 01 có chữ số 0 ở đầu nên  in "1    2" rồi xuống dòng in số 3 to ở giữa dòng, dòng đầu chỉ hiện số đầu tiên và 2 số tiếp theo, nhưng nếu là số 0 thì chỉ hiện 1 số đằng sau thôi,  chỉ hiển thị hai dòng thôi, nên các số phải hiển thị to vào để cao hợp với 2 dòng đấy , 
Ví dụ nhận được 21003 thì hiện "2   10" xuống dòng hiện số 3 ở giữa, còn nếu nhận 30112 thì hiện "3   1" xuống dòng hiện 12 
Tóm lại là chữ có 5 chữ số chia làm 3 thành phần, phần 1 là số đầu tiên, phần 2 là 2 số tiếp theo , phần 3 là hai số cuối cùng , phần 1 và phần 2 sẽ hiển thị ở dòng trên, phần 1 hiển thị đầu dòng trên , phần 2 hiển thị cuối dòng trên, cách nhau một khoảng  trắng ấy, phần 3 hiển thị ở giữa dòng thứ 2. nếu có chữ số 0 ở đầu mỗi phần thì không cần hiện số 0 đó, ví dụ phần 2 là 06 thì chỉ cần hiện 6 thôi , phần 3 là 04 thì chỉ cận hiện 4 thôi , phần 1 có 1 số nên ơhair hiển thị 

tất cả ngoại vi được cấp nguồn riêng, nối chung GND rồi 





Tôi đang làm một dự án với Arduino Uno R3. Hãy viết code cho tôi với các yêu cầu kỹ thuật chi tiết sau:

1. Quy tắc lập trình cốt lõi (Bắt buộc tuân thủ):

KHÔNG SỬ DỤNG hàm delay() làm nghẽn luồng. Bắt buộc dùng millis() (Non-blocking / State Machine) cho toàn bộ các tác vụ: đọc Serial, điều khiển tốc độ quay Servo, và cập nhật OLED. Đảm bảo hệ thống luôn lắng nghe cổng COM mượt mà.

Giao thức cổng COM: Đọc chuỗi Serial cho đến khi gặp ký tự \n (newline).

Nếu chuỗi dài 1 ký tự: xử lý logic Relay và Servo.

Nếu chuỗi dài 5 ký tự: xử lý logic màn hình OLED.

Định nghĩa rõ macro #define RELAY_ON HIGH và #define RELAY_OFF LOW để tôi có thể tự đổi nếu module relay là loại Active LOW.

2. Cấu hình Pin & Relay (Mặc định khởi động là LOW):

D2 nối RelayBplus. Nhận '1' -> ON, nhận '2' -> OFF.

D7, D8 nối RelayCamFont_1 và RelayCamFont_2. Nhận '3' -> cả hai ON, nhận '4' -> cả hai OFF.

D10, D12 nối RelayCamRear_1 và RelayCamRear_2. Nhận '5' -> cả hai ON, nhận '6' -> cả hai OFF.

3. Cấu hình Servo:

Chân D5 (ServoSDcard): Mặc định khởi động từ từ quay về góc 0 độ. Khi nhận '7', quay đến góc 93 độ RẤT CHẬM (tầm 3 giây bằng logic millis, không dùng delay). Đạt 93 độ thì chờ 1 giây (cũng dùng millis), rồi từ từ quay về 0 độ thật chậm để hoàn thành chu kỳ. Tuyệt đối giới hạn góc quay tối đa là 93 độ.

Chân D3 (ServoOled): Mặc định khởi động quay về 0 độ. Khi nhận '8', quay nhanh góc 90 độ, đợi 0.5s (dùng millis), rồi quay nhanh về 0 độ.

4. Cấu hình OLED 1.3 inch (Chân SDA, SCL):

Sử dụng chip driver SH1106 (giao tiếp I2C). Khuyên dùng thư viện U8g2 để tối ưu việc in chữ cỡ lớn, hoặc thư viện Adafruit_SH110X.

Khi khởi động: Hiện chữ "YURA" to ở giữa màn hình.

Khi nhận chuỗi 5 chữ số từ cổng COM: Tách làm 3 phần. P1 (số thứ 1), P2 (số thứ 2 và 3), P3 (số thứ 4 và 5).

Xử lý số 0 vô nghĩa: Nếu P2 hoặc P3 có số 0 ở đầu (VD: 06, 04) thì bỏ số 0 đi, chỉ in 6 và 4. P1 luôn in.

Bố cục hiển thị (dùng font to nhất có thể chiếm trọn 2 dòng):

Dòng 1: In P1 ở sát mép trái, in P2 ở sát mép phải (ở giữa là khoảng trắng).

Dòng 2: In P3 căn giữa.
(Ví dụ: nhận '10203' -> Dòng 1 in "1     2", Dòng 2 in "3". Nhận '21003' -> Dòng 1 in "2    10", Dòng 2 in "3")

Khi mà nhận được giá trị là 44 ở cổng COM thì thực hiện việc xóa màn hình lại 
Màn hình tôi đang gắn ngược, nên vẽ màn hình ngược để hiển thị với tôi cho đúng 



Nguồn ngoại vi đã được cấp riêng và nối chung GND


Với ServoSDcard, hãy thiết kế một Enum State Machine rõ ràng gồm các trạng thái: IDLE (đứng yên), MOVING_TO_93 (đang tiến lên từ từ), WAITING_1S (chờ ở 93 độ), và MOVING_TO_HOME (đang lùi về từ từ). Dùng một biến lastServoMoveTime để kiểm soát tốc độ quay (ví dụ: mỗi 30ms tăng/giảm 1 độ).

Có một bảng comment ở đầu file code để liệt kê các giá trị có thể nhận ở cổng COM, và tác dụng của nó nhé 
KHi code phải thêm coment tiếng việt vào mỗi biến, mỗi hàm và các phần logic quan trọng để tôi dễ hiểu và bảo trì sau này.
Có một file header riêng để định nghĩa các macro, enum, và khai báo biến toàn cục để giữ cho file chính gọn gàng và dễ quản lý.
Có một bảng comment ở đầu file header để liệt kê các giá trị có thể nhận ở cổng COM, và tác dụng của nó nhé 