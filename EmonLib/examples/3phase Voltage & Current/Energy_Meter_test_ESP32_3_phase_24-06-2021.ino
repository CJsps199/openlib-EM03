// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>
#include <AutoConnect.h>
#include <WebServer.h>
#include "EmonLibCJ.h" // Include Emon Library
EnergyMonitor3ph emon1;             // Create an instance

//const char* ssid = "trompies back";
//const char* password = "trompies83";
const char* mqtt_server = "192.168.0.166";
//const int ledPin         = 2;
float temperature        = 0;
float humidity           = 0;
float kwh_l1             = 0;
float kwh_l2             = 0;
float kwh_l3             = 0;
float kwh_last_l1        = 0;
float kwh_last_l2        = 0;
float kwh_last_l3        = 0;
float kwh_now_l1         = 0;
float kwh_now_l2         = 0;
float kwh_now_l3         = 0;
float realPower_l1       = 0;
float realPower_l2       = 0;
float realPower_l3       = 0;//extract Real Power into variable
float apparentPower_l1   = 0;
float apparentPower_l2   = 0;
float apparentPower_l3   = 0;//extract Apparent Power into variable
float powerFactor_l1     = 0;
float powerFactor_l2     = 0;
float powerFactor_l3     = 0;//extract Power Factor into Variable
float supplyVoltage_l1   = 0;
float supplyVoltage_l2   = 0;
float supplyVoltage_l3   = 0;//extract Vrms into Variable
float Irms_l1            = 0;
float Irms_l2            = 0;
float Irms_l3            = 0;
float peakLoad_l1        = 0;
float peakLoad_l2        = 0;
float peakLoad_l3        = 0;
float peakCur_l1         = 0;
float peakCur_l2         = 0;
float peakCur_l3         = 0;
byte number              = 0;
byte reset_counter       = 0;
#define DHTpin 16
#define DHTtype DHT11

DHT dht(DHTpin, DHTtype);

WiFiClient espClient;

WebServer Server;

AutoConnect       Portal(Server);
AutoConnectConfig Config;       // Enable autoReconnect supported on v0.9.4
AutoConnectAux    Timezone;

PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//void callback(char* topic, byte* message, unsigned int length) {
//  Serial.print("Message arrived on topic: ");
//  Serial.print(topic);
//  Serial.print(". Message: ");
//  String messageTemp;
//  
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)message[i]);
//    messageTemp += (char)message[i];
//  }
//  Serial.println();
//
//  // Feel free to add more if statements to control more GPIOs with MQTT
//
//  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
//  // Changes the output state according to the message
//  if (String(topic) == "spsem/output") {
//    Serial.print("Changing output to ");
//    if(messageTemp == "on"){
//      Serial.println("on");
//      digitalWrite(ledPin, HIGH);
//    }
//    else if(messageTemp == "off"){
//      Serial.println("off");
//      digitalWrite(ledPin, LOW);
//    }
//  }
//}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("SPSEM03Client")) {
      Serial.println("connected");
      // Subscribe
//      client.subscribe("spsem/output");
    } else {
      Serial.println("failed, connection...");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      reset_counter ++;
      if (reset_counter >= 2){
        Serial.println("ERROR CONNECTING...");
        Serial.println("RESTARTING NOW.");
        delay(250);
        ESP.restart();
      }
    }
  }
}

