
#include <ESP8266WiFi.h>
#include <SimpleDHT.h>

// Declaracion e inicializacion de variables utilizadas para
// realizar la conexion a la red Wifi local
const char WiFiSSID[] = "ESP8266 Thing macID"; //cambia macID por el ID del dispositivo servidor
const char WiFiPSK[] = "setisaedu";

// Informacion de nuestro servidor
const char host[] = "192.168.4.1"; //Direccion IP de nuestro servidor
const int httpPort = 80; //Puerto de enlace

String macID; //String donde se almacenara nuestra macID
//Variables donde se almacenan las mediciones de temperatura y humedad
int value1;
int value2;

// Inicializacion de pines
const int LED_PIN = 5;
int pinDHT11 = 2;

//Instanciacion de libreria para controlar nuestro sensor dht11
SimpleDHT11 dht11;

void setup() {
  initHardware(); // Inicializammos nuestros pines fisicos
  connectWiFi();  // Conexion nuestra red Wifi
}

void loop() {
  delay(1000);  //Esperamos un segundo a para realizar una nueva lectura del sensor DHT11
  byte temperature = 0;
  byte humidity = 0;

  // Se realiza la lectura de temperatura y humedad
  if (dht11.read(pinDHT11, &temperature, &humidity, NULL)) {
    Serial.print("Read DHT11 failed.");
    return;
  }

  // Se almacenana en las variables de salida
  value1 = (int)temperature;
  value2 = (int)humidity;

  // Se imprimen en el puerto serial para comprobar su correcta lectura
  Serial.print(value1); Serial.println("ºC"); 
  Serial.print(value2); Serial.println("%");

  // Inicia la conexion con el servidor
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Se intenta la conexion con el servidor
  WiFiClient client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // Si ha existido conexion se conntinua, y se concatena la macID de nuestros ESP8266
  // y los valores de temperatura y humedad leidos, para convertirlo en una URL
  String url = macID;
  url += "&value1=";
  url += value1;
  url += "&value1=";
  url += value2;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // Se envia la solicitud al servidor
  client.print(String("GET /") + url + "/ HTTP/1.1\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Se lee todo lo recibido desde el servidor y se imprime en el puerto serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void connectWiFi()
{
  
  // Obtenemos la macID de nuestro ESP8266
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  Serial.print(macID);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WiFiSSID);
  
  // Configuramos nuestro dispositivo IoT como estacion
  WiFi.mode(WIFI_STA);

  // Se inicia la conexion a nuestro servidor ESP8266 Thing + macID
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Se espera a que nuestro ESP8266 se conecte a la red Wifi local
  byte ledStatus = LOW;
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    Serial.print(".");
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(100);
  }
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initHardware()
{
  Serial.begin(115200); //Inicializacion y configuracion de puerto serial
  pinMode(LED_PIN, OUTPUT); // Configuracion de nuestro LED
  digitalWrite(LED_PIN, HIGH);
}
