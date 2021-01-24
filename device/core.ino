/** 
 * Author: Szymon Pluta, 2021
 * contact@szymonpluta.com
 */

#include <SPI.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>


/** Server configuration. */
const String SERVER_ADDRESS   = "http://192.168.43.252:8080";


/** REST endpoints. */
const String API_SPEED        = "/api/speed";
const String API_TEMPERATURES = "/api/temperatures";
const String API_INFO         = "/api/info/next";
const String API_COVID        = "/api/covid-cases";


/** Device configuration. */
#define HOW_OFTEN_TEMPERATURES_REQUEST 15000 // Temperature measurements are sent to the database every 15 seconds.

#define DISPLAY_SPEED_DURATION         4500  // Show speed for 4.5 seconds 
#define DISPLAY_MESSAGE_DURATION       3000
#define DISPLAY_COVID_CASES_DURATION   4000
#define DISPLAY_TEMPERATURE_DURATION   4000
#define DISPLAY_SPLASH_SCREEN_DURATION 6000
#define SENSOR_1_INVALIDATE_AFTER      12000
#define LED_DURATION                   150

#define SENSORS_GAP_DISTANCE 1000 // Two cases to distinguish here:
                                  // 1. For an nearly-instantaneous speed, the gap between the sensors
                                  //    should be as small as possible in a real environment (e.g. 100 centimeters).
                                  // 2. For an average speed on some distance,
                                  //    the gap between the sensors could be of an arbitrary length (e.g. 5 kilometers).
                                  //
                                  // Brand new solution (2021) on A1 Highway that works similarly to my solution:
                                  // https://auto.dziennik.pl/aktualnosci/artykuly/8071287,odcinkowy-pomiar-predkosci-autostrada-a1-lokalizacja-mandat-kierowca-predkosc.html 

#define SENSOR_DETECT_RANGE    20   // The detection range should be as high as possible in a real environment (up to 400 centimeters).
#define SENSOR_MAX_RANGE       400

#define SERIAL_PORT            115200
#define INITIAL_STARTUP_DELAY  5000   // Initial 5 seconds start-up delay.

enum OLED_Mode {
  MESSAGE,
  SPEED,
  TEMPERATURES,
  COVID
};

OLED_Mode OLED_mode = TEMPERATURES; // initial screen

// GPIO pins values
#define D0  16
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5  14
#define D6  12
#define D7  13
#define D8  15
#define D9  3

// Buzzer
#define PIN_BUZZER D0
#define BUZZER_SHORT_BEEP_DELAY 250
#define BUZZER_LONG_BEEP_DELAY  750

// Ultrasonic sensors
#define PIN_TRIG1 D6
#define PIN_TRIG2 D4
#define PIN_ECHO1 D5
#define PIN_ECHO2 D3

// Temperature sensors
#define PIN_TEMPERATURE D7

#define PIN_LED D8

OneWire oneWire(PIN_TEMPERATURE);
DallasTemperature temperature_sensors(&oneWire);

// Temperature request data transfer object
struct Temperatures {
  double value1;
  double value2;
};

Temperatures temperatures;

bool is_first_temperature_request = true;

// OLED 0.96"
#define OLED_ADDRESS  0x3C
#define WIDTH         128
#define HEIGHT        64

Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, -1);

// LCD 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

enum Content_Type {
  APPLICATION_JSON,
  TEXT_PLAIN
};

// Program memory
ESP8266WiFiMulti WiFiMulti;

long measured_time  = 0;
long distance1      = 0;
long distance2      = 0;

boolean s1_detected = false;
boolean s2_detected = false;

boolean is_LED_on = false;

// The solution currently does not implement cache invalidation.
boolean is_covid_cached              = false;

unsigned long s1_detected_time       = 0;
unsigned long s2_detected_time       = 0;
unsigned long detected_time_diff     = 0;

double calculated_speed              = 0.0f;

unsigned long prev_temperatures_time = 0;
unsigned long curr_temperatures_time = 0;

unsigned long prev_message_time      = 0;
unsigned long prev_covid_time        = 0;

unsigned long time_since_LED_on      = 0;

String displayed_message;
String covid_cases;

int message_counter = 0;