void energy_measure(){

  emon1.calcVI_l1(50,2000);
//  emon1.calcIrms_l1(25);// Calculate all. No.of half wavelengths (crossings), time-out
//  emon1.serialprint_l1();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

  realPower_l1       = emon1.realPower_l1;        //extract Real Power into variable
  apparentPower_l1   = emon1.apparentPower_l1;    //extract Apparent Power into variable
  powerFactor_l1     = emon1.powerFactor_l1;      //extract Power Factor into Variable
  supplyVoltage_l1   = emon1.Vrms_l1;             //extract Vrms into Variable
  Irms_l1            = (realPower_l1/supplyVoltage_l1);

  kwh_now_l1 = (realPower_l1/1800)/1000;
  kwh_l1 = kwh_now_l1 + kwh_last_l1;
  kwh_last_l1 = kwh_l1;

  peakCur_l1 = realPower_l1;

  emon1.calcVI_l2(50,2000);
//  emon1.calcIrms_l2(25);// Calculate all. No.of half wavelengths (crossings), time-out
//  emon1.serialprint_l2();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)

  realPower_l2       = emon1.realPower_l2;        //extract Real Power into variable
  apparentPower_l2   = emon1.apparentPower_l2;    //extract Apparent Power into variable
  powerFactor_l2     = emon1.powerFactor_l2;      //extract Power Factor into Variable
  supplyVoltage_l2   = emon1.Vrms_l2;             //extract Vrms into Variable
  Irms_l2            = (realPower_l2/supplyVoltage_l2);

  kwh_now_l2 = (realPower_l2/1800)/1000;
  kwh_l2 = kwh_now_l2 + kwh_last_l2;
  kwh_last_l2 = kwh_l2;

  peakCur_l2 = realPower_l2;

  emon1.calcVI_l3(50,2000);
//  emon1.calcIrms_l3(25);// Calculate all. No.of half wavelengths (crossings), time-out
//  emon1.serialprint_l3();           // Print out all variables (realpower, apparent power, Vrms, Irms, power factor)
  
  realPower_l3       = emon1.realPower_l3;        //extract Real Power into variable
  apparentPower_l3   = emon1.apparentPower_l3;    //extract Apparent Power into variable
  powerFactor_l3     = emon1.powerFactor_l3;      //extract Power Factor into Variable
  supplyVoltage_l3   = emon1.Vrms_l3;             //extract Vrms into Variable
  Irms_l3            = (realPower_l3/supplyVoltage_l3);             //extract Irms into Variable

  kwh_now_l3 = (realPower_l3/1800)/1000;
  kwh_l3 = kwh_now_l3 + kwh_last_l3;
  kwh_last_l3 = kwh_l3;

  peakCur_l3 = realPower_l3;

  if (number >= 10){
    if (peakCur_l1 > peakLoad_l1){
    
    peakLoad_l1 = peakCur_l1;
    }

    if (peakCur_l2 > peakLoad_l2){
    
    peakLoad_l2 = peakCur_l2;
    }

    if (peakCur_l3 > peakLoad_l3){
    
    peakLoad_l3 = peakCur_l3;
    }
  }

//  Serial.println(Irms);
  number++;
}

static const char AUX_TIMEZONE[] PROGMEM = R"(
{
  "title": "TimeZone",
  "uri": "/timezone",
  "menu": true,
  "element": [
    {
      "name": "caption",
      "type": "ACText",
      "value": "Sets the time zone to get the current local time.",
      "style": "font-family:Arial;font-weight:bold;text-align:center;margin-bottom:10px;color:DarkSlateBlue"
    },
    {
      "name": "timezone",
      "type": "ACSelect",
      "label": "Select TZ name",
      "option": [],
      "selected": 3
    },
    {
      "name": "newline",
      "type": "ACElement",
      "value": "<br>"
    },
    {
      "name": "start",
      "type": "ACSubmit",
      "value": "OK",
      "uri": "/start"
    }
  ]
}
)";

typedef struct {
  const char* zone;
  const char* ntpServer;
  int8_t      tzoff;
} Timezone_t;

static const Timezone_t TZ[] = {
  { "Europe/London", "europe.pool.ntp.org", 0 },
  { "Europe/Berlin", "europe.pool.ntp.org", 1 },
  { "SouthAfrica/Johannesburg", "za.pool.ntp.org", +2 },
  { "Europe/Helsinki", "europe.pool.ntp.org", 2 },
  { "Europe/Moscow", "europe.pool.ntp.org", 3 },
  { "Asia/Dubai", "asia.pool.ntp.org", 4 },
  { "Asia/Karachi", "asia.pool.ntp.org", 5 },
  { "Asia/Dhaka", "asia.pool.ntp.org", 6 },
  { "Asia/Jakarta", "asia.pool.ntp.org", 7 },
  { "Asia/Manila", "asia.pool.ntp.org", 8 },
  { "Asia/Tokyo", "asia.pool.ntp.org", 9 },
  { "Australia/Brisbane", "oceania.pool.ntp.org", 10 },
  { "Pacific/Noumea", "oceania.pool.ntp.org", 11 },
  { "Pacific/Auckland", "oceania.pool.ntp.org", 12 },
  { "Atlantic/Azores", "europe.pool.ntp.org", -1 },
  { "America/Noronha", "south-america.pool.ntp.org", -2 },
  { "America/Araguaina", "south-america.pool.ntp.org", -3 },
  { "America/Blanc-Sablon", "north-america.pool.ntp.org", -4},
  { "America/New_York", "north-america.pool.ntp.org", -5 },
  { "America/Chicago", "north-america.pool.ntp.org", -6 },
  { "America/Denver", "north-america.pool.ntp.org", -7 },
  { "America/Los_Angeles", "north-america.pool.ntp.org", -8 },
  { "America/Anchorage", "north-america.pool.ntp.org", -9 },
  { "Pacific/Honolulu", "north-america.pool.ntp.org", -10 },
  { "Pacific/Samoa", "oceania.pool.ntp.org", -11 }
};

