double Liquid_level = 0;
void setup() {
	Serial.begin(9600);
	pinMode(7, INPUT);
	Serial.println("Serial start");
}

void loop() {
	Liquid_level = digitalRead(7);
	Serial.print("Liquid_level= ");
	Serial.println(Liquid_level);
	//Serial.println(Liquid_level,DEC);
	delay(1000);
}