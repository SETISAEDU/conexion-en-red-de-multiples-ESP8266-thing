#include <ESP8266WiFi.h>

// Declaracion e inicializacion de variables utilizadas para
// realizar la conexion a la red Wifi local
const char WiFiSSID[] = "SSID_del_wifi_local"; //SSID de la red Wifi
const char WiFiPSK[] = "contraseña_del_wifi_local";   //Contraseña WPA, WPA2 o WEP


// Declaracion de la contraseña de nuestro Punto de Acceso (AP)
const char WiFiAPPSK[] = "setisaedu";

// Inicializacion de pines
const int LED_PIN = 5;

// Declaracion de variables que almacenan la temperatura
// y humedad de los dos dispositivos clientes
int tmp_dev1, tmp_dev2;
int hmd_dev1, hmd_dev2;

String value1;
String value2;

// Declaracion del objeto que actua como servidor y configura
// el puerto 80 que es la que respondera a solicitudes HTTP
WiFiServer server(80);

void setup() {
  initHardware(); //Iniciamos nuestros pines fisicos
  setupWiFi();    //Configuramos y conectamos nuestra red Wifi
  server.begin(); //Se inicia el dispositivo como un servidor
}

void loop()
{

  String s;
  // Si el cliente esta conectado
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Se realiza la lectura de la solicitud
  String req = client.readStringUntil('\r');
  Serial.print(req);

  client.flush();

  int val = -1;
  if (req.indexOf("/macID_dispositivo1") != -1){  //Dispositivo 1; escribe el ID del dispositivo 1, cliente ESP8266
    val = 0; // Se escribira el LED en bajo

    // Se decodifican los valores recibidos por les clientes ESP8266
    // y se almacenan en variables internas
    decoder_values(req); //decodifica los valores de la trama: /macID_del_cliente&values1=##$values2=##
    tmp_dev1=value1.toInt();
    hmd_dev1=value2.toInt();
    Serial.println("");
    Serial.print("Temperatura_dev1: ");
    Serial.println(tmp_dev1);
    Serial.print("Humedad_dev1: ");
    Serial.println(hmd_dev1);
  }
  else if (req.indexOf("/macID_dispositivo2") != -1){  //Dispositivo 2; escribe el ID del dispositivo 2, cliente ESP8266
    val = 1; // Se escribira el LED en alto

    // Se decodifican los valores recibidos por les clientes ESP8266
    // y se almacenan en variables internas
    decoder_values(req); //decodifica los valores de la trama: /macID_del_cliente&values1=##$values2=##
    tmp_dev2=value1.toInt();
    hmd_dev2=value2.toInt();
    Serial.println("");
    Serial.print("Temperatura_dev2: ");
    Serial.println(tmp_dev2);
    Serial.print("Humedad_dev2: ");
    Serial.println(hmd_dev2);
  }
  else if (req.indexOf("/menu") != -1)
  val = -2; // Se imprimira el menu de la pagina web
  else if (req.indexOf("/?var=dev1") != -1)
  val = -3; // Se imprimira la temperatura y humedad sensada por el dispositivo 1
  else if (req.indexOf("/?var=dev2") != -1)
  val = -4; // Se imprimira la temperatura y humedad sensada por el dispositivo 1
  
  // Se setea GPIO5 de acuerdo a la solicitud
  if (val >= 0)
    digitalWrite(LED_PIN, val);

   client.flush();

   if (val >= 0){
      s = "LED is now ";
      s += (val) ? "high" : "low";
    }

    else if (val < -1){
      // Si nuestro clente es otro dispositivo
      // Se prepara la respuesta. Iniciamos con un header comun: 
      s = "HTTP/1.1 200 OK\r\n";
      s += "Content-Type: text/html\r\n\r\n";
      s += "<!DOCTYPE HTML>\r\n<html>\r\n";

      if (val == -2) {
          // Se imprimen en una pagina web el menu para elegir que dispositivo se desea realizar la lectura
          s += "<head><title>ESP8266 WEB SERVER AP AND STA.</title></head>";
          s += "<body><center><br><br><br><h1> ¿De cual dispositivo deseas realizar la lectura de temperatura y humedad? </h1>";
          s += "<form action=\"direccion_Ip_local/\" method=\"GET\">";
          s += "<input type=\"radio\" name=\"var\" value=\"dev1\">Dispositivo 1.<br>";
          s += "<input type=\"radio\" name=\"var\" value=\"dev2\">Dispositivo 2.<br><br>";
          s += "<input type=\"submit\" value=\"OK!\"></form></center>";
          s += "</body>";
      
        }

        if (val == -3) {
          // Se imprimen en una pagina web los valores de temperatura y humedad 
          // obtenidos previamente por el Dispositivo 1, cliente ESP8266
          s += "<head><title>Mediciones de dispositivo 1.</title>";
          s += "<meta http-equiv=\"refresh\" content=\"1\"></head>"; //Funcion que realiza el refrescamiento cada segundo
          s += "<center><h2> Variables sensadas por Dispositivo 1</h2>";
          s += "Temperatura = " + String(tmp_dev1) + "°<br>"; // Go to the next line.
          s += "Humedad = " + String(hmd_dev1) + "%";
          s += "<p><em> La pagina se actualiza cada segundo.</em></p></body></center>";
        }
        if (val == -4) {
          // Se imprimen en una pagina web los valores de temperatura y humedad 
          // obtenidos previamente por el Dispositivo 1, cliente ESP8266
          s += "<head><title>Mediciones de dispositivo 2.</title>";
          s += "<meta http-equiv=\"refresh\" content=\"1\"></head>"; //Funcion que realiza el refrescamiento cada segundo
          s += "<center><h2> Variables sensadas por Dispositivo 2</h2>";
          s += "Temperatura = " + String(tmp_dev2) + "°<br>";
          s += "Humedad = " +  String(hmd_dev2) + "%";
          s += "<p><em> La pagina se actualiza cada segundo.</em></p></body></center>";
        }
        s += "</html>\n";
        }
        else
        {
          s += "Invalid Request.<br> Try /led/1, /led/0, or /read.";
        }


  // Se envia la respuesta a nuestro cliente
  client.print(s);
  delay(1);
  Serial.println("Cliente desconectado");
  Serial.println("");
  Serial.println("");
}

