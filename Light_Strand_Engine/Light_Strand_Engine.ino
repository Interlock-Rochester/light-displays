// D15 strand
//
//	use the WS2801 strand with an atmega 168p
//
//    2016-11-17  -  updated for 92(!!!) LEDs for RMMF Saturday.
//
// v6
//      fadey traditional pattern
//
// v5
//      traditional colors animation
//      arctic twilight animation
//      xmas colors animation
//      fadey happy fun animation
//
// v4 (Green White Green)
//	autorun
//	basic animations
//	refresh
//
// v3 (red green blue)
//	incorrect autorun (didn't kick it off, but used EEPROM)
//



#include <EEPROM.h>

#include "SPI.h"
#include "WS2801.h"

/*****************************************************************************
Example sketch for driving WS2801 pixels
*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
// The colors of the wires may be totally different so
// BE SURE TO CHECK YOUR PIXELS TO SEE WHICH WIRES TO USE!

int dataPin = 12;
int clockPin = 11;
int indicatorPin = 8;

int buttonA = 13;
int buttonB = 10;

// Don't forget to connect the ground wire to Arduino ground,
// and the +5V wire to a +5V supply

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
#define kNLights 92
WS2801 strip = WS2801(kNLights, dataPin, clockPin);

// Optional: leave off pin numbers to use hardware SPI
// (pinout is then specific to each board and can't be changed)
//WS2801 strip = WS2801(25);

int cx = 0; // pattern selected
int ph = 0; // phase in the pattern
unsigned long patternTimeout = 0; // when the next step should occur


void setup() 
{
  // random setup
  randomSeed( analogRead( 0 ) + analogRead( 1 ) );
  
  // LED setup
  pinMode( indicatorPin, OUTPUT );
  digitalWrite( indicatorPin, LOW );

  // button setup
  pinMode( buttonA, INPUT );
  pinMode( buttonB, INPUT );
  
  // eeprom/settings setup
  eeprom_init();
  
  // strip setup
  strip.begin();

  // Update LED contents, to start they are all 'off'
  allColored( Color( 0, 0, 0 ));
  strip.show();

  // serial console setup
  Serial.begin( 9600 );
  
  patternTimeout = 0;
}

// eeprom usage
// 0..4 - "Scott" - for validation
#define kPattern	(5)	// storage of the current pattern

void eeprom_init()
{
  boolean valid = true;

  // validate the eeprom has been initialized
  if( EEPROM.read( 0 ) != 'S' ) valid = false;
  if( EEPROM.read( 1 ) != 'c' ) valid = false;
  if( EEPROM.read( 2 ) != 'o' ) valid = false;
  if( EEPROM.read( 3 ) != 't' ) valid = false;
  if( EEPROM.read( 4 ) != '2' ) valid = false;
  
  if( valid == true ) {
    // yup.  let's read our settings
    eeprom_read();
    return;
  }
  
  // never used before... let's initialize.
  // sentinel
  EEPROM.write( 0, 'S' );
  EEPROM.write( 1, 'c' );
  EEPROM.write( 2, 'o' );
  EEPROM.write( 3, 't' );
  EEPROM.write( 4, '2' );

  // init the variables
  eeprom_write();
}


void eeprom_read()
{
  cx = EEPROM.read( kPattern );
  patternTimeout = 0;
}

void eeprom_write()
{
  EEPROM.write( kPattern, (cx & 0x0ff) );
}

#ifdef NEVER
unsigned char colorlut[256] = {
0,
/* 1 */
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 1..16 */
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 17..32 */
3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, /* 33..64 */
5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, /* 65..96 */
 9,  9, 10, 10, 11, 11, 12, 12,  13, 13, 14, 14, 15, 15, 16, 16, /* 97..128 */
17, 18, 19, 20, 21, 22, 23, 24,  25, 26, 27, 28, 29, 30, 31, 31, /* 129..160 */
};
#endif

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  // adjust the values to be log based for our strand.
  /*
  float lf = (log( 255 ) - 1.0);

  float rf = ( log( r ) - 1.0 ) * 255.0 / lf;
  float gf = ( log( g ) - 1.0 ) * 255.0 / lf;
  float bf = ( log( b ) - 1.0 ) * 255.0 / lf;
  
  r = int( rf );
  g = int( gf );
  b = int( bf );
  */
  
  // and set the color
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

void setLED( int idx, uint32_t c )
{
  strip.setPixelColor( idx, c );  
  strip.show();
}

void updatePattern( int p )
{
  // if it was the same, just return
  if( p == cx ) return;
  
  // otherwise, set it, save it, do it.
  cx = p;
  //eeprom_write();
  patternTimeout = 0;
}

