/*  
    Copyright 2021, 2022, 2023 Andreas Petersik (andreas.petersik@gmail.com)
    
    This file is part of the Chesstimation Project.

    Chesstimation is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Chesstimation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Chesstimation.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  Serial.println("=== Simple TFT Test ===");
  
  // Initialize TFT
  tft.begin();
  tft.setRotation(1);
  
  // Clear screen and draw simple elements
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("CHESSTIMATION");
  
  tft.drawRect(10, 50, 100, 50, TFT_RED);
  tft.fillRect(20, 60, 80, 30, TFT_GREEN);
  
  Serial.println("TFT test complete");
}

void loop()
{
  delay(1000);
}