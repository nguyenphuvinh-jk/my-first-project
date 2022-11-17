
/*
  Pin  Function                       ESP-8266 Pin
  TX  TXD                             TXD
  RX  RXD                             RXD
  A0  Analog input, max 3.3V input    A0
  D0  IO                              GPIO16
  D1  IO, SCL                         GPIO5
  D2  IO, SDA                         GPIO4
  D3  IO, 10k Pull-up                 GPIO0
  D4  IO, 10k Pull-up, BUILTIN_LED    GPIO2
  D5  IO, SCK                         GPIO14
  D6  IO, MISO                        GPIO12
  D7  IO, MOSI                        GPIO13
  D8  IO, 10k Pull-down, SS           GPIO15
  G   Ground                          GND
  5V  5V                              -
  3V3 3.3V                            3.3V
  RST Reset                           RST

  GPIO0: pull low during boot for flash mode (connects to a push button).
  Other than that, usable as GPIO but beware of waht you connect to it.
  GPIO1, 3: TX and RX (Serial). Also usable as GPIO if not using serial.
  GPIO2: is pulled up during boot & internal LED (active LOW).
  GPIO15: fixed external pull-down (for boot).
  GPIO4, 5, 12-14, 16: nothing special.
  Conclusion
  Stick to A0, D0, D5, D6 and D7.
*/

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 webserver</title>
    <style>
        /* The switch - the box around the slider */
        .switch {
        position: relative;
        display: inline-block;
        width: 60px;
        height: 34px;
        }

        /* Hide default HTML checkbox */
        .switch input {
        opacity: 0;
        width: 0;
        height: 0;
        }

        /* The slider */
        .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: .4s;
        transition: .4s;
        }

        .slider:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: white;
        -webkit-transition: .4s;
        transition: .4s;
        }

        input:checked + .slider {
        background-color: #2196F3;
        }

        input:focus + .slider {
        box-shadow: 0 0 1px #2196F3;
        }

        input:checked + .slider:before {
        -webkit-transform: translateX(26px);
        -ms-transform: translateX(26px);
        transform: translateX(26px);
        }

        /* Rounded sliders */
        .slider.round {
        border-radius: 34px;
        }

        .slider.round:before {
        border-radius: 50%;
        }
        .info {
            font-size: 1.4rem;
            margin-bottom: 10px;
        }
        .wrapper {
            width: fit-content;
            margin: 0 auto;
        }
    </style>
</head>
<body>
    <h1 style="text-align: center;">Nhóm 4</h1>
    <section id="infos">
        <div class="wrapper">
            <div class="info" id="moisture">Độ ẩm đất:
                <span class="info" id="mois-data">%</span>
            </div>
            <div class="info" id="mode">
                <span>Chế độ hoạt động:</span>
                <span class="info" id="mode-data">Off</span>
            </div>
            <label class="switch">
                <input id="btn-toggle" type="checkbox" onchange="btnToggle(this)">
                <span class="slider round"></span>
              </label>
        </div>
    </section>
</body>
<script>
    const demo = document.querySelector("#demo");
    const btnToggle = document.querySelector("#btn-toggle");
    const mode = document.querySelector("#mode-data");
    const moisture = document.querySelector("#mois-data");
    btnToggle.addEventListener('change', function(){
        let xhr = new XMLHttpRequest();
        if (this.checked) {
            mode.innerHTML = "On";
            xhr.open("GET", "/update?mode=on", true)
        }
        else {
            mode.innerHTML = "Off";
            xhr.open("GET", "/update?mode=off", true)
        }
        xhr.send();
    });
    setInterval(function() {
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
            document.getElementById("mois-data").innerHTML = this.responseText + "%";
            }
        };
        xhr.open("GET", "/moisture", true);
        xhr.send();
        }, 1000 ) ;
</script>
</html>
)rawliteral";
// end of html

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "ESP-Nhom-4";
const char* password = "12345678";

AsyncWebServer server(80); // port 80

int mois;
const long interval = 1000; // thoi gian update 
unsigned long previousMillis = 0;
String data = "";
bool isFirst = true;
String modeData;
void setup() {
  Serial.begin(9600);
  Wire.begin(D1, D2);
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
   
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", String(mois).c_str());
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      modeData = request->getParam("mode")->value();
    }
    request->send(200, "text/plain", "OK");
  });
   
   // start server
   server.begin();
}

// lay du lieu tu arduino (mois)
void requestData() {
  Wire.requestFrom(8, 10);
  while (Wire.available()) {
    char c = Wire.read();
    // kiem tra c co phai la ky tu khong
    if (c == 255) {
      break;
    }
    data = data + c;
  }
  mois = data.toInt();
}
// lay du lieu tu arduino cho lan dau tien
void loadData() {
  if (isFirst) {
    requestData();
    isFirst = false;
  }
}
// gui du lieu mode sang arduino
void transData() {
  Wire.beginTransmission(8);
  Wire.write(modeData.c_str());
  Wire.endTransmission();
  Serial.print("Mode = ");
  Serial.println(modeData);
}

void loop() {
  // nap du lieu cho lan chay dau tien de tranh delay o phan sau
  loadData();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    data = "";
    requestData(); // lay du lieu do am tu arduino
    transData(); // gui du lieu mode (on/off) sang arduino
  }
}
