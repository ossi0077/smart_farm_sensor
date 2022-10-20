//Water senosr model number  : SEN0204
//TDS sensor model number    : SEN0244
//PH sensor model number     : SEN0169
//1602 LCD I2C module
//Arduino MEGA

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//PH sensor part
#define Offset 0.00  //deviation compensate
#define LED 13
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
//D7, D8, D9, D10, D11, D12, D13, D20(SDA), D21(SCL)
//A0, A1
#define waterLevel 7   //water sensor input pin number
#define phLevel A0     //ph sensor input pin number(analog)
#define tdsLevel A1    //ec sensor input pin number(analog)
#define RelayWater 13  //water relay pin number
#define RelayPH 12     //ph relay pin number
#define RelayTDS 11    //ec relay pin number
#define btnWater 10    //button(water delay time contorl) pin number
#define btnPH 9        //button(PH delay time contorl) pin number
#define btnTDS 8       //button(EC delay time contorl) pin number

LiquidCrystal_I2C lcd1602(0x27, 16, 2);  //16*2 LCD I2C module setting

unsigned int RelayWater_time = 1;  //water relay on time variable
unsigned int RelayPH_time = 1;     //ph relay on time variable
unsigned int RelayTDS_time = 1;    //ec relay on time variable

int Liquid_level = 0;   //water sensor state variable
static float PH_level;  //ph sensor state variable
double TDS_level = 0;   //tds sensor state variable

//User setting value
//time unit [ms]
unsigned int time_interval = 60;  //1 push button time interval value(default : 1min)
unsigned int time_max = 600;      //max time value(default : 10min);
unsigned int time_min = 60;       //min time vlaue(default : 1min);

void setup() {
    pinMode(waterLevel, INPUT);
    pinMode(btnWater, INPUT_PULLUP);
    pinMode(btnPH, INPUT_PULLUP);
    pinMode(btnTDS, INPUT_PULLUP);
    pinMode(tdsLevel, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(RelayWater, OUTPUT);
    pinMode(RelayPH, OUTPUT);
    pinMode(RelayTDS, OUTPUT);
    digitalWrite(RelayWater, LOW);
    digitalWrite(RelayPH, LOW);
    digitalWrite(RelayTDS, LOW);
    lcd1602.init();
    lcd1602.backlight();
    lcd1602.setCursor(0, 0);
    lcd1602.print("gongzipsa");
    delay(1000);
}

void loop() {
    Liquid_level = digitalRead(waterLevel);

    //TDS_level value
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
    {
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
        TDS_level = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5;
    }

    //PH_level value
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();
    static float voltage;
    if (millis() - samplingTime > samplingInterval) {
        pHArray[pHArrayIndex++] = analogRead(phLevel);
        if (pHArrayIndex == ArrayLenth) pHArrayIndex = 0;
        voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
        PH_level = 3.5 * voltage + Offset;
        samplingTime = millis();
    }


    lcdCurrent();  //lcd에 현재 농도 표기

    if (Liquid_level == 0) {
        relayon(RelayWater_time, RelayPH_time, RelayTDS_time);
    }
    else if (Liquid_level == 1) {
        if (digitalRead(btnWater) == 0) {
            RelayWater_time = btnfuc(RelayWater_time);
            btnlcd(&RelayWater_time);
        }
        else if (digitalRead(btnTDS) == 0) {
            RelayTDS_time = btnfuc(RelayTDS_time);
            btnlcd(&RelayTDS_time);
        }
        else if (digitalRead(btnPH) == 0) {
            RelayPH_time = btnfuc(RelayPH_time);
            btnlcd(&RelayPH_time);
        }
    }
}

void relayon(int dlywater, int dlyph, int dlyec) {
    lcd1602.init();
    lcd1602.setCursor(0, 0);
    lcd1602.print("Relay On");
    lcd1602.setCursor(0, 1);
    lcd1602.print("Water pump");

    digitalWrite(RelayWater, HIGH);
    delay(dlywater * 1000);
    digitalWrite(RelayWater, LOW);

    lcd1602.init();
    lcd1602.setCursor(0, 0);
    lcd1602.print("Relay On");
    lcd1602.setCursor(0, 1);
    lcd1602.print("PH pump");
    digitalWrite(RelayPH, HIGH);
    delay(dlyph * 1000);
    digitalWrite(RelayPH, LOW);

    lcd1602.init();
    lcd1602.setCursor(0, 0);
    lcd1602.print("Relay On");
    lcd1602.setCursor(0, 1);
    lcd1602.print("TDS pump");
    digitalWrite(RelayTDS, HIGH);
    delay(dlyec * 1000);
    digitalWrite(RelayTDS, LOW);
}

unsigned int btnfuc(unsigned int relaytime) {
    if (relaytime == time_max)
        relaytime = time_min;
    else
        relaytime = relaytime + time_interval;
    return relaytime;
}

void lcdCurrent() {
    lcd1602.setCursor(0, 0);
    lcd1602.print("PH :");
    lcd1602.print(PH_level, 3);
    lcd1602.setCursor(0, 1);
    lcd1602.print("TDC:");
    lcd1602.print(TDS_level, 3);
}

void btnlcd(unsigned int* timeset) {
    if (timeset == &RelayWater_time) {
        lcd1602.init();
        lcd1602.backlight();
        lcd1602.setCursor(0, 0);
        lcd1602.print("water time [s]");
        lcd1602.setCursor(0, 1);
        lcd1602.print(RelayWater_time, DEC);
    }
    else if (timeset == &RelayPH_time) {
        lcd1602.init();
        lcd1602.backlight();
        lcd1602.setCursor(0, 0);
        lcd1602.print("PH time [s]");
        lcd1602.setCursor(0, 1);
        lcd1602.print(RelayPH_time, DEC);
    }
    else if (timeset == &RelayTDS_time) {
        lcd1602.init();
        lcd1602.backlight();
        lcd1602.setCursor(0, 0);
        lcd1602.print("TDS time [s]");
        lcd1602.setCursor(0, 1);
        lcd1602.print(RelayTDS_time, DEC);
    }
    delay(1000);
    lcd1602.init();
    lcdCurrent();
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
        Serial.println("Error number for the array to avraging!/n");
        return 0;
    }
    if (number < 5) {  //less than 5, calculated directly statistics
        for (i = 0; i < number; i++) {
            amount += arr[i];
        }
        avg = amount / number;
        return avg;
    }
    else {
        if (arr[0] < arr[1]) {
            min = arr[0];
            max = arr[1];
        }
        else {
            min = arr[1];
            max = arr[0];
        }
        for (i = 2; i < number; i++) {
            if (arr[i] < min) {
                amount += min;  //arr<min
                min = arr[i];
            }
            else {
                if (arr[i] > max) {
                    amount += max;  //arr>max
                    max = arr[i];
                }
                else {
                    amount += arr[i];  //min<=arr<=max
                }
            }  //if
        }    //for
        avg = (double)amount / (number - 2);
    }  //if
    return avg;
}