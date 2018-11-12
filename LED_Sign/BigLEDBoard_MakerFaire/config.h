
#ifndef __CONFIG_H__
#define __CONFIG_H__

/* -- Big Board Config -- */

/* frame buffer */
#define kDisplayNPanels (2)
#define kPixelsPerPanel (85)

#define kDisplayWidth  (kPixelsPerPanel * kDisplayNPanels) /* width of the display in columns of 16 bit words */
#define kDisplayHeight (16)

/* refresh info */
// 10/920 is great for super fast updates
#define kHoldTime (10)
#define kDelayPerLine (1000) //507 for 100Hz rate 920 for 60Hz

/* -- Graphics -- */
extern const PROGMEM unsigned int gfx_interlock[];
extern const PROGMEM unsigned int gfx_rochester[];
extern const PROGMEM unsigned int gfx_interlock_bold[];
extern const PROGMEM unsigned int gfx_rochester_bold[];

extern const PROGMEM unsigned int gfx_ghost_A[];
extern const PROGMEM unsigned int gfx_ghost_B[];
extern const PROGMEM unsigned int gfx_ghost_I[];

/* -- Font stuff -- */

/* Font selection */
#define kFont_CapsCaps    'C'
#define kFont_ReadyP9     'R'
#define kFont_NarrowCaps  'N'
#define kFont_Onyx        'O'
#define kFont_Images      'I'

/* font spacing */
extern int textSpacing;
extern int textSpaceAddition;

/* -- Button/Inputs -- */


#define kButtons  A0
#define kKnobA    A1
#define kKnobB    A2

#define kButton_None  ' '
#define kButton_Down  'd'
#define kButton_Right 'r'
#define kButton_Left  'l'

#define kButton_repeat  '~'

#endif
