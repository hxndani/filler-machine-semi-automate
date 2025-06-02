#include <LiquidCrystal_I2C.h> //library LCD 
#include <EEPROM.h> //library EEPROM

// Konstanta
#define konstantaChanger 10 // 0,01 s untuk timer (10ms)

// Alamat EEPROM
const int timerAddress = 0;
const int speedMotorPwmAddress = sizeof(unsigned long); // Alamat untuk menyimpan kecepatan PWM setelah timer

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pinout
const byte ssrPin = PB11; // Pin yang terhubung ke SSR untuk kontrol solenoid (buka/tutup keran)
const byte mosfetPin = PB1;       // Pin PWM untuk kontrol Pompa Air
const byte buttonPowerPin = PB15; // Untuk mulai/mati
const byte buttonPlusPin = PB14;  // Tambah waktu/timer atau kecepatan motor pompa
const byte buttonMinPin = PB13;   // Kurang waktu/timer atau kecepatan motor pompa
const byte buttonSavePin = PB12;  // Untuk menyimpan timer/kecepatan motor pompa
const byte buttonMenuPin = PB3;   // Button untuk ganti menu

// Variabel Timer
unsigned long defaultSetTimer = 60000; // Waktu default (ms) - 60 detik
unsigned long currentSetTimer;        // Timer yang aktif saat ini

volatile unsigned long currentTimer = 0;
volatile bool isFilling = false;

// Variabel Kecepatan Motor PWM
byte defaultSpeedMotorPwm = 64; // Nilai PWM default (25% dari 255)
byte currentSpeedMotorPwm;      // Kecepatan PWM yang aktif saat ini

// Variabel Tombol
unsigned long lastDebounceTimePower = 0; 
unsigned long lastDebounceTimePlusMin = 0; 
unsigned long lastDebounceTimeSave = 0; 
unsigned long lastDebounceTimeMenu = 0; 
unsigned long lastContinuousPressTime = 0;

const unsigned long debounceDelayPower = 200;      // Debounce untuk tombol power
const unsigned long debounceDelayPlusMin = 50;     // Debounce untuk tombol plus/min
const unsigned long debounceDelayMenu = 250;       // Debounce untuk tombol menu
const unsigned long incrementDecrementInterval = 150; // Interval untuk continuous press

// Variabel Pesan "Saved!"
bool isDisplayingSaveMessage = false;
unsigned long saveMessageStartTime = 0;
const unsigned long saveMessageDuration = 1000; // Durasi pesan "Saved" (ms)

// State Menu: 0 = Timer, 1 = Kecepatan Motor
byte currentMenu = 0;
unsigned long previewSetTimer = 0; // Untuk menyimpan nilai timer sementara saat di edit
byte previewSpeedMotorPwm = 0;     // Untuk menyimpan nilai speed motor sementara saat di edit


void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(ssrPin, OUTPUT);
  pinMode(mosfetPin, OUTPUT);
  pinMode(buttonPowerPin, INPUT_PULLUP);
  pinMode(buttonPlusPin, INPUT_PULLUP);
  pinMode(buttonMinPin, INPUT_PULLUP);  
  pinMode(buttonSavePin, INPUT_PULLUP);
  pinMode(buttonMenuPin, INPUT_PULLUP);

  digitalWrite(ssrPin, LOW);
  analogWrite(mosfetPin, 0); // Matiin motor saat diawal

  // Baca settingan dari EEPROM
  EEPROM.get(timerAddress, currentSetTimer);
  if (currentSetTimer == 0xFFFFFFFF || currentSetTimer == 0) { // Kalo EEPROM kosong atau nilai invalid
    currentSetTimer = defaultSetTimer;
    EEPROM.put(timerAddress, currentSetTimer);
  }

  EEPROM.get(speedMotorPwmAddress, currentSpeedMotorPwm);
  if (currentSpeedMotorPwm == 0xFF || currentSpeedMotorPwm == 0) { // Kalo EEPROM kosong atau nilai invalid
    currentSpeedMotorPwm = defaultSpeedMotorPwm;
    EEPROM.put(speedMotorPwmAddress, currentSpeedMotorPwm);
  }

  // Set nilai preview sama dengan nilai yang tersimpan
  previewSetTimer = currentSetTimer;
  previewSpeedMotorPwm = currentSpeedMotorPwm;

  Serial.begin(9600);
  Serial.print("Set Timer (ms): ");
  Serial.println(currentSetTimer);
  Serial.print("Speed Motor PWM: ");
  Serial.println(currentSpeedMotorPwm);

  displayCurrentMenu(); // Tampilin menu di awal
}

