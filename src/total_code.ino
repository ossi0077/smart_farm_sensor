//Water senosr model number  : SEN0204
//TDS sensor model number    : SEN0244
//PH sensor model number     : SEN0169
//1602 LCD I2C module
//Arduino MEGA
//relay * 3

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//PH sensor part
#define Offset 0.00  //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth 40     //times of collection
int pHArray[ArrayLenth];  //Store the average value of the sensor feedback
int pHArrayIndex = 0;

//TDS sensor part
#define VREF 5.0           // analog reference voltage(Volt) of the ADC
#define SCOUNT 30          // sum of sample point
int analogBuffer[SCOUNT];  // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, temperature = 25;

//Used pin number
//D6, D7, D8, D9, D11, D12, D13, D20(SDA), D21(SCL)
//A0, A1
#define waterLevel_max 6  //water sensor input pin number(stop relay)
#define waterLevel_min 7  //water sensor input pin number(start relay)
#define phLevel A0        //ph sensor input pin number(analog)
#define tdsLevel A1       //tds sensor input pin number(analog)
#define RelayWater 13     //water relay pin number
#define RelayPH 12        //ph relay pin number
#define RelayTDS 11       //tds relay pin number
#define btnPH 9           //button(PH delay time contorl) pin number
#define btnTDS 8          //button(TDS delay time contorl) pin number


LiquidCrystal_I2C lcd1602(0x27, 16, 2);  //16*2 LCD I2C module setting
int Liquid_level_bottom = 0;             //water sensor state variable
int Liquid_level_top = 0;                //water sensor state variable
static float PH_level;                   //ph sensor state variable
double TDS_level_raw = 0;                //tds sensor state variable
double TDS_level = TDS_level_raw / 500;  //실사용 TDS 센서 값

//User setting value
static float PH_goal = 6.2;      //Target PH
static float PH_goal_max = 7.0;  //최대 PH 농도 설정값
static float PH_goal_min = 5.0;  //최대 PH 농도 설정값
static float PH_interval = 0.2;  //버튼 push 시 증가량
double TDS_goal = 1.0;           //Target TDS
double TDS_goal_max = 1.5;       //최대 TDS 농도 설정값
double TDS_goal_min = 0.5;       //최대 TDS 농도 설정값
double TDS_interval = 0.1;       //버튼 push 시 증가량

void setup() {
  Serial.begin(9600);
  pinMode(waterLevel_min, INPUT);
  pinMode(waterLevel_max, INPUT);
  pinMode(btnPH, INPUT_PULLUP);
  pinMode(btnTDS, INPUT_PULLUP);
  pinMode(tdsLevel, INPUT);
  pinMode(RelayWater, OUTPUT);
  pinMode(RelayPH, OUTPUT);
  pinMode(RelayTDS, OUTPUT);
  digitalWrite(RelayWater, LOW);
  digitalWrite(RelayPH, LOW);
  digitalWrite(RelayTDS, LOW);
  lcd1602.init();
  lcd1602.backlight();
  lcd1602.setCursor(0, 0);
  lcd1602.print(F("Power On"));
  delay(1500);
  Serial.begin(9600);
}
int tick = 2000;
int btnFlg1 = 0;
int btnFlg2 = 0;
int btn1Chk(){
  if(digitalRead(btnPH) == 0){
    btnFlg1 = 1;
    return 0;
  }
  if(btnFlg1 == 1){
    btnFlg1 = 0;
    return 1;
  }
  return 0;
}

int btn2Chk(){
  if(digitalRead(btnTDS) == 0){
    btnFlg2 = 1;
    return 0;
  }
  if(btnFlg2 == 1){
    btnFlg2 = 0;
    return 1;
  }
  return 0;
}


void loop() {
  Liquid_level_bottom = digitalRead(waterLevel_min);  //하단 수위 센서 값
  Liquid_level_top = digitalRead(waterLevel_max);  //상단 수위 센서 값
  sensingPH();                                     //PH 농도 측정
  sensingTDS();                                    //TDS 농도 측정
  if(tick == 5000){
    lcdCurrent();  //lcd에 현재 농도 표기
    tick = 0 ;
  }
  tick++;

  if (Liquid_level_bottom == 0) {

    lcdRelayOn(1);
    digitalWrite(RelayWater, HIGH);
    Liquid_level_top = digitalRead(waterLevel_max);
    while (Liquid_level_top != 1)
      Liquid_level_top = digitalRead(waterLevel_max);
    digitalWrite(RelayWater, LOW);

    lcdRelayOn(2);
    digitalWrite(RelayTDS, HIGH);
    sensingTDS();
    while (TDS_level < TDS_goal)
      sensingTDS();
    digitalWrite(RelayTDS, LOW);

    lcdRelayOn(3);
    digitalWrite(RelayPH, HIGH);
    sensingPH();
    while (PH_level < PH_goal)
      sensingPH();
    digitalWrite(RelayPH, LOW);

    lcd1602.init();
    lcd1602.setCursor(0, 0);
    lcd1602.print(F("All Relay OFF"));
    delay(1500);
    lcd1602.clear();
  } else if (Liquid_level_bottom == 1) {
    if (btn1Chk() == 1) {  //PH 목표 농도 설정 코드
      if (PH_goal < PH_goal_max - PH_interval) {
        PH_goal += PH_interval;
        sensingPH();
      } else {
        PH_goal = PH_goal_min;
        sensingPH();
      }
      delay(100);
    } 
    if (btn2Chk() == 1) {
      if (TDS_goal < TDS_goal_max) {
        TDS_goal += TDS_interval;
        sensingTDS();
      } else {
        TDS_goal = TDS_goal_min;
        sensingTDS();
      }
      delay(100);
    }
  }
}

void lcdRelayOn(int i) {  //water(1),tds(2),ph(3)
  lcd1602.init();
  lcd1602.setCursor(0, 0);
  lcd1602.print(F("Relay ON"));
  lcd1602.setCursor(0, 1);
  switch (i) {
    case 1:
      lcd1602.print(F("water relay"));
      break;
    case 2:
      lcd1602.print(F("TDS relay"));
      break;
    case 3:
      lcd1602.print(F("PH relay"));
      break;
    default:
      break;
  }
}

void sensingPH() {  //PH 농도 센서
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float voltage;

  if (millis() - samplingTime > samplingInterval) {
    pHArray[pHArrayIndex++] = analogRead(phLevel);
    if (pHArrayIndex == ArrayLenth)
      pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    PH_level = 3.5 * voltage + Offset;
    samplingTime = millis();
  }
}

void sensingTDS() {  //TDS 농도 센서
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U) {  //every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(tdsLevel);  //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVolatge = averageVoltage / compensationCoefficient;
    TDS_level_raw = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;
  }
  TDS_level = TDS_level_raw / 500;
}

void lcdCurrent() {
  lcd1602.setCursor(12, 0);
  lcd1602.print(F("  "));


  lcd1602.setCursor(0, 0);
  lcd1602.print(TDS_level, 3);
  lcd1602.print(F("  "));
  lcd1602.print(PH_level, 3);

  lcd1602.setCursor(0, 1);
  lcd1602.print(TDS_goal, 3);
  lcd1602.print(F("  "));
  lcd1602.print(PH_goal, 1);
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println(F("Error number for the array to avraging!/n"));
    return 0;
  }
  if (number < 5) {  //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0];
      max = arr[1];
    } else {
      min = arr[1];
      max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;  //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i];  //min<=arr<=max
        }
      }  //if
    }    //for
    avg = (double)amount / (number - 2);
  }  //if
  return avg;
}