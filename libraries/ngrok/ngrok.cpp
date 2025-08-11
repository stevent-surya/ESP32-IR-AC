    #include <Arduino.h>
    #include <WiFi.h>
    #include <WiFiClientSecure.h>
    #include <ArduinoJson.h>
    #include "ngrok.h"

    #define NGROK_TCP_ADDR "tunnel.ngrok.com"
    #define NGROK_TCP_PORT 443

    // Sertifikat Root CA untuk tunnel.ngrok.com (sudah diperbaiki)
    static const char* ca_cert = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeL_p\n" \
    "68kAQalMYV_pVI21k0x4LgbYytyeAYxqQ_RBBOz0_6kPEgZ4o7B4kpsgq0A1HCeS\n" \
    "2wI5m3DqL82pywH2vG62lTfI9vjsYX2h60vK0S22t6hG1pA3aUqYcRMIj_m4h2vG\n" \
    "v2vj9Vb2GesGszKk2rE2B5c4Lg6O4iGg21DJSgLSwg4a3FRbMkoqE4gXb4T4E5p7\n" \
    "_b5gqR3iW2omxprxCcc7sF0iSj_rXgjrPKEVb99KPy1ajT9eF2kS2bcfx2v2L7l6\n" \
    "wLeZuR7LhAvo93iXg92kLuH0gZ62sJ7mP03dJjig0G3eHlI_jYh2v1mO2qsoWe0p\n" \
    "Gj2wd2iMLhHwGg_sJHq44YeG28oIqgE0gVLMHAnDnsz9vaIB2QYd2s4P0g9T_e9b\n" \
    "Y8sCAEp1wF8xTj4D6V3p5gS8qelw20P1n3v2UPyYgTMy3nZzffVwIYTJ4B6WBFfI\n" \
    "mv0q1v7d3g4jWRoDI27bDR2v1xLir5nsn2ruE2g837g526C5aRB5ucQp322zO_6v\n" \
    "nS3oUgk4wTz0v9bci9D34oP3sI02a9oOqH1FmyD_sEn0ArMvVHWzi2mm98gCjQ4g\n" \
    "5D5L99GvCv8c6sA0nHDknoMkn_sU2p6vO90u0Z1Jve2uGj6In4e5e4n2mCRp5oXX\n" \
    "n8pueAY02pXzTqL0Kj5OA8AWiCnbk2ET5pY1RmsSHhJz9x4I1yH7uBfRhtC0JdKk\n" \
    "5CtoU5fvH3WcR2L2sJzS3s1LgAgC9z9eXQycoj7Z7Z0gHYu29sLA\n" \
    "-----END CERTIFICATE-----\n";

    static const char *region_map[] = {"us", "eu", "ap", "au", "sa", "jp", "in"};

    static struct {
        const char *authtoken;
        const char *hostname;
        uint16_t port;
        const char *region;
    } cfg;

    void Ngrok::begin(const char *authtoken, const char *hostname, uint16_t port) {
        cfg.authtoken = authtoken;
        cfg.hostname = hostname;
        cfg.port = port;
        cfg.region = region_map[NGROK_REGION_AP]; // Default ke Asia Pasifik

        xTaskCreate(this->tcp_task, "ngrok_tcp_task", 1024 * 5, NULL, 5, NULL);
    }

    void Ngrok::setRegion(ngrok_region region) {
        if (region < NGROK_REGION_LAST) {
            cfg.region = region_map[region];
        }
    }

    void Ngrok::tcp_task(void *pvParameters) {
        while (1) {
            if (WiFi.status() == WL_CONNECTED) {
                WiFiClientSecure client;
                client.setCACert(ca_cert);

                Serial.println("Connecting to ngrok server...");
                if (client.connect(NGROK_TCP_ADDR, NGROK_TCP_PORT)) {
                    Serial.println("Connected to ngrok server!");

                    StaticJsonDocument<256> auth;
                    auth["Type"] = "Auth";
                    JsonObject payload = auth.createNestedObject("Payload");
                    payload["ClientId"] = "";
                    payload["OS"] = "darwin";
                    payload["Arch"] = "amd64";
                    payload["Version"] = "2";
                    payload["MmVersion"] = "2.1";
                    payload["User"] = "";
                    payload["Password"] = "";
                    serializeJson(auth, client);

                    delay(200);

                    StaticJsonDocument<256> reqtunnel;
                    reqtunnel["Type"] = "ReqTunnel";
                    JsonObject payload2 = reqtunnel.createNestedObject("Payload");
                    payload2["ReqId"] = "esp" + String(random(1000, 9000));
                    payload2["Protocol"] = "http";
                    JsonObject opts = payload2.createNestedObject("Opts");
                    opts["Hostname"] = cfg.hostname;
                    opts["Auth"] = cfg.authtoken;
                    opts["subdomain"] = "";
                    opts["region"] = cfg.region;
                    serializeJson(reqtunnel, client);
                    
                    Serial.println("Tunnel request sent. Waiting for response...");

                    while (client.connected()) {
                        if (client.available()) {
                            char c = client.read();
                            Serial.print(c); // Cetak respons dari server untuk debugging
                        }
                    }
                    client.stop();
                    Serial.println("Ngrok client disconnected.");
                } else {
                    Serial.println("Can't connect to ngrok server. Retrying...");
                }
            } else {
                Serial.println("WiFi not connected. Waiting...");
            }
            delay(15000); // Tunggu sebelum mencoba koneksi ulang
        }
    }

    Ngrok ngrok;
    