static const unsigned char PROGMEM logo[1024] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xe1, 0x81, 0x81, 0x99, 0xb9, 0x83, 0xff, 0xe1, 0xc3, 0x99, 0xf9, 0x3d, 0x81, 0x89, 0xc3, 0x99, 
  0x81, 0xc5, 0x85, 0xbd, 0x9b, 0x8b, 0xff, 0x85, 0x89, 0x81, 0xf3, 0x7c, 0x89, 0x9b, 0x89, 0x99, 
  0x9f, 0xe7, 0xe7, 0x99, 0xb9, 0xb9, 0xff, 0x9f, 0xb9, 0x95, 0xf3, 0x2d, 0xbf, 0x89, 0xb9, 0xd3, 
  0xbf, 0xef, 0xef, 0x91, 0x99, 0x83, 0xff, 0xbf, 0x3d, 0x91, 0xe7, 0xa4, 0x8b, 0xa9, 0xbd, 0xc7, 
  0x35, 0xe7, 0xe7, 0x81, 0xbd, 0x87, 0xff, 0x3f, 0xb9, 0x25, 0xe7, 0x25, 0x83, 0x8b, 0x19, 0xe7, 
  0x91, 0xef, 0xef, 0x9d, 0x99, 0x99, 0xff, 0x9f, 0x3d, 0xa9, 0xcf, 0xa5, 0x9f, 0xb1, 0xbd, 0xc7, 
  0x9d, 0xe7, 0xe7, 0xb9, 0xb9, 0xbc, 0xff, 0x9f, 0x99, 0x9d, 0xcf, 0x91, 0xbf, 0x91, 0x99, 0xd3, 
  0x89, 0xa3, 0xe7, 0x9b, 0x83, 0x89, 0xe7, 0x85, 0x81, 0xb9, 0x8f, 0x91, 0x8b, 0x93, 0x89, 0x99, 
  0xe1, 0x81, 0xef, 0xb9, 0xc7, 0x83, 0xe7, 0xe1, 0xc7, 0x9d, 0x9f, 0x93, 0x80, 0xb1, 0xc3, 0x99, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x80, 0x00, 0x07, 0xe0, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x03, 0xe0, 0x00, 0x00, 0xa0, 0x00, 0x0e, 0x00, 0x00, 0xc0, 0x00, 0x01, 0xfc, 0x01, 
  0x80, 0x07, 0x0f, 0xf8, 0x70, 0x00, 0xa3, 0x03, 0x0e, 0x1f, 0xf0, 0xe0, 0x00, 0x06, 0x03, 0x01, 
  0x80, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0xa3, 0x03, 0x04, 0x1f, 0xe0, 0x40, 0x00, 0x08, 0x00, 0x81, 
  0x80, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0xa1, 0x87, 0x00, 0x18, 0x00, 0x00, 0x00, 0x10, 0x00, 0x61, 
  0x80, 0x1f, 0xff, 0x7f, 0xfc, 0x00, 0xa3, 0x33, 0x3e, 0x18, 0x03, 0xe0, 0x00, 0x27, 0x07, 0x21, 
  0x80, 0x1f, 0xe0, 0x03, 0xf8, 0x00, 0xa1, 0xb6, 0x3e, 0x18, 0x07, 0xe0, 0x00, 0x4f, 0x8f, 0x91, 
  0x80, 0x3f, 0x80, 0x00, 0xfc, 0x00, 0xa1, 0xbb, 0x0e, 0x1c, 0x00, 0xe0, 0x00, 0x9f, 0xdf, 0xc9, 
  0x80, 0x3e, 0x03, 0x80, 0x3f, 0x00, 0xa1, 0xb6, 0x06, 0x0f, 0xc0, 0x60, 0x00, 0x9d, 0xdd, 0xc9, 
  0x82, 0xf8, 0x06, 0x60, 0x1f, 0x20, 0xa1, 0xf6, 0x0c, 0x1f, 0xc0, 0xc0, 0x01, 0x1f, 0xdf, 0xc9, 
  0x87, 0xf0, 0x43, 0xe1, 0x0f, 0xf0, 0xa1, 0xae, 0x06, 0x18, 0x00, 0x60, 0x01, 0x0f, 0x8f, 0x85, 
  0x8f, 0xf1, 0xa1, 0xc2, 0xc7, 0xf8, 0xa1, 0xee, 0x0e, 0x1c, 0x00, 0xe0, 0x01, 0x07, 0x07, 0x05, 
  0x8f, 0xe3, 0x31, 0xc6, 0xc3, 0xf8, 0xa1, 0xde, 0x7f, 0x98, 0x07, 0xf8, 0x01, 0x00, 0x00, 0x05, 
  0x9f, 0xc1, 0x31, 0xc4, 0x41, 0xfc, 0xa1, 0xee, 0x7f, 0xd8, 0x07, 0xfc, 0x01, 0x00, 0x00, 0x05, 
  0x8f, 0x83, 0xa3, 0xc6, 0xc1, 0xf8, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x18, 0x00, 0xc5, 
  0x87, 0x83, 0xe1, 0xc3, 0xe0, 0xf0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x3c, 0x01, 0xe9, 
  0x87, 0x83, 0xb3, 0xe6, 0xe0, 0xf0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9f, 0x07, 0xe9, 
  0x8f, 0x07, 0xbb, 0xed, 0xe0, 0x70, 0xa0, 0xf0, 0xfc, 0x3f, 0xcf, 0xf3, 0xf0, 0x8f, 0xff, 0x89, 
  0x8f, 0x07, 0xcf, 0xfd, 0xf0, 0x78, 0xa1, 0xfc, 0xff, 0x3f, 0xef, 0xf3, 0xf8, 0x43, 0xff, 0x11, 
  0x8e, 0x0f, 0xe7, 0xf3, 0xf8, 0x78, 0xa3, 0x88, 0xc7, 0x30, 0x0c, 0x03, 0x1c, 0x21, 0xfc, 0x21, 
  0xff, 0x3f, 0xf8, 0x07, 0xfe, 0x3e, 0xa3, 0x80, 0xc3, 0x30, 0x04, 0x03, 0x0e, 0x30, 0x00, 0x61, 
  0xbe, 0x07, 0xff, 0xff, 0xf0, 0x7e, 0xa1, 0x80, 0xe3, 0xb0, 0x0e, 0x03, 0x8e, 0x08, 0x00, 0x81, 
  0xfe, 0x0f, 0xff, 0xff, 0xf0, 0x3e, 0xa1, 0xf0, 0xc3, 0x3f, 0x8f, 0xe3, 0x0c, 0x07, 0x07, 0x01, 
  0xfe, 0x0f, 0xfb, 0xef, 0xf8, 0x3f, 0xa0, 0x78, 0xc7, 0x3f, 0x8f, 0xe3, 0x06, 0x00, 0xf8, 0x01, 
  0xbe, 0x0f, 0xeb, 0xcf, 0xf8, 0x7e, 0xa0, 0x3c, 0xfe, 0x30, 0x0c, 0x03, 0x0e, 0x00, 0x00, 0x01, 
  0xff, 0x13, 0xa9, 0xcb, 0x6c, 0x7e, 0xa0, 0x0c, 0xfc, 0x30, 0x0c, 0x01, 0x8c, 0x00, 0x00, 0x01, 
  0x8f, 0x26, 0xaf, 0xf9, 0xe0, 0x78, 0xa2, 0x1c, 0xc0, 0x38, 0x0c, 0x03, 0x1c, 0x00, 0x00, 0x01, 
  0x8f, 0x06, 0xbf, 0xfe, 0xb0, 0x70, 0xa3, 0xfc, 0x60, 0x3f, 0xcf, 0xf3, 0xf8, 0x00, 0x00, 0x01, 
  0x87, 0x05, 0x7f, 0xfe, 0x10, 0x70, 0xa1, 0xf0, 0xc0, 0x3f, 0xef, 0xfb, 0xf0, 0x00, 0x00, 0x01, 
  0x87, 0x0c, 0x39, 0xce, 0x18, 0xf0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x87, 0x81, 0x63, 0xe3, 0x40, 0xf0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xc1, 0xc1, 0xc1, 0xc1, 0xf8, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x9f, 0xc2, 0xaf, 0xfa, 0xa1, 0xfc, 0xa3, 0xf0, 0xff, 0x0f, 0x0f, 0xc3, 0x00, 0x3c, 0x70, 0xe1, 
  0x8f, 0xe1, 0xaf, 0xfa, 0xc3, 0xf8, 0xa3, 0xfc, 0xff, 0x1f, 0xcf, 0xf3, 0x00, 0x3c, 0x30, 0xc1, 
  0x8f, 0xf0, 0x07, 0xf0, 0x07, 0xf0, 0xa3, 0x1c, 0x18, 0x38, 0x8c, 0x73, 0x80, 0x3c, 0x39, 0xc1, 
  0x87, 0xf8, 0x01, 0x80, 0x0f, 0xf0, 0xa3, 0x8c, 0x18, 0x38, 0x0c, 0x33, 0x00, 0x3c, 0x19, 0x81, 
  0x82, 0x7c, 0x00, 0x80, 0x1f, 0x20, 0xa3, 0x0e, 0x18, 0x18, 0x0e, 0x3b, 0x00, 0x66, 0x1d, 0x81, 
  0x80, 0x3e, 0x00, 0x80, 0x3e, 0x00, 0xa3, 0x0e, 0x18, 0x1f, 0x0c, 0x33, 0x80, 0x76, 0x0f, 0x01, 
  0x80, 0x1f, 0x80, 0x00, 0xfc, 0x00, 0xa3, 0x06, 0x18, 0x07, 0x8c, 0x73, 0x00, 0x66, 0x0f, 0x01, 
  0x80, 0x1f, 0xf0, 0x07, 0xf8, 0x00, 0xa3, 0x8c, 0x18, 0x03, 0xcf, 0xe3, 0x00, 0xe7, 0x06, 0x01, 
  0x80, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0xa3, 0x0e, 0x18, 0x00, 0xcf, 0xc3, 0x80, 0xff, 0x06, 0x01, 
  0x80, 0x1f, 0xff, 0xff, 0xfc, 0x00, 0xa3, 0x1c, 0x18, 0x21, 0xcc, 0x03, 0x00, 0xff, 0x06, 0x01, 
  0x80, 0x0f, 0xbf, 0xfe, 0xf8, 0x00, 0xa3, 0xf8, 0xff, 0x3f, 0xc6, 0x03, 0xfe, 0xc3, 0x06, 0x01, 
  0x80, 0x03, 0x07, 0xf0, 0x60, 0x00, 0xa3, 0xf0, 0xff, 0x3f, 0x0c, 0x03, 0xfd, 0xc3, 0x86, 0x01, 
  0x80, 0x00, 0x07, 0xe0, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x07, 0xe0, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void setup() {
  delay(INITIAL_STARTUP_DELAY);

  init_ESP8266();
  init_WiFi();
  init_LCD();
  init_OLED();
  init_sensors();

  buzzer_system_success();

  draw_splash_screen(DISPLAY_SPLASH_SCREEN_DURATION);
}

