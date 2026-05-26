#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

#define TOUCH_PIN     2
#define ANGRY_TIMEOUT 600000UL
#define AFTERGLOW_MS  3000UL

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t state = 1;
unsigned long lastTouchMs = 0;
unsigned long afterglowEndMs = 0;

int osc(unsigned long t, unsigned long period, int amp) {
  if (period == 0) return 0;
  int phase = (t % period) * 2 * amp / period;
  if (phase > amp) phase = 2 * amp - phase;
  return phase - amp;
}

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) for (;;);
  display.clearDisplay();
  display.display();
  lastTouchMs = millis();
}

/* ==================== SUZUME CAT ==================== */
void drawCat(int x, int y, uint8_t s, unsigned long t) {
  int bounce = 0, tailSway = 0;

  if (s == 2) {
    bounce = abs(((t / 60) % 12) - 6);
    tailSway = osc(t, 120, 10);
  } else if (s == 3) {
    bounce = abs(((t / 90) % 8) - 4);
    tailSway = osc(t, 180, 6);
  } else if (s == 1) {
    bounce = osc(t, 400, 2);
    tailSway = osc(t, 500, 5);
  } else {
    bounce = 4;
    tailSway = 0;
  }

  int by = y + bounce;

  // --- BODY (slightly smaller so paws/tail show) ---
  display.fillRoundRect(x - 14, by - 2, 28, 20, 9, SSD1306_WHITE);
  display.fillCircle(x, by + 2, 10, SSD1306_WHITE);
  display.fillCircle(x - 8, by + 6, 7, SSD1306_WHITE);
  display.fillCircle(x + 8, by + 6, 7, SSD1306_WHITE);

  // --- PAWS (chunky, clearly below body) ---
  int pawY = by + 14;
  // Left paw
  display.fillRoundRect(x - 13, pawY, 8, 9, 4, SSD1306_WHITE);
  display.fillCircle(x - 9, pawY + 6, 2, SSD1306_BLACK); // toe bean
  // Right paw
  display.fillRoundRect(x + 5, pawY, 8, 9, 4, SSD1306_WHITE);
  display.fillCircle(x + 9, pawY + 6, 2, SSD1306_BLACK); // toe bean

  // --- TAIL (drawn AFTER body so it is visible, thick curled) ---
  int tx[6], ty[6];
  if (s == 0) { // Angry: tail down straight
    tx[0] = x + 14; ty[0] = by + 2;
    tx[1] = x + 18; ty[1] = by + 8;
    tx[2] = x + 20; ty[2] = by + 14;
    tx[3] = x + 18; ty[3] = by + 20;
    for (int i = 0; i < 4; i++) display.fillCircle(tx[i], ty[i], 3, SSD1306_WHITE);
  } else {
    // Happy/Waiting: big curled tail up behind
    tx[0] = x + 12; ty[0] = by - 2;
    tx[1] = x + 20 + tailSway; ty[1] = by - 10;
    tx[2] = x + 16 + tailSway; ty[2] = by - 22;
    tx[3] = x + 4 + tailSway;  ty[3] = by - 28;
    tx[4] = x - 8 + tailSway;  ty[4] = by - 24;
    tx[5] = x - 14 + tailSway; ty[5] = by - 14;
    for (int i = 0; i < 6; i++) display.fillCircle(tx[i], ty[i], 4, SSD1306_WHITE);
    display.fillCircle(tx[5] - 3, ty[5] + 4, 3, SSD1306_WHITE); // tail tip
  }

  // --- HEAD (big round) ---
  int hy = by - 16;
  display.fillCircle(x, hy, 17, SSD1306_WHITE);

  // Fluffy cheeks
  display.fillCircle(x - 14, hy + 4, 8, SSD1306_WHITE);
  display.fillCircle(x + 14, hy + 4, 8, SSD1306_WHITE);
  display.fillCircle(x - 10, hy + 10, 6, SSD1306_WHITE);
  display.fillCircle(x + 10, hy + 10, 6, SSD1306_WHITE);

  // --- EARS ---
  if (s == 0) { // Flat back
    display.fillTriangle(x - 10, hy - 8, x - 18, hy - 2, x - 4, hy - 2, SSD1306_WHITE);
    display.fillTriangle(x + 10, hy - 8, x + 18, hy - 2, x + 4, hy - 2, SSD1306_WHITE);
  } else { // Perky
    display.fillTriangle(x - 12, hy - 10, x - 18, hy - 24, x - 4, hy - 16, SSD1306_WHITE);
    display.fillTriangle(x + 12, hy - 10, x + 18, hy - 24, x + 4, hy - 16, SSD1306_WHITE);
    display.drawLine(x - 12, hy - 14, x - 14, hy - 20, SSD1306_BLACK);
    display.drawLine(x + 12, hy - 14, x + 14, hy - 20, SSD1306_BLACK);
  }

  // --- EYES (big) ---
  int eyeY = hy - 2;
  int eyeX_L = x - 8;
  int eyeX_R = x + 8;

  if (s == 0) { // Angry slits
    display.fillTriangle(eyeX_L - 4, eyeY - 2, eyeX_L + 4, eyeY + 2, eyeX_L - 4, eyeY + 2, SSD1306_BLACK);
    display.fillTriangle(eyeX_R + 4, eyeY - 2, eyeX_R - 4, eyeY + 2, eyeX_R + 4, eyeY + 2, SSD1306_BLACK);
  } else if (s == 2 || s == 3) { // Happy ^ ^
    display.drawLine(eyeX_L - 4, eyeY + 2, eyeX_L, eyeY - 3, SSD1306_BLACK);
    display.drawLine(eyeX_L, eyeY - 3, eyeX_L + 4, eyeY + 2, SSD1306_BLACK);
    display.drawLine(eyeX_R - 4, eyeY + 2, eyeX_R, eyeY - 3, SSD1306_BLACK);
    display.drawLine(eyeX_R, eyeY - 3, eyeX_R + 4, eyeY + 2, SSD1306_BLACK);
  } else { // Waiting: big round with highlights
    display.fillCircle(eyeX_L, eyeY, 7, SSD1306_BLACK);
    display.fillCircle(eyeX_R, eyeY, 7, SSD1306_BLACK);
    display.drawCircle(eyeX_L, eyeY, 6, SSD1306_WHITE);
    display.drawCircle(eyeX_R, eyeY, 6, SSD1306_WHITE);
    display.fillCircle(eyeX_L + 1, eyeY + 1, 3, SSD1306_WHITE);
    display.fillCircle(eyeX_R - 1, eyeY + 1, 3, SSD1306_WHITE);
    display.fillCircle(eyeX_L - 2, eyeY - 2, 2, SSD1306_WHITE);
    display.fillCircle(eyeX_R + 2, eyeY - 2, 2, SSD1306_WHITE);

    // Blink
    if ((t / 3000) % 3 == 0) {
      display.fillRect(x - 16, eyeY - 5, 14, 10, SSD1306_WHITE);
      display.fillRect(x + 2, eyeY - 5, 14, 10, SSD1306_WHITE);
      display.drawLine(eyeX_L - 4, eyeY, eyeX_L + 4, eyeY, SSD1306_BLACK);
      display.drawLine(eyeX_R - 4, eyeY, eyeX_R + 4, eyeY, SSD1306_BLACK);
    }
  }

  // --- NOSE ---
  display.fillTriangle(x, hy + 4, x - 2, hy + 7, x + 2, hy + 7, SSD1306_BLACK);

  // --- MOUTH ---
  if (s == 2 || s == 3) { // Happy open
    display.drawLine(x - 3, hy + 10, x, hy + 13, SSD1306_BLACK);
    display.drawLine(x, hy + 13, x + 3, hy + 10, SSD1306_BLACK);
  } else if (s == 0) { // Angry frown
    display.drawLine(x - 4, hy + 11, x, hy + 8, SSD1306_BLACK);
    display.drawLine(x, hy + 8, x + 4, hy + 11, SSD1306_BLACK);
  } else { // Neutral
    display.drawPixel(x, hy + 10, SSD1306_BLACK);
  }

  // --- WHISKERS ---
  if (s != 2 && s != 3) {
    display.drawLine(x - 18, hy + 4, x - 30, hy + 2, SSD1306_WHITE);
    display.drawLine(x - 18, hy + 7, x - 30, hy + 8, SSD1306_WHITE);
    display.drawLine(x + 18, hy + 4, x + 30, hy + 2, SSD1306_WHITE);
    display.drawLine(x + 18, hy + 7, x + 30, hy + 8, SSD1306_WHITE);
  }

  // --- Sparkles ---
  if (s == 2 || s == 3) {
    int sx = x - 25 + ((t / 200) % 6) * 10;
    int sy = 8 + osc(t + sx, 400, 5);
    display.drawPixel(sx, sy, SSD1306_WHITE);
    display.drawPixel(sx + 1, sy - 1, SSD1306_WHITE);
    display.drawPixel(sx - 1, sy + 1, SSD1306_WHITE);
  }
}

