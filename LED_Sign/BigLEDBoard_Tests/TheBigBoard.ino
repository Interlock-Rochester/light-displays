
#include "config.h"


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

// internal framebuffer
unsigned int displayBuffer[kDisplayWidth] = {0}; /* buffer to store the active frame */


void bigBoard_init( void )
{
  /* initialize board IO */
  // DDRB and DDRD 1 is output 0 is input
  DDRD = B11111110; PORTD = B11100010;
  DDRB = B00000111; PORTB = B00000000;
  displayClear();  //reset the display
  bufferClear();
}

void bigBoard_poll( void )
{
  bufferToDisplay( kHoldTime );
}

void bigBoard_delay( unsigned long ms )
{
  ms += millis();

  while( millis() < ms ) {
    bigBoard_poll();
  }
}

unsigned int * bigBoard_getDisplayBuffer( void )
{
  return &displayBuffer[0];
}


/* ***********************************
 *  Buffer fillers
 */

void bufferFill( int val ) 
{
  for (int i = 0; i < kDisplayWidth; i++) {
    displayBuffer[i] = val;
  }
}

void bufferFillRandom() 
{
  for (int i = 0; i < kDisplayWidth; i++) {
    displayBuffer[i] = random( 0, 0xFFFF );
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

void bufferShiftLeft( unsigned int inval )
{
  for (int i = 0 ; i < kDisplayWidth-1 ; i++) {
    displayBuffer[i] = displayBuffer[i+1];
  }
  displayBuffer[kDisplayWidth-1] = inval;
}

void bufferShiftRight( unsigned int inval )
{
  for (int i = kDisplayWidth-1 ; i > 0 ; i--) {
    displayBuffer[i] = displayBuffer[i-1];
  }
  displayBuffer[0] = inval;
}

void bufferShiftUp( void )
{
  for( int i= 0 ; i < kDisplayWidth ; i++ ) {
    displayBuffer[i] = displayBuffer[i] >> 1;
  }
}


void bufferShiftDown( void )
{
  for( int i= 0 ; i < kDisplayWidth ; i++ ) {
    displayBuffer[i] = displayBuffer[i] << 1;
  }
}


void bufferRollUp( void )
{
  for( int i=0 ; i < 16 ; i++ ) {
    bufferShiftUp();
    bigBoard_poll();
  }
}


void bufferRollDown( void )
{
  for( int i=0 ; i < 16 ; i++ ) {
    bufferShiftDown();
    bigBoard_poll();
  }
}


/////////////////////////////////////////////////////////


void bufferSetPixel( unsigned char pixel, int xpos, int ypos )
{
  if( xpos >= 0  && xpos < kDisplayWidth ) {
    if( ypos >= 0 && ypos < 16 ) {
      if( pixel ) {
        displayBuffer[xpos] |= 1<<ypos;
      } else {
        displayBuffer[xpos] &= ~(1<<ypos);
      }
    }
  }
}

int bufferRenderBitmap( int xpos, int ypos, const unsigned char * pattern, int nBytes )
{
  unsigned char data;
  int yy;
  
  // quick and dirty for now
  for( int bno = 0 ; bno < nBytes ; bno++ )
  {
    if( xpos >= 0  && xpos < kDisplayWidth ) {
      data = pgm_read_byte_near( pattern + bno );
      for( yy=0 ; yy<8 ; yy++ )
      {
        bufferSetPixel( data & (1<<yy), xpos, ypos+yy );
      }
    }
    xpos++;
  }
  return( xpos );
}


extern const unsigned char * fontdata;
extern const int * fontglyphs;

#define kMinGlyph  (0x20)
#define kMaxGlyph  (200)


int textSpacing = 1;

int bufferRenderGlyph( char ch, int x, int y )
{
  if( (ch >= kMinGlyph) && (ch < kMaxGlyph) ) { // only printables
    int gpos = pgm_read_word_near( fontglyphs + ch-kMinGlyph );
    int len = pgm_read_word_near( fontglyphs + ch-kMinGlyph +1 ) - gpos;
    
    bufferRenderBitmap( x, y, fontdata + gpos, len );
    return len + textSpacing;
  }
  return 0;
}

int bufferRenderText( const char * text, int x, int y )
{ 
  int tp = 0;
  do{
    unsigned char ch = text[tp];
    if( ch == '\0' ) return x;
    
    x += bufferRenderGlyph( ch, x, y );
    tp++;
  } while (1);
  return 0;
}

int bufferRenderTextProgmem( const char * text, int x, int y )
{ 
  int tp = 0;
  do{
    unsigned char ch = pgm_read_byte_near( text+tp );
    if( ch == '\0' ) return x;
    
    x += bufferRenderGlyph( ch, x, y );
    tp++;
  } while (1);
  return 0;
}

int font_textWidth( char * text )
{
  //return( (strlen( text ) * 5) -1 );
  int w = 0;
  int spacing = 1;
  
  for( int tp = 0 ; text[tp] != '\0' ; tp++ )
  {
    unsigned char ch = text[tp];
    if( (ch >= kMinGlyph) && (ch < kMaxGlyph) ) { // only printables
      int len = pgm_read_word_near( fontglyphs + ch - kMinGlyph + 1) - 
                pgm_read_word_near( fontglyphs + ch - kMinGlyph );
      w += (len+spacing);
    }
  }
  return w-1; // -1 because we remove the end space
}

int bufferRenderTextCentered( const char * text, int y )
{
  int x = (kDisplayWidth - font_textWidth( text ))/2;
  bufferRenderText( text, x, y );
}


/////////////////////////////////////////////////////////

void bufferCopyProgmem( const unsigned int * data )
{
  unsigned int coldata;
  
  for( int i=0 ; i <kDisplayWidth ; i++ )
  {
    coldata = pgm_read_word_near( i + data );
    displayBuffer[i] = coldata;
  }
}

void bufferRenderBitmap( const unsigned int * graphicData, int x, int y )
{
  unsigned int width = GFX_GetWidth( *graphicData );
  unsigned int offset = GFX_GetOffset( *graphicData );
  graphicData++;

  // TODO: This.
}

/////////////////////////////////////////////////////////

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
      delayMicroseconds( kDelayPerLine ); //507 for 100Hz rate 920 for 60Hz
    }
  }
  PORTD = PORTD | B01100000;  //disables the display so last line isnt left on
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
