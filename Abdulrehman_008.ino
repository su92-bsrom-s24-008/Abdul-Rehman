
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "RGB-TEST";
const char* password = "12345678";

WebServer server(80);
bool robotRunning = false;

void setRobotState(bool running) {
  robotRunning = running;

  if (running) {
    led.setPixelColor(0, led.Color(0, 255, 0));
    Serial.println("ROBOT OPERATIONAL - GREEN LED");
  } else {
    led.setPixelColor(0, led.Color(255, 0, 0));
    Serial.println("SAFETY HALT - RED LED");
  }

  led.show();
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Abdulrehman | Robot Control</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  min-height: 100vh;
  background: #080d18;
  background-image:
    radial-gradient(circle at 50% 0%, #173f69 0%, transparent 55%);
  color: #ffffff;
  font-family: Arial, sans-serif;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 20px;
}

.card {
  width: 100%;
  max-width: 470px;
  padding: 35px 25px;
  text-align: center;
  background: #111c2e;
  border: 1px solid #284767;
  border-radius: 22px;
  box-shadow: 0 20px 60px #0008;
}

.tag {
  display: inline-block;
  background: #123554;
  color: #60caff;
  border: 1px solid #285e86;
  border-radius: 30px;
  padding: 9px 16px;
  font-size: 12px;
  font-weight: bold;
  letter-spacing: 2px;
}

h1 {
  font-size: 28px;
  margin: 22px 0 10px;
  letter-spacing: 1px;
}

.subtitle {
  color: #91a8c2;
  font-size: 14px;
}

.student {
  margin-top: 25px;
  padding: 18px;
  background: #182b43;
  border: 1px solid #2d4b69;
  border-radius: 12px;
  font-size: 22px;
  font-weight: bold;
  color: #6ed0ff;
  letter-spacing: 1px;
}

.student-label {
  display: block;
  font-size: 11px;
  font-weight: normal;
  color: #91a8c2;
  margin-bottom: 9px;
  letter-spacing: 2px;
}

.status-box {
  margin: 25px 0;
  padding: 23px 12px;
  background: #0a1424;
  border: 1px solid #263d58;
  border-radius: 14px;
}

.status-label {
  font-size: 12px;
  color: #91a8c2;
  letter-spacing: 2px;
  margin-bottom: 15px;
}

.status {
  font-size: 19px;
  font-weight: bold;
}

.green {
  color: #4ade80;
}

.red {
  color: #fb7185;
}

button {
  display: block;
  width: 100%;
  border: none;
  padding: 18px;
  margin: 14px 0;
  border-radius: 11px;
  font-size: 16px;
  font-weight: bold;
  color: white;
  cursor: pointer;
  transition: 0.2s;
}

.start {
  background: linear-gradient(90deg, #075985, #0284c7);
  box-shadow: 0 5px 20px #0284c733;
}

.stop {
  background: #27364b;
  border: 1px solid #52657c;
}

button:active {
  transform: scale(0.97);
}

#message {
  color: #7dd3fc;
  font-size: 13px;
  min-height: 20px;
}

.footer {
  margin-top: 25px;
  padding-top: 20px;
  border-top: 1px solid #2a4059;
  color: #8198b3;
  font-size: 12px;
}

</style>
</head>

<body>
<div class="card">

<div class="tag">
ESP32-S3 SYSTEM
</div>

<h1>ROBOT CONTROL</h1>

<p class="subtitle">
Wireless Robot Monitoring & Control
</p>

<div class="student">
<span class="student-label">DEVELOPED BY</span>
ABDULREHMAN
</div>

<div class="status-box">
  <div class="status-label">LIVE SYSTEM STATUS</div>
  <div id="status" class="status">Loading...</div>
</div>

<button class="start" onclick="control(1)">
START ROBOT
</button>

<button class="stop" onclick="control(0)">
SAFETY HALT
</button>

<p id="message"></p>

<div class="footer">
ESP32-S3 | Local Web Server | Port 80
</div>

</div>

<script>
async function refreshStatus() {
  try {
    const response = await fetch('/robot/status');

    if (!response.ok) {
      throw new Error('Server Error');
    }

    const data = await response.json();
    const status = document.getElementById('status');

    if (data.running) {
      status.textContent = '● OPERATIONAL - GREEN LED';
      status.className = 'status green';
    } else {
      status.textContent = '● SAFETY HALT - RED LED';
      status.className = 'status red';
    }

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

async function control(state) {
  try {
    const response = await fetch(
      '/robot/status?state=' + state
    );

    if (!response.ok) {
      throw new Error('Command Failed');
    }

    document.getElementById('message').textContent =
      'Command Successful';

    await refreshStatus();

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

refreshStatus();
setInterval(refreshStatus, 2000);
</script>

</body>
</html>
)rawliteral";

void handleRobotStatus() {
  if (server.hasArg("state")) {
    String state = server.arg("state");

    if (state == "1") {
      setRobotState(true);
    } else if (state == "0") {
      setRobotState(false);
    } else {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid state\"}"
      );
      return;
    }
  }

  String response = "{\"running\":";
  response += robotRunning ? "true" : "false";
  response += "}";

  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);

  led.begin();
  led.setBrightness(50);
  setRobotState(false);

  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    ssid,
    password,
    1,
    0,
    4
  );

  if (!started) {
    Serial.println("WiFi Access Point Failed");
    return;
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 ROBOT CONTROL");
  Serial.println("Name: Abdulrehman");
  Serial.println("================================");

  Serial.println("WiFi Access Point Started");

  Serial.print("Network Name: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on(
    "/robot/status",
    HTTP_GET,
    handleRobotStatus
  );

  server.onNotFound([]() {
    server.send(
      404,
      "text/plain",
      "Page Not Found"
    );
  });

  server.begin();

  Serial.println("HTTP SERVER STARTED ON PORT 80");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
