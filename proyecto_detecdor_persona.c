#define DESVIO 20
#define VALOR_MAX_POTE 900
#define VALOR_MIN_POTE 100
#define FACTOR_DE_ESCALA_CM 0.01723
#define PIN_SENSOR_ULTRASONIDO 7
#define PIN_POTENCIOMETRO A3
#define PIN_ALARMA 8
#define MAX_CANT_SENSORES                           1

#define UMBRAL_DIFERENCIA_TIMEOUT                   50
#define UMBRAL_DISTANCIA_MAXIMA                    366
#define UMBRAL_DISTANCIA_LEJOS                     240
#define UMBRAL_DISTANCIA_MEDIO                     120
#define UMBRAL_DISTANCIA_CERCA                       0

#define PIN_LED_RGB_ROJO     2
#define PIN_LED_RGB_VERDE    4
#define PIN_LED_RGB_AZUL     3

#define SENSOR_ULTRASONIDO   0

#define DOS_MICROSEGUNDOS                      2
#define DIEZ_MICROSEGUNDOS                     10

#define MAX_STATES 4
#define MAX_EVENTS 6

enum states          { ST_INIT,  ST_IDLE        , ST_WAITING_RESPONSE   , ST_ERROR                                                  } current_state;
String states_s [] = {"ST_INIT", "ST_IDLE"      , "ST_WAITING_RESPONSE" , "ST_ERROR"                                                };

enum events          { EV_CONT,   EV_DIST_LEJOS   , EV_DIST_MEDIO          , EV_DIST_CERCA   , EV_TIMEOUT  , EV_ACK_BOTHOM    } new_event;
String events_s [] = {"EV_CONT",  "EV_DIST_LEJOS", "EV_DIST_MEDIO"        , "EV_DIST_CERCA", "EV_TIMEOUT" , "EV_ACK_BOTHOM"  };

typedef void (*transition)();

transition state_table[MAX_STATES][MAX_EVENTS] =
{
      {init_    , error          , error         , error         , error       , error       	  } , // state ST_INIT
      {none     , dist_lejos_a   , dist_media_a  , dist_cerca_a  , none        , dist_cerca_a     } , // state ST_IDLE
      {none     , dist_lejos     , dist_media    , dist_cerca    , none        , dist_cerca       } , // state ST_WAITING_RESPONSE
      {error    , error          , error         , error         , error       , error       	  }   // state ST_ERROR
      
     //EV_CONT  , EV_DIST_LEJOS  , EV_DIST_MEDIO  , EV_DIST_CERCA , EV_TIMEOUT  , EV_ACK_BOTHOM
};
//----------------------------------------------

struct stSensor
{
  int  pin;
  int  estado;
  long valor_actual;
  long valor_previo;
};
stSensor sensores[MAX_CANT_SENSORES];

int pin_alarma = PIN_ALARMA;

int pin_sensor_ultrasonido = PIN_SENSOR_ULTRASONIDO;

int hertz = 0;

int inches = 0;

bool timeout;

long lct;

int cm = 0;

int pos = 0;

// funciones

