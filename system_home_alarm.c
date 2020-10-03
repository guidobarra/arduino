// Habilitacion de debug para la impresion por el puerto serial ...
//----------------------------------------------
#define SERIAL_DEBUG_ENABLED 1

#if SERIAL_DEBUG_ENABLED
  #define DebugPrint(str)\
      {\
        Serial.println(str);\
      }
#else
  #define DebugPrint(str)
#endif

#define DebugPrintEstado(estado,evento)\
      {\
        String est = estado;\
        String evt = evento;\
        String str;\
        str = "-----------------------------------------------------";\
        DebugPrint(str);\
        str = "EST-> [" + est + "]: " + "EVT-> [" + evt + "].";\
        DebugPrint(str);\
        str = "-----------------------------------------------------";\
        DebugPrint(str);\
      }
//----------------------------------------------

//FACTOR DE ESCALA
#define FACTOR_DE_ESCALA_CM                    0.01723

//ERROR
#define ERROR_STRING "-------------ERROR--------------"

//BAUDIOS
#define BAUDIOS_9600                              9600

//CANTIDAD DE SENSORES
#define MAX_CANT_SENSORES                            4

//UMBRALES DE DISTANCIAS
#define UMBRAL_DISTANCIA_MAXIMA                    366
#define UMBRAL_DISTANCIA_LEJOS                     240
#define UMBRAL_DISTANCIA_MEDIO                     120
#define UMBRAL_DISTANCIA_CERCA                       0

//UMBRALES DEL POTENCIOMETRO
#define UMBRAL_POTE_BRILLO_MAX                     680
#define UMBRAL_POTE_BRILLO_MEDIO                   340
#define UMBRAL_POTE_BRILLO_MIN                       0
#define VALOR_MAX_POTE                            1024
#define VALOR_MIN_POTE                               0

//PINES LED RGB
#define PIN_LED_RGB_ROJO                             2
#define PIN_LED_RGB_VERDE                            4
#define PIN_LED_RGB_AZUL                             3

//PINES ULTRASONIDO
#define PIN_ULTRASONIDO_UNO                          7
#define PIN_ULTRASONIDO_DOS                          6
#define PIN_ULTRASONIDO_TRES                         5

//PIN POTENCIOMETRO
#define PIN_POTENCIOMETRO                           A3

//PIN ALARMA
#define PIN_ALARMA                                   8

//PIN PWM
#define PIN_PWM_BRILLO_LED                           9

//PINES BOTHOM
#define PIN_BOTHOM_ACTIVAR_ALARMA                   A1
#define PIN_BOTHOM_DESACTIVAR_ALARMA                A0

//NUMEROS DEL SENSOR DE ULTRASONIDO
#define NRO_UNO_ULTRASONIDO                          0
#define NRO_DOS_ULTRASONIDO                          1
#define NRO_TRES_ULTRASONIDO                         2
#define NRO_UNO_POTE                                 3
#define CANT_SENSOR_ULTRASONIDO                      3

//TIEMPOS
#define DOS_MICROSEGUNDOS                            2
#define DIEZ_MICROSEGUNDOS                          10
#define UMBRAL_DIFERENCIA_TIMEOUT                   50
#define FRECUENCIA_TONO                            400

// CICLO DE TRABAJO
#define DUTY_CYCLE_BRILLO_MAX                      255
#define DUTY_CYCLE_BRILLO_MEDIO                    170
#define DUTY_CYCLE_BRILLO_MIN                       20

// CANTIDAD DE EVENTOS Y ESTADOS
#define MAX_STATES                                   4
#define MAX_EVENTS                                   9

// ESTADOS DEL SENSOR ULTRASONIDO
#define ESTADO_DIST_CERCA                            1
#define ESTADO_DIST_MEDIO                            2
#define ESTADO_DIST_LEJOS                            3
#define ESTADO_DIST_MAXIMO                         100

//VALORES CONSTANTES
#define CERO                                         0
#define DESVIO                                       5

enum states          { ST_INIT,  ST_ALARMA_DESACTIVADA        , ST_ALARMA_ACTIVADA   , ST_ERROR                                                  } current_state;
String states_s [] = {"ST_INIT", "ST_ALARMA_DESACTIVADA"      , "ST_ALARMA_ACTIVADA" , "ST_ERROR"                                                };

