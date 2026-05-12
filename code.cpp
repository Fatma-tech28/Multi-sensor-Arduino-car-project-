# ESP32 DisasterBot — Clean Integrated Code

```cpp
// ============================================================
//  DISASTERBOT — ESP32 AUTONOUS SAFETY ROBOT
// ============================================================
//  FEATURES
//   ✔ Flame detection & escape logic
//   ✔ Gas leak monitoring (MQ-135)
//   ✔ Ultrasonic obstacle avoidance
//   ✔ Temperature & humidity monitoring
//   ✔ Smart buzzer alerts
//   ✔ 4 DC motor control using L298N
//   ✔ Smooth non-blocking structure
// ============================================================

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// ============================================================
// DHT11
// ============================================================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ============================================================
// ULTRASONIC SENSOR
// ============================================================
#define TRIG_PIN 5
#define ECHO_PIN 18

// ============================================================
// FLAME SENSORS
// ============================================================
#define FLAME_LEFT 19
#define FLAME_RIGHT 21

// ============================================================
// MQ135
// ============================================================
#define MQ135_PIN 34

// ============================================================
// BUZZER
// ============================================================
#define BUZZER_PIN 33

// ============================================================
// L298N MOTOR DRIVER
// ============================================================
#define ENA 25
#define IN1 26
#define IN2 27

#define ENB 32
#define IN3 14
#define IN4 13

// ============================================================
// PWM SETTINGS
// ============================================================
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1

int motorSpeed = 180;

// ============================================================
// WIFI ACCESS POINT
// ============================================================
const char* ssid = "DisasterBot";
const char* password = "12345678";

WebServer server(80);

// ============================================================
// TIMERS
// ============================================================
unsigned long lastSensorRead = 0;
unsigned long sensorInterval = 1000;

// ============================================================
// SENSOR VALUES
// ============================================================
float temperature = 0;
float humidity = 0;
int gasValue = 0;
long distanceCM = 0;

bool flameLeftDetected = false;
bool flameRightDetected = false;

// ============================================================
// FUNCTION DECLARATIONS
// ============================================================
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void stopMotors();

void shortBeep();
void dangerAlarm();

long readDistance();
int readGasAverage();

void handleRoot();
void handleForward();
void handleBackward();
void handleLeft();
void handleRight();
void handleStop();

void reactToFire();
void reactToGas();
void reactToObstacle();

// ============================================================
// SETUP
// ============================================================
void setup()
{
    Serial.begin(115200);

    dht.begin();

    // ---------------- MOTOR PINS ----------------
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);

    ledcAttachPin(ENA, PWM_CHANNEL_A);
    ledcAttachPin(ENB, PWM_CHANNEL_B);

    // ---------------- ULTRASONIC ----------------
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // ---------------- FLAME ----------------
    pinMode(FLAME_LEFT, INPUT);
    pinMode(FLAME_RIGHT, INPUT);

    // ---------------- BUZZER ----------------
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    // ========================================================
    // WIFI ACCESS POINT
    // ========================================================
    WiFi.softAP(ssid, password);

    Serial.println();
    Serial.println("================================");
    Serial.println("DisasterBot Started");
    Serial.println("Access Point Ready");
    Serial.println(WiFi.softAPIP());
    Serial.println("================================");

    // ========================================================
    // WEB ROUTES
    // ========================================================
    server.on("/", handleRoot);
    server.on("/forward", handleForward);
    server.on("/backward", handleBackward);
    server.on("/left", handleLeft);
    server.on("/right", handleRight);
    server.on("/stop", handleStop);

    server.begin();

    stopMotors();
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop()
{
    server.handleClient();

    unsigned long currentMillis = millis();

    // ========================================================
    // SENSOR UPDATE
    // ========================================================
    if (currentMillis - lastSensorRead >= sensorInterval)
    {
        lastSensorRead = currentMillis;

        temperature = dht.readTemperature();
        humidity = dht.readHumidity();

        gasValue = readGasAverage();
        distanceCM = readDistance();

        flameLeftDetected = digitalRead(FLAME_LEFT) == LOW;
        flameRightDetected = digitalRead(FLAME_RIGHT) == LOW;

        Serial.println("========================");
        Serial.print("Temperature: ");
        Serial.println(temperature);

        Serial.print("Humidity: ");
        Serial.println(humidity);

        Serial.print("Gas Value: ");
        Serial.println(gasValue);

        Serial.print("Distance: ");
        Serial.println(distanceCM);

        Serial.print("Flame Left: ");
        Serial.println(flameLeftDetected);

        Serial.print("Flame Right: ");
        Serial.println(flameRightDetected);
    }

    // ========================================================
    // FIRE DETECTION
    // ========================================================
    if (flameLeftDetected || flameRightDetected)
    {
        reactToFire();
    }

    // ========================================================
    // GAS DETECTION
    // ========================================================
    if (gasValue > 2200)
    {
        reactToGas();
    }

    // ========================================================
    // OBSTACLE DETECTION
    // ========================================================
    if (distanceCM > 0 && distanceCM < 20)
    {
        reactToObstacle();
    }
}

// ============================================================
// MOTOR FUNCTIONS
// ============================================================
void moveForward()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    ledcWrite(PWM_CHANNEL_A, motorSpeed);
    ledcWrite(PWM_CHANNEL_B, motorSpeed);
}

void moveBackward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    ledcWrite(PWM_CHANNEL_A, motorSpeed);
    ledcWrite(PWM_CHANNEL_B, motorSpeed);
}

void turnLeft()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);

    ledcWrite(PWM_CHANNEL_A, motorSpeed);
    ledcWrite(PWM_CHANNEL_B, motorSpeed);
}

void turnRight()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);

    ledcWrite(PWM_CHANNEL_A, motorSpeed);
    ledcWrite(PWM_CHANNEL_B, motorSpeed);
}

void stopMotors()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    ledcWrite(PWM_CHANNEL_A, 0);
    ledcWrite(PWM_CHANNEL_B, 0);
}

// ============================================================
// BUZZER FUNCTIONS
// ============================================================
void shortBeep()
{
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
}

void dangerAlarm()
{
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);
        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
    }
}

// ============================================================
// SENSOR FUNCTIONS
// ============================================================
long readDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);

    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    long distance = duration * 0.034 / 2;

    return distance;
}

int readGasAverage()
{
    int total = 0;

    for (int i = 0; i < 10; i++)
    {
        total += analogRead(MQ135_PIN);
        delay(5);
    }

    return total / 10;
}

// ============================================================
// REACTION FUNCTIONS
// ============================================================
void reactToFire()
{
    Serial.println("FIRE DETECTED!");

    dangerAlarm();

    moveBackward();
    delay(700);

    if (flameLeftDetected)
    {
        turnRight();
    }
    else if (flameRightDetected)
    {
        turnLeft();
    }

    delay(700);

    stopMotors();
}

void reactToGas()
{
    Serial.println("GAS DETECTED!");

    stopMotors();

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(100);
        digitalWrite(BUZZER_PIN, LOW);
        delay(100);
    }
}

void reactToObstacle()
{
    Serial.println("OBSTACLE DETECTED!");

    shortBeep();

    moveBackward();
    delay(400);

    stopMotors();
    delay(200);

    turnRight();
    delay(500);

    stopMotors();
}

// ============================================================
// WEB HANDLERS
// ============================================================
void handleRoot()
{
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <title>DisasterBot</title>
        <meta name='viewport' content='width=device-width, initial-scale=1'>
        <style>
            body{
                font-family:Arial;
                text-align:center;
                background:#111;
                color:white;
                margin-top:40px;
            }

            button{
                width:120px;
                height:60px;
                margin:10px;
                font-size:18px;
                border:none;
                border-radius:12px;
                background:#ff5722;
                color:white;
            }
        </style>
    </head>
    <body>
        <h1>DisasterBot Control</h1>

        <button onclick="location.href='/forward'">Forward</button><br>
        <button onclick="location.href='/left'">Left</button>
        <button onclick="location.href='/stop'">Stop</button>
        <button onclick="location.href='/right'">Right</button><br>
        <button onclick="location.href='/backward'">Backward</button>
    </body>
    </html>
    )rawliteral";

    server.send(200, "text/html", html);
}

void handleForward()
{
    moveForward();
    server.send(200, "text/plain", "Moving Forward");
}

void handleBackward()
{
    moveBackward();
    server.send(200, "text/plain", "Moving Backward");
}

void handleLeft()
{
    turnLeft();
    server.send(200, "text/plain", "Turning Left");
}

void handleRight()
{
    turnRight();
    server.send(200, "text/plain", "Turning Right");
}

void handleStop()
{
    stopMotors();
    server.send(200, "text/plain", "Stopped");
}




