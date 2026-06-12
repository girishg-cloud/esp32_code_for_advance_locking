#include <WiFi.h>
#include <WebServer.h>
#include "esp_wpa2.h"


const char* ssid1 = "IITR_WIFI";
const char* ssid2 = "IITR_WIFI_HIGHSPEED";
// Your credentials (same for both)
#define EAP_IDENTITY "enrollment no" //please enter your enrollment no before running the code
#define EAP_USERNAME "enrollment no" //please enter your enrollment no before running the code
#define EAP_PASSWORD "enrollment no" //please enter your wifi password before running the code

// Motor Pins
#define ENA 12
#define IN1 14
#define IN2 27

WebServer server(80);

// ===== HTML PAGE =====
String webpage = R"====(
<!DOCTYPE html>
<html>
<head>
<title>Lock Function</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  height: 100vh;
  display: flex;
  justify-content: center;
  align-items: center;
  font-family: 'Segoe UI', sans-serif;
  background: linear-gradient(135deg, #0f172a, #1e293b);
  color: white;
}

.container {
  text-align: center;
  padding: 30px;
  border-radius: 20px;
  background: rgba(255, 255, 255, 0.05);
  backdrop-filter: blur(15px);
  box-shadow: 0 8px 30px rgba(0,0,0,0.5);
  width: 90%;
  max-width: 350px;
}

h1 {
  margin-bottom: 25px;
  font-size: 28px;
  letter-spacing: 1px;
}

button {
  width: 80%;
  padding: 15px;
  margin: 10px;
  font-size: 16px;
  border: none;
  border-radius: 12px;
  cursor: pointer;
  transition: 0.3s;
}

button:hover {
  transform: scale(1.05);
  opacity: 0.9;
}

.cw {
  background: linear-gradient(45deg, #22c55e, #16a34a);
}

.ccw {
  background: linear-gradient(45deg, #3b82f6, #2563eb);
}

.stop {
  background: linear-gradient(45deg, #ef4444, #dc2626);
}

.status {
  margin-top: 20px;
  font-size: 18px;
  padding: 10px;
  border-radius: 10px;
  background: rgba(255,255,255,0.1);
}
</style>
</head>

<body>

<div class="container">
  <h1>Lock Function</h1>

  <button class="cw" onclick="rotateCW()"> Open</button>
  <button class="ccw" onclick="rotateCCW()"> Close</button>
  <button class="stop" onclick="stopMotor()"> Stop</button>

  <div></div>
</div>

<script>
function rotateCW() {
  fetch('/cw');
  document.getElementById('status').innerText = "Status: Open";
}

function rotateCCW() {
  fetch('/ccw');
  document.getElementById('status').innerText = "Status: Closed";
}

function stopMotor() {
  fetch('/stop');
  document.getElementById('status').innerText = "Status: Stopped";
}
</script>

</body>
</html>
)====";

// ===== MOTOR FUNCTIONS =====
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, LOW);
}

void rotateCW() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(ENA, HIGH);
  delay(2000);
  stopMotor();
}

void rotateCCW() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(ENA, HIGH);
  delay(2000);
  stopMotor();
}

// ===== WEB HANDLERS =====
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleCW() {
  rotateCW();
  server.send(200, "text/plain", "CW Done");
}

void handleCCW() {
  rotateCCW();
  server.send(200, "text/plain", "CCW Done");
}

void handleStop() {
  stopMotor();
  server.send(200, "text/plain", "Stopped");
}

// ===== AUTO WIFI CONNECT =====
void connectEnterpriseWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  Serial.println("Scanning networks...");
  int n = WiFi.scanNetworks();

  String selectedSSID = "";

  for (int i = 0; i < n; i++) {
    String foundSSID = WiFi.SSID(i);
    Serial.println(foundSSID);

    if (foundSSID == ssid1) {
      selectedSSID = ssid1;
      break;
    }
    if (foundSSID == ssid2) {
      selectedSSID = ssid2;
      break;
    }
  }

  if (selectedSSID == "") {
    Serial.println("No known college WiFi found!");
    return;
  }

  Serial.print("Connecting to: ");
  Serial.println(selectedSSID);

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));

  esp_wifi_sta_wpa2_ent_enable();

  WiFi.begin(selectedSSID.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  stopMotor();

  connectEnterpriseWiFi();

  server.on("/", handleRoot);
  server.on("/cw", handleCW);
  server.on("/ccw", handleCCW);
  server.on("/stop", handleStop);

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}