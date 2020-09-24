#define DESVIO 20
#define VALOR_MAX_POTE 900
#define VALOR_MIN_POTE 100
#define FACTOR_DE_ESCALA 0.01723

int hertz = 0;

int inches = 0;

int cm = 0;

int pos = 0;

// funciones
bool valorFueraDelRango(int valorAnt, int valorActual) {
  
  int valorMax = valorActual + DESVIO;
  int valorMin = valorActual - DESVIO;
  
  return !(valorMin < valorAnt && valorAnt < valorMax);
}

bool valorPotePermitido(int valorPote) {
  
  return (VALOR_MIN_POTE < valorPote && valorPote < VALOR_MAX_POTE);
}

long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

void maquina_estados_detector_presencia()
{
  // 0 a 1023 valores
  // 0 a 5V
  int valorActual = analogRead(A3);
  
  // if button press on A0 is detected
  if (digitalRead(A0) == HIGH) {
    tone(8, 440, 100); // play tone 57 (A4 = 440 Hz)
  }
  // if button press on A1 is detected
  if (digitalRead(A1) == HIGH) {
    tone(8, 494, 100); // play tone 59 (B4 = 494 Hz)
  }
  // if button press on A0 is detected
  if (digitalRead(A2) == HIGH) {
    tone(8, 523, 100); // play tone 60 (C5 = 523 Hz)
  }

  
  if ( valorFueraDelRango(hertz, valorActual) ) {
    hertz = valorActual;
    //pp = 0.9*hertz;
    Serial.println(hertz);
    
  }
  if (valorPotePermitido(hertz)) {
    tone(8, hertz, 100);
  }
  // measure the ping time in cm
  cm = FACTOR_DE_ESCALA * readUltrasonicDistance(7, 7);
  Serial.print(cm);
  Serial.println("cm");
  delay(100); // Wait for 100 millisecond(s)
  delay(10); // Delay a little bit to improve simulation performance
}

void do_init() 
{
  pinMode(A0, INPUT);
  pinMode(8, OUTPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  Serial.begin(9600);
}

void setup()
{
  do_init();
}

void loop()
{
  maquina_estados_detector_presencia();
}