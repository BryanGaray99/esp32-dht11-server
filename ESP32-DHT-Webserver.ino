#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>

const char *ssid = "";
const char *password = "";


// Instancias del servidor web y del sensor DHT
WebServer server(80);
DHT dht(26, DHT11);

// Pines para los LEDs que indican el estado de temperatura y humedad
const int redTempPin = 4;
const int yellowTempPin = 16;
const int redHumidityPin = 5;
const int yellowHumidityPin = 18;

void handleRoot() {
  char msg[1500];

  // Leer temperatura y humedad del sensor DHT
  float temperature = readDHTTemperature();
  float humidity = readDHTHumidity();

  // Verificar límites de temperatura y humedad y controlar LEDs en consecuencia
  if (temperature > 35.0) {
    digitalWrite(redTempPin, HIGH);
    digitalWrite(yellowTempPin, LOW);
  } else if (temperature < 15.0) {
    digitalWrite(redTempPin, LOW);
    digitalWrite(yellowTempPin, HIGH);
  } else {
    digitalWrite(redTempPin, LOW);
    digitalWrite(yellowTempPin, LOW);
  }

  if (humidity > 60.0) {
    digitalWrite(redHumidityPin, HIGH);
    digitalWrite(yellowHumidityPin, LOW);
  } else if (humidity < 30.0) {
    digitalWrite(redHumidityPin, LOW);
    digitalWrite(yellowHumidityPin, HIGH);
  } else {
    digitalWrite(redHumidityPin, LOW);
    digitalWrite(yellowHumidityPin, LOW);
  }

  // Preparar respuesta HTML con datos de temperatura y humedad
  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='1'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>ESP32 DHT Server</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h3>Servidor Web con ESP32 para monitoreo de Temperatura y Humedad</h3>\
      <h3>PSE - Bryan Garay</h3>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperatura</span>\
        <span style='color:%s;'>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Humedad</span>\
        <span style='color:%s;'>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
  </body>\
</html>",
           (temperature > 35.0) ? "#ca3517" : ((temperature < 15.0) ? "#ffeb3b" : ""),
           temperature,
           (humidity > 60.0) ? "#ca3517" : ((humidity < 30.0) ? "#ffeb3b" : ""),
           humidity
          );

  // Enviar respuesta HTTP al cliente
  server.send(200, "text/html", msg);
}

void setup(void) {
  // Inicializar comunicación serial y sensor DHT
  Serial.begin(115200);
  dht.begin();

  // Configurar modos de los pines para los LEDs
  pinMode(redTempPin, OUTPUT);
  pinMode(yellowTempPin, OUTPUT);
  pinMode(redHumidityPin, OUTPUT);
  pinMode(yellowHumidityPin, OUTPUT);

  // Conectar a WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Esperar la conexión a WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Imprimir detalles de la conexión
  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar mDNS
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder iniciado");
  }

  // Configurar el manejador de la URL raíz
  server.on("/", handleRoot);

  // Iniciar el servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop(void) {
  // Manejar las solicitudes de clientes entrantes
  server.handleClient();

  // Permitir que la CPU cambie a otras tareas
  delay(2);
}

// Función para leer la temperatura del sensor DHT
float readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("¡Error al leer del sensor DHT!");
    return -1;
  } else {
    Serial.println(t);
    return t;
  }
}

// Función para leer la humedad del sensor DHT
float readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("¡Error al leer del sensor DHT!");
    return -1;
  } else {
    Serial.println(h);
    return h;
  }
}