#include <Arduino.h>
#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

void testPins() {
  Serial.println("=== Pin Configuration Test ===");
  Serial.printf("TFT_MISO: %d\n", 19);
  Serial.printf("TFT_MOSI: %d\n", 23);  
  Serial.printf("TFT_SCLK: %d\n", 18);
  Serial.printf("TFT_CS: %d\n", 5);
  Serial.printf("TFT_DC: %d\n", 17);
  Serial.printf("TFT_BL: %d\n", 16);
  Serial.printf("TFT_RST: %d\n", 4);
  
  // Test backlight pin explicitly
  Serial.println("Testing backlight pin...");
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  delay(1000);
  Serial.println("Backlight OFF");
  
  digitalWrite(16, HIGH);
  delay(1000);
  Serial.println("Backlight ON");
}

void testSPI() {
  Serial.println("=== SPI Test ===");
  
  // Initialize SPI manually
  SPI.begin(18, 19, 23, 5); // SCLK, MISO, MOSI, CS
  Serial.println("SPI initialized");
  
  // Test SPI communication with simple commands
  pinMode(5, OUTPUT);  // CS
  pinMode(17, OUTPUT); // DC
  
  digitalWrite(5, HIGH);
  digitalWrite(17, HIGH);
  delay(10);
  
  Serial.println("SPI pins configured");
}

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("=== Hardware Detection Test ===");
  
  testPins();
  testSPI();
  
  // Try different TFT initializations
  Serial.println("=== TFT Initialization Test ===");
  
  tft.begin();
  Serial.println("TFT begin() called");
  
  // Get basic info
  Serial.printf("Width: %d, Height: %d\n", tft.width(), tft.height());
  
  // Try minimal drawing with different approaches
  Serial.println("Testing simple fill colors...");
  
  // Method 1: Direct color fill
  tft.fillScreen(0xFFFF); // White
  delay(2000);
  
  tft.fillScreen(0xF800); // Red  
  delay(2000);
  
  tft.fillScreen(0x07E0); // Green
  delay(2000);
  
  tft.fillScreen(0x001F); // Blue
  delay(2000);
  
  // Method 2: Using TFT_eSPI color constants
  tft.fillScreen(TFT_WHITE);
  delay(1000);
  
  tft.fillScreen(TFT_RED);
  delay(1000);
  
  tft.fillScreen(TFT_BLACK);
  
  // Try text with different settings
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.print("TEST");
  
  Serial.println("All tests complete");
  Serial.println("Check display for any colors or text");
}

void loop()
{
  delay(5000);
  Serial.println("Still running...");
}