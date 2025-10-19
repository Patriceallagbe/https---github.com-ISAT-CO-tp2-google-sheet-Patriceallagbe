#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Configuration du réseau Wi-Fi
const char* ssid = "Proximus-Home-242271";
const char* password = "mz4ckna5zy6aj95b";

// Configuration du capteur DHT11
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Définition des broches utilisées
#define RED_PIN    12
#define GREEN_PIN  14
#define BLUE_PIN   27
#define BUTTON_PIN 25
#define LDR_PIN    34   // Entrée analogique pour la LDR

// Paramètres de l’écran OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Informations du script Google Apps Script
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbxhnaKRNccoYj7cvOu4syd5wjXmv-HIF53xQgWAuI7NH9cySowetNxr-nsZP32vtQJx";

// Objet pour la connexion HTTPS
WiFiClientSecure client;

// Fonction d’envoi des données au script Google
void sendData(float temp, float hum, int lumi) {
  client.setInsecure();  // Désactivation de la vérification du certificat SSL

  Serial.println("\nConnexion à Google...");
  if (!client.connect(host, httpsPort)) {
    Serial.println("Erreur : connexion échouée !");
    return;
  }

  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + String(temp, 1) +
               "&humidity=" + String(hum, 1) + "&luminosity=" + String(lumi);

  Serial.println("Requête envoyée : " + url);

  client.println(String("GET ") + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("User-Agent: ESP32_TP2");
  client.println("Connection: close");
  client.println();

  Serial.println("Données envoyées avec succès !");
  client.stop();
}

// Fonction de configuration initiale
void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  dht.begin();

  // Initialisation de l’écran OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erreur lors de l'initialisation de l'écran OLED !");
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Connexion WiFi...");
  display.display();

  // Connexion au Wi-Fi
  WiFi.mode(WIFI_STA);
  IPAddress dns(8,8,8,8);  // DNS Google pour éviter les erreurs DNS
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, dns);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecté !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
  display.println("WiFi connecté !");
  display.display();
  delay(1000);
  display.clearDisplay();
}

// Boucle principale
void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {  // Vérifie si le bouton est pressé
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int ldrValue = analogRead(LDR_PIN);
    int lumiPercent = map(ldrValue, 0, 4095, 0, 100);

    Serial.println("===== MESURES =====");
    Serial.printf("Température: %.1f°C | Humidité: %.1f%% | Luminosité: %d%%\n", temp, hum, lumiPercent);

    // Affichage des valeurs sur l’OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Temp: %.1f C\n", temp);
    display.printf("Hum : %.1f %%\n", hum);
    display.printf("Lumi: %d %%\n", lumiPercent);
    display.display();

    // Allumage de la LED bleue pendant 100 ms pour indiquer la transmission
    digitalWrite(BLUE_PIN, HIGH);
    sendData(temp, hum, lumiPercent);
    delay(100);
    digitalWrite(BLUE_PIN, LOW);

    // Message de confirmation sur l’écran
    display.setCursor(0,48);
    display.println("Donnees envoyees !");
    display.display();

    delay(1000); // Anti-rebond du bouton
  }
}
