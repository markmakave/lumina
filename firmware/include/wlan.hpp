#pragma once

#include <string_view>
#include <array>

#include <cstring>

#include <nvs_flash.h>

#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#define ESP_CHECK(x) if(auto err = x; err != ESP_OK) { ESP_LOGE("WLAN", "Error on " #x": %s", esp_err_to_name(err)); }

namespace lumina
{

class ipv4
{
public:

    union
    {
        uint32_t u32;
        std::array<uint8_t, 4> u8;
    };

public:

    ipv4()
    :   u32{}
    {}

    ipv4(std::string_view ip)
    {
        std::sscanf(ip.data(), "%hhu.%hhu.%hhu.%hhu", &u8[0], &u8[1], &u8[2], &u8[3]);
    }

    ipv4(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    :   u8{ a, b, c, d }
    {}

    ipv4(uint32_t ip)
    :   u32(ip)
    {}

    ipv4 operator& (ipv4 mask)
    {
        return { u32 & mask.u32 };
    }

    ipv4 operator| (ipv4 mask)
    {
        return { u32 | mask.u32 };
    }

    ipv4 operator^ (ipv4 mask)
    {
        return { u32 ^ mask.u32 };
    }

    ipv4 operator~ ()
    {
        return { ~u32 };
    }

    friend std::ostream& operator<< (std::ostream& os, ipv4 ip)
    {
        return os << static_cast<int>(ip.u8[0]) << "." << static_cast<int>(ip.u8[1]) << "." << static_cast<int>(ip.u8[2]) << "." << static_cast<int>(ip.u8[3]);
    }

    std::string to_string() const
    {
        return std::to_string(u8[0]) + "." + std::to_string(u8[1]) + "." + std::to_string(u8[2]) + "." + std::to_string(u8[3]);
    }

};

enum mode { STA, AP };

template <mode>
class wlan;


template <>
class wlan<STA>
{
public:

    enum status { DISCONNECTED, CONNECTING, CONNECTED };

public:

    wlan()
    :    _status(DISCONNECTED)
    {
        ESP_CHECK(nvs_flash_init());
        ESP_CHECK(esp_netif_init());

        ESP_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_CHECK(esp_wifi_init(&wifi_init_config));
        
        ESP_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler, this));
        ESP_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wlan::_event_handler, this));

        ESP_LOGI("WLAN", "initialized as STA");
    }

    ~wlan()
    {
        ESP_CHECK(esp_wifi_deinit());
        ESP_CHECK(esp_event_loop_delete_default());
        ESP_CHECK(nvs_flash_deinit());

        ESP_LOGI("WLAN", "deinitialized");
    }

    void connect(std::string_view ssid, std::string_view password)
    {
        if (_mode == AP)
        {
            ESP_LOGE("WLAN", "Cannot connect to AP in AP mode");
            return;
        }

        wifi_config_t wifi_config = {};
        std::strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid.data(), sizeof(wifi_config.sta.ssid));
        std::strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password.data(), sizeof(wifi_config.sta.password));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;

        ESP_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_CHECK(esp_wifi_start());

        ESP_LOGI("WLAN", "connecting to %s", ssid.data());
    }

    void disconnect()
    {
        if (_mode == AP)
        {
            ESP_LOGE("WLAN", "Cannot disconnect from AP in AP mode");
            return;
        }

        ESP_CHECK(esp_wifi_disconnect());

        ESP_LOGI("WLAN", "disconneting wifi");
    }

    ipv4 ip() const
    {
        return _ip;
    }

    ipv4 mask() const
    {
        return _mask;
    }

protected:

    static void _event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        auto& wlan = *static_cast<lumina::wlan<STA>*>(arg);

        if (event_base == WIFI_EVENT)
            switch (event_id)
            {
                case WIFI_EVENT_STA_START:
                    ESP_CHECK(esp_wifi_connect());
                    break;

                case WIFI_EVENT_STA_CONNECTED:
                    ESP_LOGI("WLAN", "wifi connected");
                    wlan._status = CONNECTED;
                    break;

                case WIFI_EVENT_STA_DISCONNECTED:
                    {
                        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;

                        auto get_reason = [](wifi_event_sta_disconnected_t* event) -> std::string {
                            switch (event->reason) {
                                case WIFI_REASON_AUTH_EXPIRE:
                                    return "Authentication expired";
                                case WIFI_REASON_NO_AP_FOUND:
                                    return "No AP found";
                                case WIFI_REASON_AUTH_FAIL:
                                    return "Authentication failed";
                                // Handle other reason codes as necessary
                                default:
                                    return "Other";
                            }
                        };

                        ESP_LOGI("WLAN", "wifi disconnected due to: %s", get_reason(event).c_str());

                        wlan._status = DISCONNECTED;
                        wlan._ip = {};
                        wlan._mask = {};
                    }
                    break;
                
                default:
                    ESP_LOGI("WLAN", "unhandled WIFI event %ld", event_id);
                    break;
            }
        else if (event_base == IP_EVENT)
            switch (event_id)
            {
                case IP_EVENT_STA_GOT_IP:
                    ESP_LOGI("WLAN", "got ip");
                    wlan._ip = ipv4(reinterpret_cast<ip_event_got_ip_t*>(event_data)->ip_info.ip.addr);
                    wlan._mask = ipv4(reinterpret_cast<ip_event_got_ip_t*>(event_data)->ip_info.netmask.addr);
                    break;

                default:
                    ESP_LOGI("WLAN", "unhandled IP event %ld", event_id);
                    break;
            }
    }

