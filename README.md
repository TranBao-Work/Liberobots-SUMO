# 🤖 Liberobot-SUMO: Cuộc thi Robot Sumo

---

## 🚀 Giới thiệu dự án

Dự án **Liberobot-SUMO** là một robot sumo tự hành được phát triển trên nền tảng ESP32, sử dụng các cảm biến siêu âm và hồng ngoại để phát hiện đối thủ và rìa sàn đấu. Robot được thiết kế để hoạt động hiệu quả trong các cuộc thi sumo robot, kết hợp cả chế độ điều khiển thủ công và chế độ tự động với thuật toán thông minh.

Điểm nổi bật của dự án này là việc áp dụng **State Machine** (máy trạng thái) và kỹ thuật lập trình **non-blocking** để đảm bảo hoạt động mượt mà, phản hồi nhanh và tránh bị ngắt kết nối Bluetooth, điều thường xảy ra với các vòng lặp `while/for/do-while` truyền thống.

---

## ✨ Tính năng nổi bật

* **Điều khiển linh hoạt:**
    * **Chế độ thủ công:** Điều khiển robot qua ứng dụng **Dabble** trên điện thoại thông minh.
    * **Chế độ tự động:** Robot tự động tìm kiếm, tấn công đối thủ và tránh rìa sàn đấu.
* **Thuật toán tìm kiếm và tấn công thông minh:** Sử dụng dữ liệu từ 3 cảm biến siêu âm (trái, trước, phải) và áp dụng thuật toán **Trung bình có trọng số (Weighted Average)** để xác định góc tấn công tối ưu, giúp robot phản ứng nhanh chóng khi phát hiện đối thủ.
* **Hệ thống né tránh rìa sàn đấu:** Các cảm biến hồng ngoại ở phía dưới giúp robot phát hiện rìa sàn đấu và lùi lại kịp thời, tránh bị rơi ra khỏi khu vực thi đấu.
* **Kiến trúc lập trình hiệu quả:**
    * **State Machine:** Quản lý hành vi của robot một cách có tổ chức, dễ mở rộng và gỡ lỗi. Các trạng thái bao gồm `WAITING`, `SEARCHING`, `ROTATING_LEFT`, `ROTATING_RIGHT`, `MOVING_FORWARD`, `ESCAPING`.
    * **Non-blocking code:** Tránh sử dụng các vòng lặp chặn (`while`, `for`, `do-while`) trong các tác vụ liên tục, giúp duy trì kết nối Bluetooth ổn định và cho phép robot thực hiện nhiều tác vụ cùng lúc.
* **Phanh động cơ (Brake Mode):** Chức năng phanh tức thì giúp robot dừng lại nhanh chóng, tăng cường khả năng kiểm soát trong cả chế độ thủ công và tự động.
* **Kiểm soát tốc độ:** Có thể điều chỉnh tốc độ của robot qua ứng dụng Dabble, phù hợp với các tình huống chiến thuật khác nhau.

---

## 🛠️ Công nghệ sử dụng

* **Vi điều khiển:** ESP32
* **Ngôn ngữ lập trình:** C++ (cho Arduino IDE)
* **Thư viện điều khiển Bluetooth:** `DabbleESP32.h`, `GamePadModule.h` (từ thư viện Dabble)
* **Cảm biến:**
    * Cảm biến siêu âm (HC-SR04)
    * Cảm biến hồng ngoại (IR sensors)
* **Động cơ:** Động cơ DC có điều khiển PWM
* **Môi trường phát triển:** Arduino IDE

---

## ⚙️ Hướng dẫn cài đặt & Chạy dự án

Để chạy dự án này, bạn cần có:

1.  **Phần cứng:**
    * Board ESP32
    * 2 động cơ DC (có bộ truyền động motor driver)
    * 3 cảm biến siêu âm (HC-SR04)
    * 2 cảm biến hồng ngoại
    * Các dây nối và nguồn điện phù hợp.