void rootPage() {
  String  content =
    "<html>"
    "<head>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<script type=\"text/javascript\">"
    "setTimeout(\"location.reload()\", 9000);"
    "</script>"
    "</head>"
    "<body>"
    "<h2 align=\"center\" style=\"color:black;margin:40px;\"><b>Energy Meter EM03</b></h2>"
    "<h3 align=\"center\" style=\"color:green;margin:40px;\">SPS EM-03</h3>"
    "<h2 align=\"center\" style=\"color:black;margin:20px;\">{{DateTime}}</h2>"
    "<p style=\"text-align:center;\">Click on Settings to continue to setup:</p>"
    "<p></p><p style=\"padding-top:30px;text-align:center\">" AUTOCONNECT_LINK(COG_32) "</p>"
    "<h5 align=\"center\" style=\"color:black;margin:20px;\">Smart Power Solutions | CJ De Beer</h5>"
    "<h6 align=\"center\" style=\"color:blue;margin:10px;\">Email: sps0b101@gmail.com | Cell: 061 533 4455</h6>"
    "</body>"
    "</html>";
  static const char *wd[7] = { "Sun","Mon","Tue","Wed","Thr","Fri","Sat" };
  struct tm *tm;
  time_t  t;
  char    dateTime[26];

  t = time(NULL);
  tm = localtime(&t);
  sprintf(dateTime, "%04d/%02d/%02d(%s) %02d:%02d:%02d.",
    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
    wd[tm->tm_wday],
    tm->tm_hour, tm->tm_min, tm->tm_sec);
  content.replace("{{DateTime}}", String(dateTime));
  Server.send(200, "text/html", content);
}

void startPage() {
  // Retrieve the value of AutoConnectElement with arg function of WebServer class.
  // Values are accessible with the element name.
  String  tz = Server.arg("timezone");

  for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) {
    String  tzName = String(TZ[n].zone);
    if (tz.equalsIgnoreCase(tzName)) {
      configTime(TZ[n].tzoff * 3600, 0, TZ[n].ntpServer);
      Serial.println("Time zone: " + tz);
      Serial.println("ntp server: " + String(TZ[n].ntpServer));
      break;
    }
  }

  // The /start page just constitutes timezone,
  // it redirects to the root page without the content response.
  Server.sendHeader("Location", String("http://") + Server.client().localIP().toString() + String("/"));
  Server.send(302, "text/plain", "");
  Server.client().flush();
  Server.client().stop();
}

void setup()
{  
  Serial.begin(115200);
  Serial.println(" *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*  ");
  Serial.println("                                                ");
  Serial.println("          __________SPS EM03__________          ");
  Serial.println("                                                ");
  Serial.println("          ____Energy Meter 3 Phase____          ");
  Serial.println("                                                ");
  Serial.println(" *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*  ");
  Serial.println("                                                ");
  Serial.println("          _Configure Via Access Point_          ");
  Serial.println("                                                ");
  Serial.println("          ____SSID: SPSem03___________          ");
  Serial.println("          ____Password: '12345678'____          ");
  Serial.println("                                                ");
  Serial.println("                                                ");
  Serial.println(" *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*  ");
  Serial.println("                                                ");
  Serial.println("          ____Smart Power Solutions___          ");
  Serial.println("          _________CJ De Beer_________          ");
  Serial.println("                                                ");
  Serial.println(" *_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*  ");
  delay(1000);

  Config.autoReconnect = true;
  Config.hostName = "SPSem03";
  Portal.config(Config);

  Timezone.load(AUX_TIMEZONE);

  AutoConnectSelect&  tz = Timezone["timezone"].as<AutoConnectSelect>();
  for (uint8_t n = 0; n < sizeof(TZ) / sizeof(Timezone_t); n++) {
    tz.add(String(TZ[n].zone));
  }
  
  Server.on("/", rootPage);
  Server.on("/start", startPage); 

  Portal.join({ Timezone });

  Serial.println("Creating portal and trying to connect...");
  // Establish a connection with an autoReconnect option.
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
//    #if defined(ARDUINO_ARCH_ESP8266)
//    Serial.println(WiFi.hostname());
//    #elif defined(ARDUINO_ARCH_ESP32)
    Serial.println(WiFi.getHostname());
//    #endif
  }

//  setup_wifi();
  client.setServer(mqtt_server, 1883);
