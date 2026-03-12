#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Hàm phụ trợ để lấy tên file/thư mục từ một đường dẫn dài
// Ví dụ: truyền vào "a/aa/b", hàm sẽ trả về "b"
char* get_basename(char *path) {
    char *p;
    // Đi lùi từ cuối chuỗi lên để tìm dấu '/' cuối cùng
    for(p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    return p;
}

// Hàm đệ quy chính để in cây thư mục [cite: 61]
void tree(char *path, int depth) {
    int fd;
    struct stat st;

    // 1. Mở file/thư mục [cite: 59]
    if((fd = open(path, 0)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    // 2. Lấy thông tin (stat) của file/thư mục [cite: 59]
    if(fstat(fd, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path); 
        close(fd);
        return;
    }

    // 3. In thụt lề dựa trên độ sâu (depth) [cite: 61]
    for(int i = 0; i < depth; i++){
        printf("  "); // Hai khoảng trắng cho mỗi cấp độ [cite: 62, 63]
    }

    char *name = get_basename(path);

    // 4. In tên file hoặc thư mục [cite: 64, 65]
    if(st.type == T_DIR){
        printf("%s/\n", name); // Thêm dấu '/' nếu là thư mục [cite: 51, 64]
    } else {
        printf("%s\n", name); // Không thêm hậu tố nếu là file thường [cite: 65]
    }

    // 5. Nếu là thư mục, tiến hành đệ quy đọc các file bên trong [cite: 49]
    if(st.type == T_DIR){
        char buf[512], *p;
        struct dirent de;

        // Kiểm tra xem đường dẫn nối thêm có bị tràn bộ đệm không [cite: 69]
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("tree: path too long\n");
            close(fd);
            return;
        }
        
        // Copy đường dẫn hiện tại vào buf và thêm dấu '/' [cite: 68]
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // Đọc từng entry trong thư mục [cite: 60]
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            
            // Bỏ qua thư mục hiện tại "." và thư mục cha ".." để tránh lặp vô hạn [cite: 52, 66, 67]
            if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            // Nối tên file/thư mục con vào đường dẫn cha [cite: 70]
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0; // Kết thúc chuỗi

            // Gọi đệ quy với độ sâu tăng thêm 1 [cite: 61]
            tree(buf, depth + 1);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    // Nếu không truyền đối số, mặc định dùng thư mục hiện tại "." [cite: 48]
    if (argc == 1) {
        tree(".", 0);
    } 
    // Nếu có truyền đối số, duyệt qua từng thư mục được truyền vào [cite: 47]
    else {
        for(int i = 1; i < argc; i++){
            tree(argv[i], 0);
        }
    }
    exit(0);
}