enum events          { EV_CONT ,   EV_DIST_LEJOS , EV_DIST_MEDIO   , EV_DIST_CERCA  , EV_POTE_BRILLO_MAX  , EV_POTE_BRILLO_MEDIO  , EV_POTE_BRILLO_MIN  , EV_PUL_ACTIVAR  , EV_PUL_DESACTIVAR  , EV_UNKNOW  } new_event;
String events_s [] = {"EV_CONT",  "EV_DIST_LEJOS", "EV_DIST_MEDIO" , "EV_DIST_CERCA", "EV_POTE_BRILLO_MAX", "EV_POTE_BRILLO_MEDIO", "EV_POTE_BRILLO_MIN", "EV_PUL_ACTIVAR", "EV_PUL_DESACTIVAR", "EV_UNKNOW"};

typedef void (*transition)();

transition state_table[MAX_STATES][MAX_EVENTS] =
{
      {init_    , error          , error         , error         , error              , error                 , error             , error           , error        } , // state ST_INIT
      {none     , dist_lejos   	 , dist_media    , dist_cerca    , none               , none                  , none              , dist_cerca      , none         } , // state ST_ALARMA_DESACTIVADA
      {none     , none           , none          , dist_cerca    , brillo_max         , brillo_media          , brillo_min        , dist_cerca      , init_        } , // state ST_ALARMA_ACTIVADA
      {error    , error          , error         , error         , error              , error                 , error             , error           , error        }   // state ST_ERROR

     //EV_CONT  , EV_DIST_LEJOS  , EV_DIST_MEDIO , EV_DIST_CERCA , EV_POTE_BRILLO_MAX , EV_POTE_BRILLO_MEDIO  , EV_POTE_BRILLO_MIN, "EV_PUL_ACTIVAR", "EV_PUL_DESACTIVAR"
};

struct stSensor
{
  int  pin;
  int  estado;
  long valor_actual;
  long valor_previo;
};
stSensor sensores[MAX_CANT_SENSORES];

int g_pin_alarma = PIN_ALARMA;

int g_pin_ultrasonido_uno = PIN_ULTRASONIDO_UNO;

int g_pin_ultrasonido_dos = PIN_ULTRASONIDO_DOS;

int g_pin_ultrasonido_tres = PIN_ULTRASONIDO_TRES;

bool timeout;

long lct;

//FUNCIONES
//inicializacion de los pines y sensores
//----------------------------------------------
void init_sensor_ultrasonido(int nro_sensor_ultra, int pin_sensor_ultra)
{
  sensores[nro_sensor_ultra].valor_actual = CERO;
  sensores[nro_sensor_ultra].valor_previo = CERO;
  sensores[nro_sensor_ultra].estado = ESTADO_DIST_MAXIMO;
  sensores[nro_sensor_ultra].pin = pin_sensor_ultra;
}
//----------------------------------------------

//----------------------------------------------
void init_sensor_potenciometro(int nro_sensor_pote, int pin_sensor_pote)
{
  sensores[nro_sensor_pote].valor_actual = CERO;
  sensores[nro_sensor_pote].valor_previo = CERO;
  sensores[nro_sensor_pote].estado = CERO;
  sensores[nro_sensor_pote].pin = pin_sensor_pote;
}
//----------------------------------------------

//----------------------------------------------
void init_pines()
{
  pinMode(PIN_BOTHOM_DESACTIVAR_ALARMA, INPUT);
  pinMode(g_pin_alarma, OUTPUT);
  pinMode(PIN_BOTHOM_ACTIVAR_ALARMA, INPUT);
}
//----------------------------------------------

//----------------------------------------------
void init_sensores()
{
  init_sensor_ultrasonido(NRO_UNO_ULTRASONIDO, g_pin_ultrasonido_uno);
  init_sensor_ultrasonido(NRO_DOS_ULTRASONIDO, g_pin_ultrasonido_dos);
  init_sensor_ultrasonido(NRO_TRES_ULTRASONIDO, g_pin_ultrasonido_tres);
  init_sensor_potenciometro(NRO_UNO_POTE, PIN_POTENCIOMETRO);
}
//----------------------------------------------

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
	  sensores[num_sensor_ultrasonido].estado = ESTADO_DIST_CERCA;
    }
    else if( verificar_umbral_distancia_media(valor_actual) )
    {
	  sensores[num_sensor_ultrasonido].estado = ESTADO_DIST_MEDIO;
    }
    else if( verificar_umbral_distancia_lejos(valor_actual) )
    {
	  sensores[num_sensor_ultrasonido].estado = ESTADO_DIST_LEJOS;
    } else
    {
      return false;
    }

    return true;
}
// --------------------------------------------

