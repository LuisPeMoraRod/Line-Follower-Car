// Import required libraries
#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    true

// Set number of relays
#define NUM_SWITCHES  4

// Assign each GPIO to a relay
// int relayGPIOs[NUM_SWITCHES] = {5, 4, 14, 12, 13};


int switches[NUM_SWITCHES] = {0, 0, 0, 0};

const int pinSensorL = 2;
const int pinSensorR = 0;

int sensorL;
int sensorR;

const int RIGHT = 1;
const int LEFT = 0;
int lastDir = 1; // variable to control the last direction

const int motorL_A = 12;
const int motorL_B = 14;

const int motorR_A = 4;
const int motorR_B = 5;

byte speed = 255;

// Replace with your network credentials
const char* ssid = "Tatooine"; // "REPLACE_WITH_YOUR_SSID";
const char* password = "HanSoloRIP"; // "REPLACE_WITH_YOUR_PASSWORD";

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_SWITCHES; i++){
      buttons+= "<h4>Switch #" + String(i) + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(pinSensorL, INPUT);
  pinMode(pinSensorR, INPUT);
  
  pinMode(motorL_A, OUTPUT);
  pinMode(motorL_B, OUTPUT);
  pinMode(motorR_A, OUTPUT);
  pinMode(motorR_B, OUTPUT);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;

      //update switch
      switches[inputMessage.toInt()-1] = inputMessage2.toInt();
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
}
  
void loop() {
  //Serial.println(String(switches[0]) + String(switches[1]) + String(switches[2]) + String(switches[3]));

  sensorL = digitalRead(pinSensorL);
  sensorR = digitalRead(pinSensorR);

  if ((switches[0] == 1) && (switches[1] == 1) && (switches[2] == 1) && (switches[3] == 1)){
    stand_by();
  }
  else if ((switches[0] == 0) && (switches[1] == 0) && (switches[2] == 1) && (switches[3] == 1)){
    line_follow(sensorL, sensorR);
  }else{
    stand_by();
  }
   
}

void stand_by(){
  digitalWrite(motorL_A, LOW);
  digitalWrite(motorL_B, LOW);
  digitalWrite(motorR_A, LOW);
  digitalWrite(motorR_B, LOW);
}

void move_forward(){
  digitalWrite(motorL_A, HIGH);
  digitalWrite(motorL_B, LOW);
  digitalWrite(motorR_A, HIGH);
  digitalWrite(motorR_B, LOW);
}

void move_left(){
  digitalWrite(motorL_A, LOW);
  digitalWrite(motorL_B, LOW);
  digitalWrite(motorR_A, HIGH);
  digitalWrite(motorR_B, LOW);
}

void move_right(){
  digitalWrite(motorL_A, HIGH);
  digitalWrite(motorL_B, LOW);
  digitalWrite(motorR_A, LOW);
  digitalWrite(motorR_B, LOW);
}

void line_follow(int sensorL, int sensorR){
  
  Serial.println(String(sensorL)+String(sensorR));
  
  if ((sensorL == 0) and (sensorR == 0)){
    if (lastDir == RIGHT){
      move_right();
    } else {
      move_left();
    }
  } else if ((sensorL == 0) and (sensorR == 1)){
    lastDir = RIGHT;
    move_right();
  }else if ((sensorL == 1) and (sensorR == 0)){
    lastDir = LEFT;
    move_left();
  }else if ((sensorL == 1) and (sensorR == 1)){
    move_forward();
  }

}
