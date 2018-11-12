
/* BigLEDBoard
 * 
 * v2.00  2017-11-13 sdl
 *        fonts, etc presentation for makerFaire
 */

#include "config.h"
#include <TimerOne.h>

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

  //Timer1.initialize( 8000 );
  //Timer1.attachInterrupt( bufferToDisplayIRQ ); //bigBoard_interrupt_poll );
}

/* ***********************************
 *  user interface and main loop
 */
extern long ccc;
void loop()
{
  Presentation_Loop();
  return;
  
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


typedef struct SSlide {
  char * heading;
  unsigned long waitPer;
  char * items[10];
} SSLIDE;


void ScrollDisplayItems( struct SSlide * ssl )
{ 
  int x,y;
  char ** text = NULL;
  int centerX;
  int textWidth;

  if( !ssl ) return;

  bufferClear();
  SelectFontID( kFont_Onyx );
  textSpaceAddition = 5;
  
  for( int i=0 ; i < 16 ; i++ ) {
    bufferRenderTextCentered( ssl->heading, 0 );
    bufferClearMask( 0xffff>>i );
    bigBoard_poll();
  }
  bufferRenderTextCentered( ssl->heading, 0 );

  SelectFontID( kFont_ReadyP9 ); //kFont_CapsCaps );
  textSpaceAddition = 4;

  text = ssl->items;
  #define kScrollSpeed (4)
  while( *text ) {

    if( false ) {
      // scroll in
      textWidth = font_textWidth( *text );
      centerX = (kDisplayWidth - textWidth)/2;
      // scroll in
      for( x = kDisplayWidth ; x > centerX ; x-=kScrollSpeed ) {
        bufferAndMask( 0x00FF );
        bufferRenderText( *text, x, 8 );
        bigBoard_poll();
      }
      // wait
      bigBoard_delay( ssl->waitPer );
      // scroll out
      for( x = centerX ; x > (0-textWidth-10) ; x-=kScrollSpeed ) {
        bufferAndMask( 0x00FF );
        bufferRenderText( *text, x, 8 );
        bigBoard_poll();
      }
    }

    // pull up
    for( y = 16 ; y >= 8 ; y-- ) {
      bufferAndMask( 0x00FF );
      bufferRenderTextCentered( *text, y );
      bigBoard_poll();
    }
    // wait
    bigBoard_delay( ssl->waitPer );
    // push down
    for( y = 8 ; y <16 ; y++ ) {
      bufferAndMask( 0x00FF );
      bufferRenderTextCentered( *text, y );
      bigBoard_poll();
    }
    
    text++;
  }
  
  //bufferWipeUp();
  
  bufferRollUp( );
}

SSLIDE sl_who = {
  "Who We Are...",
  4000,
  { "We're a 501(c)(3) non-profit",
    "We hack and repurpose stuff",
    "(like this LED sign)",
    "Share interests in science,",
    "technology, art and culture",
    NULL
  }
};

SSLIDE sl_interests = {
  "Our Interests",
  2000,
  { "Computer Security, SDR",
    "Ham Radio, 3D Printing",
    "Software, Lockpicking",
    NULL
  }
};

SSLIDE sl_facilities = {
  "Facilities",
  4000,
  { "Lounge, Board Room, Offices",
    "Electronics Bench, Dev Kits ",
    "3D Printers, HAM Shack",
    "Wood Shop, Laser Cutter",
    NULL }
};

SSLIDE sl_associated = {
  "Associated Groups",
  1000,
  { "AdaSpace",
    "BSidesRoc",
    "2600 Rochester",
    "Toool",
    NULL }
};

SSLIDE sl_meetups = {
  "Meetups ",
  3000,
  { "Every Tuesday Evening",
    "1st Tue - Monthly Meeting",
    "1st Fri - 2600",
    "2nd Tue - How Do I?",
    "3rd Sat - Linux",
    NULL }
};

SSLIDE sl_where = {
  "Where are we?",
  3000,
  { "Hungerford at Main & Goodman",
    "Hungerford - Door 7",
    "http://interlockroc.org",
    NULL }
};


#define kGhostSpeed   (3)

void GhostWipe()
{
  int frame = 0;
  
  unsigned int *vbuff = bigBoard_getDisplayBuffer();

  // ghost wipe
  for( int x=-16 ; x<kDisplayWidth ; x+=kGhostSpeed )
  {
    // clear a path...
    for( int xc = x-kGhostSpeed ; xc < x+14 ; xc++ ) {
      if( xc >=0 && xc < kDisplayWidth ) {
        vbuff[xc] = 0x0000;
      }
    }
  
    if( frame & 8 ) bufferRenderTallBitmap( x, 1, gfx_ghost_A );
    else bufferRenderTallBitmap( x, 1, gfx_ghost_B );

    if( frame & 1 ) {  bufferRenderTallBitmapOverlay( x, 1, gfx_ghost_I ); }
    
    bigBoard_poll();
    frame++;
  }
}


void InterlockBigText()
{
  unsigned int mask;
  bufferClear();
  bigBoard_poll();
  //delay( 250 );

  // roll in of live text
  mask = 1;
  for( int i = -16 ; i<=0 ; i++ ) {
    bufferClear();
    bufferCopyProgmemBitmap( gfx_interlock, (kDisplayWidth-99)/2 ); // faster?
//    bufferRenderTallBitmap( (kDisplayWidth-99)/2, 0, gfx_interlock );
    mask |= mask << 1;
    bufferAndMask( mask );
    bigBoard_poll();
  }

  
  bufferCopyProgmemBitmap( gfx_interlock, (kDisplayWidth-99)/2 ); // faster?
//    bufferRenderTallBitmap( (kDisplayWidth-99)/2, 0, gfx_interlock );
  bigBoard_delay(1000);
  bufferWipeUp( );

  // roll up wipe of live text
  mask = 1;
  for( int i = -16 ; i<=0 ; i++ ) {
    bufferClear();
    bufferCopyProgmemBitmap( gfx_rochester, (kDisplayWidth-105)/2 ); // faster?
    //bufferRenderTallBitmap( (kDisplayWidth-105)/2, 0, gfx_rochester );
    mask |= mask << 1;
    bufferAndMask( mask );
    bigBoard_poll();
  }
  
  bufferCopyProgmemBitmap( gfx_rochester, (kDisplayWidth-105)/2 ); // faster?
  //bufferRenderTallBitmap( (kDisplayWidth-105)/2, 0, gfx_rochester );
  bigBoard_delay(1000);
  
  GhostWipe();
}

void Presentation_Loop( )
{
  InterlockBigText();
  ScrollDisplayItems( &sl_who );
  ScrollDisplayItems( &sl_interests );
  
  InterlockBigText();
  ScrollDisplayItems( &sl_facilities );
  ScrollDisplayItems( &sl_associated );
  
  InterlockBigText();
  ScrollDisplayItems( &sl_meetups );
  ScrollDisplayItems( &sl_where );
}


void Bitmap_loop()
{
  static unsigned char frame = 0;
  bufferClear();

/*
  if( frame & 0x01 ) bufferRenderTallBitmap( frame, 1, gfx_interlock );
  else bufferRenderTallBitmap( frame, 1, gfx_rochester );
    
  // For copying a full screenbuffer (for animations?) 
  
  bigBoard_poll();
  bigBoard_delay( 500 );
  bufferRollUp( );
  //bigBoard_delay( 500 );

  frame++;
*/
  frame = 0;
  for( int x=-16 ; x<kDisplayWidth ; x+=1 )
  {
      bufferClear();
      if( frame & 8 ) bufferRenderTallBitmap( x, 1, gfx_ghost_A );
      else bufferRenderTallBitmap( x, 1, gfx_ghost_B );

      if( frame & 1 ) {  bufferRenderTallBitmapOverlay( x, 1, gfx_ghost_I ); }
      
      bigBoard_poll();
      frame++;
  }
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