//ORDEN DE PRIORIDAD
// ESTADO_DIST_CERCA, ESTADO_DIST_MEDIO, ESTADO_DIST_LEJOS, ESTADO_DIST_MAXIMO
// --------------------------------------------
int get_mayor_prioridad_sensores_ultrasonido()
{

  int mayor_prioridad_estado = ESTADO_DIST_MAXIMO; // tiene la menor prioridad
  //recorro los sensores de ultrasonido y me quedo con el que tiene la mayor prioridad
  for(int i=CERO; i < CANT_SENSOR_ULTRASONIDO; i++)
  {
  	if( sensores[i].estado < mayor_prioridad_estado )
  	{
  		mayor_prioridad_estado = sensores[i].estado;
  	}
  }

  // retorno el estado de mayor prioridad de los sensores
  return mayor_prioridad_estado;
}
// --------------------------------------------

// --------------------------------------------
bool event_mayor_prioridad_sensor_distancia(int estado_mayor_prioridad)
{
  if( estado_mayor_prioridad ==  ESTADO_DIST_CERCA)
  {
    new_event = EV_DIST_CERCA;
  }
  else if( estado_mayor_prioridad ==  ESTADO_DIST_MEDIO )
  {
    new_event = EV_DIST_MEDIO;
  }
  else if( estado_mayor_prioridad ==  ESTADO_DIST_LEJOS  )
  {
    new_event = EV_DIST_LEJOS;
  } else
  {
    return false;
  }

  return true;
}
// --------------------------------------------