void loop() {
  handlePowerButton();
  handlePlusButton();
  handleMinusButton();
  handleSaveButton();
  handleMenuButton();

  // Tampilin pesan "Saved!" kalo flag aktif
  if (isDisplayingSaveMessage) {
    lcd.setCursor(0, 0);
    lcd.print("    Saved!      ");
    lcd.setCursor(0, 1);
    lcd.print("                "); // Kosongkan baris kedua
    if (millis() - saveMessageStartTime >= saveMessageDuration) {
      isDisplayingSaveMessage = false;
      displayCurrentMenu(); // Tampilin menu lagi kalo pesan udah hilang
    }
  } else {
    // Jika sedang mengisi, tampilin countdown. Jika tidak, tampilin menu.
    if (isFilling) {
      if (millis() - currentTimer >= currentSetTimer) {
        stopFilling();
      }
      // Saat mengisi, selalu tampilin countdown timer
      displayCountdown((currentSetTimer - (millis() - currentTimer)) / 1000.0);
    } else {
      displayCurrentMenu(); // Selalu tampilin menu saat tidak mengisi
    }
  }
}

void handlePowerButton() {
  int readingPower = digitalRead(buttonPowerPin);
  if (readingPower == LOW && (millis() - lastDebounceTimePower > debounceDelayPower)) {
    lastDebounceTimePower = millis();
    if (!isFilling) {
      // Baca ulang timer dan speed motor dari EEPROM setiap kali tombol power ditekan saat tidak mengisi
      EEPROM.get(timerAddress, currentSetTimer);
      EEPROM.get(speedMotorPwmAddress, currentSpeedMotorPwm);
      currentMenu = 0; // **PAKSA MENU KE TIMER SAAT MULAI MENGISI**
      startFilling();
    } else {
      stopFilling();
    }
  }
}

void handlePlusButton() {
  int readingPlus = digitalRead(buttonPlusPin);
  if (readingPlus == LOW) {
    if (!isFilling) { // Hanya bisa diatur saat tidak filling
      if (millis() - lastDebounceTimePlusMin > debounceDelayPlusMin) {
        if (currentMenu == 0) { // Menu Timer
          previewSetTimer += konstantaChanger;
          if (previewSetTimer > 3600000) previewSetTimer = 3600000; // Maksimal 1 jam (3.600.000 ms)
        } else if (currentMenu == 1) { // Menu Kecepatan Motor
          previewSpeedMotorPwm++;
          if (previewSpeedMotorPwm > 255) previewSpeedMotorPwm = 255;
        }
        lastDebounceTimePlusMin = millis();
        lastContinuousPressTime = millis(); // Set waktu untuk continuous press
        displayCurrentMenu();
      } else if (millis() - lastContinuousPressTime > incrementDecrementInterval) {
        // Continuous press
        if (currentMenu == 0) { // Menu Timer
          previewSetTimer += konstantaChanger;
          if (previewSetTimer > 3600000) previewSetTimer = 3600000;
        } else if (currentMenu == 1) { // Menu Kecepatan Motor
          previewSpeedMotorPwm++;
          if (previewSpeedMotorPwm > 255) previewSpeedMotorPwm = 255;
        }
        lastContinuousPressTime = millis();
        displayCurrentMenu();
      }
    }
  } else {
    lastContinuousPressTime = 0; // Reset continuous press timer
  }
}

void handleMinusButton() {
  int readingMin = digitalRead(buttonMinPin);
  if (readingMin == LOW) {
    if (!isFilling) { // Hanya bisa diatur saat tidak filling
      if (millis() - lastDebounceTimePlusMin > debounceDelayPlusMin) {
        if (currentMenu == 0) { // Menu Timer
          if (previewSetTimer >= konstantaChanger) {
            previewSetTimer -= konstantaChanger;
          } else {
            previewSetTimer = 0;
          }
        } else if (currentMenu == 1) { // Menu Kecepatan Motor
          if (previewSpeedMotorPwm > 0) {
            previewSpeedMotorPwm--;
          } else {
            previewSpeedMotorPwm = 0;
          }
        }
        lastDebounceTimePlusMin = millis();
        lastContinuousPressTime = millis(); // Set waktu untuk continuous press
        displayCurrentMenu();
      } else if (millis() - lastContinuousPressTime > incrementDecrementInterval) {
        // Continuous press
        if (currentMenu == 0) { // Menu Timer
          if (previewSetTimer >= konstantaChanger) {
            previewSetTimer -= konstantaChanger;
          } else {
            previewSetTimer = 0;
          }
        } else if (currentMenu == 1) { // Menu Kecepatan Motor
          if (previewSpeedMotorPwm > 0) {
            previewSpeedMotorPwm--;
          } else {
            previewSpeedMotorPwm = 0;
          }
        }
        lastContinuousPressTime = millis();
        displayCurrentMenu();
      }
    }
  } else {
    lastContinuousPressTime = 0; // Reset continuous press timer
  }
}

