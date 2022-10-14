#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

enum state {SYNCHRONISATION, WAITING_PRESS, WAITING_RELEASE};
const long one_second = 1000;
int scrollCount = 0;
int arrow = 0;
#define red 0x1
#define yellow 0x3
#define green 0x2
#define purple 0x5
#define white 0x7
String channelArr[26][4];

byte upArrow[8] = {B00100, B01110, B11111, B00100, B00100, B00100, B00100, B00000};
byte downArrow[8] = {B00000, B00100, B00100, B00100, B00100, B11111, B01110, B00100};
byte none[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00000};

void sortChannels() {
  String temp[4];
  for (int i = 0; i < 26; i++) {
    for (int j = 0; j < 25 - i; j++) {
      if (channelArr[j + 1][0] != NULL) {
        const char* channel1 = channelArr[j][0].c_str();
        const char* channel2 = channelArr[j + 1][0].c_str();
        if (strcmp(channel1, channel2) > 0) {
          
          temp[0] = channelArr[j][0];
          temp[1] = channelArr[j][1];
          temp[2] = channelArr[j][2];
          temp[3] = channelArr[j][3];
          //temp[4] = channelArr[j][4];
          //temp[5] = channelArr[j][5];
          channelArr[j][0] = channelArr[j + 1][0];
          channelArr[j][1] = channelArr[j + 1][1];
          channelArr[j][2] = channelArr[j + 1][2];
          channelArr[j][3] = channelArr[j + 1][3];
          //channelArr[j][4] = channelArr[j + 1][4];
          //channelArr[j][5] = channelArr[j + 1][5];
          channelArr[j + 1][0] = temp[0];
          channelArr[j + 1][1] = temp[1];
          channelArr[j + 1][2] = temp[2];
          channelArr[j + 1][3] = temp[3];
          //channelArr[j + 1][4] = temp[4];
          //channelArr[j + 1][5] = temp[5];
        }
      }
    }
  }
  scrollPrint(9);
}

void addChannel(String msgInput) {
  if (msgInput.length() > 16) {
    msgInput = msgInput.substring(1, 16);
  } else {
    msgInput = msgInput.substring(1);
  }
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][0].substring(0, 1) == msgInput.substring(0, 1)) {
      channelArr[i][0] = msgInput;
      scrollPrint(9);
      break;
    } else {
      if (channelArr[i][0] == NULL) {
        channelArr[i][0] = msgInput; //channel description
        channelArr[i][1] = "255"; //max
        channelArr[i][2] = "0"; //min
        //channelArr[i][3] = "\0"; //recent value
        //channelArr[i][4] = NULL; //total
        //channelArr[i][5] = "0"; //counter
        sortChannels();
        break;
      }
    }
  }
}

void addValue(String msgInput) {
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][0].substring(0, 1) == msgInput.substring(1, 2)) {
       channelArr[i][3] = msgInput.substring(1);
       //channelArr[i][5] = String(channelArr[i][5].toInt() + 1);
       //channelArr[i][4] = String(channelArr[i][4].toInt() + msgInput.substring(2).toInt());
       scrollPrint(9);
       break;
    }
  }
}

void addMax(String msgInput) {
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][0].substring(0, 1) == msgInput.substring(1, 2)) {
      channelArr[i][1] = msgInput.substring(2);
      break;
    }
  }
}

void addMin(String msgInput) {
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][0].substring(0, 1) == msgInput.substring(1, 2)) {
      channelArr[i][2] = msgInput.substring(2);
      break;
    }
  }
}

boolean maxCheck () {
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][3].substring(1).toInt() > channelArr[i][1].toInt() && channelArr[i][3] != NULL && channelArr[i][0] != NULL) {
      return true;
      break;
    }  
  }
  return false;
}

boolean minCheck () {
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][3].substring(1).toInt() < channelArr[i][2].toInt() && channelArr[i][3] != NULL && channelArr[i][0] != NULL) {
      return true;
      break;
    }  
  }
  return false;
}

void printChannelArr() {
  for (int i = 0; i < 26; i++) {
    Serial.print(channelArr[i][0] + ", " + channelArr[i][1] + ", " + channelArr[i][2] + ", " + channelArr[i][3] /*+ ", " + channelArr[i][4] + ", " + channelArr[i][5]*/+ "\n");
  }
}

/*void printValueArr() {
  for (int i = 0; i < 12; i++) {
    Serial.print(valueArr[i] + "\n");
  }
}*/

void msgCheck(String msgInput) {
  int inputLen = msgInput.length();
  if (msgInput.substring(0, 1) == "C" && msgInput.substring(2) != NULL) {
    addChannel(msgInput);
  } else if (msgInput.substring(0, 1) == "X" && msgInput.substring(2) != NULL) {
    addMax(msgInput);
  } else if (msgInput.substring(0, 1) == "N" && msgInput.substring(2) != NULL) {
    addMin(msgInput);
  } else if (msgInput.substring(0, 1) == "V" && msgInput.substring(2) != NULL) {
    addValue(msgInput);
  } else {
    Serial.print("ERROR: "+msgInput);
  }
  printChannelArr();
}

