#pragma once

#include <string_view>
#include <array>
#include <cstring>

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"

#define ESP_CHECK(x) if(auto err = x; err != ESP_OK) { ESP_LOGE("WLAN", "Error: %s", esp_err_to_name(err)); }

namespace lumina
{

class wlan
{
public:

    enum status { CONNECTED, DISCONNECTED };
    enum protocol { TCP, UDP };

public:

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

    };

public:

    wlan()
    :   _status(DISCONNECTED)
    {
        ESP_CHECK(nvs_flash_init());
        ESP_CHECK(esp_netif_init());

        ESP_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        ESP_CHECK(esp_wifi_init(&wifi_init_config));

        ESP_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wlan::_event_handler, this));
        ESP_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wlan::_event_handler, this));

        ESP_LOGI("WLAN", "initialized");
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
        ESP_CHECK(esp_wifi_disconnect());

        ESP_LOGI("WLAN", "disconneting wifi");
    }

    enum status status() const
    {
        return _status;
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
        auto& wlan = *static_cast<lumina::wlan*>(arg);

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

    enum status _status;
    ipv4 _ip, _mask;
};

}
