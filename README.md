# ENIT EcoCar Communication System

## Overview

This project is part of the **ENIT EcoCar** initiative for the **Eco Shell Marathon**, focused on creating an energy-efficient electric vehicle. The **communication system** enables seamless interaction between the driver and the external team using an **ESP32 with SIM800L module**.

### Key Features

- 📩 **Receive & Display SMS**: External team messages appear on an OLED screen for the driver.
- ✅ **Yes/No Responses**: Two buttons allow the driver to send quick "YES" or "NO" replies via SMS.
- 🌡 **Temperature Monitoring**: Sends motor and battery temperatures via SMS every minute.

---

## Project Structure

This project is divided into three Arduino sketches:

| File                  | Functionality |
|-----------------------|---------------|
| `affichage_sms_oled.ino` | Receives SMS and displays messages on the OLED screen |
| `yes_no_sms.ino`         | Sends "YES" or "NO" SMS messages via two buttons |
| `temperature_sim.ino`    | Reads temperature and sends periodic SMS updates |

---

## Hardware Requirements

- **TTGO T-Call ESP32 SIM800L**: ESP32 board with GSM modem.
- **SSD1306 OLED Display (128x64, I2C)**  
  - SDA: GPIO 21  
  - SCL: GPIO 22  
  - Voltage: 3.3V
- **LM35 Temperature Sensor**  
  - Analog output to GPIO 35
- **Push Buttons (x2)**  
  - "YES" on GPIO 13  
  - "NO" on GPIO 27  
  - With pull-up resistors
- **SIM Card** with active SMS plan
- **Power Supply**: USB or battery (IP5306 supported)

---

## Pin Configuration

| Component       | GPIO Pins           |
|-----------------|---------------------|
| OLED (I2C)      | SDA: GPIO 21, SCL: GPIO 22 |
| SIM800L         | RX: GPIO 26, TX: GPIO 27 |
| LM35 Sensor     | GPIO 35 (Analog)    |
| Button "YES"    | GPIO 13             |
| Button "NO"     | GPIO 27             |
| Modem Control   | PWKEY: GPIO 4, RST: GPIO 5, POWER_ON: GPIO 23 |

---

## Software Requirements

- **Arduino IDE** with ESP32 board support
- **Libraries** (install via Library Manager):
  - `TinyGSM` — for GSM communication
  - `Adafruit_GFX` & `Adafruit_SSD1306` — for OLED display
  - `Wire` — I2C communication support

Ensure to select the **TTGO T-Call ESP32** board in the Arduino IDE.

---

## Setup Instructions

### 🔌 Hardware Setup

1. Connect OLED, LM35, and buttons according to the pin configuration.
2. Insert a valid SIM card into the SIM800L slot.
3. Power the TTGO T-Call via USB or battery.

### 💻 Software Setup

1. Install Arduino IDE and add ESP32 board support.
2. Install required libraries listed above.
3. Clone or download this repository.
4. Open each `.ino` file in the Arduino IDE.

### ⚙️ Configuration

- Update `simPIN` in each sketch if your SIM card requires a PIN (default: `"0000"`).
- Set `SMS_TARGET` to the recipient phone number in:
  - `yes_no_sms.ino`
  - `temperature_sim.ino`
- Adjust `offsetVoltage` in `temperature_sim.ino` for LM35 calibration (default: `1.75`).

### 🚀 Uploading

- Upload each sketch to the TTGO T-Call board.
- Use separate boards for each sketch **or** merge sketches (see *Future Improvements*).

### 🧪 Testing

- Open Serial Monitor (115200 baud) to view logs.
- Send an SMS to the SIM number and check OLED.
- Press buttons to test "YES"/"NO" responses.
- Monitor temperature SMS delivery every 60 seconds.

---

## Usage

### 📥 Receiving Messages

- External team sends SMS to SIM card number.
- Message is displayed on the OLED screen.
- The driver acknowledges by pressing a response button.

### 📤 Responding to Messages

- Press GPIO 13 to send **"YES"**.
- Press GPIO 27 to send **"NO"**.

### 🌡 Temperature Monitoring

- Sends temperature to `SMS_TARGET` every 60 seconds.
- Serial Monitor shows debug logs and signal quality.

---

## Future Improvements

- 🔀 **Merge Sketches**: Use task scheduling (e.g., FreeRTOS) to integrate all features into one sketch.
- 🔘 **Multiple Responses**: Add more button-based responses.
- 🔄 **Robustness**: Improve GSM error handling and reconnection logic.
- 🔋 **Power Saving**: Enable ESP32 sleep modes to optimize energy usage.
- 💾 **Data Logging**: Log temperature data to SD card or EEPROM for race analysis.

---

## Contributing

Contributions are welcome! Follow these steps:

```bash
git clone https://github.com/your-username/enit-ecocar-communication-system.git
cd enit-ecocar-communication
