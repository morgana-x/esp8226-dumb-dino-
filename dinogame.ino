/*
DUMB DINO

WIRING INSTRUCTIONS FOR ESP8226 MINI D1 WIFI

SCREEN  | ESP8226
GND     > G
VDD     > 3.3V
SCK     > D1
SDA     > D2

JOYSTICK| ESP8226
GND     > G
VRY     > A0
5V      > 5V

*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define ANALOGUE_HIGH 870 // FOR JOY STICK DEADZONE
#define ANALOGUE_LOW 850 // FOR JOY STICK DEADZONE
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int analogPin = A0;
uint analogValue = 0;
uint score =0;
uint highscore = 0;
short py = 0; // pos y
short vy = 0; // vel y
const int px = 10; // pos x
const ushort pw = 4; // For hitbox
bool CROUCH = false;
bool gameover = false;
ushort height = 8;
ushort nextscore = 0;

short cactusx[8]; // cactus positions
short cactush[8]; // cactus heights
short clouds[5] = {150, 215, 290, 340, 450}; // Cloud x positions

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    for(;;);
  }
  resetCactuses();

  display.clearDisplay();
  display.setTextSize(9);
  display.setTextColor(WHITE);
  display.setCursor(34, 0);
  display.println("5"); // Junpei
  display.display();

  delay(2000);
}
void resetCactus(ushort i)
{
    cactusx[i] = SCREEN_WIDTH + (i*50) + random(20,42);
    cactush[i] = random(5,11); 
}
void resetCactuses()
{
  for (ushort i=0; i<8;i++)
      resetCactus(i);
}
void processAnalog()
{
  analogValue = analogRead(analogPin);
  CROUCH = (analogValue >  ANALOGUE_HIGH);
  height = CROUCH ? 4 : 8;
  if ( py == 0 && analogValue < ANALOGUE_LOW) // JUMP
  {
      vy = 7;
      return;
  }
  short minVel = CROUCH ? -5 : -4;
  if (py > 0 && vy > minVel)
      vy = vy -1 - (py > 0 && CROUCH ? 5 : 0); // Velocity decrease faster if crouch
  if (vy < minVel) // Ensure we are not going -100000 units down per second
      vy = minVel;
}
void resetGame()
{
  resetCactuses();
  score = 0;
  py = 0;
  vy = -5;
  gameover = false;
}

void checkdeath()
{
  for (ushort i=0; i < 8; i++) // Loop through the eight cactuses
  {
    int x = cactusx[i];
    if ( (px + pw >= x) && (px < x + 4) && py <= cactush[i])
    {
      gameover = true;
      return;
    }
  }
}

void tick()
{
  if (gameover)
  {
    if (analogRead(analogPin) > ANALOGUE_HIGH) // If crouch
    {
      resetGame();
    }
    return;
  }
  nextscore = nextscore + 1; // Just to stop score increasing too fast
  if (nextscore > 3)
  {
    score = score + 1;
    highscore = score > highscore ? score : highscore;
    nextscore = 0;
  }
  processAnalog();
  py = py + vy;
  if (py < 0)
    py = 0;
  // Speed up as score increases
  ushort speedbonus =  (score > 400 ? 2 : score/200);
  // Scroll clouds and cactuses
  for (ushort i=0; i < 5; i++)
  {
      clouds[i] = clouds[i] - (1 + speedbonus);
      if (clouds[i] < -20)
          clouds[i] = 400 + random(70);
  }

  for (ushort i=0; i < 8; i++)
  {
      cactusx[i] = cactusx[i] - (2 +speedbonus);
      if (cactusx[i] < -2)
          resetCactus(i);
  }

  checkdeath();
}
void loop() 
{
  delay(gameover ? 100 : 10);

  tick(); // Logical logic... logialic

  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println(score);
  display.setCursor(100,0);
  display.println(highscore);
  
  if (gameover)
  {
    display.setCursor(0,8);
    display.println("GAMEOVER: CROUCH=RST");
  }
  // Draw ground
  display.drawLine(0, SCREEN_HEIGHT -10, SCREEN_WIDTH, SCREEN_HEIGHT - 10, WHITE);
  // Draw Dino
  display.fillRect(px, SCREEN_HEIGHT - (10 + height) - py, CROUCH ? 7 : 5, height, WHITE);
  // Draw clouds and cactuses
  for (ushort i=0; i < 5; i++)
  {
      if (clouds[i] > SCREEN_WIDTH) // Don't bother with offscreen items
        continue;
      display.fillRect(clouds[i], 18, 25, 4, WHITE);
  }
  for (ushort i=0; i < 8; i++)
  {
      if (cactusx[i] > SCREEN_WIDTH) // Don't bother with offscreen items
        continue;
      display.fillRect(cactusx[i], SCREEN_HEIGHT - (10 + cactush[i]), 4, cactush[i], WHITE);
  }
  display.display(); 
}