//UMBRALES DE DISTANCIAS
// ---------------------------------------------
bool verificar_umbral_distancia_lejos(int distancia) {
  
  return (UMBRAL_DISTANCIA_LEJOS <= distancia);
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_umbral_distancia_media(int distancia) {
  
  return (UMBRAL_DISTANCIA_MEDIO <= distancia && distancia < UMBRAL_DISTANCIA_LEJOS);
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_umbral_distancia_cerca(int distancia) {
  
  return (UMBRAL_DISTANCIA_CERCA <= distancia && distancia < UMBRAL_DISTANCIA_MEDIO);
}
// ---------------------------------------------


//UMBRALES DEL POTENCIOMETRO
// ---------------------------------------------
bool valorPotePermitido(int valorPote) {
  
  return (VALOR_MIN_POTE < valorPote && valorPote < VALOR_MAX_POTE);
}
// ---------------------------------------------

// ---------------------------------------------
bool valorFueraDelRango(int valorAnt, int valorActual) {
  
  int valorMax = valorActual + DESVIO;
  int valorMin = valorActual - DESVIO;
  
  return !(valorMin < valorAnt && valorAnt < valorMax);
}
// ---------------------------------------------

//LECTURA DEL SENSOR DE ULTRASONIDO
// ---------------------------------------------
long leer_distancia_ultrasonido(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(DOS_MICROSEGUNDOS);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(DIEZ_MICROSEGUNDOS);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}
// ---------------------------------------------

// APAGAR Y ENCENDER LED RGB
//----------------------------------------------
void apagar_leds( )
{
  digitalWrite(PIN_LED_RGB_VERDE, false);
  digitalWrite(PIN_LED_RGB_AZUL , false);
  digitalWrite(PIN_LED_RGB_ROJO , false);
}
//----------------------------------------------

//----------------------------------------------
void actualizar_indicador_led_azul( )
{
  digitalWrite(PIN_LED_RGB_VERDE, false);
  digitalWrite(PIN_LED_RGB_AZUL , true );
  digitalWrite(PIN_LED_RGB_ROJO , false);
}
//----------------------------------------------

//----------------------------------------------
void actualizar_indicador_led_verde( )
{
  digitalWrite(PIN_LED_RGB_VERDE, true );
  digitalWrite(PIN_LED_RGB_AZUL , false);
  digitalWrite(PIN_LED_RGB_ROJO , false);
}
//----------------------------------------------

//----------------------------------------------
void actualizar_indicador_led_rojo( )
{
  digitalWrite(PIN_LED_RGB_VERDE, false);
  digitalWrite(PIN_LED_RGB_AZUL , false);
  digitalWrite(PIN_LED_RGB_ROJO , true );
}
//----------------------------------------------

// ACCIONES 
//----------------------------------------------
void init_()
{
  apagar_leds();
  noTone(pin_alarma);
  current_state = ST_IDLE;
}
//----------------------------------------------

//----------------------------------------------
void error()
{
}
//----------------------------------------------

//----------------------------------------------
void none()
{
}
//----------------------------------------------

//----------------------------------------------
void dist_media_a()
{
  actualizar_indicador_led_azul( );
  current_state = ST_IDLE;
}
//----------------------------------------------

//----------------------------------------------
void dist_lejos_a()
{
  actualizar_indicador_led_verde( );
  noTone(pin_alarma);
  current_state = ST_IDLE;
}
//----------------------------------------------

//----------------------------------------------
void dist_cerca_a()
{
  actualizar_indicador_led_rojo( );
  tone(pin_alarma, 400);
  current_state = ST_WAITING_RESPONSE;
}
//----------------------------------------------

//----------------------------------------------
void dist_cerca()
{
  tone(pin_alarma, 400);
  actualizar_indicador_led_rojo( );
  current_state = ST_WAITING_RESPONSE;
}
//----------------------------------------------

//----------------------------------------------
void dist_media()
{
  actualizar_indicador_led_azul( );
  noTone(pin_alarma);
  current_state = ST_WAITING_RESPONSE;
}
//----------------------------------------------

//----------------------------------------------
void dist_lejos()
{
  noTone(pin_alarma);
  actualizar_indicador_led_verde( );
  current_state = ST_WAITING_RESPONSE;
}
// ---------------------------------------------

//VERIFICAR SENSORES
// ---------------------------------------------
bool verificar_estado_sensor_button_activar()
{
  bool edle_bothom = digitalRead(A0) == HIGH;
  if (edle_bothom) {
    new_event = EV_DIST_LEJOS;
  }
  return edle_bothom;
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_estado_sensor_button_desactivar()
{
  bool edle_bothom = digitalRead(A1) == HIGH;
  if (edle_bothom) {
    new_event = EV_ACK_BOTHOM;
  }
  return edle_bothom;
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_estado_sensor_ultrasonido()
{
  sensores[SENSOR_ULTRASONIDO].valor_actual = FACTOR_DE_ESCALA_CM * leer_distancia_ultrasonido(pin_sensor_ultrasonido, pin_sensor_ultrasonido);
  
  int valor_actual = sensores[SENSOR_ULTRASONIDO].valor_actual;
  int valor_previo = sensores[SENSOR_ULTRASONIDO].valor_previo;
  
  if( valor_actual != valor_previo && current_state != ST_WAITING_RESPONSE)
  {
    sensores[SENSOR_ULTRASONIDO].valor_previo = valor_actual;
    
    if( verificar_umbral_distancia_cerca(valor_actual) )
    {
      new_event = EV_DIST_CERCA;
    }
    else if( verificar_umbral_distancia_media(valor_actual) )
    {
      new_event = EV_DIST_MEDIO;
    }
    if( verificar_umbral_distancia_lejos(valor_actual) )
    {
      new_event = EV_DIST_LEJOS;
    }
    
    return true;
  }
  
  return false;
}
// ---------------------------------------------

//----------------------------------------------
void get_new_event( )
{
  long ct = millis();
  int  diferencia = (ct - lct);
  timeout = (diferencia > UMBRAL_DIFERENCIA_TIMEOUT)? (true):(false);

  if( timeout )
  {
    // Doy acuse de la recepcion del timeout
    timeout = false;
    lct     = ct;
    
    if( (verificar_estado_sensor_ultrasonido() == true) || 
      	(verificar_estado_sensor_button_activar() == true) ||
       	(verificar_estado_sensor_button_desactivar() == true)  
      )
    {
      return;
    }
  }
  
  // Genero evento dummy ....
  new_event = EV_CONT;
}
// ---------------------------------------------

//MAQUINA DE ESTADOS
// ---------------------------------------------
void maquina_estados_detector_presencia()
{
  get_new_event();

  if( (new_event >= 0) && (new_event < MAX_EVENTS) && (current_state >= 0) && (current_state < MAX_STATES) )
  {
    if( new_event != EV_CONT )
    {
    }
    
    
    state_table[current_state][new_event]();
  }
  else
  {
  }
  
  // Consumo el evento...
  new_event   = EV_CONT;	
  
}
//----------------------------------------------


void do_init() 
{
  pinMode(A0, INPUT);
  pinMode(pin_alarma, OUTPUT);
  pinMode(A1, INPUT);
    
  current_state = ST_INIT;// Inicializo el evento inicial
  
  //manejo de timeout
  timeout = false;
  lct     = millis(); 
  
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