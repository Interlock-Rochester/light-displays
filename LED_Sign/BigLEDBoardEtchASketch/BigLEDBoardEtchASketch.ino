/* EtchASketch
 * 
 * v1.01  2016-11-26 sdl
 *        three cursor/color modes (erase, draw, skip)
 *
 * v1.00  2016-11-25 sdl
 *        using display driver from Will Kommeritz
 * 
 * 
 * Hook up the LED display on pins 2-10
 * A0 ---o -+- o---[gnd]   (Button A)
 * A1 ---o -+- o---[gnd]   (Button B)
 * A2 - wiper on pot for x position - +5/GND to outer connections
 * A3 - wiper on pot for y position - +5/GND to outer connections
 * 
 */

/* Button A: toggle draw/erase/skip
 * Button B: tap Save to EEProm
 *           hold and tap A to load preset patterns
 */

#include <EEPROM.h>

// ==================== ARDUINO BITMASK INFO ====================
// dataA = 2           PD2   pin 1 on CN0/CN1  active HIGH
// dataB = 3           PD3   pin 2 on CN0/CN1  active HIGH
// dataC = 4           PD4   pin 3 on CN0/CN1  active HIGH
// G2AU38_ENABLE = 5   PD5   pin 4 on CN0/CN1  active LOW top
// G2AU37_ENABLE = 6   PD6   pin 5 on CN0/CN1  active LOW botom
// G1_ENABLE = 7       PD7   pin 6 on CN0/CN1  active HIGH

// G2B_ENABLE = 8      PB0   pin 7 on CN0/CN1  active LOW
// serialClock = 9     PB1   pin 8 on CN0/CN1 B1 for 9  rising edge
// serialData = 10     PB2   pin 9 on CN0/CN1 B2 for 10  active HIGH

/* frame buffer */
#define kDisplayWidth  (85)                      /* width of the display in columns of 16 bit words */
unsigned int displayBuffer[kDisplayWidth] = {0}; /* buffer to store the active frame */

/* user interface pin usage */
#define kPinButtonA (A0)
#define kPinButtonB (A1) /* closer to device */
#define kPinKnob1   (A2)
#define kPinKnob2   (A3)

void setup()
{
  /* start up serial */
  Serial.begin(115200);
  Serial.println( "Ready." );

  /* initialize user interface */
  pinMode( kPinButtonA, INPUT_PULLUP );
  pinMode( kPinButtonB, INPUT_PULLUP );
  pinMode( kPinKnob1, INPUT );
  pinMode( kPinKnob2, INPUT );

  /* initialize board IO */
  // DDRB and DDRD 1 is output 0 is input
  DDRD = B11111110; PORTD = B11100010;
  DDRB = B00000111; PORTB = B00000000;
  displayClear();  //reset the display
  bufferClear();

  /* preload the buffer from the EEPROM */
  LoadBufferFromEEPROM();
}


/* draw/nav mode */
#define kDrawColorErase  (0)
#define kDrawColorDraw   (1)
#define kDrawColorSkip   (2)
#define kDrawColorMod    (3) /* largest +1 */
unsigned char drawColor = kDrawColorSkip;

/* fill mode */
#define kFillModeClear (0)
#define kFillModeFill  (1)
#define kFillModeCount (2)
#define kFillModeF0    (3)
#define kFillMode0F    (4)
#define kFillModeEE    (5)
#define kFillModeMax   (5) /* same as last mode */
unsigned char fillMode = kFillModeClear;


/* ***********************************
 *  user interface and main loop
 */
 
void buttonPoll()
{
  static unsigned char lastA = 1;
  static unsigned char lastB = 1;
  unsigned char a,b;

  a = digitalRead( kPinButtonA );
  b = digitalRead( kPinButtonB );

  if( a != lastA ) {
    if( a == LOW ) { /* high-to-low */
      delay( 50 ); /* cheapo-debounce */      
      SaveBufferToEEPROM();
    }
  }

  /* only change color with B if A is not being pressed */
  if( a != LOW ) {
    if( b != lastB ) {
      if( b == LOW ) { /* high-to-low */
        delay( 50 ); /* cheapo-debounce */
        drawColor = (drawColor+1) % kDrawColorMod;
      }
    }
  } else {
    if( b != lastB ) {
      if( b == LOW ) { /* high-to-low */
        delay( 50 ); /* cheapo-debounce */
        fillMode++;
        if( fillMode > kFillModeMax ) fillMode = 0;

        switch( fillMode ) {
        case( kFillModeClear ): bufferFill( 0x0000 );   break;
        case( kFillModeFill ):  bufferFill( 0xFFFF );   break;
        case( kFillModeCount ): bufferFillCounting();   break;
        case( kFillModeF0 ):    bufferFill( 0xF0F0 );   break;
        case( kFillMode0F ):    bufferFill( 0x0F0F );   break;
        case( kFillModeEE ):
        default:                LoadBufferFromEEPROM(); break;
        }
      }
    }    
  }

  lastA = a;
  lastB = b;  
}

void loop() 
{
  buttonPoll();
  bufferFillSketch();
  bufferToDisplay( 50 );  
}


/* ***********************************
 *  Buffer fillers
 */

void bufferFill( int val) 
{
  for (int i = 0; i < kDisplayWidth; i++) {
    displayBuffer[i] = val;
  }
}

void bufferClear()
{
  bufferFill( 0x0000 );
}

