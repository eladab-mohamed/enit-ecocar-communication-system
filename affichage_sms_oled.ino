

//code y affichi ala baby ecran oled bl esp32 l sms li youslelha
/*

| OLED Pin | TTGO T-Call ESP32 |
|----------|------------------|
| VCC      | **3.3V**          |
| GND      | GND               |
| SDA      | **GPIO 21**       |
| SCL      | **GPIO 22**       |

*/





#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGsmClient.h>

// Configuration OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Configuration SIM800L
const char simPIN[] = "0000";  // PIN SIM si nécessaire

// Pins TTGO T-Call
#define MODEM_RST 5
#define MODEM_PWKEY 4
#define MODEM_POWER_ON 23
#define MODEM_TX 27
#define MODEM_RX 26
#define I2C_SDA 21
#define I2C_SCL 22
#define SerialMon Serial
#define SerialAT Serial1

TinyGsm modem(SerialAT);

// Fonction pour envoyer des commandes AT et attendre la réponse
String sendATCommand(const char* command, unsigned long timeout = 1000) {
  SerialMon.print("Envoi commande: ");
  SerialMon.println(command);
  
  SerialAT.println(command);
  
  String response = "";
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
      delay(1); // Court délai pour stabiliser la lecture
    }
    // Si nous avons reçu "OK" ou "ERROR", sortir de la boucle
    if (response.endsWith("OK\r\n") || response.endsWith("ERROR\r\n")) {
      break;
    }
  }
  
  SerialMon.print("Réponse: ");
  SerialMon.println(response);
  return response;
}

void setup() {
  SerialMon.begin(115200);
  SerialMon.println("Démarrage du programme");
  delay(1000);
  
  // Initialisation OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    SerialMon.println("Échec OLED !");
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Initialisation...");
  display.display();
  
  // Initialisation matérielle du modem
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);
  
  // Reset le modem et attendre qu'il soit prêt
  SerialMon.println("Redémarrage du modem...");
  digitalWrite(MODEM_RST, LOW);
  delay(100);
  digitalWrite(MODEM_RST, HIGH);
  delay(3000);  // Attendre que le modem démarre
  
  // Configuration de base du modem via commandes AT directes
  sendATCommand("AT", 1000);  // Test de communication
  sendATCommand("ATE0", 1000);  // Désactiver l'écho (plus propre pour analyser les réponses)
  
  // TRÈS IMPORTANT: Configurer en mode TEXTE (pas PDU)
  sendATCommand("AT+CMGF=1", 1000);
  
  // Configuration de l'encodage des caractères
  sendATCommand("AT+CSCS=\"GSM\"", 1000);
  
  // Configuration pour la notification des nouveaux SMS
  sendATCommand("AT+CNMI=2,1,0,0,0", 1000);
  
  // Déverrouiller la SIM si nécessaire
  if (strlen(simPIN) > 0) {
    String pinCommand = "AT+CPIN=";
    pinCommand += simPIN;
    sendATCommand(pinCommand.c_str(), 1000);
  }
  
  // Vérifier la qualité du signal
  String signalQuality = sendATCommand("AT+CSQ", 1000);
  SerialMon.print("Qualité du signal: ");
  SerialMon.println(signalQuality);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Prêt à recevoir");
  display.println("des SMS...");
  display.display();
  
  SerialMon.println("Système initialisé");
}

void loop() {
  // Lire les données disponibles du modem
  String notification = "";
  while (SerialAT.available()) {
    char c = SerialAT.read();
    notification += c;
    delay(1);  // Court délai pour stabiliser la lecture
  }
  
  // Traiter la notification si elle existe
  if (notification.length() > 0) {
    SerialMon.print("Données reçues: ");
    SerialMon.println(notification);
    
    // Vérifier si c'est une notification de nouveau SMS
    if (notification.indexOf("+CMTI:") >= 0) {
      // Extraire l'index du message
      int commaPos = notification.indexOf(",");
      if (commaPos > 0) {
        int smsIndex = notification.substring(commaPos + 1).toInt();
        SerialMon.print("Nouveau SMS à l'index: ");
        SerialMon.println(smsIndex);
        processSMS(smsIndex);
      }
    }
  }
  
  delay(100);  // Court délai pour éviter de surcharger le CPU
}

