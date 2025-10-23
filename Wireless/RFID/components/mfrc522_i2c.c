#include "mfrc522_i2c.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "MFRC522";

void mfrc522_i2c_init(MFRC522_t *dev, i2c_port_t port, uint8_t addr)
{
    dev->i2c_port = port;
    dev->i2c_addr = addr;
    dev->uid_size = 0;
    memset(dev->uid, 0, sizeof(dev->uid));
}

uint8_t mfrc522_pcd_read_register(MFRC522_t *dev, uint8_t reg)
{
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Write register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    
    // Read data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read register 0x%02X", reg);
        return 0;
    }
    
    return data;
}

void mfrc522_pcd_write_register(MFRC522_t *dev, uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(dev->i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write register 0x%02X", reg);
    }
}

void mfrc522_pcd_init(MFRC522_t *dev)
{
    // Soft reset
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_SOFT_RESET);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    // Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    mfrc522_pcd_write_register(dev, MFRC522_REG_T_MODE, 0x8D);
    mfrc522_pcd_write_register(dev, MFRC522_REG_T_PRESCALER, 0x3E);
    mfrc522_pcd_write_register(dev, MFRC522_REG_T_RELOAD_H, 0x00);
    mfrc522_pcd_write_register(dev, MFRC522_REG_T_RELOAD_L, 0x30);
    
    // 48dB gain
    mfrc522_pcd_write_register(dev, MFRC522_REG_RX_SEL, 0x86);
    mfrc522_pcd_write_register(dev, MFRC522_REG_RF_CFG, 0x7F);
    
    // Enable antenna
    uint8_t value = mfrc522_pcd_read_register(dev, MFRC522_REG_TX_CONTROL);
    if ((value & 0x03) != 0x03) {
        mfrc522_pcd_write_register(dev, MFRC522_REG_TX_CONTROL, value | 0x03);
    }
}

bool mfrc522_picc_is_new_card_present(MFRC522_t *dev)
{
    uint8_t buffer[2];
    uint8_t valid_bits = 7;
    
    // Clear interrupts
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMM_IRQ, 0x7F);
    
    // Send request
    buffer[0] = 0x26;
    mfrc522_pcd_write_register(dev, MFRC522_REG_BIT_FRAMING, 0x07);
    
    // Transceive data
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);
    mfrc522_pcd_write_register(dev, MFRC522_REG_FIFO_DATA, buffer[0]);
    mfrc522_pcd_write_register(dev, MFRC522_REG_BIT_FRAMING, valid_bits);
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);
    
    // Wait for completion
    uint16_t i;
    for (i = 2000; i > 0; i--) {
        uint8_t n = mfrc522_pcd_read_register(dev, MFRC522_REG_COMM_IRQ);
        if (n & 0x30) {
            break;
        }
    }
    
    // Stop now
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
    
    return (i > 0);
}

bool mfrc522_picc_read_card_serial(MFRC522_t *dev)
{
    // This is a simplified version - you'll need to implement the full anticollision protocol
    // For now, we'll just read the UID assuming it's present
    
    uint8_t buffer[10];
    uint8_t valid_bits = 0;
    
    // Clear interrupts
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMM_IRQ, 0x7F);
    
    // Send SELECT command
    buffer[0] = 0x93;
    buffer[1] = 0x70;
    mfrc522_pcd_write_register(dev, MFRC522_REG_BIT_FRAMING, 0x00);
    
    // Write to FIFO
    for (uint8_t i = 0; i < 2; i++) {
        mfrc522_pcd_write_register(dev, MFRC522_REG_FIFO_DATA, buffer[i]);
    }
    
    // Transceive data
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);
    mfrc522_pcd_write_register(dev, MFRC522_REG_BIT_FRAMING, valid_bits);
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_TRANSCEIVE);
    
    // Wait for completion
    uint16_t i;
    for (i = 2000; i > 0; i--) {
        uint8_t n = mfrc522_pcd_read_register(dev, MFRC522_REG_COMM_IRQ);
        if (n & 0x30) {
            break;
        }
    }
    
    // Read data from FIFO
    if (i > 0) {
        uint8_t length = mfrc522_pcd_read_register(dev, MFRC522_REG_FIFO_LEVEL);
        for (uint8_t j = 0; j < length; j++) {
            dev->uid[j] = mfrc522_pcd_read_register(dev, MFRC522_REG_FIFO_DATA);
        }
        dev->uid_size = length;
    }
    
    // Stop now
    mfrc522_pcd_write_register(dev, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
    
    return (i > 0);
}