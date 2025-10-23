
#ifndef MFRC522_I2C_H
#define MFRC522_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// MFRC522 Registers
#define MFRC522_REG_RESET_CTRL         0x0F
#define MFRC522_REG_COMMAND            0x01
#define MFRC522_REG_FIFO_DATA          0x09
#define MFRC522_REG_FIFO_LEVEL         0x0A
#define MFRC522_REG_CONTROL            0x0C
#define MFRC522_REG_BIT_FRAMING        0x0D
#define MFRC522_REG_COLL               0x0E
#define MFRC522_REG_MODE               0x11
#define MFRC522_REG_TX_CONTROL         0x14
#define MFRC522_REG_TX_AUTO            0x15
#define MFRC522_REG_TX_SEL             0x16
#define MFRC522_REG_RX_SEL             0x17
#define MFRC522_REG_RX_THRESHOLD       0x18
#define MFRC522_REG_DEMOD              0x19
#define MFRC522_REG_CRC_RESULT_H       0x21
#define MFRC522_REG_CRC_RESULT_L       0x22
#define MFRC522_REG_T_MODE             0x2A
#define MFRC522_REG_T_PRESCALER        0x2B
#define MFRC522_REG_T_RELOAD_H         0x2C
#define MFRC522_REG_T_RELOAD_L         0x2D
#define MFRC522_REG_T_COUNTER_VAL_H    0x2E
#define MFRC522_REG_T_COUNTER_VAL_L    0x2F
#define MFRC522_REG_VERSION            0x37

// Commands
#define MFRC522_CMD_IDLE               0x00
#define MFRC522_CMD_MEM                0x01
#define MFRC522_CMD_GEN_RANDOM_ID      0x02
#define MFRC522_CMD_CALC_CRC           0x03
#define MFRC522_CMD_TRANSMIT           0x04
#define MFRC522_CMD_NO_CMD_CHANGE      0x07
#define MFRC522_CMD_RECEIVE            0x08
#define MFRC522_CMD_TRANSCEIVE         0x0C
#define MFRC522_CMD_MF_AUTHENT         0x0E
#define MFRC522_CMD_SOFT_RESET         0x0F

typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_addr;
    uint8_t uid[10];
    uint8_t uid_size;
} MFRC522_t;

// UID structure
typedef struct {
    uint8_t uidByte[10];
    uint8_t size;
} Uid;

// MFRC522 functions
void mfrc522_i2c_init(MFRC522_t *dev, i2c_port_t port, uint8_t addr);
void mfrc522_pcd_init(MFRC522_t *dev);
uint8_t mfrc522_pcd_read_register(MFRC522_t *dev, uint8_t reg);
void mfrc522_pcd_write_register(MFRC522_t *dev, uint8_t reg, uint8_t value);
bool mfrc522_picc_is_new_card_present(MFRC522_t *dev);
bool mfrc522_picc_read_card_serial(MFRC522_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* MFRC522_I2C_H */