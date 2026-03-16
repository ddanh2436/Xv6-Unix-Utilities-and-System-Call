#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    // xargs cần ít nhất 1 lệnh để thực thi
    if (argc < 2) {
        fprintf(2, "Usage: xargs command...\n");
        exit(1);
    }

    char *xargv[MAXARG];
    // Copy các tham số ban đầu (ví dụ: "echo", "bye") vào mảng xargv
    for (int i = 1; i < argc; i++) {
        xargv[i - 1] = argv[i];
    }

    char buf[1024];
    char *p = buf;
    int xargv_idx = argc - 1; // Vị trí để chèn thêm tham số từ stdin

    // Đọc từng ký tự một từ standard input (0)
    while (read(0, p, 1) > 0) {
        if (*p == '\n') {
            *p = 0; // Thay '\n' bằng null-terminator để kết thúc chuỗi
            
            xargv[xargv_idx] = buf; // Gắn chuỗi vừa đọc vào làm tham số cuối
            xargv[xargv_idx + 1] = 0; // Đánh dấu kết thúc mảng tham số

            // Dùng fork và exec để chạy lệnh
            if (fork() == 0) {
                exec(xargv[0], xargv);
                fprintf(2, "exec %s failed\n", xargv[0]);
                exit(1);
            } else {
                wait(0); // Tiến trình cha đợi tiến trình con (lệnh) chạy xong
            }
            p = buf; // Reset lại con trỏ p để chuẩn bị đọc dòng tiếp theo
        } else {
            p++;
        }
    }
    exit(0);
}