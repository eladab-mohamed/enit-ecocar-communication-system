// Définition du modem SIM800 - DOIT ÊTRE AVANT les includes
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

// Librairies nécessaires
#include <Wire.h>
#include <TinyGsmClient.h>

// Configuration pour le module SIM800L
const char simPIN[] = "0000"; // Laissez vide si pas de PIN
#define SMS_TARGET "+21621324404" // Votre numéro de téléphone

// Configuration du bouton
#define BUTTON1_PIN 13 // Choisissez une broche GPIO libre pour le bouton
bool button1Pressed = false;

#define BUTTON2_PIN 27 // Choisissez une broche GPIO libre pour le bouton
bool button2Pressed = false;

// TTGO T-Call pins
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22

// Définition des ports série
#define SerialMon Serial
#define SerialAT Serial1

// Initialisation du modem
TinyGsm modem(SerialAT);

#define IP5306_ADDR 0x75
#define IP5306_REG_SYS_CTL0 0x00

// Fonction pour maintenir l'alimentation
bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

void sendSMS(char* mess) {
  // Vérifier la qualité du signal avant l'envoi
  int rssi = modem.getSignalQuality();
  SerialMon.print("Qualité du signal: ");
  SerialMon.print(rssi);
  SerialMon.println(" (0-31, 31 est le meilleur)");
  
  if (rssi > 0) {
    SerialMon.println("Envoi du SMS " + String(mess) + "...");
    bool smsResult = modem.sendSMS(SMS_TARGET, mess);
    if (smsResult) {
      SerialMon.println("SMS envoyé avec succès!");
    } else {
      SerialMon.println("Échec de l'envoi du SMS");
    }
  } else {
    SerialMon.println("Signal trop faible pour envoyer un SMS");
  }
}

void setup() {
  // Initialisation du port série pour le débogage
  SerialMon.begin(115200);
  delay(1000);
  SerialMon.println("Démarrage du programme...");
  
  // Configuration du bouton
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  
  // Maintenir l'alimentation en fonctionnant sur batterie
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  
  // Configuration des pins du modem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  // Configuration de la communication avec le module SIM800L
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);
  
  // Initialisation du modem
  SerialMon.println("Initialisation du modem...");
  modem.restart();
  delay(5000); // Attendre plus longtemps pour l'initialisation
  
  // Déverrouillage de la carte SIM si nécessaire
  if (strlen(simPIN) && modem.getSimStatus() != 3) {
    modem.simUnlock(simPIN);
  }
  
  // Vérification du statut du modem
  SerialMon.println("Attente d'appui sur le bouton pour envoyer un SMS...");
}

void loop() {
  // Vérifier l'état du bouton
  if (digitalRead(BUTTON1_PIN) == LOW) { // Bouton appuyé (LOW car PULLUP)
    if (!button1Pressed) {
      button1Pressed = true;
      SerialMon.println("Bouton appuyé - Envoi du SMS...");
      sendSMS("YES");
      delay(1000); // Anti-rebond
    }
  } else {
    button1Pressed = false;
  }
  
  if (digitalRead(BUTTON2_PIN) == LOW) { // Bouton appuyé (LOW car PULLUP)
    if (!button2Pressed) {
      button2Pressed = true;
      SerialMon.println("Bouton appuyé - Envoi du SMS...");
      sendSMS("NO");
      delay(1000); // Anti-rebond
    }
  } else {
    button2Pressed = false;
  }


  delay(100); // Petit délai pour éviter de surcharger le processeur
}