/* ==================== MESSAGE ==================== */
void drawMessage(uint8_t s) {
  const char* msg;
  if (s == 2)       msg = "Purr... <3";
  else if (s == 3)  msg = "Yay! :3";
  else if (s == 0)  msg = "I'm angry!";
  else              msg = "Pet me please";

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 56);
  display.print(msg);
}

/* ==================== MAIN LOOP ==================== */
void loop() {
  bool touched = digitalRead(TOUCH_PIN) == HIGH;
  unsigned long now = millis();

  if (touched) {
    state = 2;
    lastTouchMs = now;
    afterglowEndMs = 0;
  } else {
    if (state == 2) {
      afterglowEndMs = now + AFTERGLOW_MS;
      state = 3;
    } else if (state == 3 && now >= afterglowEndMs) {
      state = (now - lastTouchMs >= ANGRY_TIMEOUT) ? 0 : 1;
    } else if (state != 3) {
      state = (now - lastTouchMs >= ANGRY_TIMEOUT) ? 0 : 1;
    }
  }

  int catX = 64;
  int catY;
  if (state == 2)       catY = 36 - abs(((now / 70) % 14) - 7);
  else if (state == 3)  catY = 36 - abs(((now / 90) % 10) - 5);
  else if (state == 0)  catY = 40;
  else                  catY = 36;

  display.clearDisplay();
  drawCat(catX, catY, state, now);
  drawMessage(state);
  display.display();
  delay(30);
}
