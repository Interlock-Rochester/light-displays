/* Arlene
**  Arduino+Relay Light Engine (Nee Engine)
**
**  Conceptually based on Computer Science House's "DAMIT"
**  Digital And Audio Manipulation of Incandescent Technology
**
**  
*/

#define kPinLED (8)
#define kPinButton (A2)

static int outPins[8] = { 11, 10, 13, 12, 5, 7, A3, 3 };



void setup() {
  // initialize serial:
  Serial.begin( 19200 );
  Serial.println( "Ready." );
  
  // LED
  pinMode( kPinLED, OUTPUT);
  digitalWrite( kPinLED, LOW );
  
  // button!
  pinMode( kPinButton, INPUT_PULLUP );
   
  // make the pins outputs:
  for( int i=0 ; i<8 ; i++ ) {
    pinMode( outPins[i], OUTPUT );
    digitalWrite( outPins[i], LOW );
  }
}

void displayByte( unsigned char b )
{
  digitalWrite( outPins[0], ( b & 0x01 )? HIGH : LOW );
  digitalWrite( outPins[1], ( b & 0x02 )? HIGH : LOW );
  digitalWrite( outPins[2], ( b & 0x04 )? HIGH : LOW );
  digitalWrite( outPins[3], ( b & 0x08 )? HIGH : LOW );
  digitalWrite( outPins[4], ( b & 0x10 )? HIGH : LOW );
  digitalWrite( outPins[5], ( b & 0x20 )? HIGH : LOW );
  digitalWrite( outPins[6], ( b & 0x40 )? HIGH : LOW );
  digitalWrite( outPins[7], ( b & 0x80 )? HIGH : LOW );
}


unsigned char patterns[11][16] = {
  {  8,  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }, /* up roll */
  {  8,  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 }, /* down roll */
  { 14,  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02 }, /* cylon */
  {  2,  0xaa, 0x55 }, /* flasher */
  {  3,  0x49, 0x92, 0x24 }, /* marquee */ 
  {  4,  0x11, 0x22, 0x44, 0x88 }, /* roll left */
  {  4,  0x88, 0x44, 0x22, 0x11 }, /* roll left */
  {  2,  0x00, 0xff }, /* flash */
  {  1,  0x00 }, /* off */
  {  1,  0xFF }, /* on */
  {  0 }
};

static int pattern = -1;
static int patlen = -1;
static int patstep = -1;
static unsigned long timeout = 0;
static long patTimeout = 125; /* milliseconds per step */

static int indicator = LOW;


void newPattern( int p )
{
  if( p > 9 ) p = 0;
  if( p < 0 ) p = 0;
  
  if( p == pattern ) return;
  
  pattern = p;
  patlen = patterns[p][0];
  patstep = 0;
  timeout = millis() + patTimeout;
  
  switch( p ) {
  case( 0 ): Serial.println( "Up Roll" ); break;
  case( 1 ): Serial.println( "Down Roll" ); break;
  case( 2 ): Serial.println( "Cylon" ); break;
  case( 3 ): Serial.println( "2-Marquee" ); break;
  case( 4 ): Serial.println( "3-Marquee" ); break;
  case( 5 ): Serial.println( "Roll Left" ); break;
  case( 6 ): Serial.println( "Roll Right" ); break;
  case( 7 ): Serial.println( "Flash" ); break;
  case( 8 ): Serial.println( "Off" ); break;
  case( 9 ): Serial.println( "On" ); break;
  }
}

#define kStep (25)

long taps[8];
#define kLongPressDuration (1000)

static long lastPressStart = 0;
static long thisPressDelta = 0;
static int lastPress = -1;
static int pressHandled = 0;

void buttonDown()
{
  // for tap tempo
  thisPressDelta = millis() - lastPressStart;
  
  // for long press detect
  lastPressStart = millis();
  pressHandled = 0;
}

void buttonPressing()
{
  long thisDuration = millis() - lastPressStart;
  
  if( pressHandled ) return;
  
  if( thisDuration > kLongPressDuration ) {    
    /* clear for a moment */
    displayByte( 0x00 );
    delay( 50 );

    /* long press, next pattern */      
    newPattern( pattern+1 );
    pressHandled = 1;
  }
}

void buttonUp()
{
  long thisDuration = millis() - lastPressStart;
    
  if( thisDuration > kLongPressDuration || thisPressDelta > kLongPressDuration ) {
    /* handle the case where the previous press was a long press */
    /* throw it out. */
    
    return;    
  }
  
  for( int i=7 ; i>0 ; i-- ) {
    taps[i] = taps[i-1];
  }
  taps[0] = thisPressDelta;
  
  long avg = 0;
  for( int j=0 ; j<4 ; j++ ) {
    //Serial.println( taps[j], DEC );
    avg += taps[j];
  }
  patTimeout = avg/4;
}



void checkButton()
{
  int thisPress = digitalRead( kPinButton );
    
  if( thisPress == LOW && lastPress == LOW ) {
    /* HOLDING */
    buttonPressing();    
  }
  
  if( thisPress == lastPress ) return;
  
  /* was it a button press? */
  if( thisPress == LOW && lastPress == HIGH ) {
    /* PRESS */
    buttonDown();
    delay( 50 ); // cheapo debounce
  }

  
  if( thisPress == HIGH && lastPress == LOW ) {
    /* RELEASE */
    buttonUp();
  }
  lastPress = thisPress;
}

void flash()
{
  displayByte( 0xFF );
  delay( 100 );
  displayByte( 0x00 );
}

void off()
{
  displayByte( 0x00 );
  delay( 500 );
}

void loop()
{
  long lpt = patTimeout;
  
  checkButton();
  
  if( Serial.available() ) {
    char ch = Serial.read();
    if( ch == '1' ) newPattern( 0 );
    if( ch == '2' ) newPattern( 1 );
    if( ch == '3' ) newPattern( 2 );
    if( ch == '4' ) newPattern( 3 );
    if( ch == '5' ) newPattern( 4 );
    if( ch == '6' ) newPattern( 5 );
    if( ch == '7' ) newPattern( 6 );
    if( ch == '8' ) newPattern( 7 );
    if( ch == '9' ) newPattern( 8 );
    if( ch == '0' ) newPattern( 9 );
    if( ch == 'b' ) { buttonDown(); buttonUp(); }
    if( ch == 'f' ) flash();
    if( ch == 'o' ) off();
    
    if( ch == '-' ) patTimeout -= kStep;
    if( ch == '_' ) patTimeout -= kStep;
    if( ch == '=' ) patTimeout += kStep;
    if( ch == '+' ) patTimeout += kStep;
  }
  if( patTimeout <= 0 ) patTimeout = 0;
  
  if( patTimeout != lpt ) {
    Serial.print( ">> " );
    Serial.print( patTimeout, DEC );
    Serial.println( " ms" );
  }
  
  if( pattern == -1 ) {
    newPattern( 2 );
  }
  
  /* step if there's a timeout */
  if( millis() > timeout ) {
    patstep++;
    if( patstep >= patlen ) {
      patstep = 0;
    }
    timeout = millis() + patTimeout;
    
    if( indicator == LOW )
    {
      indicator = HIGH;
    } else {
      indicator = LOW;
    }
    digitalWrite( kPinLED, indicator );
  }
  
  displayByte( patterns[pattern][patstep+1] );
}