void internalPattern( int x )
{
  int light;
  int v;
  
  patternTimeout = millis() + 5000;
  switch( x & 0x07 ) {
    case( 0 ):
      // all off
      allColored( Color( 0, 0, 0 ));
      break;
      
    case( 1 ):
      // Christmas -- red/green
      for( light=0 ; light < kNLights ; light += 2 )
      {
        strip.setPixelColor(light, Color( 255, 0, 0 ));
        strip.setPixelColor(light+1, Color( 0, 255, 0 ));
      }
      strip.show();
      break;
      
   case( 2 ):
      // red/green marquee
      ph++;
      for( light=0 ; light < kNLights ; light += 5 )
      {
        strip.setPixelColor( (light+0+ph)%kNLights, Color( 255, 0, 0 ));
        strip.setPixelColor( (light+1+ph)%kNLights, Color( 0, 255, 0 ));
        strip.setPixelColor( (light+2+ph)%kNLights, Color( 0, 0, 0 ));
        strip.setPixelColor( (light+3+ph)%kNLights, Color( 0, 0, 0 ));
        strip.setPixelColor( (light+4+ph)%kNLights, Color( 0, 0, 0 ));
      }
      strip.show();
      patternTimeout = millis() + 100;

      break;
      
    case( 3 ):
      // traditional color strand
      //  red green blue orange purple
      for( int light=0 ; light < kNLights ; light += 5 )
      {
        strip.setPixelColor( light+0, 255, 0, 0 ); // r
        strip.setPixelColor( light+1, 0, 255, 0 ); // g
        strip.setPixelColor( light+2, 0, 0, 255 ); // b
        strip.setPixelColor( light+3, 255, 50, 0 ); // o
        strip.setPixelColor( light+4, 255, 0, 75 ); // p
      }
      strip.show();
      break;

    case( 4 ):
      // red-
      ph++;
      {
        int r,g,b;
        switch( ph % 6 )
        {
          case( 0 ):  r=255; g=0; b=0; break;
          case( 1 ):  r=255; g=255; b=0; break;
          case( 2 ):  r=0; g=255; b=0; break;
          case( 3 ):  r=0; g=255; b=255; break;
          case( 4 ):  r=0; g=0; b=255; break;
          case( 5 ):  r=255; g=0; b=255; break;
        }
        
        for( int x=0 ; x<128 ; x+=4 )
        {
              float percent = (float) x / 128.0;
              allColored( Color( (int)(r * percent), (int)(g * percent), (int)(b *percent) ));
              delayMicroseconds( 100 );
        }
        for( int x=128 ; x>=0 ; x -=4 )
        {
              float percent = (float) x / 128.0;
              allColored( Color( (int)(r * percent), (int)(g * percent), (int)(b *percent) ));
              delayMicroseconds( 100 );
        }
        patternTimeout = millis() + 500;
      }

      break;
      
    case( 5 ):
      // cycling traitional
      ph++;
      
      for( int light=0 ; light < kNLights ; light += 5 )
      {
          strip.setPixelColor( (light+0+ph)%kNLights, 255, 0, 0 ); // r
          strip.setPixelColor( (light+1+ph)%kNLights, 0, 255, 0 ); // g
          strip.setPixelColor( (light+2+ph)%kNLights, 0, 0, 255 ); // b
          strip.setPixelColor( (light+3+ph)%kNLights, 255, 50, 0 ); // o
          strip.setPixelColor( (light+4+ph)%kNLights, 255, 0, 75 ); // p
      }
      strip.show();
      patternTimeout = millis() + 1000;
      break;
      
    case( 6 ):
      // cycling traitional with fade effect
      // 1. Fade out the old

      for( v = 192; v>=0 ; v-=2 ) {      
        for( int light=0 ; light < kNLights ; light += 5 )
        {
            strip.setPixelColor( (light+0+ph)%kNLights, v, 0, 0 ); // r
            strip.setPixelColor( (light+1+ph)%kNLights, 0, v, 0 ); // g
            strip.setPixelColor( (light+2+ph)%kNLights, 0, 0, v ); // b
            strip.setPixelColor( (light+3+ph)%kNLights, v, (int)( (float)v*0.2), 0 ); // o
            strip.setPixelColor( (light+4+ph)%kNLights, v, 0, (int)( (float)v*0.3) ); // p
        }
        strip.show();
        //delayMicroseconds( 20 );
        //delay(1);
      }

      
      // 2. Fade in the new
      ph++;
      

      for( v = 0; v<192 ; v+=2 ) {      
        for( int light=0 ; light < kNLights ; light += 5 )
        {
            strip.setPixelColor( (light+0+ph)%kNLights, v, 0, 0 ); // r
            strip.setPixelColor( (light+1+ph)%kNLights, 0, v, 0 ); // g
            strip.setPixelColor( (light+2+ph)%kNLights, 0, 0, v ); // b
            strip.setPixelColor( (light+3+ph)%kNLights, v, (int)( (float)v*0.2), 0 ); // o
            strip.setPixelColor( (light+4+ph)%kNLights, v, 0, (int)( (float)v*0.3) ); // p
        }
        strip.show();
        //delayMicroseconds( 20 );
      }
      patternTimeout = millis() + 200;
      break;
      
    case( 7 ):
    default:
      // arctic twilight
#define kReallyDim   8
      allColored( Color( 0, 0, kReallyDim ));
      
      light = random( 0, kNLights );
      
      // bright to dim
      for( int i=255; i >=0 ; i -= 2 )
      {
        setLED( light, Color( i, i, max( kReallyDim, i ) ));
        //delayMicroseconds( 10 ); // slowish fade
      }

      allColored( Color( 0, 0, kReallyDim )); // make sure
      patternTimeout = millis() + random( 1000, 3000 ); // do it from NOW.
      break;
  }
}


