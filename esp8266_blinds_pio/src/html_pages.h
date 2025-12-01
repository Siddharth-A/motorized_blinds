const char WIFI_SETUP[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Configuration</title>
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
        }

        .container {
            background-color: #2d2d2d;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.5);
            width: 100%;
            max-width: 400px;
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

        .form-container {
            padding: 30px;
            background-color: #2d2d2d;
        }

        .form-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            color: #e0e0e0;
            font-weight: 500;
        }

        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 12px;
            border: 1px solid #444;
            border-radius: 4px;
            font-size: 16px;
            background-color: #1a1a1a;
            color: #e0e0e0;
            transition: border-color 0.3s;
        }

        input[type="text"]:focus,
        input[type="password"]:focus {
            outline: none;
            border-color: #b794f6;
        }

        .submit-button {
            width: 100%;
            padding: 12px;
            background-color: #9b7fb8;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            transition: background-color 0.3s;
        }

        .submit-button:hover {
            background-color: #b794f6;
        }

        .submit-button:active {
            background-color: #8b6fa8;
        }

        .clear-button {
            width: 100%;
            padding: 12px;
            background-color: #d32f2f;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 16px;
            font-weight: bold;
            cursor: pointer;
            transition: background-color 0.3s;
            margin-top: 10px;
        }

        .clear-button:hover {
            background-color: #f44336;
        }

        .clear-button:active {
            background-color: #b71c1c;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="title-bar">
            WiFi Configuration
        </div>
        <div class="form-container">
            <form id="wifiForm" method="POST" action="/wifi-config">
                <div class="form-group">
                    <label for="wifiName">WiFi Name (SSID):</label>
                    <input type="text" id="wifiName" name="ssid" required>
                </div>

                <div class="form-group">
                    <label for="wifiPassword">Password:</label>
                    <input type="password" id="wifiPassword" name="password" required>
                </div>

                <button type="submit" class="submit-button">Submit</button>
            </form>
            <button type="button" class="clear-button" id="clearButton">Clear Settings</button>
        </div>
    </div>

    <script type="text/javascript">
        // Function to send WiFi credentials via XMLHttpRequest
        // function sendWiFiData(ssid, password) {
        //     var xhttp = new XMLHttpRequest();

        //     xhttp.onreadystatechange = function() {
        //         // Function call when the ready state changes
        //         if (this.readyState == 4 && this.status == 200) {
        //             // Handle successful response
        //             alert("WiFi credentials submitted successfully!");
        //             // Optionally update page content with response
        //             // document.getElementById("status").innerHTML = this.responseText;
        //         } else if (this.readyState == 4 && this.status != 200) {
        //             // Handle error
        //             alert("Error submitting WiFi credentials. Status: " + this.status);
        //         }
        //     };

        //     // Send as GET request with query parameters
        //     xhttp.open("GET", "/wifi-config?ssid=" + encodeURIComponent(ssid) + "&password=" + encodeURIComponent(password), true);
        //     xhttp.send();
        // }

        // Function to send WiFi credentials via POST (alternative method)
        function sendWiFiDataPOST(ssid, password) {
            var xhttp = new XMLHttpRequest();

            xhttp.onreadystatechange = function() {
                // Function call when the ready state changes
                if (this.readyState == 4 && this.status == 200) {
                    // Handle successful response
                    alert("WiFi credentials submitted successfully!");
                    // Optionally update page content with response
                    // document.getElementById("status").innerHTML = this.responseText;
                } else if (this.readyState == 4 && this.status != 200) {
                    // Handle error
                    alert("Error submitting WiFi credentials. Status: " + this.status);
                }
            };

            // Send as POST request
            xhttp.open("POST", "/wifi-config", true);
            xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xhttp.send("ssid=" + encodeURIComponent(ssid) + "&password=" + encodeURIComponent(password));
        }

        // Function to clear EEPROM settings
        function clearEEPROM() {
            if (confirm("Are you sure you want to clear all saved WiFi settings? This action cannot be undone.")) {
                var xhttp = new XMLHttpRequest();

                xhttp.onreadystatechange = function() {
                    // Function call when the ready state changes
                    if (this.readyState == 4 && this.status == 200) {
                        // Handle successful response
                        alert("EEPROM cleared successfully! All WiFi settings have been erased.");
                        console.log("EEPROM cleared: " + this.responseText);
                    } else if (this.readyState == 4 && this.status != 200) {
                        // Handle error
                        alert("Error clearing EEPROM. Status: " + this.status);
                        console.error("Error clearing EEPROM: " + this.status);
                    }
                };

                // Send GET request to clear EEPROM
                xhttp.open("GET", "/clear-eeprom", true);
                xhttp.send();
            }
        }

        // Handle form submission
        document.getElementById("wifiForm").addEventListener("submit", function(event) {
            event.preventDefault(); // Prevent default form submission

            var ssid = document.getElementById("wifiName").value;
            var password = document.getElementById("wifiPassword").value;

            // Print SSID and password to console
            console.log("SSID: " + ssid);
            console.log("Password: " + password);

            if (ssid && password) {
                // Use GET method (similar to the reference script)
                // sendWiFiData(ssid, password);

                // Alternative: Use POST method (uncomment to use instead)
                sendWiFiDataPOST(ssid, password);
            } else {
                alert("Please enter both WiFi name and password.");
            }
        });

        // Handle clear button click
        document.getElementById("clearButton").addEventListener("click", function() {
            clearEEPROM();
        });
    </script>
</body>
</html>
)=====";

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