void scrollPrint(int pressed) {
  lcd.createChar(0, none);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);
  
  if (channelArr[1][3] == NULL && channelArr[0][3] != NULL) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(0);
    lcd.setCursor(1, 0);
    printLine(channelArr[0][3], channelArr[0][0]);
  } else if (channelArr[2][3] == NULL && channelArr[0][3] != NULL && channelArr[1][3] != NULL) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(0);
    lcd.setCursor(1, 0);
    printLine(channelArr[0][3], channelArr[0][0]);
    lcd.setCursor(0, 1);
    lcd.write(0);
    lcd.setCursor(1, 1);
    printLine(channelArr[1][3], channelArr[1][0]);
  } else if (channelArr[0][3] != NULL) {
    if (pressed == 4) {
      if (channelArr[scrollCount + 2][3] != NULL) {
        if (scrollCount < 20) {
          scrollCount += 1;
          lcd.clear();
          if (channelArr[scrollCount + 2][3] == NULL || scrollCount > 20) {
            lcd.setCursor(0, 0);
            lcd.write(1);
            lcd.setCursor(1, 0);
            printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
            lcd.setCursor(0, 1);
            lcd.write(0);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          } else {
            lcd.setCursor(0, 0);
            lcd.write(1);
            lcd.setCursor(1, 0);
            printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
            lcd.setCursor(0, 1);
            lcd.write(2);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          }
        }
      }
    }
    if (pressed == 8) {
      if (scrollCount > 0) {
        scrollCount -= 1;
        lcd.clear();
        if (scrollCount == 0) {
          lcd.setCursor(0, 0);
          lcd.write(0);
          lcd.setCursor(1, 0);
          printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
          lcd.setCursor(0, 1);
          if (channelArr[scrollCount + 2][3] == NULL) {
            lcd.write(0);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          } else {
            lcd.write(2);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          }
        } else {
          lcd.setCursor(0, 0);
          lcd.write(1);
          lcd.setCursor(1, 0);
          printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
          lcd.setCursor(0, 1);
          if (channelArr[scrollCount + 2][3] == NULL) {
            lcd.write(0);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          } else {
            lcd.write(2);
            lcd.setCursor(1, 1);
            printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
          }
        }
      }
    }
    if (pressed == 1) {
      if (scrollCount == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(0);
        lcd.setCursor(1, 0);
        printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.setCursor(1, 0);
        printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
      }
    }
    if (pressed == 9) {
      if (scrollCount == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(0);
        lcd.setCursor(1, 0);
        printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
      } else if (channelArr[scrollCount + 2][3] == NULL) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.setCursor(1, 0);
        printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
        lcd.setCursor(0, 1);
        lcd.write(0);
        lcd.setCursor(1, 1);
        printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.setCursor(1, 0);
        printLine(channelArr[scrollCount][3], channelArr[scrollCount][0]);
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printLine(channelArr[scrollCount + 1][3], channelArr[scrollCount + 1][0]);
      }
    }
  }
}

void printLine(String printMsg, String channel) {
  String finalPrint;
  if (printMsg.length() == 4) {
    finalPrint = printMsg;
  } else if (printMsg.length() == 3) {
    finalPrint = (printMsg.substring(0, 1) + " " + printMsg.substring(1, 3));
  } else {
    finalPrint = (printMsg.substring(0, 1) + "  " + printMsg.substring(1, 2));
  }
  lcd.print(finalPrint + " " + /*channelAvg(channel) + " " +*/ channel.substring(1));
}

void freeMemory () {
  char top;
  extern int __heap_start, *__brkval;
  lcd.print("Free SRAM: "+String((int)&top - (int)__brkval)+"B");
}

/*String channelAvg (String channel) {
  int count = 0;
  int total = 0;
  for (int i = 0; i < 26; i++) {
    if (channelArr[i][0].substring(0,1) == channel) {
      int avg = channelArr[i][4].toInt() / channelArr[i][5].toInt();
      return(String(avg));
    }
  }
}*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop() {
  // put your main code here, to run repeatedly:
  static enum state s = SYNCHRONISATION;
  static long pressTime;
  static int pressed;
  static int __stacktop;

  switch (s) {
    case SYNCHRONISATION:
      lcd.setBacklight(purple);
      Serial.print(F("Q"));
      if (Serial.available()) {
        char inputVal = Serial.read();
        if (inputVal == 'X') {
          lcd.setBacklight(white);
          Serial.println(F("\nBASIC"));
          s = WAITING_PRESS;
        } else {
          s = SYNCHRONISATION;
        }
      }
      delay(one_second);
      break;
    case WAITING_RELEASE:
      pressed = lcd.readButtons();
      if (pressed == 1) {
        if (millis() - pressTime >= 1000) {
          lcd.clear();
          lcd.setBacklight(purple);
          lcd.setCursor(0, 0);
          lcd.print("F118143");
          lcd.setCursor(0, 1);
          freeMemory();
          s = WAITING_RELEASE;
          delay(10);
        }
      } else {
        lcd.clear();
        scrollPrint(9);
        s = WAITING_PRESS;
      }
      break;
    case WAITING_PRESS:
      boolean isMax = maxCheck();
      boolean isMin = minCheck();
      if (isMax == true && isMin == true) {
        lcd.setBacklight(yellow);
      } else if (isMax == true) {
        lcd.setBacklight(red);
      } else if (isMin == true) {
        lcd.setBacklight(green);
      } else {
        lcd.setBacklight(white);
      }
      if (Serial.available()) {
        String inputString = Serial.readString();
        Serial.print(inputString);
        if (inputString.length() > 1) {
          msgCheck(inputString);
          Serial.print(inputString);
          scrollPrint(0);
        }
      }
      pressed = lcd.readButtons();
      if (pressed == 1) {
        pressTime = millis();
        s = WAITING_RELEASE;
      } else if (pressed == 4 || pressed == 8) {
        if (channelArr[2][3] != NULL) {
          scrollPrint(pressed);
          delay(100);
        }
      } else {
        s = WAITING_PRESS;
      }
      break;
  }
}