void loop() {
  if (!is_WiFi_connected()) {
    handle_WiFi_not_connected();
  } else {
    process_ultrasonic_sensors();
    process_temperature_sensors();
    process_LED();
    update_OLED_mode();
    draw_OLED(); 
  }
}

void init_ESP8266() {
  Serial.begin(SERIAL_PORT);
  Serial.println("Started setup");

  pinMode(PIN_TRIG1,       OUTPUT);
  pinMode(PIN_TRIG2,       OUTPUT);
  pinMode(PIN_ECHO1,       INPUT);
  pinMode(PIN_ECHO2,       INPUT);
  pinMode(PIN_TEMPERATURE, INPUT);
  pinMode(PIN_LED,         OUTPUT);
  pinMode(PIN_BUZZER,      OUTPUT);
}

void init_WiFi() {
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("d4gEk80Px4YdfSs1", "in4matiz");
}

void init_LCD() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
}

void init_OLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    OLED_failed();
  }
}

void OLED_failed() {
  Serial.println(F("SSD1306 allocation failed"));
  lcd.setCursor(0, 0);
  lcd.printf("OLED init failed");
  buzzer_OLED_failed();
}

void init_sensors() {
  temperature_sensors.begin();
}

void draw_splash_screen(int duration) {
  display.clearDisplay();
  display.drawBitmap(0, 0, logo, 128, 64, 1);
  display.display();
  delay(duration);
}

