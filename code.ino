#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <WiFi.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <vector>
#include <time.h>
#include <Preferences.h>

// --- PUSTAKA UNTUK SERVER ASINKRON DAN WEBSOCKETS ---
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

// --- PUSTAKA BARU UNTUK KOMUNIKASI NEXTION ---
#include <HardwareSerial.h>

// <<-- PERBAIKAN STABILITAS: Memindahkan HTML ke Flash (PROGMEM) untuk menghemat RAM -->>
const char HELLO_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='id'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Welcome</title>
    <style>
        body, html { height: 100%; margin: 0; display: flex; justify-content: center; align-items: center; background-color: #000000; font-family: -apple-system, BlinkMacSystemFont, 'Inter', 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif; overflow: hidden; transition: opacity 2s ease-out; }
        .text-container { text-align: center; position: relative; width: 400px; height: 100px; display: flex; justify-content: center; align-items: center; }
        .text-container span { position: absolute; font-size: 56px; font-weight: 300; color: #FFFFFF; opacity: 0; animation-timing-function: ease-in-out; animation-fill-mode: forwards; }
        .text-container span:nth-child(1) { animation-name: text-anim; animation-duration: 2.5s; animation-delay: 0s; }
        .text-container span:nth-child(2) { animation-name: text-anim; animation-duration: 2.5s; animation-delay: 2.5s; }
        @keyframes text-anim { 0% { opacity: 0; } 20% { opacity: 1; } 80% { opacity: 1; } 100% { opacity: 0; } }
        .logo-container { display: flex; justify-content: center; align-items: center; width: 100%; height: 100%; opacity: 0; transition: opacity 2s ease-in-out; }
        #loading-logo { width: 80vw; max-width: 580px; height: auto; }
    </style>
</head>
<body>
    <div class='text-container'>
        <span>Hello</span>
        <span>Selamat Datang</span>
    </div>
    <div class='logo-container' style='display: none;'>
        <img id='loading-logo' src='data:image/png;base64,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'>
    </div>
    <script>
        const textContainer = document.querySelector('.text-container');
        const logoContainer = document.querySelector('.logo-container');
        setTimeout(() => {
            textContainer.style.display = 'none';
            logoContainer.style.display = 'flex';
            setTimeout(() => { logoContainer.style.opacity = 1; }, 20);
        }, 5000);
        setTimeout(() => { logoContainer.style.opacity = 0; }, 10000);
        setTimeout(() => { document.body.style.opacity = 0; }, 11500);
        setTimeout(() => { window.location.href = '/dashboard'; }, 12000);
    </script>
</body>
</html>
)rawliteral";

const char DASHBOARD_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>ESP AC Live Control</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        @keyframes pageFadeIn { from { opacity: 0; } to { opacity: 1; } }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            text-align: center; 
            margin: 0; 
            padding: 20px; 
            display: flex; 
            flex-direction: column; 
            align-items: center; 
            transition: background 0.8s ease, color 0.8s ease; 
            animation: pageFadeIn 0.5s ease-in-out; 
        }
        .container, .section-box {
            transition: background-color 0.8s ease, color 0.8s ease, border-color 0.8s ease, box-shadow 0.3s ease; 
        }
        
        /* === DYNAMIC THEMES === */
        
        .theme-off { color: #EAEAEA; background: linear-gradient(135deg, #2C3E50 0%, #000000 100%); }
        .theme-off .container { background: rgba(0, 0, 0, 0.2); border: 1px solid rgba(255, 255, 255, 0.1);}
        .theme-off h1, .theme-off h2, .theme-off .header-container { color: #FFFFFF; }
        .theme-off .clock { color: #f1c40f; }
        .theme-off .section-box h2 { border-bottom-color: rgba(255,255,255,0.2); }
        .theme-off .fan-bar { fill: #4a5568; } 
        .theme-off .fan-bar.active { fill: #a0aec0; } 

        .theme-fan { color: #FFFFFF; background: linear-gradient(135deg, #01aaff 0%, #005c99 100%); }
        .theme-fan .container { background: rgba(0, 48, 80, 0.3); border: 1px solid rgba(1, 170, 255, 0.3); }
        .theme-fan h1, .theme-fan h2, .theme-fan .clock, .theme-fan .header-container { color: #FFFFFF; }
        .theme-fan .section-box h2 { border-bottom-color: rgba(255, 255, 255, 0.3); }
        .theme-fan .fan-bar { fill: #2c5282; } 
        .theme-fan .fan-bar.active { fill: #63b3ed; }

        .theme-cool { color: #002D62; background: linear-gradient(135deg, #FFFFFF 0%, #E6F7FF 100%); }
        .theme-cool .container { background: rgba(255, 255, 255, 0.7); border: 1px solid rgba(0, 45, 98, 0.2); }
        .theme-cool h1, .theme-cool h2, .theme-cool .clock, .theme-cool .header-container { color: #002D62; }
        .theme-cool .section-box h2 { border-bottom-color: rgba(0, 45, 98, 0.2); }
        .theme-cool .status-item p, .theme-cool .room-stats-item p, .theme-cool .footer, .theme-cool .weather-info p { color: #333; }
        .theme-cool .fan-bar { fill: #e2e8f0; }
        .theme-cool .fan-bar.active { fill: #4299e1; }

        .theme-dry { color: #FFFFFF; background: linear-gradient(135deg, #ff4500 0%, #ff8c00 70%, #1a1a1a 100%); }
        .theme-dry .container { background: rgba(40, 10, 0, 0.3); border: 1px solid rgba(255, 69, 0, 0.3); }
        .theme-dry h1, .theme-dry h2, .theme-dry .clock, .theme-dry .header-container { color: #FFFFFF; }
        .theme-dry .section-box h2 { border-bottom-color: rgba(255, 255, 255, 0.3); }
        .theme-dry .fan-bar { fill: #78350f; }
        .theme-dry .fan-bar.active { fill: #f59e0b; }

        .container { max-width: 700px; width:90%; margin: 20px auto; padding: 25px; border-radius: 12px; box-shadow: 0 8px 16px rgba(0, 0, 0, 0.3); }
        
        .main-title {
            text-align: center;
            margin: 0 0 15px 0;
            font-size: 1.6em;
            font-weight: bold;
        }

        .header-container {
            display: flex;
            justify-content: space-between;
            align-items: center;
            width: 100%;
            margin-bottom: 25px;
        }
        .weather-widget { 
            display: flex; 
            align-items: center; 
            gap: 15px;
        }
        .weather-info { 
            text-align: left;
        }
        #ext-temp {
            font-size: 2.5em; 
            font-weight: bold; 
            margin: 0;
            line-height: 1; 
        }
        #location { 
            font-size: 1.1em;
            font-weight: bold; 
            margin: 0;
            margin-bottom: 5px; 
        }
        .weather-icon { font-size: 4em; }

        .right-header-content {
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        .clock { 
            font-size: 2em; 
            font-weight: bold; 
            letter-spacing: 1.5px;
            margin-bottom: 10px;
            width: 200px;
            text-align: center;
        }

        h2 { font-size: 1.6em; margin-top: 20px; margin-bottom:10px; padding-bottom:5px; border-bottom: 1px solid; text-align: center;}
        
        .section-box {
            width: 100%;
            padding: 15px;
            border-radius: 12px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
            background: rgba(0,0,0,0.1);
            text-align: left;
            margin-bottom: 20px;
            box-sizing: border-box;
        }

        .room-stats-grid { display: flex; justify-content: space-around; align-items: center; text-align: center; }
        .room-stats-item p { margin: 0 0 5px 0; font-size: 1.1em; font-weight: bold; }
        .room-stats-item span { font-size: 2.5em; font-weight: bold; display: block; }
        
        .ac-status-section .main-status-row { display: flex; justify-content: space-between; align-items: center; text-align: center; margin-bottom: 15px; }
        .ac-status-section .main-status-item { flex: 1; }
        .ac-status-section .main-status-item p { margin: 0 0 5px 0; font-size: 1.1em; font-weight: bold; }
        .ac-status-section .main-status-item span { font-size: 2.5em; font-weight: bold; display: block; }
        .ac-status-section .sub-status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(100px, 1fr)); gap: 10px; margin-top: 15px; align-items: center;}
        .ac-status-section .sub-status-grid .status-item { display: flex; flex-direction: column; align-items: center; justify-content: center; text-align:center; }
        .ac-status-section .sub-status-grid .status-item p { font-size: 0.95em; margin: 0; font-weight: bold; }
        .ac-status-section .fan-display { display: flex; align-items: center; justify-content: center; gap: 8px; margin-top: 4px; }
        .fan-icon-svg { width: 40px; height: 20px; }
        .fan-bar { transition: fill 0.3s ease; }
        #fanVal { font-size: 1.1em; font-weight: bold; }

        .footer { margin-top: 30px; font-size: 0.9em; opacity: 0.7; }
        .chart-container { position: relative; height: 250px; width: 100%; border-radius: 8px; padding: 15px; box-sizing: border-box; margin-top: 10px; background: rgba(0,0,0,0.1); }
        
        #power-toggle { display: none; }
        .switch { position: relative; width: fit-content; height: fit-content; padding: 10px 20px; background-color: rgb(46, 46, 46); border-radius: 50px; cursor: pointer; display: flex; align-items: center; justify-content: center; gap: 10px; color: white; font-size: 0.9em; font-weight: 600; transition: all 0.3s; margin: 0 auto 15px auto; }
        .switch svg { height: 1em; }
        .switch svg path { fill: white; }
        #power-toggle:checked + .switch { background-color: rgb(1, 170, 255); box-shadow: 0px 0px 20px rgba(1, 170, 255, 0.6); }
        
        .controls-container { width: 100%; display: flex; flex-direction: column; align-items: center; }
        .controls-wrapper {
            display: grid;
            grid-template-columns: repeat(3, 1fr); 
            gap: 15px;
            width: 100%;
            margin-top: 15px;
            transition: max-height 0.5s ease-out, opacity 0.3s ease-out, transform 0.5s ease-out;
            transform-origin: top;
            overflow: hidden;
            max-height: 300px; 
            opacity: 1;
            transform: scaleY(1);
            padding: 20px;
            box-sizing: border-box;
            margin-bottom: 25px;
        }
        .controls-hidden {
            max-height: 0;
            opacity: 0;
            transform: scaleY(0);
            pointer-events: none;
            margin: 0 !important;
            padding: 0 !important;
        }
        .controls-wrapper button { 
            padding: 1em; 
            font-size: 12px; 
            text-transform: uppercase; 
            letter-spacing: 2.5px; 
            font-weight: 500; 
            color: #000; 
            background-color: #fff; 
            border: none; 
            border-radius: 45px; 
            box-shadow: 0px 8px 15px rgba(0, 0, 0, 0.1); 
            transition: all 0.3s ease 0s; 
            cursor: pointer; 
            outline: none; 
            min-width: 100px;
        }
        .controls-wrapper button:hover { background-color: #01aaff; box-shadow: 0px 15px 20px rgba(46, 157, 229, 0.4); color: #fff; transform: translateY(-7px); }
        .controls-wrapper button:active { transform: translateY(-1px); }
        .controls-wrapper button:nth-child(7) {
            grid-column: 2;
        }

        .language-btn-container {
            position: relative;
            display: inline-block;
        }
        .btn {
            width: 200px;
            height: 50px;
            border-radius: 5px;
            border: none;
            transition: all 0.5s ease-in-out;
            font-size: 20px;
            font-family: Verdana, Geneva, Tahoma, sans-serif;
            font-weight: 600;
            display: flex;
            align-items: center;
            background: #040f16;
            color: #f5f5f5;
            cursor: pointer;
        }
        .btn:hover {
            box-shadow: 0 0 20px 0px #2e2e2e3a;
        }
        .btn .icon {
            position: absolute;
            height: 40px;
            width: 70px;
            display: flex;
            justify-content: center;
            align-items: center;
            transition: all 0.5s;
        }
        .btn .text {
            transform: translateX(55px);
        }
        .btn:hover .icon {
            width: 175px; 
        }
        .btn:hover .text {
            transition: all 0.5s;
            opacity: 0;
        }
        .btn:focus {
            outline: none;
        }
        .btn:active .icon {
            transform: scale(0.85);
        }
        .language-dropdown {
            display: none;
            position: absolute;
            background-color: #040f16;
            min-width: 175px;
            box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);
            z-index: 1;
            border-radius: 5px;
            padding: 5px 0;
            right: 0;
            top: 50px;
        }
        .language-dropdown a {
            color: white;
            padding: 12px 16px;
            text-decoration: none;
            display: block;
            text-align: left;
            font-size: 16px;
        }
        .language-dropdown a:hover {
            background-color: #01aaff;
        }
        .language-btn-container:hover .language-dropdown {
            display: block;
        }

    </style>
</head>
<body>
    <div class='container'>
        <h1 class="main-title">KulBet AC Live Control</h1>
        <div class="header-container">
            <div class='weather-widget'>
                <div class='weather-info'>
                    <p id='location' data-translate-key="loading">Loading...</p>
                    <p id='ext-temp'>--°C</p>
                </div>
                <div id='weather-icon' class='weather-icon'>☀️</div>
            </div>
            <div class="right-header-content">
                 <div id='clock' class='clock'>Loading...</div>
                 <div class="language-btn-container">
                       <button class="btn">
                           <span class="icon">
                               <svg viewBox="0 0 175 80" width="40" height="40">
                                   <rect width="80" height="15" fill="#f0f0f0" rx="10"></rect>
                                   <rect y="30" width="80" height="15" fill="#f0f0f0" rx="10"></rect>
                                   <rect y="60" width="80" height="15" fill="#f0f0f0" rx="10"></rect>
                               </svg>
                           </span>
                           <span class="text" data-translate-key="language_btn">LANGUAGE</span>
                       </button>
                       <div class="language-dropdown">
                           <a href="#" onclick="setLanguage('id'); return false;">Indonesia</a>
                           <a href="#" onclick="setLanguage('en'); return false;">English</a>
                           <a href="#" onclick="setLanguage('de'); return false;">German</a>
                           <a href="#" onclick="setLanguage('ja'); return false;">Japanese</a>
                           <a href="#" onclick="setLanguage('zh'); return false;">Chinese</a>
                       </div>
                 </div>
            </div>
        </div>
        
        <div class='section-box ac-status-section'>
            <h2 data-translate-key="ac_status">❄️ Status AC</h2>
            <div class="main-status-row">
                <div class="main-status-item">
                    <p data-translate-key="status">Status</p>
                    <span id='statusVal'>OFF</span>
                </div>
                <div class="main-status-item">
                    <p data-translate-key="temperature">Temperature</p>
                    <span id='tempVal'>...</span>
                </div>
                <div class="main-status-item">
                    <p data-translate-key="mode">Mode</p>
                    <span id='modeVal'>...</span>
                </div>
            </div>
            <div class='sub-status-grid'>
                <div class='status-item fan-indicator'>
                    <p data-translate-key="fan">Fan</p>
                    <div class='fan-display'>
                        <svg id="fanIcon" class="fan-icon-svg" viewBox="0 0 46 20">
                            <rect class="fan-bar" id="fan-bar-1" x="2" y="14" width="6" height="6" rx="2"></rect>
                            <rect class="fan-bar" id="fan-bar-2" x="10" y="11" width="6" height="9" rx="2"></rect>
                            <rect class="fan-bar" id="fan-bar-3" x="18" y="8" width="6" height="12" rx="2"></rect>
                            <rect class="fan-bar" id="fan-bar-4" x="26" y="5" width="6" height="15" rx="2"></rect>
                            <rect class="fan-bar" id="fan-bar-5" x="34" y="2" width="6" height="18" rx="2"></rect>
                        </svg>
                        <span id='fanVal'>...</span>
                    </div>
                </div>
                <div class='status-item'><p><strong data-translate-key="swing_h">Swing H:</strong> <span id='swinghVal'>...</span></p></div>
                <div class='status-item'><p><strong data-translate-key="swing_v">Swing V:</strong> <span id='swingvVal'>...</span></p></div>
                <div class='status-item'><p><strong data-translate-key="powerful">Powerful:</strong> <span id='powerfulVal'>...</span></p></div>
            </div>
        </div>
        
        <div class='controls-container'>
            <h2 data-translate-key="controls">🎛️ Kontrol</h2>
            <input type="checkbox" id="power-toggle" />
            <label for="power-toggle" class="switch">
                <span data-translate-key="start_btn">Start</span>
                <svg class="slider" viewBox="0 0 512 512" height="1em" xmlns="http://www.w3.org/2000/svg">
                    <path d="M288 32c0-17.7-14.3-32-32-32s-32 14.3-32 32V256c0 17.7 14.3 32 32 32s32-14.3 32-32V32zM143.5 120.6c13.6-11.3 15.4-31.5 4.1-45.1s-31.5-15.4-45.1-4.1C49.7 115.4 16 181.8 16 256c0 132.5 107.5 240 240 240s240-107.5 240-240c0-74.2-33.8-140.6-86.6-184.6c-13.6-11.3-33.8-9.4-45.1 4.1s-9.4 33.8 4.1 45.1c38.9 32.3 63.5 81 63.5 135.4c0 97.2-78.8 176-176 176s-176-78.8-176-176c0-54.4 24.7-103.1 63.5-135.4z"></path>
                </svg>
            </label>
            <div class="controls-wrapper">
                <button onclick="sendCommand('MODE')" data-translate-key="mode_btn">Mode</button>
                <button onclick="sendCommand('TEMPUP')" data-translate-key="temp_up_btn">Temp Up</button>
                <button onclick="sendCommand('TEMPDOWN')" data-translate-key="temp_down_btn">Temp Down</button>
                <button onclick="sendCommand('FAN')" data-translate-key="fan_btn">Fan</button>
                <button onclick="sendCommand('SWINGH')" data-translate-key="swing_h_btn">Swing H</button>
                <button onclick="sendCommand('SWINGV')" data-translate-key="swing_v_btn">Swing V</button>
                <button onclick="sendCommand('POWERFUL')" data-translate-key="powerful_btn">Powerful</button>
            </div>
        </div>

        <div class='section-box room-stats'>
            <h2 data-translate-key="room_conditions">❖ Kondisi Ruangan</h2>
            <div class='room-stats-grid'>
                <div class='room-stats-item'>
                    <p data-translate-key="temperature">Temperature</p>
                    <span id='roomTempVal'>--°C</span>
                </div>
                <div class='room-stats-item'>
                    <p data-translate-key="humidity">Humidity</p>
                    <span id='roomHumVal'>--%</span>
                </div>
            </div>
        </div>
        
        <div class='section-box history-section'>
            <h2 data-translate-key="sensor_history">📊 Riwayat Sensor Langsung</h2>
            <div class="chart-container"><canvas id="historyChart"></canvas></div>
        </div>

        <div class='footer'><p data-translate-key="footer_title">ESP32 Control Panel</p><p data-translate-key="footer_subtitle">Proyek Protokol Rombel A-1 Program Studi Rumpun Mekatronika Angkatan 56</p><p class="names">Isma • Dewangga • Fita • Alfa • Gabriel • Indra • Joshua • Leon • Stevent • Syrillus</p></div>
    </div>
    <script>
        let historyChart;
        let webSocket;
        let currentLang = 'id';
        let lastWeatherData = null; 
        let onlineFeaturesInitialized = false;

        const translations = {
            'id': {
                loading: "Memuat...",
                room_conditions: "❖ Kondisi Ruangan",
                temperature: "Suhu",
                humidity: "Kelembapan",
                ac_status: "❄️ Status AC",
                status: "Status",
                mode: "Mode",
                fan: "Kipas",
                swing_h: "Ayunan H:",
                swing_v: "Ayunan V:",
                powerful: "Kuat:",
                controls: "🎛️ Kontrol",
                start_btn: "Mulai",
                mode_btn: "Mode",
                temp_up_btn: "Suhu Naik",
                temp_down_btn: "Suhu Turun",
                fan_btn: "Kipas",
                swing_h_btn: "Ayunan H",
                swing_v_btn: "Ayunan V",
                powerful_btn: "Kuat",
                sensor_history: "📊 Riwayat Sensor Langsung",
                footer_title: "Panel Kontrol AC ESP32",
                footer_subtitle: "Proyek Protokol Rombel A-1 Program Studi Rumpun Mekatronika Angkatan 56",
                chart_temp_label: "Suhu (°C)",
                chart_humidity_label: "Kelembapan (%)",
                chart_temp_axis: "Suhu",
                chart_humidity_axis: "Kelembapan",
                language_btn: "BAHASA",
                ap_mode_title: "KulBet AC - Mode Darurat"
            },
            'en': {
                loading: "Loading...",
                room_conditions: "❖ Room Conditions",
                temperature: "Temperature",
                humidity: "Humidity",
                ac_status: "❄️ AC Status",
                status: "Status",
                mode: "Mode",
                fan: "Fan",
                swing_h: "Swing H:",
                swing_v: "Swing V:",
                powerful: "Powerful:",
                controls: "🎛️ Controls",
                start_btn: "Start",
                mode_btn: "Mode",
                temp_up_btn: "Temp Up",
                temp_down_btn: "Temp Down",
                fan_btn: "Fan",
                swing_h_btn: "Swing H",
                swing_v_btn: "Swing V",
                powerful_btn: "Powerful",
                sensor_history: "📊 Live Sensor History",
                footer_title: "ESP32 AC Control Panel",
                footer_subtitle: "Protocol Project Group A-1 Mechatronics Study Program Class of 56",
                chart_temp_label: "Temperature (°C)",
                chart_humidity_label: "Humidity (%)",
                chart_temp_axis: "Temperature",
                chart_humidity_axis: "Humidity",
                language_btn: "LANGUAGE",
                ap_mode_title: "KulBet AC - Emergency Mode"
            },
            'de': {
                loading: "Wird geladen...",
                room_conditions: "❖ Raumbedingungen",
                temperature: "Temperatur",
                humidity: "Feuchtigkeit",
                ac_status: "❄️ AC-Status",
                status: "Status",
                mode: "Modus",
                fan: "Lüfter",
                swing_h: "Schwenk H:",
                swing_v: "Schwenk V:",
                powerful: "Leistungsstark:",
                controls: "🎛️ Steuerungen",
                start_btn: "Start",
                mode_btn: "Modus",
                temp_up_btn: "Temp Hoch",
                temp_down_btn: "Temp Runter",
                fan_btn: "Lüfter",
                swing_h_btn: "Schwenk H",
                swing_v_btn: "Schwenk V",
                powerful_btn: "Leistungsstark",
                sensor_history: "📊 Live-Sensorverlauf",
                footer_title: "ESP32 AC-Bedienfeld",
                footer_subtitle: "Protokollprojekt Gruppe A-1 Mechatronik-Studiengang Klasse 56",
                chart_temp_label: "Temperatur (°C)",
                chart_humidity_label: "Feuchtigkeit (%)",
                chart_temp_axis: "Temperatur",
                chart_humidity_axis: "Feuchtigkeit",
                language_btn: "SPRACHE",
                ap_mode_title: "KulBet AC - Notfallmodus"
            },
            'ja': {
                loading: "読み込み中...",
                room_conditions: "❖ 室内の状況",
                temperature: "温度",
                humidity: "湿度",
                ac_status: "❄️ エアコンの状態",
                status: "状態",
                mode: "モード",
                fan: "ファン",
                swing_h: "スイングH:",
                swing_v: "スイングV:",
                powerful: "パワフル:",
                controls: "🎛️ コントロール",
                start_btn: "開始",
                mode_btn: "モード",
                temp_up_btn: "温度上昇",
                temp_down_btn: "温度下降",
                fan_btn: "ファン",
                swing_h_btn: "スイングH",
                swing_v_btn: "スイングV",
                powerful_btn: "パワフル",
                sensor_history: "📊 ライブセンサー履歴",
                footer_title: "ESP32 ACコントロールパネル",
                footer_subtitle: "プロトコルプロジェクト グループA-1 メカトロニクス研究プログラム 56期生",
                chart_temp_label: "温度 (°C)",
                chart_humidity_label: "湿度 (%)",
                chart_temp_axis: "温度",
                chart_humidity_axis: "湿度",
                language_btn: "言語",
                ap_mode_title: "KulBet AC - 緊急モード"
            },
            'zh': {
                loading: "加载中...",
                room_conditions: "❖ 房间状况",
                temperature: "温度",
                humidity: "湿度",
                ac_status: "❄️ 空调状态",
                status: "状态",
                mode: "模式",
                fan: "风扇",
                swing_h: "水平摆动:",
                swing_v: "垂直摆动:",
                powerful: "强力模式:",
                controls: "🎛️ 控制",
                start_btn: "开始",
                mode_btn: "模式",
                temp_up_btn: "温度升高",
                temp_down_btn: "温度降低",
                fan_btn: "风扇",
                swing_h_btn: "水平摆动",
                swing_v_btn: "垂直摆动",
                powerful_btn: "强力模式",
                sensor_history: "📊 实时传感器历史",
                footer_title: "ESP32 空调控制面板",
                footer_subtitle: "协议项目 A-1组 机电一体化研究项目 第56届",
                chart_temp_label: "温度 (°C)",
                chart_humidity_label: "湿度 (%)",
                chart_temp_axis: "温度",
                chart_humidity_axis: "湿度",
                language_btn: "语言",
                ap_mode_title: "KulBet AC - 紧急模式"
            }
        };

        function setLanguage(lang) {
            if (!translations[lang]) return;
            currentLang = lang;
            localStorage.setItem('language', lang);
            const langDict = translations[lang];

            document.querySelectorAll('[data-translate-key]').forEach(element => {
                const key = element.getAttribute('data-translate-key');
                if (langDict[key]) {
                    element.innerText = langDict[key];
                }
            });
            
            if (lastWeatherData) {
                document.getElementById('location').innerText = lastWeatherData.city;
            }

            const isAPMode = document.body.classList.contains('ap-mode');
            if (isAPMode) {
                document.querySelector('.main-title').innerText = langDict.ap_mode_title || 'AC Control - Emergency Mode';
            }


            if (historyChart) {
                const chartOptions = historyChart.options;
                const chartData = historyChart.data;

                chartData.datasets[0].label = langDict.chart_temp_label;
                chartData.datasets[1].label = langDict.chart_humidity_label;
                chartOptions.scales.y.title.text = langDict.chart_temp_axis;
                chartOptions.scales.y1.title.text = langDict.chart_humidity_axis;
                
                historyChart.update('none');
            }
        };

        var initWebSocket = function() {
            const gateway = `ws://${window.location.hostname}:81/`;
            webSocket = new WebSocket(gateway);
            webSocket.onopen = () => { console.log('WebSocket opened'); };
            webSocket.onclose = () => { setTimeout(initWebSocket, 2000); };
            webSocket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                
                const isOnline = data.wifiMode === 'STA';
                handleOnlineFeatures(isOnline);
                
                updateUI(data, isOnline);
            };
        };

        function handleOnlineFeatures(isOnline) {
            document.querySelector('.header-container').style.display = isOnline ? 'flex' : 'none';
            document.querySelector('.history-section').style.display = isOnline ? 'block' : 'none';
            document.querySelector('.footer').style.display = isOnline ? 'block' : 'none';

            if (!isOnline) {
                if (window.onlineIntervals) {
                    clearInterval(window.onlineIntervals.time);
                    clearInterval(window.onlineIntervals.weather);
                    window.onlineIntervals = null;
                }
                onlineFeaturesInitialized = false;
            } else if (!onlineFeaturesInitialized) {
                console.log("STA mode detected. Initializing online features.");
                initializeChart();
                updateTime();
                updateExternalWeather();
                window.onlineIntervals = {
                    time: setInterval(updateTime, 1000),
                    weather: setInterval(updateExternalWeather, 300000)
                };
                onlineFeaturesInitialized = true;
            }
        }

        var updateUI = function(data, isOnline) {
            const mainTitle = document.querySelector('.main-title');
            if (mainTitle) {
                const langDict = translations[currentLang];
                mainTitle.innerText = isOnline ? 'KulBet AC Live Control' : (langDict.ap_mode_title || 'KulBet AC - Emergency Mode');
            }

            if (data.resetChart && historyChart) {
                historyChart.data.labels = [];
                historyChart.data.datasets.forEach((dataset) => { dataset.data = []; });
            }
            setTheme(data.mode, data.isACOn);
            document.getElementById('statusVal').innerText = data.isACOn ? 'ON' : 'OFF';
            document.getElementById('modeVal').innerText = data.mode;
            document.getElementById('tempVal').innerText = data.temperature + '°C';
            
            const fanSpeed = data.fanSpeed;
            document.getElementById('fanVal').innerText = fanSpeed;
            for (let i = 1; i <= 5; i++) {
                const bar = document.getElementById(`fan-bar-${i}`);
                if (bar) {
                    bar.classList.toggle('active', i <= fanSpeed);
                }
            }

            document.getElementById('swinghVal').innerText = data.swingHorizontal ? 'ON' : 'OFF';
            document.getElementById('swingvVal').innerText = data.swingVertical ? 'ON' : 'OFF';
            document.getElementById('powerfulVal').innerText = data.powerfulMode ? 'ON' : 'OFF';
            document.getElementById('roomTempVal').innerText = data.roomTemp.toFixed(1) + '°C';
            document.getElementById('roomHumVal').innerText = data.roomHumidity.toFixed(1) + '%';
            
            document.getElementById('power-toggle').checked = data.isACOn;
            
            if (data.updateChart && isOnline && historyChart) {
                const now = new Date();
                const timeLabel = now.toLocaleTimeString('id-ID', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
                historyChart.data.labels.push(timeLabel);
                historyChart.data.datasets[0].data.push(data.roomTemp);
                historyChart.data.datasets[1].data.push(data.roomHumidity);
                const MAX_CHART_POINTS = 120;
                if (historyChart.data.labels.length > MAX_CHART_POINTS) {
                    historyChart.data.labels.shift();
                    historyChart.data.datasets.forEach((ds) => { ds.data.shift(); });
                }
                historyChart.update('none');
            }
        };

        var sendCommand = function(command) { webSocket.send(command); };
        var updateTime = function() { fetch('/time').then(r=>r.json()).then(d=>{document.getElementById('clock').innerText=d.time}); };
        var updateExternalWeather = function() {
            fetch('/externalweather').then(r=>r.json()).then(data=>{
                if (!data || data.city === "N/A") return;
                lastWeatherData = data; 
                document.getElementById('location').innerText = data.city;
                document.getElementById('ext-temp').innerText = data.temperature.toFixed(1) + '°C';
                let icon = '❓';
                switch (data.condition.toLowerCase()) {
                    case 'clear': icon = '☀️'; break;
                    case 'clouds': icon = '🌥️'; break;
                    case 'rain': case 'drizzle': icon = '🌧️'; break;
                    case 'thunderstorm': icon = '⛈️'; break;
                    case 'snow': icon = '❄️'; break;
                    case 'mist': case 'smoke': case 'haze': case 'fog': icon = '🌫️'; break;
                }
                document.getElementById('weather-icon').innerHTML = icon;
            });
        };

        var initializeChart = function() {
            if (historyChart) return;
            const ctx = document.getElementById('historyChart').getContext('2d');
            historyChart = new Chart(ctx, {
                type: 'line',
                data: { labels: [], datasets: [{ label: 'Suhu (°C)', yAxisID: 'y', data: [], borderColor: 'rgba(255, 99, 132, 1)', backgroundColor: 'rgba(255, 99, 132, 0.2)'}, { label: 'Kelembapan (%)', yAxisID: 'y1', data: [], borderColor: 'rgba(54, 162, 235, 1)', backgroundColor: 'rgba(54, 162, 235, 0.2)'}] },
                options: { 
                    responsive: true, 
                    maintainAspectRatio: false, 
                    scales: { 
                        x: { ticks: { color: '#FFFFFF' }, grid: { color: 'rgba(255,255,255,0.1)' } }, 
                        y: { type: 'linear', position: 'left', ticks: { color: '#ff6384' }, title: { display: true, text: 'Suhu', color: '#FFFFFF' } }, 
                        y1: { type: 'linear', position: 'right', grid: { drawOnChartArea: false }, ticks: { color: '#36a2eb' }, title: { display: true, text: 'Kelembapan', color: '#FFFFFF' } }
                    }, 
                    plugins: { 
                        legend: { labels: { color: '#FFFFFF' }}
                    }
                }
            });
        };
        
        var setTheme = function(mode, isACOn) {
            let themeClass = 'theme-off';
            let primaryTextColor = '#FFFFFF';
            let gridColor = 'rgba(255, 255, 255, 0.1)';
            const controlsWrapper = document.querySelector('.controls-wrapper');

            if(isACOn) {
                controlsWrapper.classList.remove('controls-hidden');
                switch(mode) {
                    case 'COOL': 
                        themeClass = 'theme-cool'; 
                        primaryTextColor = '#000000';
                        gridColor = 'rgba(0, 0, 0, 0.1)';
                        break;
                    case 'DRY': 
                        themeClass = 'theme-dry'; 
                        break;
                    case 'FAN ONLY': 
                        themeClass = 'theme-fan'; 
                        break;
                }
            } else {
                controlsWrapper.classList.add('controls-hidden');
            }
            document.body.className = themeClass;
            if (document.body.classList.contains('ap-mode')) {
                document.body.classList.add('ap-mode');
            }

            if (historyChart) {
                const chartOptions = historyChart.options;
                
                chartOptions.scales.x.ticks.color = primaryTextColor;
                chartOptions.scales.y.title.color = primaryTextColor;
                chartOptions.scales.y1.title.color = primaryTextColor;
                chartOptions.plugins.legend.labels.color = primaryTextColor;
                chartOptions.scales.x.grid.color = gridColor;
                chartOptions.scales.y.grid.color = gridColor;
                chartOptions.scales.y.ticks.color = '#ff6384';
                chartOptions.scales.y1.ticks.color = '#36a2eb';
                historyChart.update('none');
            }
        };

        document.addEventListener('DOMContentLoaded', () => {
            const savedLang = localStorage.getItem('language') || 'id';
            
            initWebSocket();
            setLanguage(savedLang); 
            
            document.getElementById('power-toggle').addEventListener('change', function() {
                sendCommand(this.checked ? 'ON' : 'OFF');
            });
        });
    </script>
</body>
</html>
)rawliteral";

// --- Konfigurasi NTP (Jam Online) ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;  // Offset GMT untuk WIB (UTC+7)
const int   daylightOffset_sec = 0;    // Indonesia tidak punya daylight saving

// --- Konfigurasi Weather API ---
const char* openWeatherMapApiKey = "0282197f6f7a93232cb21c28695e4d76";
String locationCity = "Detecting...";
float externalTemp = 0.0;
float externalHumidity = 0.0;
String weatherCondition = "Unknown";
unsigned long lastWeatherCheckTime = 0;
const unsigned long WEATHER_CHECK_INTERVAL = 15 * 60 * 1000; // Cek cuaca setiap 15 menit

// --- Konfigurasi Perangkat Keras ---
#define BUZZER_PIN 16 // Pin GPIO16 untuk buzzer
#define DHTPIN 5      // Pin GPIO5 untuk DHT22
#define DHTTYPE DHT22 // Tipe sensor DHT22
const uint16_t kIrLed = 4; // GPIO pin untuk IR LED

// --- KONFIGURASI BARU UNTUK NEXTION ---
#define NEXTION_TX_PIN 21 // ESP32 TX pin, terhubung ke Nextion RX
#define NEXTION_RX_PIN 22 // ESP32 RX pin, terhubung ke Nextion TX
HardwareSerial NextionSerial(1); // Gunakan UART1 untuk Nextion

// --- Inisialisasi Objek ---
DHT dht(DHTPIN, DHTTYPE);
IRDaikinESP ac(kIrLed);
AsyncWebServer server(80);
WebSocketsServer webSocket(81);
Preferences preferences;

// --- Variabel Global ---
float currentRoomTemp = 0;
float currentRoomHumidity = 0;
unsigned long lastDHTReadTime = 0;
const unsigned long DHT_READ_INTERVAL = 2000; // Baca DHT setiap 2 detik

// --- Variabel Status AC ---
uint8_t currentFanSpeed = 1;
bool swingHorizontal = false;
bool swingVertical = false;
uint8_t currentTemperature = 20;
uint8_t currentMode = kDaikinCool;
bool isACOn = false;
bool isPowerfulModeOn = false;

// --- Konfigurasi WiFi ---
const char* sta_ssid = "Gabriel's Hotspot";
const char* sta_password = "enakajalu";

// Mode Access Point (Jika koneksi ke WiFi utama gagal)
const char* ap_ssid = "ESP_REMOTE";
const char* ap_password = "password123";
IPAddress apIP(192, 168, 4, 1);
IPAddress apGateway(192, 168, 4, 1);
IPAddress apSubnet(255, 255, 255, 0);

// --- Pencatatan Riwayat & Akumulasi ---
struct HistoryEntry {
    uint32_t timestamp;
    float temperature;
    float humidity;
};
std::vector<HistoryEntry> sensorHistory;
const size_t MAX_HISTORY_ENTRIES = 60;
unsigned long lastHistoryLogTime = 0;
const unsigned long HISTORY_LOG_INTERVAL = 1 * 60 * 1000; // Log setiap 1 menit

// <<-- PERBAIKAN: Memisahkan interval UI dan interval Grafik -->>
unsigned long lastDataBroadcastTime = 0;
const unsigned long DATA_BROADCAST_INTERVAL = 5000; // Broadcast data UI setiap 5 detik agar responsif
unsigned long lastChartLogTime = 0;
const unsigned long CHART_LOG_INTERVAL = 60000; // Tambah titik ke grafik setiap 1 menit

// --- Variabel Baru untuk Timer Reset Grafik ---
unsigned long lastChartResetTime = 0;
const unsigned long CHART_RESET_INTERVAL = 5 * 60 * 1000;

// --- Variabel Baru untuk Timer Update Status Nextion ---
unsigned long lastNextionStatusUpdateTime = 0;
const unsigned long NEXTION_STATUS_UPDATE_INTERVAL = 1000; // Update setiap 1 detik

// --- Integrasi Google Sheets ---
const char* GOOGLE_SCRIPT_ID = "AKfycby8bQGII_WLqvjx99QX-rhXfTDRBx2qdrfizG5Cd5yIyMYrw2pBlTCa4vUNct3n4njV";

// --- Prototipe Fungsi ---
void turnACOn(); void turnACOff(); void changeFanSpeed(); void toggleSwingHorizontal(); void toggleSwingVertical(); void increaseTemperature(); void decreaseTemperature(); void changeMode(); void activatePowerfulMode(); void deactivatePowerfulMode(); void readDHTSensor(); void playButtonSound(); void logSensorData(); void sendDataToGoogleSheet(); void getWeatherData();
void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void handleNextionInput();
void updateNextionDisplay();
void broadcastState(bool resetChart = false, bool updateChart = true);
String getStatusJSON(bool resetChart = false, bool updateChart = true);
String getModeString(uint8_t mode);
void serveHelloPage(AsyncWebServerRequest *request);
void serveDashboardPage(AsyncWebServerRequest *request);
void handleTime(AsyncWebServerRequest *request);
void handleExternalWeather(AsyncWebServerRequest *request);
void handleHistory(AsyncWebServerRequest *request);
void saveACState();
void loadACState();

// =================================================================
//  SETUP
// =================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\nAC Control with Async Web Server, WebSockets, and Nextion");

    NextionSerial.begin(9600, SERIAL_8N1, NEXTION_RX_PIN, NEXTION_TX_PIN);
    Serial.println("Nextion communication ready.");

    digitalWrite(BUZZER_PIN, LOW);
    dht.begin();
    ac.begin();

    preferences.begin("ac-state", false);
    loadACState();

    WiFi.mode(WIFI_AP_STA);
    
    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println("==================================================");
    Serial.println("Access Point (AP) Mode ALWAYS ON");
    Serial.print("AP SSID: "); Serial.println(ap_ssid);
    Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());
    Serial.println("==================================================");

    Serial.print("Attempting to connect to main WiFi: "); Serial.println(sta_ssid);
    WiFi.begin(sta_ssid, sta_password);
    int connect_timeout = 20;
    while (WiFi.status() != WL_CONNECTED && connect_timeout > 0) {
        delay(500); Serial.print("."); connect_timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnection to main WiFi successful (STA Mode)");
        Serial.print("STA IP address: "); Serial.println(WiFi.localIP());
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        struct tm timeinfo;
        if(getLocalTime(&timeinfo)){
          Serial.println("Time successfully synchronized");
        }
        getWeatherData();
        lastWeatherCheckTime = millis();
    } else {
        Serial.println("\nFailed to connect to main WiFi. Device is only accessible via AP.");
    }

    server.on("/", HTTP_GET, serveHelloPage);
    server.on("/dashboard", HTTP_GET, serveDashboardPage);
    server.on("/time", HTTP_GET, handleTime);
    server.on("/externalweather", HTTP_GET, handleExternalWeather);
    server.on("/history", HTTP_GET, handleHistory);

    server.begin();
    Serial.println("Async HTTP server started. Listening on all interfaces.");

    webSocket.begin();
    webSocket.onEvent(handleWebSocketMessage);
    Serial.println("WebSocket server started. Listening on all interfaces.");

    updateNextionDisplay(); 
    
    NextionSerial.print("page 0"); 
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
}

// =================================================================
//  LOOP
// =================================================================
void loop() {
    webSocket.loop();
    handleNextionInput(); 
    readDHTSensor();

    unsigned long currentMillis = millis();

    if (currentMillis - lastNextionStatusUpdateTime >= NEXTION_STATUS_UPDATE_INTERVAL) {
        lastNextionStatusUpdateTime = currentMillis;
        updateNextionDisplay();
    }

    // <<-- PERBAIKAN: Logika broadcast dipisah -->>
    // Kirim data UI (tanpa update grafik) secara berkala agar UI responsif
    if (currentMillis - lastDataBroadcastTime >= DATA_BROADCAST_INTERVAL) {
        lastDataBroadcastTime = currentMillis;
        broadcastState(false, false);
    }

    // Kirim data untuk update grafik secara terpisah dengan intervalnya sendiri
    if (WiFi.status() == WL_CONNECTED && (currentMillis - lastChartLogTime >= CHART_LOG_INTERVAL)) {
        lastChartLogTime = currentMillis;
        
        bool shouldResetChart = (isACOn && (currentMillis - lastChartResetTime >= CHART_RESET_INTERVAL));
        if (shouldResetChart) {
            lastChartResetTime = currentMillis;
        }
        
        // Kirim data dengan flag updateChart=true
        broadcastState(shouldResetChart, true);
    }


    if (isACOn && WiFi.status() == WL_CONNECTED && currentMillis - lastHistoryLogTime >= HISTORY_LOG_INTERVAL) {
        lastHistoryLogTime = currentMillis;
        logSensorData();
        sendDataToGoogleSheet();
    }

    if (WiFi.status() == WL_CONNECTED && currentMillis - lastWeatherCheckTime >= WEATHER_CHECK_INTERVAL) {
        lastWeatherCheckTime = currentMillis;
        getWeatherData();
    }
}

// =================================================================
//  Fungsi-Fungsi Bantuan & Kontrol
// =================================================================

void handleNextionInput() {
    if (NextionSerial.available()) {
        String receivedData = "";
        delay(50); 

        while (NextionSerial.available()) {
            char c = NextionSerial.read();
            if (c >= 32 && c <= 126) { 
                receivedData += c;
            }
        }

        if (receivedData.length() > 0) {
            Serial.print("Menerima dari Nextion: [");
            Serial.print(receivedData);
            Serial.println("]");

            playButtonSound(); 

            if (receivedData == "on") turnACOn();
            else if (receivedData == "off") turnACOff();
            else if (isACOn) {
                if (receivedData == "temp_up") increaseTemperature();
                else if (receivedData == "temp_down") decreaseTemperature();
                else if (receivedData == "fan") changeFanSpeed();
                else if (receivedData == "mode") changeMode();
                else if (receivedData == "swing_h") toggleSwingHorizontal();
                else if (receivedData == "swing_v") toggleSwingVertical();
                else if (receivedData == "powerful") {
                    if (!isPowerfulModeOn) activatePowerfulMode();
                    else deactivatePowerfulMode();
                }
            }
            
            // Kirim status UI saja, jangan update grafik saat tombol ditekan
            broadcastState(false, false); 
            updateNextionDisplay(); 
        }
    }
}

void updateNextionDisplay() {
    String command;
    String statusON = "ON";
    String statusOFF = "OFF";
    String placeholder = "-";

    // 1. Update Temperature Display (t0.txt)
    if (isACOn) {
        NextionSerial.print("t0.txt=\"");
        NextionSerial.print(currentTemperature);
        NextionSerial.write(0xB0); // Mengirim byte untuk '°'
        NextionSerial.print("C\"");
    } else {
        NextionSerial.print("t0.txt=\"--\"");
    }
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 2. Update Fan Speed (f.txt)
    command = "f.txt=\"" + (isACOn ? String(currentFanSpeed) : placeholder) + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 3. Update Mode (m.txt)
    command = "m.txt=\"" + (isACOn ? getModeString(currentMode) : statusOFF) + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 4. Update Swing Vertical (sv.txt)
    command = "sv.txt=\"" + (isACOn ? (swingVertical ? statusON : statusOFF) : placeholder) + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 5. Update Swing Horizontal (sh.txt)
    command = "sh.txt=\"" + (isACOn ? (swingHorizontal ? statusON : statusOFF) : placeholder) + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 6. Update Powerful Mode (p.txt)
    command = "p.txt=\"" + (isACOn ? (isPowerfulModeOn ? statusON : statusOFF) : placeholder) + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 7. Update Room Temperature (dt.txt)
    // <<-- MODIFIKASI: Menggunakan metode pengiriman byte terpisah agar konsisten dan andal -->>
    NextionSerial.print("dt.txt=\"");
    NextionSerial.print(String(currentRoomTemp, 1));
    NextionSerial.write(0xB0); // Mengirim byte untuk '°'
    NextionSerial.print("C\"");
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 8. Update Room Humidity (dh.txt)
    command = "dh.txt=\"" + String(currentRoomHumidity, 1) + "%\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);

    // 9. Update STA IP Address (ip.txt)
    String ipAddress = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "AP Mode Only";
    command = "ip.txt=\"" + ipAddress + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    delay(20);
    
    // 10. Update WiFi SSID (ssid.txt)
    String currentSSID = (WiFi.status() == WL_CONNECTED) ? String(sta_ssid) : String(ap_ssid);
    command = "ssid.txt=\"" + currentSSID + "\"";
    NextionSerial.print(command);
    NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
}


void broadcastState(bool resetChart, bool updateChart) {
    String jsonState = getStatusJSON(resetChart, updateChart);
    webSocket.broadcastTXT(jsonState);
}

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            String statusJSON = getStatusJSON(true, WiFi.status() == WL_CONNECTED);
            webSocket.sendTXT(num, statusJSON);
            break;
        }
        case WStype_TEXT: {
            String command = String((char*)payload);
            command.toUpperCase();
            playButtonSound();

            if (command == "ON") turnACOn();
            else if (command == "OFF") turnACOff();
            else if (isACOn) {
                if (command == "FAN") changeFanSpeed();
                else if (command == "SWINGH") toggleSwingHorizontal();
                else if (command == "SWINGV") toggleSwingVertical();
                else if (command == "TEMPUP") increaseTemperature();
                else if (command == "TEMPDOWN") decreaseTemperature();
                else if (command == "MODE") changeMode();
                else if (command == "POWERFUL") {
                    if (!isPowerfulModeOn) activatePowerfulMode();
                    else deactivatePowerfulMode();
                }
            }
            // Kirim status UI saja, jangan update grafik saat tombol ditekan
            broadcastState(false, false); 
            updateNextionDisplay(); 
            break;
        }
    }
}

void getWeatherData() {
    if (WiFi.status() != WL_CONNECTED) { return; }
    String city = "Surakarta"; 
    locationCity = city; 

    HTTPClient http;
    http.setTimeout(5000);
    
    http.begin("http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + openWeatherMapApiKey + "&units=metric");
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        JSONVar myObject = JSON.parse(http.getString());
        if (JSON.typeof(myObject) != "undefined") {
            externalTemp = (double) myObject["main"]["temp"];
            externalHumidity = (double) myObject["main"]["humidity"];
            String mainCondition = JSON.stringify(myObject["weather"][0]["main"]);
            mainCondition.replace("\"", "");
            weatherCondition = mainCondition;
            Serial.println("Weather data updated for " + city);
        }
    } else {
        Serial.printf("Weather data failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void playButtonSound() {
    tone(BUZZER_PIN, 2000, 50);
}

void readDHTSensor() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastDHTReadTime >= DHT_READ_INTERVAL) {
        lastDHTReadTime = currentMillis;
        float newTemp = dht.readTemperature();
        float newHumidity = dht.readHumidity();
        if (!isnan(newTemp)) { currentRoomTemp = newTemp; }
        if (!isnan(newHumidity)) { currentRoomHumidity = newHumidity; }
    }
}

void logSensorData() {
    if (WiFi.status() != WL_CONNECTED) return;
    HistoryEntry newEntry;
    newEntry.timestamp = time(nullptr);
    newEntry.temperature = currentRoomTemp;
    newEntry.humidity = currentRoomHumidity;
    if (sensorHistory.size() >= MAX_HISTORY_ENTRIES) {
        sensorHistory.erase(sensorHistory.begin());
    }
    sensorHistory.push_back(newEntry);
}

void sendDataToGoogleSheet() {
    if (isnan(currentRoomTemp) || isnan(currentRoomHumidity) || WiFi.status() != WL_CONNECTED) { return; }
    HTTPClient https;
    https.setTimeout(5000);
    WiFiClientSecure client;
    client.setInsecure();
    String url = "https://script.google.com/macros/s/" + String(GOOGLE_SCRIPT_ID) + "/exec" +
                 "?temperature=" + String(currentRoomTemp, 2) +
                 "&humidity=" + String(currentRoomHumidity, 2) +
                 "&timestamp=" + String(time(nullptr));
    if (https.begin(client, url)) {
        https.GET();
        https.end();
    }
}

// =================================================================
//  Fungsi-Fungsi untuk Handler Server Web (Async)
// =================================================================

void serveHelloPage(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", HELLO_PAGE);
}

void serveDashboardPage(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", DASHBOARD_PAGE);
}

void handleTime(AsyncWebServerRequest *request) {
    if (WiFi.status() != WL_CONNECTED) {
        request->send(200, "application/json", "{\"time\":\"N/A\"}");
        return;
    }
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        request->send(503, "application/json", "{\"time\":\"Syncing...\"}");
        return;
    }
    char timeString[9];
    sprintf(timeString, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    String json = "{\"time\":\"" + String(timeString) + "\"}";
    request->send(200, "application/json", json);
}

void handleHistory(AsyncWebServerRequest *request) {
    if (WiFi.status() != WL_CONNECTED) {
        request->send(200, "application/json", "[]");
        return;
    }
    String json = "[";
    for (size_t i = 0; i < sensorHistory.size(); ++i) {
        json += "{";
        json += "\"timestamp\":" + String(sensorHistory[i].timestamp) + ",";
        json += "\"temperature\":" + String(sensorHistory[i].temperature, 1) + ",";
        json += "\"humidity\":" + String(sensorHistory[i].humidity, 1);
        json += "}";
        if (i < sensorHistory.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    request->send(200, "application/json", json);
}

void handleExternalWeather(AsyncWebServerRequest *request) {
    if (WiFi.status() != WL_CONNECTED) {
        request->send(200, "application/json", "{\"city\":\"N/A\", \"temperature\":0, \"humidity\":0, \"condition\":\"N/A\"}");
        return;
    }
    String json = "{";
    json += "\"city\":\"" + locationCity + "\",";
    json += "\"temperature\":" + String(externalTemp, 1) + ",";
    json += "\"humidity\":" + String(externalHumidity, 1) + ",";
    json += "\"condition\":\"" + weatherCondition + "\"";
    json += "}";
    request->send(200, "application/json", json);
}

String getStatusJSON(bool resetChart, bool updateChart) {
    String json = "{";
    json += "\"isACOn\":" + String(isACOn ? "true" : "false") + ",";
    json += "\"mode\":\"" + getModeString(currentMode) + "\",";
    json += "\"temperature\":" + String(currentTemperature) + ",";
    json += "\"fanSpeed\":" + String(currentFanSpeed) + ",";
    json += "\"swingHorizontal\":" + String(swingHorizontal ? "true" : "false") + ",";
    json += "\"swingVertical\":" + String(swingVertical ? "true" : "false") + ",";
    json += "\"powerfulMode\":" + String(isPowerfulModeOn ? "true" : "false") + ",";
    json += "\"roomTemp\":" + String(currentRoomTemp, 1) + ",";
    json += "\"roomHumidity\":" + String(currentRoomHumidity, 1) + ",";
    json += "\"resetChart\":" + String(resetChart ? "true" : "false") + ",";
    json += "\"updateChart\":" + String(updateChart ? "true" : "false") + ",";
    json += "\"wifiMode\":\"" + String(WiFi.status() == WL_CONNECTED ? "STA" : "AP") + "\"";
    json += "}";
    return json;
}

String getModeString(uint8_t mode) {
    switch (mode) {
        case kDaikinCool: return "COOL";
        case kDaikinDry: return "DRY";
        case kDaikinFan: return "FAN ONLY";
        default: return "UNKNOWN";
    }
}

void turnACOn() {
    if (!isACOn) {
        isACOn = true;
        isPowerfulModeOn = false;
        lastChartResetTime = millis();
        lastHistoryLogTime = millis();
        sensorHistory.clear();
        logSensorData();
        NextionSerial.print("page 1");
        NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    }
    ac.on();
    ac.setMode(currentMode);
    ac.setTemp(currentTemperature);
    ac.setSwingVertical(swingVertical);
    ac.setSwingHorizontal(swingHorizontal);
    ac.setFan(currentFanSpeed);
    ac.setPowerful(isPowerfulModeOn);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void turnACOff() {
    if (isACOn) {
        isACOn = false;
        isPowerfulModeOn = false;
        ac.off();
        ac.send();
        sensorHistory.clear();
        NextionSerial.print("page 0");
        NextionSerial.write(0xFF); NextionSerial.write(0xFF); NextionSerial.write(0xFF);
    }
    saveACState();
    updateNextionDisplay(); 
}

void changeFanSpeed() {
    if (!isACOn) return;
    isPowerfulModeOn = false;
    currentFanSpeed++;
    if (currentFanSpeed > 5) currentFanSpeed = 1;
    ac.setFan(currentFanSpeed);
    ac.setPowerful(false);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void toggleSwingHorizontal() {
    if (!isACOn) return;
    swingHorizontal = !swingHorizontal;
    ac.setSwingHorizontal(swingHorizontal);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void toggleSwingVertical() {
    if (!isACOn) return;
    isPowerfulModeOn = false;
    swingVertical = !swingVertical;
    ac.setPowerful(false);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void increaseTemperature() {
    if (!isACOn) return;
    isPowerfulModeOn = false;
    if (currentTemperature < 30) currentTemperature++;
    ac.setTemp(currentTemperature);
    ac.setPowerful(false);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void decreaseTemperature() {
    if (!isACOn) return;
    isPowerfulModeOn = false;
    if (currentTemperature > 16) currentTemperature--;
    ac.setTemp(currentTemperature);
    ac.setPowerful(false);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void changeMode() {
    if (!isACOn) return;
    isPowerfulModeOn = false;
    if (currentMode == kDaikinCool) { currentMode = kDaikinDry; }    
    else if (currentMode == kDaikinDry) { currentMode = kDaikinFan; }    
    else if (currentMode == kDaikinFan) { currentMode = kDaikinCool; }
    ac.setMode(currentMode);
    ac.setPowerful(false);
    ac.send();
    saveACState();
    updateNextionDisplay(); 
}

void activatePowerfulMode() {
    if (!isACOn || isPowerfulModeOn) return;
    isPowerfulModeOn = true;
    ac.setPowerful(true);
    ac.send();
    updateNextionDisplay(); 
}

void deactivatePowerfulMode() {
    if (!isACOn || !isPowerfulModeOn) return;
    isPowerfulModeOn = false;
    ac.setPowerful(false);
    ac.setMode(currentMode);
    ac.setTemp(currentTemperature);
    ac.setFan(currentFanSpeed);
    ac.setSwingHorizontal(swingHorizontal);
    ac.setSwingVertical(swingVertical);
    ac.send();
    updateNextionDisplay(); 
}

void saveACState() {
    preferences.putUChar("mode", currentMode);
    preferences.putUChar("temp", currentTemperature);
    preferences.putUChar("fan", currentFanSpeed);
    preferences.putBool("swingH", swingHorizontal);
    preferences.putBool("swingV", swingVertical);
}

void loadACState() {
    isACOn = false;
    isPowerfulModeOn = false;

    currentMode = preferences.getUChar("mode", kDaikinCool);
    currentTemperature = preferences.getUChar("temp", 20);
    currentFanSpeed = preferences.getUChar("fan", 1);
    swingHorizontal = preferences.getBool("swingH", false);
    swingVertical = preferences.getBool("swingV", false);
    
    Serial.println("Last AC settings loaded. Power is OFF by default.");
}