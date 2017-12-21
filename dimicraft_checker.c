#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>

#define CTRL_VAL_REGAX 10
#define CTRL_VAL_REGDX 11
#define CTRL_VAL_ABUF 12
#define CTRL_VAL_BFDEL 13
#define CTRL_VAL_ABDEL 14
#define CTRL_FUNC_AROR 15
#define CTRL_FUNC_AROL 16
#define CTRL_FUNC_CHGB 17
#define CTRL_FUNC_XOR 20
#define CTRL_FUNC_ROR 21
#define CTRL_FUNC_ROL 22
#define CTRL_FUNC_SHL 23
#define CTRL_FUNC_SHR 24
#define CTRL_FUNC_SUB 25
#define CTRL_FUNC_ADD 26
#define CTRL_FUNC_EXP 27
#define CTRL_CHAR_ROR 31
#define CTRL_CHAR_ROL 32
#define CTRL_CHAR_HALF 33
#define CTRL_CHAR_MERGE 34
#define CTRL_CHAR_INPUT 35
#define CTRL_CHAR_OUTPUT 36
#define CTRL_CHAR_INABF 37
#define CTRL_CHAR_OUTABF 38
#define CTRL_INTER_XOR 39
#define CTRL_INTER_CMOV 40
//#define CTRL_CTX_CODE 40 //커스텀어셈블리를 통해 레지스터 연산
//#define CTRL_CTX_ONEB 41 //1Byte 연산(한글자)

int main()
{
    int fd;
    int i;
    unsigned char buf[32] = "";
    unsigned char hint[32] = {36,-35,-116,5,26,9,-120,-93,-35,70,126,-125,-114,-126,-26,92,20,9,-124,-65,17,-116,125,123,-124,-55,-107,124,18,57,-5,0,};
    unsigned char key[32] = "[This_is_Key_Value_Not_Flag_:)]";
    unsigned char serial[32] = "";
    unsigned char chTmp;
    memset(buf, 0 ,32);
    printf("╔═════════════════════════════════════════╗\n");
    printf("║           Dimicraft Activation          ║\n");
    printf("║       30-day trial period expired       ║\n");
    printf("╚═════════════════════════════════════════╝\n");
    printf("Name    : ");
    scanf("%31s", buf);
    printf("CD-Key  : ");
    scanf("%31s", serial);
//    printf("Main Buffer Size : %d\nXor Buffer Size : %d\n", strlen(buf), strlen(xorBuf));
    fd = open("/dev/dimicraft", O_RDWR);
    if (fd < 0)
    {
        return -1;
    }

    write(fd, buf, strlen(buf));
    chTmp = buf[0];
    ioctl(fd, CTRL_VAL_REGAX, 1);
    ioctl(fd, CTRL_CHAR_INABF, &chTmp);
    for(i = 1; i<31; ++i){
        ioctl(fd, CTRL_VAL_REGAX, i);
        if(i<30)
            ioctl(fd, CTRL_VAL_REGDX, i+1);
        ioctl(fd, CTRL_INTER_XOR, i+1);
        ioctl(fd, CTRL_INTER_CMOV, 2);
    }

    ioctl(fd, CTRL_FUNC_CHGB, 0);
    ioctl(fd, CTRL_VAL_BFDEL, 0);
    write(fd, buf, strlen(buf));


    ioctl(fd, CTRL_CHAR_ROR, buf[30]);

//    read(fd, buf, strlen(hint));
//    printf("BUF[0] :");
//    for(i = 0; i<32; ++i)
//        printf("%d, ", buf[i]);
//    printf("\n");
//    ioctl(fd, CTRL_FUNC_CHGB, 0);
//    read(fd, buf, strlen(hint));
//    printf("ABUF[0] :");
//    for(i = 0; i<32; ++i)
//        printf("%d, ", buf[i]);
//    printf("\n");

    ioctl(fd, CTRL_VAL_REGAX, 0);
    ioctl(fd, CTRL_INTER_XOR, strlen(buf));


    ioctl(fd, CTRL_CHAR_HALF, 0);

    ioctl(fd, CTRL_VAL_BFDEL, 0);
    write(fd, key, strlen(key));

    for(i = 0;i<31;++i){
        ioctl(fd, CTRL_VAL_REGAX, i);
        ioctl(fd, CTRL_CHAR_OUTPUT, &chTmp);
        ioctl(fd, CTRL_VAL_REGDX, chTmp);
        ioctl(fd, CTRL_FUNC_AROL, i);
    }

////        read(fd, buf, strlen(hint));
////        printf("BP[3] :");
////        for(i = 0; i<32; ++i)
////            printf("%d, ", buf[i]);
////        printf("\n");

    for(i = 0;i<31;++i){
        ioctl(fd, CTRL_VAL_REGAX, i);
        ioctl(fd, CTRL_CHAR_OUTPUT, &chTmp);
        ioctl(fd, CTRL_VAL_REGDX, (chTmp % (i+1)) % 8);
        ioctl(fd, CTRL_FUNC_SHL, i);
    }

////        read(fd, buf, strlen(hint));
////        printf("BP[3] :");
////        for(i = 0; i<32; ++i)
////            printf("%d, ", buf[i]);
////        printf("\n");

    ioctl(fd, CTRL_FUNC_CHGB, 0);
    ioctl(fd, CTRL_VAL_REGAX, 0);
    ioctl(fd, CTRL_INTER_XOR, strlen(hint));
    ioctl(fd, CTRL_FUNC_CHGB, 0);


//    read(fd, buf, strlen(hint));
//    printf("BP[3-1] :");
//    for(i = 0; i<32; ++i)
//        printf("%d, ", buf[i]);
//    printf("\n");
//    ioctl(fd, CTRL_FUNC_CHGB, 0);
//    read(fd, buf, strlen(hint));
//    printf("BP[3-2] :");
//    for(i = 0; i<32; ++i)
//        printf("%d, ", buf[i]);
//    printf("\n");

    ioctl(fd, CTRL_VAL_BFDEL, 0);
    write(fd, hint, strlen(hint));
    ioctl(fd, CTRL_FUNC_CHGB, 0);
    ioctl(fd, CTRL_VAL_REGAX, 0);
    ioctl(fd, CTRL_INTER_XOR, strlen(hint));


    read(fd, buf, strlen(hint));
    if(strcmp(buf, serial)==0)
        printf("\n[Activated] Have a good time :)\n");
    else
        printf("\n[Error] Wrong Serial/Name\n");
    close(fd);
    return 0;
}