inline boolean is_WiFi_connected() {
  return WiFiMulti.run() == WL_CONNECTED;
}

void handle_WiFi_not_connected() {
  Serial.println("[INFO]: Not yet connected to the WiFi");
  buzzer_WiFi_not_connected();
  delay(3000);
}

void process_ultrasonic_sensors() {
  distance1 = fetch_distance_sensor_1();
  if (is_sensor_1_detecting_car()) {
    handle_sensor_1_detecting_car();
  }

  invalidate_sensor_1_detection();

  distance2 = fetch_distance_sensor_2();
  if (is_sensor_2_detecting_car()) {
    handle_sensor_2_detecting_car();
  }

  handle_sensors_range_limit();
}

String json_part1 = "{\"value1\": ";
String json_part2 = ", \"value2\": ";
String json_part3 = "}";

String get_temperatures_payload(Temperatures temperatures) {
  return json_part1 + temperatures.value1 + json_part2 + temperatures.value2 + json_part3;
}

void process_temperature_sensors() {
  curr_temperatures_time = millis();
  if (is_first_temperature_request || curr_temperatures_time - prev_temperatures_time >= HOW_OFTEN_TEMPERATURES_REQUEST) {
    is_first_temperature_request = false;
    temperatures = readTemperatures();

    String payload = get_temperatures_payload(temperatures);

    HTTP_POST(API_TEMPERATURES, payload, Content_Type::APPLICATION_JSON); // possibly takes long to complete 

    prev_temperatures_time = millis();
  }
}

