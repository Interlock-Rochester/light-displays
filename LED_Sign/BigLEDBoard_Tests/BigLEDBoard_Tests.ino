/* BigLEDBoard Tests
 * 
 * v1.01  2016-11-26 sdl
 *        three cursor/color modes (erase, draw, skip)
 *
 * v1.00  2016-11-25 sdl
 *        using display driver from Will Kommeritz
 * 
 */

#include "config.h"


#define kColsWide  (35)
extern int textSpacing;
char textBuffer[2][kColsWide];

void setup()
{
  /* start up serial */
  Serial.begin(115200);
  Serial.println( "Ready." );

  bigBoard_init();
  inputs_init();

  SelectFontID( kFont_Onyx );
  textBuffer[0][0] = '\0';
  Serial_AddString( "Hello,    \n    World!");
}

/* ***********************************
 *  user interface and main loop
 */

void loop()
{
  static unsigned char whichOne = 5;
  
  int button = inputs_GetButton();

  if( button == kButton_Down ) {
    whichOne++;
    if( whichOne > 6 ) { whichOne = 0; }
  }
  if( whichOne == 3 ) { whichOne = 4; };

  switch( whichOne ) {
    case( 0 ): return Serial_loop();
    case( 1 ): return Auto_loop();
    case( 2 ): return Fast_loop();
    //case( 3 ): return FastBlit_loop();
    case( 4 ): return Random_loop();
    case( 5 ): return Bitmap_loop();
    case( 6 ): return Controls_loop( button );
  }
}

#ifdef NEVER
const unsigned int frames[][85*2] PROGMEM = 
{
  // test bytes
  { 1,1,1,1,1,1,1,1,1,1,1,1,1 },
  { 2,2,2,2,2,2,2,2,2,2,2,2,2 },
  { 4,4,4,4,4,4,4,4,4,4,4,4,4 },
  { 8,8,8,8,8,8,8,8,8,8,8,8,8 },
  //{0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  //{9,8,7,6,5,4,3,2,1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
  {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},
  {0x5555, 0xAAAA,0x5555, 0xAAAA,0x5555, 0xAAAA,0x5555, 0xAAAA,0x5555, 0xAAAA, },
  {0x5555, 0x0000,0x5555, 0x0000,0x5555, 0x0000,0x5555, 0x0000,0x5555, 0x0000, },
  {0x1111,0x0000,0x0000,0x0000,0x1111,0x0000,0x0000,0x0000,0x1111,0x0000 },
  //{0xAA55,0x55AA,0xAA55,0x55AA,0xAA55,0x55AA,0xAA55,0x55AA,0xAA55,0x55AA}
};
#endif

void Bitmap_loop()
{
  unsigned long frame = (millis() >> 6) & 0x01;
  
  if( frame & 0x01 ) bufferRenderBitmap( gfx_interlock, 0, 0 );
    
  // For copying a full screenbuffer (for animations?) 
  //    bufferCopyProgmem( frames[frame] );
  bigBoard_poll();
  bigBoard_delay( 500 );
  bufferRollUp( );
  bigBoard_delay( 500 );
}

#ifdef NEVER
/*
unsigned int buf[4][kDisplayWidth];


void FastBlit_loop()
{
  int i;
  int counter = 10;
  static int initialized = 0;
  unsigned int *dbuf = bigBoard_getDisplayBuffer();


  if( !initialized ) {
    for( i=0; i<kDisplayWidth ; i++ ) {
        buf[0][i] = 0;
        buf[1][i] = 1 << i;
        buf[2][i] = 0x0001;
        buf[3][i] = 0xffff;
     }
     initialized = 1;
  }
     
  while( counter-- ) {
    for( i=0 ; i<4 ; i++ ) {
      memset( dbuf, &buf[i] , kDisplayWidth );
      bigBoard_poll();
    }
  }
}
*/
#endif

void Fast_loop()
{
  int counter = 10;
  static unsigned long l = 0l;
  while( counter-- ) {
    bufferFill( l++ );
    bigBoard_poll();
  }
}

void Serial_AddByte( char ch )
{
  static char lastNl = 0;
  int slen;

  if( ch == 0x07 ) {
      Font_toggle();
  }
    
  if( (ch == '\n' || ch == '\r') && !lastNl ) {
    lastNl = 1;
    strcpy( textBuffer[1], textBuffer[0] );
    textBuffer[0][0] = '\0';
    
  } else {
    lastNl = 0;
    if( ch >= 0x20 && ch <= 0x7e ) {
      if( strlen( textBuffer[0] ) < kColsWide-1 ) {
        slen = strlen( textBuffer[0] );
        textBuffer[0][slen] = ch;
        textBuffer[0][slen+1] = '\0';
      }
    }
  }
}

void Serial_AddString( char * text )
{
  while( *text ) {
    Serial_AddByte( *text );
    text++;
  }
}

void Serial_loop() 
{
  char ch;

  while( Serial.available() ) {
    ch = Serial.read();
    Serial_AddByte( ch );
  }
  
  bufferClear();
  
  bufferRenderTextCentered( textBuffer[1], 0 );
  bufferRenderTextCentered( textBuffer[0], 8 );

  bigBoard_poll();
}

void Controls_loop( int button )
{
  static unsigned char dir = 0;
  static unsigned long timeout = 1000;
  
  if( button == kButton_Right ) {
    Font_toggle();
    timeout = millis() + 1000;
  }

  /*
  if( button == kButton_Left ) textSpacing--;
  if( button == kButton_Right ) textSpacing++;
  */
  
  
  bufferFill( 0x0000 );
  int x = (int)(inputs_GetA() * (kDisplayWidth * 3)) - kDisplayWidth;
  int y = (int)(inputs_GetB() * (kDisplayHeight * 3)) - kDisplayHeight;
  
  x = bufferRenderText( "Hello,", x, y );
  bufferRenderText( "EVERYONE!", x+2, y+8 );

  if( millis() < timeout ) {
    bufferRenderText( "Font:", 0, 0 );
    bufferRenderTextProgmem( Font_name(), 2, 8 );
  }
      bigBoard_poll();

/*
  bigBoard_delay( 2000 );
 
  for( int i = 0; i < 32 ; i++ )
  {
    unsigned char d = random( 0, 2 );
    if( dir & 1 ) {
      bufferShiftUp();
    } else {
      bufferShiftDown();
    }
    if( d == 0 )  bufferShiftLeft( 0 );
    if( d == 1 )  bufferShiftRight( 0 );
    bigBoard_poll();
  }

  dir++;
  bigBoard_delay( 500 );
*/
}

void Random_loop()
{
  static long loopCount = 0;

  if( !loopCount++ ) {
    bufferFillRandom();
  } else {
    if( loopCount & 0x00080 )
    {
      bufferShiftLeft( loopCount ); //random( 0, 0xFFFF) );
      bufferShiftLeft( random( 0, 0xFFFF) );
    } else {
      bufferShiftRight( random( 0, 0xFFFF) );
      bufferShiftRight( loopCount ); //random( 0, 0xFFFF) );
    }
  }
  bigBoard_poll();
}

void Auto_loop()
{
  static long timeout = 0;
  static long mode = 0;

  if( millis() > timeout )
  {
    mode++; 
    timeout = millis()+500;
  }

  if( mode > 3 ) mode = 0;
  
  switch( mode ) {
    case( 0 ): bufferFillCounting(); break;
    case( 1 ): bufferFill( millis() +0000 ); break;
    case( 2 ): bufferFill( 0xF0F0 ); break;
    case( 3 ): bufferFill( 0x0F0F ); break;
  }
  bigBoard_poll();
}
