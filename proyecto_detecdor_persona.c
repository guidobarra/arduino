//FACTOR DE ESCALA
#define FACTOR_DE_ESCALA_CM 0.01723

//BAUDIOS
#define BAUDIOS_9600 9600

//CANTIDAD DE SENSORES
#define MAX_CANT_SENSORES                           2

//UMBRALES DE DISTANCIAS
#define UMBRAL_DISTANCIA_MAXIMA                    366
#define UMBRAL_DISTANCIA_LEJOS                     240
#define UMBRAL_DISTANCIA_MEDIO                     120
#define UMBRAL_DISTANCIA_CERCA                       0

//UMBRALES DEL POTENCIOMETRO
#define UMBRAL_POTE_BRILLO_MAX                     680
#define UMBRAL_POTE_BRILLO_MEDIO                   340
#define UMBRAL_POTE_BRILLO_MIN                     0
#define VALOR_MAX_POTE 1024
#define VALOR_MIN_POTE 0

//PINES LED RGB
#define PIN_LED_RGB_ROJO     2
#define PIN_LED_RGB_VERDE    4
#define PIN_LED_RGB_AZUL     3

//OTROS PINES
#define PIN_ULTRASONIDO 7
#define PIN_POTENCIOMETRO A3
#define PIN_ALARMA 8
#define PIN_PWM_BRILLO_LED 9

#define PIN_BOTHOM_ACTIVAR_ALARMA    A1
#define PIN_BOTHOM_DESACTIVAR_ALARMA A0

//NUMEROS DEL SENSOR DE ULTRASONIDO 
#define SENSOR_ULTRASONIDO_UNO   0
#define SENSOR_ULTRASONIDO_DOS   1

//TIEMPOS
#define DOS_MICROSEGUNDOS                      2
#define DIEZ_MICROSEGUNDOS                     10
#define UMBRAL_DIFERENCIA_TIMEOUT                   50
#define FRECUENCIA_TONO                        400

// CICLO DE TRABAJO
#define DUTY_CYCLE_BRILLO_MAX                     255
#define DUTY_CYCLE_BRILLO_MEDIO                   170
#define DUTY_CYCLE_BRILLO_MIN                     85

// CANTIDAD DE EVENTOS Y ESTADOS
#define MAX_STATES 4
#define MAX_EVENTS 8

//VALORES CONSTANTES
#define CERO 0

enum states          { ST_INIT,  ST_IDLE        , ST_WAITING_RESPONSE   , ST_ERROR                                                  } current_state;
String states_s [] = {"ST_INIT", "ST_IDLE"      , "ST_WAITING_RESPONSE" , "ST_ERROR"                                                };

enum events          { EV_CONT,   EV_DIST_LEJOS   , EV_DIST_MEDIO  , EV_DIST_CERCA  , EV_ACK_BOTHOM,     EV_POTE_BRILLO_MAX  , EV_POTE_BRILLO_MEDIO  , EV_POTE_BRILLO_MIN} new_event;
String events_s [] = {"EV_CONT",  "EV_DIST_LEJOS", "EV_DIST_MEDIO" , "EV_DIST_CERCA", "EV_ACK_BOTHOM", "EV_POTE_BRILLO_MAX" , "EV_POTE_BRILLO_MEDIO", "EV_POTE_BRILLO_MIN"  };

typedef void (*transition)();