protected:

    mode _mode;
    enum status  _status;
    ipv4 _ip, _mask;
};


template <>
class wlan<AP>
{
    static constexpr auto ENABLED_BIT = BIT0;
    static constexpr auto DISABLED_BIT = BIT1;

public:

    wlan(std::string_view ssid, std::string_view password)
    :   _ssid(ssid),
        _password(password),
        _event_group(xEventGroupCreate())
    {
        ESP_CHECK(nvs_flash_init());
        ESP_CHECK(esp_netif_init());
        ESP_CHECK(esp_event_loop_create_default());
        _netif = esp_netif_create_default_wifi_ap();

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_CHECK(esp_wifi_init(&wifi_init_config));

        ESP_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler, this));
        ESP_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler, this));

        wifi_config_t wifi_config = {};
        std::strncpy(reinterpret_cast<char*>(wifi_config.ap.ssid), _ssid.data(), sizeof(wifi_config.ap.ssid));
        wifi_config.ap.ssid_len = _ssid.size();
        std::strncpy(reinterpret_cast<char*>(wifi_config.ap.password), _password.data(), sizeof(wifi_config.ap.password));
        wifi_config.ap.max_connection = 1;
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

        ESP_LOGI("WLAN", "ap created");
    }

    ~wlan()
    {
        ESP_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler));
        ESP_CHECK(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler));

        ESP_CHECK(esp_event_loop_delete_default());
        ESP_CHECK(esp_wifi_deinit());
        ESP_CHECK(nvs_flash_deinit());

        ESP_LOGI("WLAN", "ap destroued");
    }

    void enable()
    {
        ESP_CHECK(esp_wifi_start());
        xEventGroupWaitBits(_event_group, ENABLED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

        ESP_LOGI("WLAN", "ap enabled");
    }

    void disable()
    {
        ESP_CHECK(esp_wifi_stop());
        xEventGroupWaitBits(_event_group, DISABLED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

        ESP_LOGI("WLAN", "ap disabled");
    }
    
    ipv4 ip() const
    {
        return _ip;
    }

    ipv4 mask() const
    {
        return _mask;
    }

protected:

    static void _event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
    {
        auto& wlan = *static_cast<lumina::wlan<AP>*>(arg);

        if (event_base == WIFI_EVENT)
        {
            switch (event_id)
            {
                case WIFI_EVENT_AP_START:
                    ESP_LOGI("WLAN", "ap started");

                    esp_netif_ip_info_t ip_info;
                    esp_netif_get_ip_info(wlan._netif, &ip_info);

                    wlan._ip = ipv4(ip_info.ip.addr);
                    wlan._mask = ipv4(ip_info.netmask.addr);

                    ESP_LOGI("WLAN", "ap ip: %s", wlan._ip.to_string().c_str());
                    ESP_LOGI("WLAN", "ap mask: %s", wlan._mask.to_string().c_str());

                    xEventGroupClearBits(wlan._event_group, DISABLED_BIT);
                    xEventGroupSetBits(wlan._event_group, ENABLED_BIT);

                    break;

                case WIFI_EVENT_AP_STOP:
                    ESP_LOGI("WLAN", "ap stopped");
                    xEventGroupClearBits(wlan._event_group, ENABLED_BIT);
                    xEventGroupSetBits(wlan._event_group, DISABLED_BIT);
                    break;

                case WIFI_EVENT_AP_STACONNECTED:
                    ESP_LOGI("WLAN", "station connected");
                    break;

                case WIFI_EVENT_AP_STADISCONNECTED:
                    ESP_LOGI("WLAN", "station disconnected");
                    break;

                default:
                    ESP_LOGI("WLAN", "unhandled WIFI event %ld", event_id);
            }
        } else if (event_base == IP_EVENT)
        {
            switch (event_id)
            {
                case IP_EVENT_AP_STAIPASSIGNED:
                    ESP_LOGI("WLAN", "client is assigned");
                    break;

                default:
                    ESP_LOGI("WLAN", "unhandled IP event %ld", event_id);
            }
        }
    }


    std::string_view _ssid, _password;
    ipv4 _ip, _mask;

    esp_netif_t* _netif;
    EventGroupHandle_t _event_group;
};

}
