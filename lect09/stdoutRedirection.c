#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int fd1;

    // my_file을 (없으면 만들고) 읽기/쓰기 모드로 연다
    fd1 = open("my_file",
               O_RDWR | O_CREAT | O_TRUNC,
               S_IRUSR | S_IWUSR);   // 소유자 읽기/쓰기 권한

    if (fd1 == -1) {
        perror("open");
        exit(1);
    }

    // stdout(1번 fd)을 my_file로 리다이렉션
    if (dup2(fd1, STDOUT_FILENO) == -1) {
        perror("dup2");
        exit(1);
    }

    // 필요 없어진 원본 fd는 닫아도 됨 (stdout이 이미 이를 가리키고 있음)
    close(fd1);

    // 이제부터 printf는 터미널이 아니라 my_file에 출력된다
    printf("hello, file!\n");
    printf("이 줄도 파일에 쓰입니다.\n");

    return 0;
}

