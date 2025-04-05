#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mqtt_client.h"

#include "nvs_flash.h"

// development
#include "x_wifi_config.h"
#include "frame_utils.h"

// mqtt ////////////////////////
#define MQTT_BROKER_URI "mqtt://192.168.180.74" // Public broker for testing
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_PORT 1883
#define MQTT_TOPIC_HEARTBEAT "esp32/wifi_sta/status"
#define MQTT_TOPIC_DATA "esp32/wifi_sta/data"
#define MQTT_TOPIC_SUB "esp32/wifi_sta/listen"

esp_mqtt_client_handle_t client;
uint8_t RAVEN_MQTT_CONNECTED = 0;
uint8_t RAVEN_MQTT_DATA_RECEIVED = 0;
char mqtt_rx_buffer[100];

uint8_t mqtt_data_received_and_cleared(){
    if (RAVEN_MQTT_DATA_RECEIVED) {
        RAVEN_MQTT_DATA_RECEIVED = 0;
        return 1;
    }
    return 0;
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    int msg_id;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // ESP_LOGI("MQTT", "Connected to broker");
        RAVEN_MQTT_CONNECTED = 1;
        esp_mqtt_client_publish(client, MQTT_TOPIC_HEARTBEAT, "ESP32 SoftAP is online", 0, 1, 0);

        msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUB, 0);
        ESP_LOGI("MQTT", "sent subscribe successful, msg_id=%d", msg_id);

        break;
    
    case MQTT_EVENT_DISCONNECTED:
        RAVEN_MQTT_CONNECTED = 0;
        // ESP_LOGI("MQTT", "Disconnected from broker");
        break;

    case MQTT_EVENT_ERROR:
        RAVEN_MQTT_CONNECTED = 0;
        // ESP_LOGI("MQTT", "MQTT Error occurred");
        break;
    
    case MQTT_EVENT_DATA:
        ESP_LOGI("MQTT", "Data received from broker");
        RAVEN_MQTT_DATA_RECEIVED = 1;
        // copy data to mqtt_rx_buffer
        strncpy(mqtt_rx_buffer, event->data, event->data_len);
        mqtt_rx_buffer[event->data_len] = '\0';

        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;

    default:
        break;
    }
}

void mqtt_app_start()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .broker.address.port = MQTT_PORT,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    ESP_LOGI("MQTT", "MQTT client started");
}

void app_main(void)
{
    //Initialize NVS ////////////////////////
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // wifi init 
    wifi_init_sta();
    // mqtt connect
    mqtt_app_start();
    // process frame

    while (1)
    {
      //process mqtt frame 
      if(mqtt_data_received_and_cleared()){
        int frame_length;
        printf("MQTT RX: %s\n", mqtt_rx_buffer);
        // process frame here
        uint8_t frame[100];
        frame_length = frame_str2hex(mqtt_rx_buffer, frame);

        for (size_t i = 0; i < frame_length; i++)
        {
          printf("%x ", frame[i]);
        }
        
        printf("\n");
      
      }


      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    

}
