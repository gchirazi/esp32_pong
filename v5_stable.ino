#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Bluetooth
BluetoothSerial SerialBT;

// TFT
#define TFT_CS     15
#define TFT_RST    4
#define TFT_DC     2
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Butoane
#define BTN1_UP    32
#define BTN1_DOWN  33
#define BTN2_UP    25
#define BTN2_DOWN  26

// Joc
int paddleHeight = 20;
int paddleWidth = 3;
int paddleSpeed = 2;
int screenW = 128;
int screenH = 160;
int zonaTop = 15; // Scor si nume

int paddle1Y, paddle2Y;
int ballX, ballY;
int ballSize = 3;
int ballSpeedX = 1;
int ballSpeedY = 1;

String player1 = "Player1";
String player2 = "Player2";
int scor1 = 0;
int scor2 = 0;

bool gameRunning = false;

void countdown(int secunde, String mesaj) {
  for (int i = secunde; i > 0; i--) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 40);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.print(mesaj);
    tft.setCursor(40, 80);
    tft.print(i);
    delay(1000);
  }
}

void asteaptaNumeJucatori() {
  bool gata1 = false, gata2 = false;

  while (!gata1 || !gata2) {
    tft.fillScreen(ST77XX_BLACK);

    // Titlu
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 10);
    tft.print("PING-PONG\n CHAMPIONS");

    // Jucator 1
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 50);
    tft.print("Player 1:");
    tft.setCursor(10, 60);
    tft.setTextColor(gata1 ? ST77XX_GREEN : ST77XX_RED);
    tft.print(player1);
    tft.setCursor(10, 70);
    tft.print(gata1 ? "[OK]" : "[Asteptat]");

    // Jucator 2
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, 100);
    tft.print("Player 2:");
    tft.setCursor(10, 110);
    tft.setTextColor(gata2 ? ST77XX_GREEN : ST77XX_RED);
    tft.print(player2);
    tft.setCursor(10, 120);
    tft.print(gata2 ? "[OK]" : "[Asteptat]");

    // Citire date Bluetooth
    if (SerialBT.available()) {
      String linie = SerialBT.readStringUntil('\n');
      linie.trim();
      if (linie.startsWith("1:")) {
        player1 = linie.substring(2);
        gata1 = true;
        SerialBT.println("OK, Player1 = " + player1);
      }
      if (linie.startsWith("2:")) {
        player2 = linie.substring(2);
        gata2 = true;
        SerialBT.println("OK, Player2 = " + player2);
        //delay(1000);
      }
    }

    delay(200);
  }
  // tft.setTextColor(ST77XX_WHITE);
  // tft.setCursor(10, 100);
  // tft.print("Player 2:");
  // tft.setCursor(10, 110);
  // tft.setTextColor(gata2 ? ST77XX_GREEN : ST77XX_RED);
  // tft.print(player2);
  // tft.setCursor(10, 120);
  // tft.print(gata2 ? "[OK]" : "[Asteptat]");
  // delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 50);
  tft.print("Succes!\n");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 80);
  tft.print(player1 + " vs. " + player2);
  delay(1500);
  countdown(3, "Start in");
  gameRunning = true;
}

void resetJoc() {
  paddle1Y = zonaTop + (screenH - zonaTop) / 2 - paddleHeight / 2;
  paddle2Y = paddle1Y;
  ballX = screenW / 2;
  ballY = zonaTop + (screenH - zonaTop) / 2;
  ballSpeedX = (random(0, 2) == 0) ? 2 : -2;
  ballSpeedY = (random(0, 2) == 0) ? 1 : -1;
}

void afiseazaCastigator(String nume) {
  gameRunning = false;

  if (nume == player1) scor1++;
  else if (nume == player2) scor2++;

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 50);
  tft.print(nume + "\na castigat");
  delay(2000);
  countdown(3, "Start in");
  gameRunning = true;
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32-PONG");

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);

  pinMode(BTN1_UP, INPUT_PULLUP);
  pinMode(BTN1_DOWN, INPUT_PULLUP);
  pinMode(BTN2_UP, INPUT_PULLUP);
  pinMode(BTN2_DOWN, INPUT_PULLUP);

  randomSeed(millis());

  asteaptaNumeJucatori();
  resetJoc();
}

void loop() {
  if (!gameRunning) return;

  // Control palete
  if (digitalRead(BTN1_UP) == LOW && paddle1Y > zonaTop) paddle1Y -= paddleSpeed;
  if (digitalRead(BTN1_DOWN) == LOW && paddle1Y < screenH - paddleHeight) paddle1Y += paddleSpeed;
  if (digitalRead(BTN2_UP) == LOW && paddle2Y > zonaTop) paddle2Y -= paddleSpeed;
  if (digitalRead(BTN2_DOWN) == LOW && paddle2Y < screenH - paddleHeight) paddle2Y += paddleSpeed;

  // Miscare minge
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Coliziuni margini
  if (ballY <= zonaTop || ballY >= screenH - ballSize) ballSpeedY = -ballSpeedY;

  // Coliziuni palete
  if (ballX <= paddleWidth + 2 && ballY + ballSize >= paddle1Y && ballY <= paddle1Y + paddleHeight) {
    ballSpeedX = -ballSpeedX;
  }
  if (ballX >= screenW - paddleWidth - ballSize - 2 && ballY + ballSize >= paddle2Y && ballY <= paddle2Y + paddleHeight) {
    ballSpeedX = -ballSpeedX;
  }

  // Verifică pierderi
  if (ballX <= 0) {
    afiseazaCastigator(player2);
    resetJoc();
    return;
  }
  if (ballX >= screenW) {
    afiseazaCastigator(player1);
    resetJoc();
    return;
  }

  // Desenare
  tft.fillScreen(ST77XX_BLACK);

  // Nume + scor
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 2);
  tft.print(player1);

  String scorText = String(scor1) + " - " + String(scor2);
  int scorWidth = scorText.length() * 6;
  tft.setCursor((screenW - scorWidth) / 2, 2);
  tft.print(scorText);

  tft.setCursor(screenW - (player2.length() * 6) - 5, 2);
  tft.print(player2);

  // Linie verticală sub zona de scor
  tft.drawLine(0, zonaTop, screenW, zonaTop, ST77XX_WHITE);

  // Palete si minge
  tft.fillRect(2, paddle1Y, paddleWidth, paddleHeight, ST77XX_WHITE);
  tft.fillRect(screenW - paddleWidth - 2, paddle2Y, paddleWidth, paddleHeight, ST77XX_WHITE);
  tft.fillRect(ballX, ballY, ballSize, ballSize, ST77XX_YELLOW);

  delay(12); // Refresh rate
}