void handleSaveButton() {
  int readingSave = digitalRead(buttonSavePin);
  if (readingSave == LOW && (millis() - lastDebounceTimeSave > debounceDelayPlusMin)) {
    lastDebounceTimeSave = millis();
    if (!isFilling) { // Hanya bisa disimpan saat tidak filling
      if (currentMenu == 0) { // Simpan Timer
        currentSetTimer = previewSetTimer;
        EEPROM.put(timerAddress, currentSetTimer);
        Serial.println("Timer saved: " + String(currentSetTimer));
      } else if (currentMenu == 1) { // Simpan Kecepatan Motor
        currentSpeedMotorPwm = previewSpeedMotorPwm;
        EEPROM.put(speedMotorPwmAddress, currentSpeedMotorPwm);
        Serial.println("Speed Motor saved: " + String(currentSpeedMotorPwm));
      }
      isDisplayingSaveMessage = true; // Set flag untuk menampilkan pesan "Saved"
      saveMessageStartTime = millis(); // Catat waktu mulai pesan
    }
  }
}

void handleMenuButton() {
  int readingMenu = digitalRead(buttonMenuPin);
  if (readingMenu == LOW && (millis() - lastDebounceTimeMenu > debounceDelayMenu)) {
    lastDebounceTimeMenu = millis();
    if (!isFilling) { // Hanya bisa ganti menu saat tidak filling
      currentMenu = (currentMenu + 1) % 2; // Ganti menu (0 ke 1, 1 ke 0)
      displayCurrentMenu();
    }
  }
}

void startFilling() {
  isFilling = true; //
  currentTimer = millis();
  digitalWrite(ssrPin, HIGH); // Buka keran (SSR aktif)
  analogWrite(mosfetPin, currentSpeedMotorPwm); // Pompa nyala dengan kecepatan yang tersimpan

  Serial.println("Filling started with timer: " + String(currentSetTimer) + " ms, Speed: " + String(currentSpeedMotorPwm) + " PWM");
}

void stopFilling() {
  isFilling = false;
  digitalWrite(ssrPin, LOW); // Tutup keran (SSR non-aktif)
  analogWrite(mosfetPin, 0);           // Pompa mati
  Serial.println("Filling stopped");
  displayCurrentMenu(); // Tampilkan menu setelah berhenti mengisi
}

void displayCurrentMenu() {
  // lcd.clear();
  if (currentMenu == 0) { // Menu Timer
    displaySetTimer(previewSetTimer / 1000.0);
    displayProgTimer(currentSetTimer / 1000.0); // Tampilkan nilai timer yang tersimpan di baris kedua
  } else if (currentMenu == 1) { // Menu Kecepatan Motor
    displaySetSpeedMotorPwm(previewSpeedMotorPwm);
    displayProgSpeedMotorPwm(currentSpeedMotorPwm); // Tampilkan nilai speed motor yang tersimpan di baris kedua
  }
}

void displaySetTimer(float time) {
  lcd.setCursor(0, 0);
  lcd.print("Set T: ");
  lcd.print(time, 1); 
  lcd.print(" s     ");
}

void displayProgTimer(float time) {
  lcd.setCursor(0, 1);
  lcd.print("Prog T: ");
  lcd.print(time, 1); 
  lcd.print(" s     ");
}

void displayCountdown(float time) {
  lcd.setCursor(0, 0); // Baris pertama selalu menampilkan "Timer:"
  lcd.print("Timer: ");
  lcd.setCursor(7, 0);
  lcd.print(time, 1);
  lcd.print(" s     ");
  lcd.setCursor(0, 1); // Baris kedua menampilkan "Speed:"
  float percentage = (float)currentSpeedMotorPwm / 255.0 * 100.0;
  lcd.print("Speed: ");
  lcd.print(percentage, 0);
  lcd.print(" %     ");
}

void displaySetSpeedMotorPwm(byte pwmValue) {
  float percentage = (float)pwmValue / 255.0 * 100.0;
  lcd.setCursor(0, 0);
  lcd.print("Set Spd: ");
  lcd.print(percentage, 2); 
  lcd.print(" %   ");
}

void displayProgSpeedMotorPwm(byte pwmValue) {
  float percentage = (float)pwmValue / 255.0 * 100.0;
  lcd.setCursor(0, 1);
  lcd.print("Prog Spd: ");
  lcd.print(percentage, 2); 
  lcd.print(" %   ");
}