void pollButtons()
{
  int buttonHandled = 0;
  int p = cx;
  
  if( digitalRead( buttonA ) == HIGH ){
    delay( 20 ); // debounce
    while( digitalRead( buttonA ) == HIGH) digitalWrite( indicatorPin, LOW );
    delay( 20 );
    p--;
    buttonHandled = 1;
  }
  
  if( digitalRead( buttonB ) == HIGH ){
    delay( 20 ); // debounce
    while( digitalRead( buttonB ) == HIGH ) digitalWrite( indicatorPin, LOW );
    delay( 20 );
    p++;
    buttonHandled = 1;
  }
  
  if( buttonHandled ) {
    updatePattern( p );  // update it
  }
}



int getSerial() 
{
  while( !Serial.available() ) { delay( 1 ); }
  return Serial.read();
}


void slowFoo( uint32_t c ) 
{
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
  }
}

void allColored( uint32_t c ) 
{
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}


void flash()
{
  int i;
  for (i=0; i <  20 ; i++) {
    allColored( Color( 255, 255, 255 ));
    allColored( Color( 0, 0, 0 ));
  }
}


// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

int getSerialWait()
{
  while ( !Serial.available() );
  return getSerial();
}

int useInternal = 1;


void loop() {
  int cmd;
  int ii, rr,gg,bb;
  /*
  colorWipe(Color(255, 0, 0), 0);
  colorWipe(Color(0, 0, 0), 0);
  */
  /*
  colorWipe(Color(255, 0, 0), 0);
  colorWipe(Color(255, 255, 0), 3);
  colorWipe(Color(0, 255, 0), 3);
  colorWipe(Color(0, 255, 255), 3);
  colorWipe(Color(0, 0, 255), 3);
  colorWipe(Color(255, 0, 255), 3);
  colorWipe(Color(0, 0, 0), 3);
  */
//  colorWipe(Color(0, 0, 255), 5);
//  slowFoo( Color(0, 255, 0) );
//  fastFoo( Color(0, 0, 255) );
//  flash();

  digitalWrite( indicatorPin, millis() & 0x80 );
  
  pollButtons();

  if( useInternal == 1 ) {
    // this will enable animation, as well as allow for
    // refresh to prevent "blue creep"
    if( millis() > patternTimeout ) {
      internalPattern( cx );
    }
  }

  if( Serial.available() ) {
    
    cmd = Serial.read();
    
    // current commands:
    // p<index><red><green><blue>      // set a single pixel a color
    // f<red><green><blue>             // flood fill with this color
    
    if( cmd == 'p' ) {
      ii = getSerialWait();
      rr = getSerialWait();
      gg = getSerialWait();
      bb = getSerialWait();
  
      setLED( ii, Color(rr, gg, bb) );
      useInternal = 0;
    }
    
    if( cmd == 'P' ) {
      ii = getSerialWait();
      rr = getSerialWait();
      gg = getSerialWait();
      bb = getSerialWait();
  
      // set pixel without 
      strip.setPixelColor( ii, Color(rr, gg, bb) );
      useInternal = 0;
    }
    
    if( cmd == 'F' ) {
      // flush
      strip.show();
      useInternal = 0;
    }
    
    if( cmd == 'f' ) {
      rr = getSerialWait();
      gg = getSerialWait();
      bb = getSerialWait();
      allColored( Color(rr, gg, bb) );
      useInternal = 0;
    }
    
    if( cmd == 'g' ) {
      // GO
      while( !Serial.available() ) delay( 1 );
      cmd = Serial.read();
      updatePattern( cmd - '0' );
      useInternal = 1; // force internal
    }

    if( cmd == 'i' ) {
      useInternal = 1;
    }
  }
}

// possible protocol:
//
//(start character)(command)(params..)(stop character)
//Z(command)(params..)z