transition state_table[MAX_STATES][MAX_EVENTS] =
{
      {init_    , error          , error         , error         , error        , error              , error                  , error } , // state ST_INIT
      {none     , dist_lejos   	 , dist_media    , dist_cerca    , dist_cerca   , brillo_max         , brillo_media           , brillo_min} , // state ST_IDLE
      {none     , none           , none          , none          , none         , brillo_max         , brillo_media           , brillo_min} , // state ST_WAITING_RESPONSE
      {error    , error          , error         , error         , error        , error              , error                  , error}   // state ST_ERROR
      
     //EV_CONT  , EV_DIST_LEJOS  , EV_DIST_MEDIO , EV_DIST_CERCA , EV_ACK_BOTHOM, EV_POTE_BRILLO_MAX  , EV_POTE_BRILLO_MEDIO  , EV_POTE_BRILLO_MIN  
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

int pin_ultrasonido = PIN_ULTRASONIDO;

bool timeout;

bool alarma_activa;

long lct;

//FUNCIONES

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
bool verificar_umbral_pote_brillo_max(int valor_pote) {
  
  return (UMBRAL_POTE_BRILLO_MAX <= valor_pote && valor_pote < VALOR_MAX_POTE);
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_umbral_pote_brillo_medio(int valor_pote) {
  
  return (UMBRAL_POTE_BRILLO_MEDIO <= valor_pote && valor_pote < UMBRAL_POTE_BRILLO_MAX);
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_umbral_pote_brillo_min(int valor_pote) {
  
  return (UMBRAL_POTE_BRILLO_MIN <= valor_pote && valor_pote < UMBRAL_POTE_BRILLO_MEDIO);
}
// ---------------------------------------------

//VERIFICAR UMBRALES
// ---------------------------------------------
bool verificar_umbrales_distancia(int valor_actual, int num_sensor_ultrasonido)
{
  if( verificar_umbral_distancia_cerca(valor_actual) )
    {
      sensores[num_sensor_ultrasonido].estado = EV_DIST_CERCA;
      new_event = EV_DIST_CERCA;
    }
    else if( verificar_umbral_distancia_media(valor_actual) )
    {
      sensores[num_sensor_ultrasonido].estado = EV_DIST_MEDIO;
      new_event = EV_DIST_MEDIO;
    }
    else if( verificar_umbral_distancia_lejos(valor_actual) )
    {
      sensores[num_sensor_ultrasonido].estado = EV_DIST_LEJOS;
      new_event = EV_DIST_LEJOS;
    } else 
    {
      return false;
    }
    
    return true;	
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_umbrales_potenciometro(int valor_pote, int num_potenciometro)
{		
  if( alarma_activa == true && verificar_umbral_pote_brillo_min(valor_pote) )
    {
      sensores[num_potenciometro].estado = EV_POTE_BRILLO_MIN;
      new_event = EV_POTE_BRILLO_MIN;
    }
    else if( alarma_activa == true && verificar_umbral_pote_brillo_medio(valor_pote) )
    {
      sensores[num_potenciometro].estado = EV_POTE_BRILLO_MEDIO;
      new_event = EV_POTE_BRILLO_MEDIO;
    }
    else if( alarma_activa == true && verificar_umbral_pote_brillo_max(valor_pote) )
    {
      sensores[num_potenciometro].estado = EV_POTE_BRILLO_MAX;
      new_event = EV_POTE_BRILLO_MAX;
    } else 
    {
      return false;
    }
    
    return true;	
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

// ---------------------------------------------
long leer_potenciometro()
{
  return analogRead(PIN_POTENCIOMETRO);
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
  digitalWrite(PIN_LED_RGB_VERDE, true);
  digitalWrite(PIN_LED_RGB_AZUL , false);
  digitalWrite(PIN_LED_RGB_ROJO , true);
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

// ACTUALIZAR BRILLO LED
//----------------------------------------------
void actualizar_led_brillo_max( )
{
  analogWrite(PIN_PWM_BRILLO_LED, DUTY_CYCLE_BRILLO_MAX);
}
//----------------------------------------------

//----------------------------------------------
void actualizar_led_brillo_medio( )
{
  analogWrite(PIN_PWM_BRILLO_LED, DUTY_CYCLE_BRILLO_MEDIO);
}
//----------------------------------------------

//----------------------------------------------
void actualizar_led_brillo_min( )
{
  analogWrite(PIN_PWM_BRILLO_LED, DUTY_CYCLE_BRILLO_MIN);
}
//----------------------------------------------

// ACCIONES 
//----------------------------------------------
void init_()
{
  apagar_leds();
  noTone(pin_alarma);
  alarma_activa = false;
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

//ACCIONES DE DESTANCIA
//----------------------------------------------
void dist_media()
{
  actualizar_indicador_led_azul( );
  current_state = ST_IDLE;
}
//----------------------------------------------

//----------------------------------------------
void dist_lejos()
{
  actualizar_indicador_led_verde( );
  current_state = ST_IDLE;
}
//----------------------------------------------

//----------------------------------------------
void dist_cerca()
{
  actualizar_indicador_led_rojo( );
  tone(pin_alarma, FRECUENCIA_TONO);
  current_state = ST_WAITING_RESPONSE;
  alarma_activa = true;
}
//----------------------------------------------

//ACCIONES DE BRILLO
//----------------------------------------------
void brillo_min()
{
  actualizar_led_brillo_min( );
  current_state = ST_IDLE;
  Serial.print(sensores[1].estado);
  Serial.print("\n");
}
//----------------------------------------------

//----------------------------------------------
void brillo_media()
{
  actualizar_led_brillo_medio( );
  current_state = ST_IDLE;
  Serial.print(sensores[1].estado);
  Serial.print("\n");
}
//----------------------------------------------

//----------------------------------------------
void brillo_max()
{
  actualizar_led_brillo_max( );
  current_state = ST_IDLE;
  Serial.print(sensores[1].estado);
  Serial.print("\n");
}
//----------------------------------------------

//VERIFICAR SENSORES
// ---------------------------------------------
bool verificar_estado_sensor_button_activar()
{
  bool edle_bothom = digitalRead(A0) == HIGH;
  if (edle_bothom) {
    new_event = EV_CONT;
    current_state = ST_INIT;
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
bool verificar_estado_sensor_ultrasonido(int num_sensor_ultrasonido)
{
  sensores[num_sensor_ultrasonido].valor_actual = FACTOR_DE_ESCALA_CM * leer_distancia_ultrasonido(pin_ultrasonido, pin_ultrasonido);
  
  int valor_actual = sensores[num_sensor_ultrasonido].valor_actual;
  int valor_previo = sensores[num_sensor_ultrasonido].valor_previo;
  
  if( valor_actual != valor_previo)
  {
    sensores[num_sensor_ultrasonido].valor_previo = valor_actual;
    
    return verificar_umbrales_distancia(valor_actual, num_sensor_ultrasonido);
  }
  
  return false;
  
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_estado_sensor_potenciometro(int num_potenciometro)
{
  sensores[num_potenciometro].valor_actual = leer_potenciometro();
  
  int valor_actual = sensores[num_potenciometro].valor_actual;
  int valor_previo = sensores[num_potenciometro].valor_previo;
  
  if( valor_actual != valor_previo)
  {
    sensores[num_potenciometro].valor_previo = valor_actual;
    
    return verificar_umbrales_potenciometro(valor_actual, num_potenciometro);
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
    
    if( (verificar_estado_sensor_ultrasonido(SENSOR_ULTRASONIDO_UNO) == true) || 
      	(verificar_estado_sensor_button_activar() == true) ||
       	(verificar_estado_sensor_button_desactivar() == true) ||
		(verificar_estado_sensor_potenciometro(1) == true)	
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
  
  
  new_event   = EV_CONT;// Consumo el evento...
  
}
//----------------------------------------------

//----------------------------------------------
void init_sensor_ultrasonido(int nro_sensor_ultra, int pin_sensor_ultra)
{
  sensores[nro_sensor_ultra].valor_actual = CERO;
  sensores[nro_sensor_ultra].valor_previo = CERO;
  sensores[nro_sensor_ultra].estado = CERO;
  sensores[nro_sensor_ultra].pin = pin_sensor_ultra;	
} 
//----------------------------------------------

//----------------------------------------------
void do_init() 
{
  pinMode(A0, INPUT);
  pinMode(pin_alarma, OUTPUT);
  pinMode(A1, INPUT);
  
  current_state = ST_INIT;// Inicializo el evento inicial
  
  init_sensor_ultrasonido(SENSOR_ULTRASONIDO_UNO, pin_ultrasonido);
  
  //manejo de timeout
  timeout = false;
  lct     = millis(); 
  
  alarma_activa = false;
  
  noTone(pin_alarma);
  Serial.begin(BAUDIOS_9600);
}
//----------------------------------------------

void setup()
{
  do_init();
}

void loop()
{
  maquina_estados_detector_presencia();
}