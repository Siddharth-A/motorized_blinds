const char WIFI_HOME[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Motorized Blinds - Home</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background-color: #1a1a1a;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            background-color: #2d2d2d;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.5);
            width: 100%;
            max-width: 500px;
            padding: 0;
        }

        .title-bar {
            background-color: #9b7fb8;
            color: white;
            padding: 20px;
            text-align: center;
            border-radius: 8px 8px 0 0;
            font-size: 24px;
            font-weight: bold;
        }

        .content {
            padding: 30px;
            background-color: #2d2d2d;
        }

        .status-section {
            margin-bottom: 25px;
        }

        .status-item {
            display: flex;
            justify-content: space-between;
            padding: 12px 0;
            border-bottom: 1px solid #444;
            color: #e0e0e0;
        }

        .status-item:last-child {
            border-bottom: none;
        }

        .status-label {
            font-weight: 500;
            color: #b0b0b0;
        }

        .status-value {
            color: #e0e0e0;
            font-weight: bold;
        }

        .status-value.connected {
            color: #4caf50;
        }

        .info-section {
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #444;
        }

        .info-text {
            color: #b0b0b0;
            font-size: 14px;
            line-height: 1.6;
            text-align: center;
        }

        .icon {
            display: inline-block;
            margin-right: 8px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="title-bar">
            Motorized Blinds
        </div>
        <div class="content">
            <div class="status-section">
                <div class="status-item">
                    <span class="status-label">WiFi Status</span>
                    <span class="status-value connected">Connected</span>
                </div>
                <div class="status-item">
                    <span class="status-label">Device IP</span>
                    <span class="status-value" id="deviceIP">Loading...</span>
                </div>
                <div class="status-item">
                    <span class="status-label">SSID</span>
                    <span class="status-value" id="ssid">Loading...</span>
                </div>
            </div>
            <div class="info-section">
                <p class="info-text">
                    Device is online and ready to use.
                </p>
            </div>
        </div>
    </div>

    <script type="text/javascript">
        // Get device IP from window location
        window.onload = function() {
            var hostname = window.location.hostname;
            document.getElementById("deviceIP").textContent = hostname || "Unknown";
            
            // Try to get SSID from a status endpoint (if available)
            // For now, just show a placeholder
            document.getElementById("ssid").textContent = "Connected";
        };
    </script>
</body>
</html>
)=====";