//  delay(500);
  emon1.voltage_l1(32, 52.76, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current_l1(36, 88.1);
  emon1.voltage_l2(35, 52.76, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current_l2(39, 88.1);
  emon1.voltage_l3(34, 52.76, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current_l3(33, 88.1);// Current: input pin, calibration.
//  delay(500);
  
  
//  client.setCallback(callback);

//  pinMode(ledPin, OUTPUT);
  
  dht.begin();
  
}

void loop()
{

    if (!client.connected()) {
    reconnect();
  }
  Portal.handleClient();
  energy_measure();

  long now = millis();
  if (now - lastMsg > 10) {
    lastMsg = now;

    temperature = dht.readTemperature();
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
//    Serial.print("Temperature: ");
//    Serial.println(tempString);
    client.publish("spsem/temperature", tempString);

    humidity = dht.readHumidity();
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
//    Serial.print("Humidity: ");
//    Serial.println(humString);
    client.publish("spsem/humidity", humString);

    char voltString_l1[8];
    dtostrf(supplyVoltage_l1, 1, 1, voltString_l1);
//    Serial.print("Voltage_l1: ");
//    Serial.println(voltString_l1);
    client.publish("sps/em03/voltage/l1", voltString_l1);

    char voltString_l2[8];
    dtostrf(supplyVoltage_l2, 1, 1, voltString_l2);
//    Serial.print("Voltage_l2: ");
//    Serial.println(voltString_l2);
    client.publish("sps/em03/voltage/l2", voltString_l2);

    char voltString_l3[8];
    dtostrf(supplyVoltage_l3, 1, 1, voltString_l3);
//    Serial.print("Voltage_l3: ");
//    Serial.println(voltString_l3);
    client.publish("sps/em03/voltage/l3", voltString_l3);

    char powString_l1[8];
    dtostrf(realPower_l1, 1, 1, powString_l1);
//    Serial.print("Power_l1: ");
//    Serial.println(powString_l1);
    client.publish("sps/em03/power/l1", powString_l1);

    char powString_l2[8];
    dtostrf(realPower_l2, 1, 1, powString_l2);
//    Serial.print("Power_l2: ");
//    Serial.println(powString_l2);
    client.publish("sps/em03/power/l2", powString_l2);

    char powString_l3[8];
    dtostrf(realPower_l3, 1, 1, powString_l3);
//    Serial.print("Power_l3: ");
//    Serial.println(powString_l3);
    client.publish("sps/em03/power/l3", powString_l3);

    char pfString_l1[8];
    dtostrf(powerFactor_l1, 1, 2, pfString_l1);
//    Serial.print("Power Factor_l1: ");
//    Serial.println(pfString_l1);
    client.publish("sps/em03/pf/l1", pfString_l1);

    char pfString_l2[8];
    dtostrf(powerFactor_l2, 1, 2, pfString_l2);
//    Serial.print("Power Factor_l2: ");
//    Serial.println(pfString_l2);
    client.publish("sps/em03/pf/l2", pfString_l2);

    char pfString_l3[8];
    dtostrf(powerFactor_l3, 1, 2, pfString_l3);
//    Serial.print("Power Factor_l3: ");
//    Serial.println(pfString_l3);
    client.publish("sps/em03/pf/l3", pfString_l3);

    char kwhString_l1[8];
    dtostrf(kwh_l1, 1, 2, kwhString_l1);
//    Serial.print("kWh_l1: ");
//    Serial.println(kwhString_l1);
    client.publish("sps/em03/kwh/l1", kwhString_l1);

    char kwhString_l2[8];
    dtostrf(kwh_l2, 1, 2, kwhString_l2);
//    Serial.print("kWh_l2: ");
//    Serial.println(kwhString_l2);
    client.publish("sps/em03/kwh/l2", kwhString_l2);

    char kwhString_l3[8];
    dtostrf(kwh_l3, 1, 2, kwhString_l3);
//    Serial.print("kWh_l3: ");
//    Serial.println(kwhString_l3);
    client.publish("sps/em03/kwh/l3", kwhString_l3);

    char peakString_l1[8];
    dtostrf(peakLoad_l1, 1, 2, peakString_l1);
//    Serial.print("Peak Load_l1: ");
//    Serial.println(peakString_l1);
    client.publish("sps/em03/peak/l1", peakString_l1);

    char peakString_l2[8];
    dtostrf(peakLoad_l2, 1, 2, peakString_l2);
//    Serial.print("Peak Load_l2: ");
//    Serial.println(peakString_l2);
    client.publish("sps/em03/peak/l2", peakString_l2);

    char peakString_l3[8];
    dtostrf(peakLoad_l3, 1, 2, peakString_l3);
//    Serial.print("Peak Load_l3: ");
//    Serial.println(peakString_l3);
    client.publish("sps/em03/peak/l3", peakString_l3);

  }
  
}
