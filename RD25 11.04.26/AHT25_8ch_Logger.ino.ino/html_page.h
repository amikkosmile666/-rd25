#ifndef HTML_PAGE_H
#define HTML_PAGE_H

const char htmlPage[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
<head>
    <title>RD25 - Измеритель</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        /* Официальная цветовая схема */
        body { 
            font-family: 'Segoe UI', Arial, sans-serif; 
            text-align: center; 
            margin: 20px; 
            background: #f0f4f8;
        }
        .container { 
            background: white; 
            padding: 30px; 
            border-radius: 12px; 
            box-shadow: 0 4px 12px rgba(0,0,0,0.08);
            max-width: 1000px;
            margin: 0 auto;
        }
        h1 { color: #2c3e50; margin-bottom: 30px; font-weight: 500; }
        
        /* Сетка датчиков */
        .sensors-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        .sensor-card { 
            padding: 20px; 
            border-radius: 10px;
            font-size: 16px;
            border-left: 6px solid;
            background: #fff;
            box-shadow: 0 2px 4px rgba(0,0,0,0.05);
            transition: all 0.2s;
        }
        .sensor-card:hover {
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            transform: translateY(-2px);
        }
        /* Уникальные цвета для каждого датчика */
        .sensor-0 { border-left-color: #1f3a5f; }
        .sensor-1 { border-left-color: #1e5f3a; }
        .sensor-2 { border-left-color: #8b3a3a; }
        .sensor-3 { border-left-color: #6b5e1e; }
        .sensor-4 { border-left-color: #5e2a6b; }
        .sensor-5 { border-left-color: #1a6b6b; }
        .sensor-6 { border-left-color: #b85c1a; }
        .sensor-7 { border-left-color: #4a6a8b; }
        .sensor-offline { 
            background: #f8f9fa; 
            border-left-color: #bdc3c7 !important;
            color: #7f8c8d;
            opacity: 0.85;
        }
        .sensor-offline:hover {
            opacity: 1;
        }
        
        /* WiFi блок */
        .wifi-card { 
            background: #ecf0f1; 
            padding: 15px;
            margin: 15px 0;
            border-radius: 8px;
            font-size: 14px;
            color: #2c3e50;
            border: 1px solid #d5dbdf;
        }
        
        /* Значения датчиков */
        .value { 
            font-size: 24px; 
            font-weight: 600;
            margin: 8px 0;
        }
        .temp-value { color: #2c3e50; }
        .hum-value { color: #2980b9; }
        
        /* Кнопки */
        .btn { 
            background: #2980b9; 
            color: white; 
            border: none; 
            padding: 10px 20px; 
            border-radius: 6px; 
            cursor: pointer;
            font-size: 14px;
            margin: 8px 5px;
            transition: all 0.2s;
            font-weight: 500;
        }
        .btn:hover { background: #1c6ea4; }
        .btn-danger { background: #e74c3c; }
        .btn-danger:hover { background: #c0392b; }
        .btn-warning { background: #f39c12; color: #2c3e50; }
        .btn-warning:hover { background: #e67e22; }
        .btn-info { background: #3498db; }
        .btn-info:hover { background: #2c81ba; }
        
        .status-good { color: #27ae60; font-weight: bold; }
        .status-bad { color: #e74c3c; font-weight: bold; }
        
        .last-update { color: #7f8c8d; font-size: 12px; margin-top: 20px; }
        
        .sensor-header {
            font-weight: 600;
            margin-bottom: 8px;
            font-size: 16px;
            color: #2c3e50;
        }
        .sensor-status {
            font-size: 12px;
            margin-top: 5px;
        }
        .online { color: #27ae60; }
        .offline { color: #e74c3c; }
        
        /* Статистика */
        .stats {
            background: #2c3e50;
            color: white;
            padding: 15px;
            border-radius: 8px;
            margin: 15px 0;
            font-size: 14px;
        }
        
        /* Текущее время */
        .time-card {
            background: #34495e;
            color: white;
            padding: 10px;
            margin: 10px 0;
            border-radius: 8px;
            font-size: 18px;
            font-weight: 500;
        }
        
        /* Настройки опроса */
        .settings-card {
            background: #ffffff;
            padding: 20px;
            margin: 20px 0;
            border-radius: 10px;
            border: 1px solid #d5dbdf;
            box-shadow: 0 1px 2px rgba(0,0,0,0.05);
        }
        .settings-card h3 {
            color: #2c3e50;
            margin-top: 0;
        }
        .settings-row {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin: 12px 0;
        }
        .settings-label {
            font-weight: 600;
            flex: 1;
            color: #2c3e50;
        }
        .time-input-container {
            display: flex;
            flex: 2;
            gap: 5px;
        }
        .time-input {
            width: 80px;
            padding: 8px;
            border-radius: 5px;
            border: 1px solid #bdc3c7;
            font-family: monospace;
            text-align: center;
            font-size: 16px;
        }
        .time-input:focus {
            outline: 2px solid #2980b9;
            border-color: #2980b9;
        }
        .time-separator {
            font-size: 20px;
            font-weight: bold;
            line-height: 35px;
            color: #7f8c8d;
        }
        
        /* Блок записи данных */
        .log-card {
            background: #ffffff;
            padding: 20px;
            margin: 20px 0;
            border-radius: 10px;
            border-left: 6px solid #2980b9;
            box-shadow: 0 1px 2px rgba(0,0,0,0.05);
        }
        .log-card h3 {
            color: #2c3e50;
            margin-top: 0;
        }
        .log-status {
            padding: 15px;
            border-radius: 8px;
            margin: 10px 0;
            font-weight: 500;
            min-height: 100px;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-direction: column;
            gap: 10px;
        }
        .log-enabled {
            background: #d5f5e3;
            color: #1e6f3f;
            border: 1px solid #a9dfbf;
        }
        .log-disabled {
            background: #fadbd8;
            color: #c0392b;
            border: 1px solid #f1948a;
        }
        .log-completed {
            background: #d4e6f1;
            color: #21618c;
            border: 1px solid #a9cce3;
        }
        .log-countdown {
            background: #fef9e7;
            color: #b7950b;
            border: 1px solid #f9e79f;
        }
        .log-status-text {
            font-size: 18px;
            font-weight: 600;
            text-align: center;
        }
        .countdown-timer {
            font-size: 28px;
            font-weight: bold;
            color: #c0392b;
            margin: 10px 0;
            padding: 8px 20px;
            background: #ffffff;
            border-radius: 8px;
            min-width: 200px;
            text-align: center;
            border: 1px solid #e74c3c;
            font-family: monospace;
        }
        .info-text {
            font-size: 12px;
            color: #7f8c8d;
            margin-top: 10px;
        }
        .log-files {
            background: #f8f9fa;
            padding: 12px;
            border-radius: 6px;
            margin: 10px 0;
            text-align: left;
            max-height: 250px;
            overflow-y: auto;
            border: 1px solid #e0e4e8;
        }
        .log-file-item {
            padding: 8px;
            border-bottom: 1px solid #e0e4e8;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .log-file-item:hover {
            background: #ecf0f1;
        }
        .log-file-name {
            font-weight: 500;
            flex: 2;
            word-break: break-all;
            color: #2c3e50;
        }
        .log-file-size {
            color: #7f8c8d;
            flex: 1;
            text-align: center;
            font-size: 12px;
        }
        .delete-btn {
            background: #e74c3c;
            color: white;
            border: none;
            padding: 4px 8px;
            border-radius: 3px;
            cursor: pointer;
            font-size: 11px;
            margin: 0 2px;
        }
        .delete-btn:hover {
            background: #c0392b;
        }
        .download-btn {
            background: #2980b9;
            color: white;
            border: none;
            padding: 4px 8px;
            border-radius: 3px;
            cursor: pointer;
            font-size: 11px;
            margin: 0 2px;
        }
        .download-btn:hover {
            background: #1c6ea4;
        }
        .time-format-hint {
            font-size: 11px;
            color: #95a5a6;
            margin-top: 5px;
        }
        .log-buttons {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin: 15px 0;
            flex-wrap: wrap;
        }
        .action-buttons {
            display: flex;
            gap: 10px;
            justify-content: center;
            margin-top: 15px;
            flex-wrap: wrap;
        }
        .settings-value {
            font-weight: 600;
            color: #2c3e50;
            min-width: 100px;
            text-align: right;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>RD25</h1>
        
        <div class="wifi-card">
            <strong>WiFi:</strong> 
            <span id="wifiStatus" class="%WIFI_STATUS_CLASS%">%WIFI_STATUS%</span>
            (<strong>SSID:</strong> %WIFI_SSID%)
            <strong>IP:</strong> %IP% 
            <strong>Сигнал:</strong> %RSSI% dBm
        </div>
        
        <div class="time-card">
            Время: <span id="currentTime">%CURRENT_TIME%</span>
        </div>

        <div class="stats">
            <strong>Статистика:</strong> Активных датчиков: <span id="activeSensors">%ACTIVE_SENSORS%</span> | 
            Средняя температура: <span id="avgTemp">%AVG_TEMP%</span> C | 
            Средняя влажность: <span id="avgHum">%AVG_HUM%</span> % |
            Частота опроса: <span id="pollingRate">%POLLING_RATE_FORMATTED%</span>
        </div>
        
        <div class="sensors-grid" id="sensorsGrid">
            %SENSORS_GRID%
        </div>
        
        <div class="settings-card">
            <h3>Настройки опроса</h3>
            <div class="settings-row">
                <div class="settings-label">Частота опроса:</div>
                <div class="time-input-container">
                    <input type="text" class="time-input" id="pollingHours" placeholder="ЧЧ" value="%POLLING_HOURS%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="pollingMinutes" placeholder="ММ" value="%POLLING_MINUTES%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="pollingSeconds" placeholder="СС" value="%POLLING_SECONDS%" maxlength="2">
                </div>
                <div class="settings-value" id="pollingValue">%POLLING_RATE_FORMATTED%</div>
            </div>
            <div class="time-format-hint">Формат: ЧЧ:ММ:СС (минимальный интервал 1 секунда)</div>
            <button class="btn" onclick="saveSettings()">Сохранить настройки опроса</button>
        </div>
        
        <div class="log-card">
            <h3>Настройки записи данных</h3>
            
            <div class="log-status %LOG_STATUS_CLASS%" id="logStatus">
                <div class="log-status-text" id="logStatusText">%LOG_STATUS_TEXT%</div>
                <div id="countdownTimer" class="countdown-timer" style="display: %COUNTDOWN_DISPLAY%;">%COUNTDOWN_TEXT%</div>
            </div>
            
            <div class="settings-row">
                <div class="settings-label">Интервал записи:</div>
                <div class="time-input-container">
                    <input type="text" class="time-input" id="logIntervalHours" placeholder="ЧЧ" value="%LOG_INTERVAL_HOURS%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="logIntervalMinutes" placeholder="ММ" value="%LOG_INTERVAL_MINUTES%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="logIntervalSeconds" placeholder="СС" value="%LOG_INTERVAL_SECONDS%" maxlength="2">
                </div>
                <div class="settings-value" id="logIntervalValue">%LOG_INTERVAL_FORMATTED%</div>
            </div>
            <div class="time-format-hint">Формат: ЧЧ:ММ:СС (минимальный интервал 10 секунд)</div>
            
            <div class="settings-row">
                <div class="settings-label">Длительность записи:</div>
                <div class="time-input-container">
                    <input type="text" class="time-input" id="logDurationHours" placeholder="ЧЧ" value="%LOG_DURATION_HOURS%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="logDurationMinutes" placeholder="ММ" value="%LOG_DURATION_MINUTES%" maxlength="2">
                    <span class="time-separator">:</span>
                    <input type="text" class="time-input" id="logDurationSeconds" placeholder="СС" value="%LOG_DURATION_SECONDS%" maxlength="2">
                </div>
                <div class="settings-value" id="logDurationValue">%LOG_DURATION_FORMATTED%</div>
            </div>
            <div class="time-format-hint">Формат: ЧЧ:ММ:СС (00:00:00 = бесконечно)</div>
            
            <div id="logFilesContainer" style="display: none;">
                <h4>Файлы данных:</h4>
                <div class="log-files" id="logFilesList">
                    %LOG_FILES_LIST%
                </div>
            </div>
            
            <div class="info-text">
                Интервал записи: как часто записывать данные (в формате ЧЧ:ММ:СС)<br>
                Длительность: общее время записи (00:00:00 = бесконечно)<br>
                Текущий файл данных: %CURRENT_LOG_FILE%
            </div>
            
            <div class="log-buttons">
                <button class="btn %LOG_BTN_CLASS%" id="logToggleBtn" onclick="toggleLogging()">%LOG_BTN_TEXT%</button>
            </div>
            
            <div class="action-buttons">
                <button class="btn btn-info" onclick="showLogFiles()">Показать файлы</button>
                <button class="btn btn-warning" onclick="downloadCurrentLog()">Скачать текущий файл</button>
                <button class="btn btn-danger" onclick="clearAllLogs()">Очистить все файлы</button>
            </div>
        </div>
        
        <div class="action-buttons">
            <button class="btn" onclick="updateData()">Обновить данные</button>
            <button class="btn" onclick="reconnectWiFi()">Переподключить WiFi</button>
            <button class="btn" onclick="scanSensors()">Сканировать датчики</button>
            <button class="btn" onclick="syncTime()">Синхронизировать время</button>
        </div>
        
        <div class="last-update">
            Обновлено: <span id="lastUpdate">%LAST_UPDATE%</span><br>
            Автообновление каждые <span id="autoUpdateRate">%POLLING_RATE_FORMATTED%</span>
        </div>
    </div>

    <script>
        let updateTimer;
        let timeTimer;
        let countdownTimer;
        let remainingTime = %LOG_REMAINING_TIME%;
        
        function formatTime(milliseconds) {
            const totalSeconds = Math.floor(milliseconds / 1000);
            const hours = Math.floor(totalSeconds / 3600);
            const minutes = Math.floor((totalSeconds % 3600) / 60);
            const seconds = totalSeconds % 60;
            
            if (hours > 0) {
                return String(hours).padStart(2, '0') + ":" + 
                       String(minutes).padStart(2, '0') + ":" + 
                       String(seconds).padStart(2, '0');
            } else if (minutes > 0) {
                return String(minutes).padStart(2, '0') + ":" + 
                       String(seconds).padStart(2, '0');
            } else {
                return seconds + " сек";
            }
        }
        
        function formatInterval(seconds) {
            if (seconds === 0) return '0 сек';
            if (seconds < 60) {
                return seconds + ' сек';
            } else if (seconds < 3600) {
                const minutes = Math.floor(seconds / 60);
                const secs = seconds % 60;
                return minutes + ' мин ' + (secs > 0 ? secs + ' сек' : '');
            } else if (seconds < 86400) {
                const hours = Math.floor(seconds / 3600);
                const minutes = Math.floor((seconds % 3600) / 60);
                return hours + ' час ' + (minutes > 0 ? minutes + ' мин' : '');
            } else {
                const days = Math.floor(seconds / 86400);
                const hours = Math.floor((seconds % 86400) / 3600);
                return days + ' дн ' + (hours > 0 ? hours + ' час' : '');
            }
        }
        
        function parseTimeToSeconds(timeStr) {
            if (!timeStr || timeStr === '00:00:00') return 0;
            
            const parts = timeStr.split(':');
            if (parts.length === 3) {
                const hours = parseInt(parts[0]) || 0;
                const minutes = parseInt(parts[1]) || 0;
                const seconds = parseInt(parts[2]) || 0;
                return hours * 3600 + minutes * 60 + seconds;
            }
            return 0;
        }
        
        function getTimeFromInputs(hoursId, minutesId, secondsId) {
            const hours = document.getElementById(hoursId).value.padStart(2, '0');
            const minutes = document.getElementById(minutesId).value.padStart(2, '0');
            const seconds = document.getElementById(secondsId).value.padStart(2, '0');
            return hours + ':' + minutes + ':' + seconds;
        }
        
        function updateCurrentTime() {
            const now = new Date();
            const timeStr = now.toLocaleTimeString('ru-RU');
            document.getElementById('currentTime').textContent = timeStr;
        }
        
        function updateCountdown() {
            if (remainingTime > 0) {
                remainingTime -= 1000;
                
                const countdownElement = document.getElementById('countdownTimer');
                if (countdownElement) {
                    countdownElement.textContent = formatTime(remainingTime);
                }
                
                if (remainingTime <= 0) {
                    if (countdownTimer) {
                        clearInterval(countdownTimer);
                        countdownTimer = null;
                    }
                    
                    setTimeout(() => {
                        location.reload();
                    }, 1000);
                }
            }
        }
        
        function startCountdown(initialTime) {
            remainingTime = parseInt(initialTime);
            
            if (countdownTimer) {
                clearInterval(countdownTimer);
            }
            
            if (remainingTime > 0) {
                const countdownElement = document.getElementById('countdownTimer');
                if (countdownElement) {
                    countdownElement.textContent = formatTime(remainingTime);
                    countdownElement.style.display = 'block';
                }
                
                countdownTimer = setInterval(updateCountdown, 1000);
            }
        }
        
        function updatePollingDisplay() {
            const hours = document.getElementById('pollingHours').value.padStart(2, '0');
            const minutes = document.getElementById('pollingMinutes').value.padStart(2, '0');
            const seconds = document.getElementById('pollingSeconds').value.padStart(2, '0');
            const timeStr = hours + ':' + minutes + ':' + seconds;
            const totalSeconds = parseTimeToSeconds(timeStr);
            
            if (document.getElementById('pollingValue')) {
                if (totalSeconds === 0) {
                    document.getElementById('pollingValue').textContent = '0 сек';
                } else {
                    document.getElementById('pollingValue').textContent = formatInterval(totalSeconds);
                }
            }
            if (document.getElementById('autoUpdateRate')) {
                if (totalSeconds === 0) {
                    document.getElementById('autoUpdateRate').textContent = '0 сек';
                } else {
                    document.getElementById('autoUpdateRate').textContent = formatInterval(totalSeconds);
                }
            }
        }
        
        function updateLogIntervalDisplay() {
            const hours = document.getElementById('logIntervalHours').value.padStart(2, '0');
            const minutes = document.getElementById('logIntervalMinutes').value.padStart(2, '0');
            const seconds = document.getElementById('logIntervalSeconds').value.padStart(2, '0');
            const timeStr = hours + ':' + minutes + ':' + seconds;
            const totalSeconds = parseTimeToSeconds(timeStr);
            
            if (document.getElementById('logIntervalValue')) {
                if (totalSeconds === 0) {
                    document.getElementById('logIntervalValue').textContent = '0 сек';
                } else {
                    document.getElementById('logIntervalValue').textContent = formatInterval(totalSeconds);
                }
            }
        }
        
        function updateLogDurationDisplay() {
            const hours = document.getElementById('logDurationHours').value.padStart(2, '0');
            const minutes = document.getElementById('logDurationMinutes').value.padStart(2, '0');
            const seconds = document.getElementById('logDurationSeconds').value.padStart(2, '0');
            const timeStr = hours + ':' + minutes + ':' + seconds;
            const totalSeconds = parseTimeToSeconds(timeStr);
            
            if (document.getElementById('logDurationValue')) {
                if (totalSeconds === 0) {
                    document.getElementById('logDurationValue').textContent = 'беск.';
                } else {
                    document.getElementById('logDurationValue').textContent = formatInterval(totalSeconds);
                }
            }
        }
        
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    for (let i = 0; i < 8; i++) {
                        const tempElement = document.getElementById('temp' + i);
                        const humElement = document.getElementById('hum' + i);
                        const statusElement = document.getElementById('status' + i);
                        const cardElement = document.getElementById('sensor' + i);
                        
                        if (data.sensors[i].present) {
                            if (tempElement) tempElement.textContent = data.sensors[i].temperature.toFixed(1);
                            if (humElement) humElement.textContent = data.sensors[i].humidity.toFixed(1);
                            if (statusElement) {
                                statusElement.textContent = '[OK] ВКЛ';
                                statusElement.className = 'sensor-status online';
                            }
                            if (cardElement) cardElement.classList.remove('sensor-offline');
                        } else {
                            if (tempElement) tempElement.textContent = '--.-';
                            if (humElement) humElement.textContent = '--.-';
                            if (statusElement) {
                                statusElement.textContent = '[X] ВЫКЛ';
                                statusElement.className = 'sensor-status offline';
                            }
                            if (cardElement) cardElement.classList.add('sensor-offline');
                        }
                    }
                    
                    if (document.getElementById('activeSensors')) 
                        document.getElementById('activeSensors').textContent = data.stats.activeSensors;
                    if (document.getElementById('avgTemp')) 
                        document.getElementById('avgTemp').textContent = data.stats.avgTemp.toFixed(1);
                    if (document.getElementById('avgHum')) 
                        document.getElementById('avgHum').textContent = data.stats.avgHum.toFixed(1);
                    if (document.getElementById('lastUpdate')) 
                        document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
                })
                .catch(error => {
                    console.error('Ошибка получения данных', error);
                });
        }
        
        function reconnectWiFi() {
            fetch('/reconnect')
                .then(() => {
                    alert('Переподключение WiFi...');
                    setTimeout(() => location.reload(), 3000);
                });
        }
        
        function scanSensors() {
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    alert('Сканирование завершено! Найдено датчиков: ' + data.found);
                    updateData();
                });
        }
        
        function syncTime() {
            fetch('/syncTime')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        alert('Время синхронизировано: ' + data.time);
                        location.reload();
                    } else {
                        alert('Ошибка синхронизации времени');
                    }
                });
        }
        
        function saveSettings() {
            const pollingStr = getTimeFromInputs('pollingHours', 'pollingMinutes', 'pollingSeconds');
            const pollingSec = parseTimeToSeconds(pollingStr);
            
            if (pollingSec < 1) {
                alert('Минимальный интервал опроса: 1 секунда');
                return;
            }
            
            fetch('/updateSettings?polling=' + pollingSec)
                .then(response => response.json())
                .then(data => {
                    alert('Настройки опроса сохранены!');
                    if (document.getElementById('pollingRate')) 
                        document.getElementById('pollingRate').textContent = formatInterval(pollingSec);
                    clearInterval(updateTimer);
                    updateTimer = setInterval(updateData, pollingSec * 1000);
                });
        }
        
        function toggleLogging() {
            const logIntervalStr = getTimeFromInputs('logIntervalHours', 'logIntervalMinutes', 'logIntervalSeconds');
            const logDurationStr = getTimeFromInputs('logDurationHours', 'logDurationMinutes', 'logDurationSeconds');
            
            const logIntervalSec = parseTimeToSeconds(logIntervalStr);
            const logDurationSec = parseTimeToSeconds(logDurationStr);
            
            if (logIntervalSec > 0 && logIntervalSec < 10) {
                alert('Минимальный интервал записи: 10 секунд');
                return;
            }
            
            const isLoggingActive = document.getElementById('logToggleBtn').textContent.includes('Завершить');
            
            if (!isLoggingActive) {
                fetch('/toggleLogging?interval=' + logIntervalSec + '&duration=' + logDurationSec)
                    .then(response => response.json())
                    .then(data => {
                        if (data.status === 'started') {
                            alert('Запись данных запущена!');
                            fetch('/getLogRemaining')
                                .then(response => response.json())
                                .then(logData => {
                                    if (logData.remaining > 0) {
                                        startCountdown(logData.remaining);
                                    }
                                    setTimeout(() => {
                                        location.reload();
                                    }, 500);
                                });
                        }
                    });
            } else {
                if (confirm('Завершить запись данных сейчас?')) {
                    fetch('/stopLogging')
                        .then(response => response.json())
                        .then(data => {
                            if (data.success) {
                                alert('Запись данных завершена!');
                                if (countdownTimer) {
                                    clearInterval(countdownTimer);
                                    countdownTimer = null;
                                }
                                setTimeout(() => {
                                    location.reload();
                                }, 500);
                            } else {
                                alert('Ошибка: ' + data.error);
                            }
                        });
                }
            }
        }
        
        function showLogFiles() {
            fetch('/listLogs')
                .then(response => response.json())
                .then(data => {
                    const container = document.getElementById('logFilesContainer');
                    const list = document.getElementById('logFilesList');
                    
                    if (data.files && data.files.length > 0) {
                        let html = '';
                        data.files.forEach(file => {
                            html += '<div class="log-file-item">';
                            html += '<span class="log-file-name">' + file.name + '</span>';
                            html += '<span class="log-file-size">' + file.size + ' байт</span>';
                            html += '<div>';
                            html += '<button class="download-btn" onclick="downloadLogFile(\'' + file.name + '\')">Скачать</button>';
                            html += '<button class="delete-btn" onclick="deleteLogFile(\'' + file.name + '\')">Удалить</button>';
                            html += '</div>';
                            html += '</div>';
                        });
                        if (list) list.innerHTML = html;
                        if (container) container.style.display = 'block';
                    } else {
                        if (list) list.innerHTML = '<div>Файлы данных не найдены</div>';
                        if (container) container.style.display = 'block';
                    }
                });
        }
        
        function deleteLogFile(filename) {
            if (confirm('Удалить файл ' + filename + '?')) {
                fetch('/deleteLog?file=' + encodeURIComponent(filename))
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            alert('Файл удален');
                            showLogFiles();
                        } else {
                            alert('Ошибка удаления файла: ' + (data.error || 'Неизвестная ошибка'));
                        }
                    })
                    .catch(error => {
                        alert('Ошибка при удалении файла');
                        console.error(error);
                    });
            }
        }
        
        function downloadLogFile(filename) {
            window.open('/downloadLogFile?file=' + encodeURIComponent(filename), '_blank');
        }
        
        function downloadCurrentLog() {
            fetch('/downloadCurrentLog')
                .then(response => {
                    if (response.ok) {
                        window.open('/downloadCurrentLog', '_blank');
                    } else {
                        alert('Нет текущего файла данных');
                    }
                });
        }
        
        function clearAllLogs() {
            if (confirm('ВНИМАНИЕ! Вы уверены, что хотите удалить ВСЕ файлы данных? Это действие нельзя отменить!')) {
                const clearBtn = document.querySelector('button[onclick="clearAllLogs()"]');
                const originalText = clearBtn.textContent;
                clearBtn.textContent = 'Очистка...';
                clearBtn.disabled = true;
                
                fetch('/clearLogs')
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            alert('Удалено ' + data.deleted + ' файлов');
                            if (data.deleted > 0) {
                                document.getElementById('logFilesContainer').style.display = 'none';
                                setTimeout(() => {
                                    location.reload();
                                }, 1000);
                            } else {
                                alert('Файлы данных не найдены');
                                clearBtn.textContent = originalText;
                                clearBtn.disabled = false;
                            }
                        } else {
                            alert('Ошибка при очистке файлов: ' + (data.error || 'Неизвестная ошибка'));
                            clearBtn.textContent = originalText;
                            clearBtn.disabled = false;
                        }
                    })
                    .catch(error => {
                        alert('Ошибка при очистке файлов');
                        console.error(error);
                        clearBtn.textContent = originalText;
                        clearBtn.disabled = false;
                    });
            }
        }
        
        document.addEventListener('DOMContentLoaded', function() {
            document.getElementById('pollingHours').addEventListener('input', updatePollingDisplay);
            document.getElementById('pollingMinutes').addEventListener('input', updatePollingDisplay);
            document.getElementById('pollingSeconds').addEventListener('input', updatePollingDisplay);
            
            document.getElementById('logIntervalHours').addEventListener('input', updateLogIntervalDisplay);
            document.getElementById('logIntervalMinutes').addEventListener('input', updateLogIntervalDisplay);
            document.getElementById('logIntervalSeconds').addEventListener('input', updateLogIntervalDisplay);
            
            document.getElementById('logDurationHours').addEventListener('input', updateLogDurationDisplay);
            document.getElementById('logDurationMinutes').addEventListener('input', updateLogDurationDisplay);
            document.getElementById('logDurationSeconds').addEventListener('input', updateLogDurationDisplay);
            
            updateCurrentTime();
            timeTimer = setInterval(updateCurrentTime, 1000);
            
            if (%LOG_REMAINING_TIME% > 0) {
                startCountdown(%LOG_REMAINING_TIME%);
            }
            
            updateData();
            const pollingStr = getTimeFromInputs('pollingHours', 'pollingMinutes', 'pollingSeconds');
            const pollingSec = parseTimeToSeconds(pollingStr);
            if (pollingSec > 0) {
                updateTimer = setInterval(updateData, pollingSec * 1000);
            }
        });
        
        window.addEventListener('beforeunload', function() {
            if (updateTimer) clearInterval(updateTimer);
            if (timeTimer) clearInterval(timeTimer);
            if (countdownTimer) clearInterval(countdownTimer);
        });
    </script>
</body>
</html>
)=====";

#endif