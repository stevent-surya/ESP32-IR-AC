    #ifndef NGROK_H
    #define NGROK_H

    #include <Arduino.h>

    // Enum untuk memilih region server ngrok
    enum ngrok_region {
        NGROK_REGION_US = 0, // United States
        NGROK_REGION_EU,     // Europe
        NGROK_REGION_AP,     // Asia/Pacific
        NGROK_REGION_AU,     // Australia
        NGROK_REGION_SA,     // South America
        NGROK_REGION_JP,     // Japan
        NGROK_REGION_IN,     // India
        NGROK_REGION_LAST
    };

    class Ngrok {
    public:
        // Fungsi untuk memulai koneksi ngrok
        void begin(const char* authtoken, const char* hostname, uint16_t port);
        // Fungsi untuk mengatur region server (opsional)
        void setRegion(ngrok_region region);

    private:
        // Fungsi yang berjalan di latar belakang untuk menjaga koneksi
        static void tcp_task(void* pvParameters);
    };

    extern Ngrok ngrok;

    #endif
    