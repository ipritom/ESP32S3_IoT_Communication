#include "freertos/event_groups.h"
#include "esp_event.h"

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void wifi_init_sta(void);