// ---------------------------------------------
bool verificar_umbrales_potenciometro(int valor_pote, int num_potenciometro)
{
  if( verificar_umbral_pote_brillo_min(valor_pote) )
  {
    sensores[num_potenciometro].estado = EV_POTE_BRILLO_MIN;
    new_event = EV_POTE_BRILLO_MIN;
  }
  else if( verificar_umbral_pote_brillo_medio(valor_pote) )
  {
    sensores[num_potenciometro].estado = EV_POTE_BRILLO_MEDIO;
    new_event = EV_POTE_BRILLO_MEDIO;
  }
  else if( verificar_umbral_pote_brillo_max(valor_pote) )
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
void actualizar_indicador_led_amarrillo( )
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

//----------------------------------------------
void actualizar_led_brillo_apagar( )
{
  analogWrite(PIN_PWM_BRILLO_LED, CERO);
}
//----------------------------------------------

// ACCIONES
//----------------------------------------------
void init_()
{
  DebugPrintEstado(states_s[current_state], events_s[new_event]);
  apagar_leds();
  actualizar_led_brillo_apagar();
  noTone(g_pin_alarma);
  init_pines();
  init_sensores();
  current_state = ST_ALARMA_DESACTIVADA;
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

//ACCIONES DE BRILLO
//----------------------------------------------
void brillo_min()
{
  actualizar_led_brillo_min( );
  current_state = ST_ALARMA_ACTIVADA;
}
//----------------------------------------------

//----------------------------------------------
void brillo_media()
{
  actualizar_led_brillo_medio( );
  current_state = ST_ALARMA_ACTIVADA;
}
//----------------------------------------------

//----------------------------------------------
void brillo_max()
{
  actualizar_led_brillo_max( );
  current_state = ST_ALARMA_ACTIVADA;
}
//----------------------------------------------

//ACCIONES DE DESTANCIA
//----------------------------------------------
void dist_lejos()
{
  actualizar_indicador_led_verde( );
  current_state = ST_ALARMA_DESACTIVADA;
}
//----------------------------------------------

//----------------------------------------------
void dist_media()
{
  actualizar_indicador_led_amarrillo( );
  current_state = ST_ALARMA_DESACTIVADA;
}
//----------------------------------------------

//----------------------------------------------
void dist_cerca()
{
  actualizar_indicador_led_rojo( );
  actualizar_led_brillo_min( );
  tone(g_pin_alarma, FRECUENCIA_TONO);
  current_state = ST_ALARMA_ACTIVADA;
}
//----------------------------------------------

//VERIFICAR SENSORES
// ---------------------------------------------
bool verificar_estado_sensor_button_desactivar()
{
  bool resp = (digitalRead(PIN_BOTHOM_DESACTIVAR_ALARMA) == HIGH);
  if (resp)
  {
    new_event = EV_PUL_DESACTIVAR;
  }
  return resp;
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_estado_sensor_button_activar()
{
  bool resp = (digitalRead(PIN_BOTHOM_ACTIVAR_ALARMA) == HIGH);
  if (resp)
  {
    new_event = EV_PUL_ACTIVAR;
  }
  return resp;
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_estado_sensor_ultrasonido(int num_sensor_ultrasonido)
{
  int nro_pin = sensores[num_sensor_ultrasonido].pin;
  sensores[num_sensor_ultrasonido].valor_actual = FACTOR_DE_ESCALA_CM * leer_distancia_ultrasonido(nro_pin, nro_pin);

  int valor_actual = sensores[num_sensor_ultrasonido].valor_actual;
  int valor_previo = sensores[num_sensor_ultrasonido].valor_previo;

  if( (valor_actual - valor_previo) > DESVIO || (valor_previo - valor_actual) > DESVIO )
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

  if( (valor_actual - valor_previo) > DESVIO || (valor_previo - valor_actual) > DESVIO )
  {
    sensores[num_potenciometro].valor_previo = valor_actual;

    return verificar_umbrales_potenciometro(valor_actual, num_potenciometro);
  }

  return false;

}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_sensores_ultrasonido()
{
  bool resp_sensores_ultrasonido_uno = (verificar_estado_sensor_ultrasonido(NRO_UNO_ULTRASONIDO) == true);
  bool resp_sensores_ultrasonido_dos = (verificar_estado_sensor_ultrasonido(NRO_DOS_ULTRASONIDO) == true);
  bool resp_sensores_ultrasonido_tres = (verificar_estado_sensor_ultrasonido(NRO_TRES_ULTRASONIDO) == true);
  event_mayor_prioridad_sensor_distancia(get_mayor_prioridad_sensores_ultrasonido());

  return (resp_sensores_ultrasonido_uno || resp_sensores_ultrasonido_dos || resp_sensores_ultrasonido_tres);
}
// ---------------------------------------------

// ---------------------------------------------
bool verificar_sensores()
{
  return ( (verificar_sensores_ultrasonido() == true) ||
      	   (verificar_estado_sensor_button_activar() == true) ||
       	   (verificar_estado_sensor_button_desactivar() == true) ||
		   (verificar_estado_sensor_potenciometro(NRO_UNO_POTE) == true)
		 );
}
//----------------------------------------------

//----------------------------------------------
void get_new_event( )
{
  long ct = millis();
  int  diferencia = (ct - lct);
  timeout = (diferencia > UMBRAL_DIFERENCIA_TIMEOUT)? (true):(false);

  if( timeout )
  {
    // recepcion del timeout consumida
    timeout = false;
    lct     = ct;

    if( verificar_sensores() )
    {
      return;
    }
  }

  new_event = EV_CONT;// Genero evento dummy ....
}
// ---------------------------------------------

//MAQUINA DE ESTADOS
// ---------------------------------------------
void maquina_estados_detector_presencia()
{
  get_new_event();

  if( (new_event >= CERO) && (new_event < MAX_EVENTS) && (current_state >= CERO) && (current_state < MAX_STATES) )
  {
	if( new_event != EV_CONT )
    {
      DebugPrintEstado(states_s[current_state], events_s[new_event]);
    }
    state_table[current_state][new_event]();
  }
  else
  {
    Serial.print(ERROR_STRING);
    DebugPrintEstado(states_s[ST_ERROR], events_s[EV_UNKNOW]);
  }
  
  new_event   = EV_CONT;// Consumo el evento...

}
//----------------------------------------------

//----------------------------------------------
void do_init()
{
  //manejo de timeout
  timeout = false;
  lct     = millis();

  current_state = ST_INIT;// Inicializo el evento inicial

  noTone(g_pin_alarma);
  Serial.begin(BAUDIOS_9600);
}
//----------------------------------------------

// Funciones de arduino. 
//----------------------------------------------
void setup()
{
  do_init();
}
//----------------------------------------------

//----------------------------------------------
void loop()
{
  maquina_estados_detector_presencia();
}
//----------------------------------------------