Temperatures readTemperatures() {
  return Temperatures { temperature_sensors.getTempCByIndex(0), temperature_sensors.getTempCByIndex(1) };
}

void process_LED() {
  if (is_LED_on && millis() - time_since_LED_on > LED_DURATION) {
     is_LED_on = false;
     digitalWrite(PIN_LED, LOW);
  }
}

void update_OLED_mode() {
  
  switch (OLED_mode) {
    case MESSAGE:
      if (millis() - prev_message_time > DISPLAY_MESSAGE_DURATION) {
        displayed_message = HTTP_GET(API_INFO); // possibly long to complete
        prev_message_time = millis();
        message_counter++;
        if (message_counter > 5) {
          message_counter = 0;
          OLED_mode = TEMPERATURES;
          prev_temperatures_time = millis();
        }
      }
      break;

    case TEMPERATURES:
      if (millis() - prev_temperatures_time > DISPLAY_TEMPERATURE_DURATION) {
        prev_covid_time = millis();
        OLED_mode = COVID;
      }
      break;

    case COVID:
      if (!is_covid_cached) {
        covid_cases = HTTP_GET(API_COVID);
        is_covid_cached = true;
      }
      if (millis() - prev_covid_time > DISPLAY_COVID_CASES_DURATION) {
        OLED_mode = MESSAGE;
      }
      break;

    case SPEED:
      if (millis() - s2_detected_time > DISPLAY_SPEED_DURATION) {
        OLED_mode = MESSAGE;
      }
      break;

    default:
      break;
  }
}

inline int fetch_distance_sensor_1() {
  return fetch_distance(PIN_TRIG1, PIN_ECHO1);
}

inline int fetch_distance_sensor_2() {
  return fetch_distance(PIN_TRIG2, PIN_ECHO2);
}

inline int fetch_distance(int pin_trig, int pin_echo) {
  digitalWrite(pin_trig, LOW);
  delayMicroseconds(2);
  digitalWrite(pin_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_trig, LOW);

  measured_time = pulseIn(pin_echo, HIGH);
  return measured_time / 58;
}

inline boolean is_sensor_1_detecting_car() {
  return distance1 > 0 && distance1 < SENSOR_DETECT_RANGE;
}

inline boolean is_sensor_2_detecting_car() {
  return distance2 > 0 && distance2 < SENSOR_DETECT_RANGE;
}

void handle_sensor_1_detecting_car() {
  if (!s1_detected) {
      s1_detected = true;
      s1_detected_time = millis();
    }
}

void invalidate_sensor_1_detection() {
  if (s1_detected && millis() - s1_detected_time > SENSOR_1_INVALIDATE_AFTER) {
    s1_detected = false;
  }
}

void handle_sensor_2_detecting_car() {
  if (!s2_detected && s1_detected) {
      s2_detected = true;
      s2_detected_time = millis();
      detected_time_diff = s2_detected_time - s1_detected_time;
      calculated_speed = ((double) SENSORS_GAP_DISTANCE / (double) detected_time_diff) * 36.0f;

      HTTP_POST(API_SPEED, String(calculated_speed), Content_Type::TEXT_PLAIN);
      OLED_mode = SPEED;
  } else {
    if (s2_detected) {
      s2_detected = false;
      s1_detected = false;
    }
  }
}

void handle_sensors_range_limit() {
  if (distance1 > SENSOR_MAX_RANGE) {
    distance1 = SENSOR_MAX_RANGE;
  }
  if (distance2 > SENSOR_MAX_RANGE) {
    distance2 = SENSOR_MAX_RANGE;
  }
}

void turn_on_LED() {
  is_LED_on = true;
  digitalWrite(PIN_LED, HIGH);
  time_since_LED_on = millis();
}

