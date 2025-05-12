// Définition du modem SIM800 - DOIT ÊTRE AVANT les includes
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

// Librairies nécessaires
#include <Wire.h>
#include <TinyGsmClient.h>

// Configuration pour le capteur LM35
const int lm35Pin = 35;
float offsetVoltage = 1.75; // Ajustez cette valeur en fonction de vos tests

// Configuration pour le module SIM800L
const char simPIN[] = "0000"; // Laissez vide si pas de PIN
#define SMS_TARGET "+21621324404" // Votre numéro de téléphone

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

void setup() {
  // Initialisation du port série pour le débogage
  SerialMon.begin(115200);
  delay(1000);
  SerialMon.println("Démarrage du programme...");
  
  // Configuration du capteur de température
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(lm35Pin, INPUT);
  
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
  SerialMon.println("Vérification du modem...");
  
  // Vérifier l'état de la carte SIM
  SerialMon.print("Statut de la carte SIM: ");
  int simStatus = modem.getSimStatus();
  switch (simStatus) {
    case 0: SerialMon.println("Erreur"); break;
    case 1: SerialMon.println("Verrouillée par PIN"); break;
    case 2: SerialMon.println("Verrouillée par PUK"); break;
    case 3: SerialMon.println("Prête"); break;
    default: SerialMon.println("Statut inconnu"); break;
  }
  
  // Vérifier la qualité du signal
  SerialMon.print("Qualité du signal: ");
  int rssi = modem.getSignalQuality();
  SerialMon.print(rssi);
  SerialMon.println(" (0-31, 31 est le meilleur)");
  
  // Vérifier si le modem est enregistré sur le réseau
  SerialMon.print("Statut du réseau: ");
  int regStatus = modem.getRegistrationStatus();
  switch (regStatus) {
    case 0: SerialMon.println("Non enregistré"); break;
    case 1: SerialMon.println("Enregistré (réseau domestique)"); break;
    case 2: SerialMon.println("Recherche de réseau"); break;
    case 3: SerialMon.println("Enregistrement refusé"); break;
    case 4: SerialMon.println("Inconnu"); break;
    case 5: SerialMon.println("Enregistré (roaming)"); break;
    default: SerialMon.println("Statut inconnu"); break;
  }
  
  // Vérifier l'opérateur
  String operator_name = modem.getOperator();
  SerialMon.print("Opérateur: ");
  SerialMon.println(operator_name);
  
  SerialMon.println("Le système va maintenant envoyer la température par SMS toutes les minutes.");
}

void loop() {
  // Lecture de la température
  int adcValue = analogRead(lm35Pin);
  float voltage = adcValue * (3.3 / 4095.0) - offsetVoltage;
  float temperatureC = voltage * 100.0;
  
  // Affichage de la température sur le moniteur série
  SerialMon.print("Température : ");
  SerialMon.print(temperatureC, 1);
  SerialMon.println(" °C");
  
  // Vérifier à nouveau la qualité du signal avant l'envoi
  int rssi = modem.getSignalQuality();
  SerialMon.print("Qualité du signal avant envoi: ");
  SerialMon.print(rssi);
  SerialMon.println(" (0-31)");
  
  if (rssi > 0) {
    // Préparation et envoi du SMS
    String smsMessage = "Temperature actuelle: " + String(temperatureC, 1) + " C";
    SerialMon.println("Envoi du SMS...");
    
    bool smsResult = modem.sendSMS(SMS_TARGET, smsMessage);
    if (smsResult) {
      SerialMon.println("SMS envoyé avec succès!");
    } else {
      SerialMon.println("Échec de l'envoi du SMS");
      
      // Essayer de redémarrer le modem en cas d'échec
      SerialMon.println("Tentative de redémarrage du modem...");
      modem.restart();
      delay(10000);
    }
  } else {
    SerialMon.println("Signal trop faible pour envoyer un SMS. Veuillez vérifier l'antenne.");
  }
  
  // Attendre 60 secondes avant la prochaine lecture/envoi
  SerialMon.println("Attente de 60 secondes...");
  delay(60000);
}