void processSMS(int smsIndex) {
  // Forcer le mode texte avant de lire
  sendATCommand("AT+CMGF=1", 1000);
  
  // Construire la commande pour lire le SMS
  String readCmd = "AT+CMGR=";
  readCmd += smsIndex;
  
  // Lire le SMS
  String smsResponse = sendATCommand(readCmd.c_str(), 3000);
  
  // Vérifier si nous avons une réponse valide
  if (smsResponse.indexOf("+CMGR:") >= 0) {
    SerialMon.println("Traitement du contenu SMS...");
    
    // Découper la chaîne pour isoler le message
    int headerEndPos = smsResponse.indexOf("\r\n", smsResponse.indexOf("+CMGR:"));
    // Supprimer "OK" s'il est présent à la fin
    smsResponse.replace("OK\r\n", "");
    smsResponse.trim();

    // Rechercher de nouveau les délimiteurs après nettoyage
    int messageStart = smsResponse.indexOf("\r\n", smsResponse.indexOf("+CMGR:")) + 2;

    if (messageStart > 0 && messageStart < smsResponse.length()) {
      String messageContent = smsResponse.substring(messageStart);
      messageContent.trim();

          
      // Afficher sur le moniteur série
      SerialMon.println("Contenu du message:");
      SerialMon.println(messageContent);
      
      // Afficher sur l'écran OLED
      displayMessage(messageContent);
    }
    
    // Supprimer le message lu
    String deleteCmd = "AT+CMGD=";
    deleteCmd += smsIndex;
    sendATCommand(deleteCmd.c_str(), 1000);
  }
}

void displayMessage(String message) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // 2. Diviser le message en mots
  std::vector<String> words;
  int lastIndex = 0;
  while (lastIndex < message.length()) {
    int spaceIndex = message.indexOf(' ', lastIndex);
    if (spaceIndex == -1) spaceIndex = message.length();
    String word = message.substring(lastIndex, spaceIndex);
    word.trim();
    if (word.length() > 0) words.push_back(word);
    lastIndex = spaceIndex + 1;
  }
  
  // Pour chaque mot ou groupe de mots, trouver la taille de texte appropriée
  int scrollSpeed = 1100; // Millisecondes
  
  for (size_t i = 0; i < words.size();) {
    display.clearDisplay();
    
    // Commencer par un mot
    String segment = words[i];
    size_t wordCount = 1;
    
    // Déterminer la taille de texte maximale pour ce segment
    int textSize = 4;  // Commencer avec la taille maximale
    bool fitFound = false;
    
    while (!fitFound && textSize >= 1) {
      int charWidth = 6 * textSize;
      int charHeight = 8 * textSize;
      
      // Vérifier si le segment tient horizontalement avec cette taille
      if (segment.length() * charWidth <= SCREEN_WIDTH && charHeight <= SCREEN_HEIGHT) {
        fitFound = true;
      } else {
        textSize--;  // Réduire la taille si ça ne tient pas
      }
    }
    
    // Si même avec la plus petite taille, le mot est trop long, on peut ajouter
    // une logique pour le faire défiler horizontalement (optionnel)
    if (!fitFound) {
      textSize = 1;  // Utiliser la plus petite taille possible
    }
    
    // Appliquer la taille de texte déterminée
    display.setTextSize(textSize);
    int charWidth = 6 * textSize;
    int charHeight = 8 * textSize;
    
    // Ajouter d'autres mots si possible avec cette taille de texte
    while (i + wordCount < words.size()) {
      String testSegment = segment + " " + words[i + wordCount];
      if (testSegment.length() * charWidth > SCREEN_WIDTH) {
        break;  // Ce mot supplémentaire ne tiendrait pas
      }
      segment = testSegment;
      wordCount++;
    }
    
    // Centrer et afficher
    int textWidth = segment.length() * charWidth;
    int x = max(0, (SCREEN_WIDTH - textWidth) / 2);
    int y = (SCREEN_HEIGHT - charHeight) / 2;
    
    display.setCursor(x, y);
    display.print(segment);
    display.display();
    delay(scrollSpeed);
    
    // Passer aux mots suivants
    i += wordCount;
  }
}