void bufferInvert() 
{
  for (int i = 0 ; i < kDisplayWidth ; i++) {
    displayBuffer[i] = ~displayBuffer[i];  //invert colors
  }
}

 
void bufferFillCounting( void )
{
  for( int i=0; i < kDisplayWidth ; i++ )
  {
    int j = 0;
    
    if( i&0x01 ) j |= 0x80;
    if( i&0x02 ) j |= 0x40;
    if( i&0x04 ) j |= 0x20;
    if( i&0x08 ) j |= 0x10;
    if( i&0x10 ) j |= 0x08;
    if( i&0x20 ) j |= 0x04;
    if( i&0x40 ) j |= 0x02;
    if( i&0x80 ) j |= 0x01;
    
    displayBuffer[i] = i<<8 | (j);
  }
}

int yToBit( int y )
{
  if( y < 0 ) y = 0;
  if( y > 15 ) y = 15;
  
  return( 0x01 << y );
}

void bufferFillCrosshairs()
{
  int x = map( analogRead( kPinKnob1 ), 0, 1023,  0, kDisplayWidth-1 );
  int y = yToBit( map( analogRead( kPinKnob2 ), 0, 1023,  0, 15 ));
  
  for( int i=0 ; i < kDisplayWidth ; i++ ) {
    displayBuffer[ i ] = y;
  }
  displayBuffer[ x ] = 0xffff;
}


static char cursorAnimations[3][8] = {
  { 1, 0, 0, 0,  0, 0, 0, 0 },
  { 0, 1, 1, 1,  1, 1, 1, 1 },
  { 0, 1, 0, 1,  0, 1, 0, 1 }
};

void bufferFillSketch()
{
  static int lastX = -1;
  static int lastY = -1;
  static unsigned char frame;
  static unsigned int lastVal = 0;
  
  int x = map( analogRead( kPinKnob1 ), 0, 1023,  0, kDisplayWidth-1 );
  int y = yToBit( map( analogRead( kPinKnob2 ), 0, 1023,  0, 15 ));
  int drawVal = 0;

  /* fix for first time through... */
  if( lastX < 0 ) {
    lastVal = displayBuffer[ x ];
  }

  /* if we moved, fill in the pixel appropriately */
  if( lastX != x  || lastY != y ) {
    if( lastX >= 0 ) { /* only if the value is valid */

      /* for-loop here so we don't miss pixels */
      for( int xx = min( lastX, x ) ; xx <= max( lastX, x ) ; xx++ )
      {
        switch( drawColor ) {
        case( kDrawColorErase ): displayBuffer[ xx ]   &= ~lastY;   break;
        case( kDrawColorDraw ):  displayBuffer[ xx ]   |=  lastY;   break;
        case( kDrawColorSkip ):  displayBuffer[ lastX ] =  lastVal; break;
        }
      }

      lastVal = displayBuffer[ x ]; /* store the last value */
    }
  }

  /* now flicker the current pixel as our cursor*/
  frame++;
  int ccc = 0;
  
  ccc = cursorAnimations[drawColor][ frame & 0x07 ];
  
  if( ccc ) {
    displayBuffer[ x ] |= y;
  } else {
    displayBuffer[ x ] &= ~y;
  }

  lastY = y;
  lastX = x;
}


/* ***********************************
 *  EEPROM Stuff
 */

void LoadBufferFromEEPROM()
{
  int addr = 0;
  unsigned char h,l;
  
  for( int i=0 ; i < kDisplayWidth ; i++ ) 
  {
    h = EEPROM.read( addr++ );
    l = EEPROM.read( addr++ );

    displayBuffer[ i ] = (h<<8) | l;
  }
}

void SaveBufferToEEPROM()
{
  int addr = 0;
  unsigned char h,l;
  
  for( int i=0 ; i < kDisplayWidth ; i++ ) 
  {
    h = (displayBuffer[ i ] >> 8) & 0x00ff;
    l = displayBuffer[ i ] & 0x00ff;
    EEPROM.update( addr++, h );
    EEPROM.update( addr++, l );

    displayBuffer[ i ] = (h<<8) | l;
  }
}

/* ***********************************
 *  Display Stuff
 */
 
//holds the display on for increments of 10ms
void bufferToDisplay(int holdTime) 
{
  unsigned long frameTimer = millis();
  while (millis() - frameTimer < holdTime) {
    for (int Y = 0; Y < 16; Y++) {
      PORTD = PORTD | B01100000;  //G2AU38, G2AU37 ENABLE HIGH
      for (int X = 0; X < kDisplayWidth; X++) {
        if (displayBuffer[X] >> Y & 1) PORTB = PORTB | B00000100;
        else PORTB = PORTB & B11111011;
        PORTB = PORTB | B00000010;  //pin 8 on CN0/CN1 B1 for 9
        PORTB = PORTB & B11111101;  //pin 8 on CN0/CN1 B1 for 9
      }
      PORTD = PORTD & B11100011;
      PORTD = PORTD | (Y & 7) << 2;  //set the data bits
      if (Y < 8) PORTD = PORTD & B11011111;  //G2AU38 ENABLE LOW
      else PORTD = PORTD & B10111111;  //G2AU37 ENABLE LOW
      delayMicroseconds(507);  //507 for 100Hz rate 920 for 60Hz
    }
  }
  PORTD = PORTD | B01100000;  //disables the display so last line isnt on
}

void displayClear()
{
  PORTD = B11100010;
  PORTB = B00000000;
  for (int X = 0; X < kDisplayWidth; X++) {
    PORTB = PORTB | B00000010;  //clock high
    PORTB = PORTB & B11111101;  //clock low
  }
}