void decoder_values(String _req){

  //Se decodifican las dos variables "value" que los dispositivos ESP8266 nos envian
  int Start1 = _req.indexOf("=");
  int Finish1 = _req.indexOf('&', Start1 + 1);
  int Start2 = _req.indexOf("=", Finish1 + 1);
  int Finish2 = _req.indexOf("/", Start2 + 1);

  value1 = "";
  value2 = "";
  
  for (int i = Start1 + 1; i < Finish1; i++)
  {
    value1 = value1 + _req.charAt(i);
  }
  for (int i = Start2 + 1; i < Finish2; i++)
  {
    value2 = value2 + _req.charAt(i);
  }
}

void setupWiFi() {

  //Se configura el dispositivo como punto de acceso y estacion
  WiFi.mode(WIFI_AP_STA);

  // Se obtiene la macID para nombrar nuestro dispositivo
  // Nuestra SSID sera ESP8266 Thing + macID
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) + String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, AP_NameString.length() + 1, 0);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  // Se crea el punto de acceso con el nombre ESP8266 Thing + macID 
  // con la contraseña setisaedu
  WiFi.softAP(AP_NameChar, WiFiAPPSK);

  // WiFI.begin([ssid], [passkey]) inicializa una conexion WiFI
  // para el ID [ssid], utilizando el [passkey] como una contraseña 
  // WPA, WPA2,o WEP.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Se espera a que nuestro ESP8266 se conecte a la red Wifi local
  byte ledStatus = LOW;
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    delay(100);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void initHardware() {
  //Se inicializa el puerto serial a 115200 baudios
  Serial.begin(115200);

  //Se configuran los pines fisicos de entradas y salidas
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}
