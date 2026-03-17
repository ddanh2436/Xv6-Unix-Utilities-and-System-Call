#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs command...\n");
        exit(1);
    }

    // Khai báo mảng chứa các đối số, tận dụng giới hạn MAXARG [cite: 39]
    char *xargv[MAXARG];
    
    // Copy lệnh gốc và các tham số ban đầu vào mảng xargv
    for (int i = 1; i < argc; i++) {
        xargv[i - 1] = argv[i];
    }

    char buf[1024]; 
    char c;
    int buf_idx = 0;
    int xargv_idx = argc - 1;

    // Đọc từng ký tự một cho đến khi gặp dấu xuống dòng (\n) [cite: 38]
    while (read(0, &c, 1) > 0) {
        if (c == '\n') {
            buf[buf_idx] = '\0'; // Kết thúc từ cuối cùng của dòng hiện tại
            xargv[xargv_idx] = 0; // Đánh dấu điểm kết thúc mảng tham số
            
            // Dùng fork và exec để thực thi lệnh [cite: 36]
            if (fork() == 0) {
                exec(xargv[0], xargv);
                fprintf(2, "exec %s failed\n", xargv[0]);
                exit(1);
            }
            // Tiến trình cha dùng wait để chờ tiến trình con chạy xong [cite: 36, 37]
            wait(0);
            
            // Reset lại các biến để chuẩn bị đọc dòng tiếp theo
            buf_idx = 0;
            xargv_idx = argc - 1;
            
        } else if (c == ' ') {
            // Khi gặp khoảng trắng, biến nó thành điểm kết thúc chuỗi (\0)
            // Việc này giúp tách dòng dài thành các đối số độc lập (vd: "hello" và "too")
            buf[buf_idx++] = '\0'; 
        } else {
            // Nếu đây là ký tự đầu tiên của dòng, hoặc ký tự ngay sau một khoảng trắng
            // thì lưu địa chỉ của nó vào mảng đối số xargv
            if (buf_idx == 0 || buf[buf_idx - 1] == '\0') {
                if (xargv_idx < MAXARG - 1) {
                    xargv[xargv_idx++] = &buf[buf_idx];
                }
            }
            buf[buf_idx++] = c; // Lưu ký tự vào buffer
        }
    }

    // Xử lý Edge Case: Dữ liệu kết thúc ở cuối file (EOF) nhưng không có dấu \n
    if (buf_idx > 0) {
        buf[buf_idx] = '\0';
        xargv[xargv_idx] = 0;
        if (fork() == 0) {
            exec(xargv[0], xargv);
            fprintf(2, "exec %s failed\n", xargv[0]);
            exit(1);
        }
        wait(0);
    }

    exit(0);
}