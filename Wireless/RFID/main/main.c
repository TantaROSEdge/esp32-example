#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "mfrc522_i2c.h"

#define I2C_MASTER_SCL_IO          22      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO          21      /*!< GPIO number for I2C master data */
#define I2C_MASTER_NUM             I2C_NUM_0 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ         100000  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE  0       /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE  0       /*!< I2C master doesn't need buffer */

static const char *TAG = "RFID_Reader";

// MFRC522 instance
MFRC522_t mfrc522;

void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 
                                       I2C_MASTER_RX_BUF_DISABLE, 
                                       I2C_MASTER_TX_BUF_DISABLE, 0));
}

void show_reader_details(void)
{
    uint8_t v = mfrc522_pcd_read_register(&mfrc522, MFRC522_REG_VERSION);
    ESP_LOGI(TAG, "MFRC522 Software Version: 0x%02X", v);
    
    if (v == 0x91)
        ESP_LOGI(TAG, " = v1.0");
    else if (v == 0x92)
        ESP_LOGI(TAG, " = v2.0");
    else
        ESP_LOGI(TAG, " (unknown)");
    
    if ((v == 0x00) || (v == 0xFF)) {
        ESP_LOGW(TAG, "WARNING: Communication failure, is the MFRC522 properly connected?");
    }
}

void app_main(void)
{
    // Initialize I2C
    i2c_master_init();
    ESP_LOGI(TAG, "I2C initialized successfully");
    
    // Initialize MFRC522
    mfrc522_i2c_init(&mfrc522, I2C_MASTER_NUM, 0x28);
    mfrc522_pcd_init(&mfrc522);
    
    show_reader_details();
    ESP_LOGI(TAG, "Scan PICC to see UID, type, and data blocks...");
    
    while (1) {
        // Look for new cards
        if (!mfrc522_picc_is_new_card_present(&mfrc522)) {
            vTaskDelay(50 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Read the card
        if (!mfrc522_picc_read_card_serial(&mfrc522)) {
            vTaskDelay(50 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Display UID
        ESP_LOGI(TAG, "Card UID:");
        char uid_str[50] = {0};
        char temp[10] = {0};
        
        for (uint8_t i = 0; i < mfrc522.uid.size; i++) {
            snprintf(temp, sizeof(temp), " %02X", mfrc522.uid.uidByte[i]);
            strcat(uid_str, temp);
        }
        
        ESP_LOGI(TAG, "%s", uid_str);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}