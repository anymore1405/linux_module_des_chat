#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define DRIVER_FILE_NAME "/dev/C303_DES"




int hextostring(char *in, int len, char *out)
{
    int i;

    memset(out, 0, sizeof(out));
    for (i = 0; i < len; i++)
    {
        sprintf(out, "%s%02hhx", out, in[i]);
    }
    return 0;
}

int stringtohex(char *in, int len, char *out)
{
    int i;
    int converter[105];
    converter['0'] = 0;
    converter['1'] = 1;
    converter['2'] = 2;
    converter['3'] = 3;
    converter['4'] = 4;
    converter['5'] = 5;
    converter['6'] = 6;
    converter['7'] = 7;
    converter['8'] = 8;
    converter['9'] = 9;
    converter['a'] = 10;
    converter['b'] = 11;
    converter['c'] = 12;
    converter['d'] = 13;
    converter['e'] = 14;
    converter['f'] = 15;

    memset(out, 0, sizeof(out));

    for (i = 0; i < len; i = i + 2)
    {
        char byte = converter[in[i]] << 4 | converter[in[i + 1]];
        out[i / 2] = byte;
    }
}

int main()
{
    int des_fd = open(DRIVER_FILE_NAME, O_RDWR);

    char test_data[1000] = "Hello C303 Driver", hex[100], bin[100], cipher[100];
    hextostring(test_data, strlen(test_data), hex);
    memset(test_data, 0, sizeof(test_data));
    sprintf(test_data, "e%s", hex);
    printf("%s\n", test_data);
    write(des_fd, test_data, strlen(test_data));
    read(des_fd, cipher, sizeof(cipher));

    printf("cipher: %s\n", cipher);

    memset(test_data, 0, sizeof(test_data));
    sprintf(test_data, "d%s", cipher);
    printf("%s\n", test_data);
    write(des_fd, test_data, strlen(test_data));

    memset(cipher, 0, sizeof(cipher));
    read(des_fd, cipher, sizeof(cipher));

    printf("plan text: %s\n", cipher);
    stringtohex(cipher, strlen(cipher), test_data);
    printf("test data: %s\n", test_data);

    return 0;
}