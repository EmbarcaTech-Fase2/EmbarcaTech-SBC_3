#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>

typedef unsigned char u8;
int i2c_fd = -1;
const char *i2c_fname = "/dev/i2c-1";

#define MPU6050_ADDR    0x68
#define DHT_ADDR        0x38
#define BH1750_ADDR     0x23

int i2c_init(void) {
    if ((i2c_fd = open(i2c_fname, O_RDWR)) < 0) {
        perror("Erro ao abrir I2C");
        return -1;
    }
    return i2c_fd;
}

int i2c_write(u8 slave_addr, u8 reg, u8 data) {
    u8 outbuf[2] = {reg, data};
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;

    msg.addr  = slave_addr;
    msg.flags = 0;
    msg.len   = 2;
    msg.buf   = outbuf;

    msgset.msgs  = &msg;
    msgset.nmsgs = 1;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        return -1;
    }
    return 0;
}

int i2c_read_bytes(u8 slave_addr, u8 reg, u8 *buffer, u8 length) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;

    msgs[0].addr  = slave_addr;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg;

    msgs[1].addr  = slave_addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = length;
    msgs[1].buf   = buffer;

    msgset.msgs  = msgs;
    msgset.nmsgs = 2;

    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        return -1;
    }
    return 0;
}

float mpu6050_read_temp(void) {
    u8 buf[2];
    int16_t mpu_temp_raw;
    
    if (i2c_read_bytes(MPU6050_ADDR, 0x41, buf, 2) == 0) {
        mpu_temp_raw = (int16_t)((buf[0] << 8) | buf[1]);
        return mpu_temp_raw / 340.0 + 36.53;
    }
    return 0.0;
}

void dht_read(float *temp, float *humidity) {
    u8 buf[6];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    u8 trigger[3] = {0xAC, 0x33, 0x00};
    
    msgs[0].addr = DHT_ADDR;
    msgs[0].flags = 0;
    msgs[0].len = 3;
    msgs[0].buf = trigger;
    
    msgs[1].addr = DHT_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = 6;
    msgs[1].buf = buf;
    
    msgset.msgs = msgs;
    msgset.nmsgs = 2;
    
    usleep(80000);
    
    if (ioctl(i2c_fd, I2C_RDWR, &msgset) < 0) {
        *temp = 0.0;
        *humidity = 0.0;
        return;
    }
    
    uint32_t humidity_raw = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | ((uint32_t)buf[3] >> 4);
    uint32_t temp_raw = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    
    *humidity = (humidity_raw / 1048576.0) * 100.0;
    *temp = ((temp_raw / 1048576.0) * 200.0) - 50.0;
}

uint16_t bh1750_read(void) {
    u8 buf[2];
    uint16_t lux;
    
    if (i2c_read_bytes(BH1750_ADDR, 0x00, buf, 2) == 0) {
        lux = ((uint16_t)buf[0] << 8) | buf[1];
        return lux / 1.2;
    }
    return 0;
}

int main(void) {
    i2c_init();
    
    i2c_write(MPU6050_ADDR, 0x6B, 0x00);
    
    u8 init_cmd[3] = {0xBE, 0x08, 0x00};
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data msgset;
    msg.addr = DHT_ADDR;
    msg.flags = 0;
    msg.len = 3;
    msg.buf = init_cmd;
    msgset.msgs = &msg;
    msgset.nmsgs = 1;
    ioctl(i2c_fd, I2C_RDWR, &msgset);
    usleep(100000);
    
    u8 test_bh[1] = {0x10};
    msg.addr = BH1750_ADDR;
    msg.flags = 0;
    msg.len = 1;
    msg.buf = test_bh;
    msgset.msgs = &msg;
    msgset.nmsgs = 1;
    ioctl(i2c_fd, I2C_RDWR, &msgset);
    
    while (1) {
        float mpu_temp = mpu6050_read_temp();
        float dht_temp, dht_humidity;
        dht_read(&dht_temp, &dht_humidity);
        uint16_t bh_lux = bh1750_read();

        printf("\n");
        printf("═══════════════════════════════════════════════════════════\n");
        printf("  MPU6050    │ Temperatura: %.2f °C\n", mpu_temp);
        printf("  AHT20      │ Temperatura: %.2f °C  │  Umidade: %.2f %%\n", dht_temp, dht_humidity);
        printf("  BH1750     │ Luminosidade: %u lux\n", bh_lux);
        printf("═══════════════════════════════════════════════════════════\n");

        sleep(5);
    }

    close(i2c_fd);
    return 0;
}