2.  **Phần mềm:**
    * Arduino IDE
    * Cài đặt ESP32 Board Manager trong Arduino IDE.
    * Cài đặt thư viện **Dabble** (tìm kiếm trong Library Manager của Arduino IDE).

### Bước 1: Chuẩn bị mã nguồn

Bạn có thể tải xuống mã nguồn dự án này hoặc sao chép trực tiếp vào Arduino IDE của mình.

### Bước 2: Cấu hình và Upload Code

1.  Mở file `.ino` trong Arduino IDE.
2.  Đảm bảo bạn đã chọn đúng Board ESP32 và Port COM.
3.  Kết nối các chân cảm biến và động cơ theo định nghĩa trong mã nguồn (kiểm tra các `#define` ở đầu file `Liberobot-SUMO.ino`).
4.  Upload mã nguồn lên ESP32 của bạn.

### Bước 3: Điều khiển bằng ứng dụng Dabble

1.  Tải ứng dụng **Dabble** trên điện thoại thông minh (có sẵn trên Android và iOS).
2.  Mở ứng dụng, tìm kiếm và kết nối với thiết bị Bluetooth có tên **"Liberobot-SUMO"**.
3.  Trong ứng dụng Dabble, chọn Module **GamePad**.
4.  Sử dụng các nút điều khiển để vận hành robot:
    * **Nút mũi tên (Up/Down/Left/Right):** Điều khiển robot trong chế độ thủ công.
    * **Nút Start:** Thoát khỏi chế độ tự động (chuyển sang thủ công).
    * **Nút Select:** Kích hoạt chế độ tự động.
    * **Nút Cross (X):** Bật/tắt chế độ phanh.
    * **Nút Square/Triangle/Circle:** Điều chỉnh tốc độ robot.

---

## 🧠 Thuật toán Trung bình có trọng số (Weighted Average)

Trong hàm `computeTargetAngle()`, robot sử dụng một biến thể của thuật toán trung bình có trọng số để xác định hướng tấn công tối ưu.

* Mỗi cảm biến siêu âm (trái, trước, phải) được gán một "góc" tương ứng:
    * Trái: `-45 độ`
    * Trước: `0 độ`
    * Phải: `45 độ`
* **Trọng số** cho mỗi cảm biến được tính bằng **nghịch đảo khoảng cách** đến đối thủ (`1.0 / dist`). Điều này có nghĩa là khi đối thủ càng gần, trọng số của cảm biến đó càng lớn, và góc tương ứng sẽ có ảnh hưởng mạnh mẽ hơn đến góc tấn công cuối cùng.

Công thức tính toán:
$$ Angle_{target} = \frac{\sum (Angle_i / Distance_i)}{\sum (1 / Distance_i)} $$

Ví dụ: Nếu đối thủ ở rất gần phía trước, khoảng cách `distFront` sẽ nhỏ, làm cho trọng số `1.0/distFront` rất lớn, và $Angle_{target}$ sẽ tiến gần về 0 độ (tức là đi thẳng).

---

## 📈 Tương lai phát triển

* Cải thiện thuật toán tìm kiếm và tấn công (ví dụ: áp dụng PID control).
* Thêm các chế độ hoạt động tự động khác.
* Tối ưu hóa hiệu suất và tiêu thụ năng lượng.
* Phát triển giao diện người dùng tùy chỉnh thay vì Dabble.

---

## 🤝 Đóng góp

Mọi đóng góp đều được hoan nghênh! Nếu bạn có ý tưởng cải tiến hoặc tìm thấy lỗi, đừng ngần ngại tạo một Issue hoặc Pull Request.

---

## 📧 Liên hệ

Nếu bạn có bất kỳ câu hỏi nào về dự án này, vui lòng liên hệ:

* **Trịnh Trần Quốc Bảo** - baotrinh20052006@gmail.com
* [https://www.linkedin.com/in/b%E1%BA%A3o-tr%E1%BB%8Bnh-ute-48674031b/](https://www.linkedin.com/in/b%E1%BA%A3o-tr%E1%BB%8Bnh-ute-48674031b/)

---
