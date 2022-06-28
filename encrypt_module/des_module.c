#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/crypto.h>
#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>

#define MEM_SIZE 1024

uint8_t *kernel_buffer;
dev_t dev_num;
struct class *device_class;
struct cdev *char_device;
struct crypto_cipher *tfm;
char key[8] = "ct3adl03";

// e/d
char type;
char data[MEM_SIZE];
size_t data_len = 0;

int hextostring(char *in, int len, char *out)
{
    int i;

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

    for (i = 0; i < len; i = i + 2)
    {
        char byte = converter[(int)in[i]] << 4 | converter[(int)in[i + 1]];
        out[i / 2] = byte;
    }

    return 0;
}

static int open_macro(struct inode *inode, struct file *file)
{
    return 0;
}

static int release_macro(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t read_macro(struct file *file, char *user_buf, size_t len, loff_t *off)
{
    char cipher[1000];
    char hex_cipher[1000];
    int i, j;

    printk("data_len: %ld\n", data_len);

    memset(cipher, 0, sizeof(cipher));
    memset(hex_cipher, 0, sizeof(hex_cipher));


    // data_len = 44
    // 10110101 00100010 10101010 00101110 10101101 0101
    // 
    for (i = 0; i < data_len / 8; i++)
    {
        char byte_data[8], byte_cipher[8];

        memset(byte_data, 0, sizeof(byte_data));
        memset(byte_cipher, 0, sizeof(byte_cipher));

        for (j = 0; j < 8; j++)
            byte_data[j] = data[i * 8 + j];

        printk("one data: %s\n", byte_data);

        if (type == 'e')
            crypto_cipher_encrypt_one(tfm, byte_cipher, byte_data);
        if (type == 'd')
            crypto_cipher_decrypt_one(tfm, byte_cipher, byte_data);
        for (j = 0; j < 8; j++)
            cipher[i * 8 + j] = byte_cipher[j];

    }

    hextostring(cipher, data_len, hex_cipher);
    printk("hex cipher: %s\n", hex_cipher);
    copy_to_user(user_buf, hex_cipher, strlen(hex_cipher));

    return 0;
}

static ssize_t write_macro(struct file *file, const char *user_buff, size_t len, loff_t *off)
{
    char buffer[1000], hex_data[1000];
    int i, j;

    memset(buffer, 0, sizeof(buffer));
    memset(data, 0, sizeof(data));
    memset(hex_data, 0, sizeof(hex_data));

    copy_from_user(buffer, user_buff, len);

    // buffer [e/d, ....]
    type = buffer[0];
    i = 0;
    j = 1;
    while (j < len)
    {
        hex_data[i] = buffer[j];
        i++;
        j++;
    }

    printk("type: %c\n", type);
    printk("hex_data: %s\n", hex_data);

    memset(buffer, 0, sizeof(buffer));
    stringtohex(hex_data, strlen(hex_data), data);
    printk("data: %s\n", data);

    if (strlen(hex_data) % 16 == 0)
        data_len = ((uint16_t)(strlen(hex_data) / 16)) * 8;
    else
        data_len = ((uint16_t)((strlen(hex_data) / 16) + 1)) * 8;
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = read_macro,
    .write = write_macro,
    .open = open_macro,
    .release = release_macro};

static int md_init(void)
{
    printk("Install module\n");

    tfm = crypto_alloc_cipher("des", 0, 0);
    crypto_cipher_setkey(tfm, key, 8);

    alloc_chrdev_region(&dev_num, 0, 1, "C303SocketDES");
    device_class = class_create(THIS_MODULE, "C303_des_module");
    device_create(device_class, NULL, dev_num, NULL, "C303_DES");

    kernel_buffer = kmalloc(MEM_SIZE, GFP_KERNEL);

    char_device = cdev_alloc();
    cdev_init(char_device, &fops);
    cdev_add(char_device, dev_num, 1);

    return 0;
}

static void md_exit(void)
{
    crypto_free_cipher(tfm);
    cdev_del(char_device);
    kfree(kernel_buffer);
    device_destroy(device_class, dev_num);
    class_destroy(device_class);
    unregister_chrdev_region(dev_num, 1);
    printk("Remove module\n");
}

module_init(md_init);
module_exit(md_exit);

MODULE_LICENSE("GPL");