String HTTP_GET(String REST_endpoint) {
  turn_on_LED();
  
  WiFiClient client;
  HTTPClient http;
  
  if (http.begin(client, SERVER_ADDRESS + REST_endpoint)) {

    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.GET();
    if (httpCode > 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.printf("HTTP GET: %d", httpCode);
      lcd.setCursor(0, 1);
      lcd.print(REST_endpoint);

      return http.getString();
    } else {
      lcd.setCursor(0, 0);
      lcd.printf("FAILED HTTP GET");
      lcd.setCursor(0, 1);
      lcd.printf("SERVER error");
      buzzer_server_error();
    }
    http.end();
  } else {
      lcd.setCursor(0, 0);
      lcd.printf("                ");
      lcd.printf("FAILED HTTP GET");
      lcd.setCursor(0, 1);
      lcd.printf("                ");
      lcd.printf("ESP8266 error");
      buzzer_esp8266_error();
  }
}

String HTTP_POST(String REST_endpoint, String payload, Content_Type content_type) {
  turn_on_LED();
  
  WiFiClient client;
  HTTPClient http;

  if (http.begin(client, SERVER_ADDRESS + REST_endpoint)) {

    if (content_type == TEXT_PLAIN) {
      http.addHeader("Content-Type", "text/plain");
    } else if (content_type == APPLICATION_JSON) {
      http.addHeader("Content-Type", "application/json");
    } 
    
    int httpCode = http.POST(payload);
    if (httpCode > 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.printf("HTTP POST: %d", httpCode);
      lcd.setCursor(0, 1);
      lcd.print(REST_endpoint);

      return http.getString();
    } else {
      lcd.setCursor(0, 0);
      lcd.printf("FAILED HTTP GET");
      lcd.setCursor(0, 1);
      lcd.printf("SERVER error");
      buzzer_server_error();
    }
    http.end();
  } else {
    lcd.setCursor(0, 0);
    lcd.printf("                ");
    lcd.printf("FAILED HTTP GET");
    lcd.setCursor(0, 1);
    lcd.printf("                ");
    lcd.printf("ESP8266 error");
    buzzer_esp8266_error();
  }
}

void short_beep() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(BUZZER_SHORT_BEEP_DELAY);
  digitalWrite(PIN_BUZZER, LOW);
  delay(BUZZER_SHORT_BEEP_DELAY);
}

void long_beep() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(BUZZER_LONG_BEEP_DELAY);
  digitalWrite(PIN_BUZZER, LOW);
  delay(BUZZER_LONG_BEEP_DELAY);
}

void buzzer_system_success() {
  short_beep();
}

void buzzer_WiFi_not_connected() {
  short_beep();
  short_beep();
}

void buzzer_OLED_failed() {
  long_beep();
}

void buzzer_server_error() {
  long_beep();
  short_beep();
}

void buzzer_esp8266_error() {
  long_beep();
  long_beep();
}

void draw_OLED() {
  
  display.display();
  display.clearDisplay();
    
  switch (OLED_mode) {
      case SPEED:
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        if (calculated_speed > 60) {
          display.print("SLOW DOWN!");
        } else {
          display.print(" SAFE TRIP");
        }

        if (calculated_speed < 100) {
          display.setTextSize(6);
          display.setCursor(0, 24);
          display.print((int) calculated_speed);
          display.setTextSize(2);
          display.setCursor(76, 28);
          display.print("km");
          display.setCursor(96, 40);
          display.print("/");
          display.setCursor(112, 52);
          display.print("h");
        } else {
          display.setTextSize(6);
          display.setCursor(0, 24);
          display.print((int) calculated_speed);
          display.setTextSize(1);
          display.setCursor(112, 48);
          display.print("kmh");
        }
        break;
      case TEMPERATURES:
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print(" WEATHER");
        display.setTextSize(6);
        display.setCursor(0, 24);
        display.printf("%d", (int) temperatures.value1);

        display.drawCircle(88, 32, 3, SSD1306_WHITE);
        display.setTextSize(2);
        display.setCursor(96, 28);
        display.print("C");
        break;
      case MESSAGE:
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 48);
        display.printf(displayed_message.c_str());
        break;
      case COVID:
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.printf("COVIDTODAY");
        display.setTextSize(5);
        display.setCursor(0, 24);
        display.printf(covid_cases.c_str());
        break;
      default:
        break;
    }
}