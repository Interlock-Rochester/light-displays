

void inputs_init()
{
  pinMode( kButtons, INPUT );
  pinMode( kKnobA, INPUT );
  pinMode( kKnobB, INPUT );
}

int inputs_GetButton()
{
  static int lastButton = 0;
  int thisButton = kButton_None;
  
  int v = analogRead( kButtons );
       if( v < 100 ) thisButton =  kButton_Down;
  else if( v < 200 ) thisButton =  kButton_Right;
  else if( v < 400 ) thisButton =  kButton_Left;

  if( (thisButton == lastButton) && 
      (thisButton != kButton_None) )
  {
    return( kButton_repeat );
  }
  lastButton = thisButton;
  return thisButton;
}


float inputs_GetA()
{
  return( (1024-analogRead( kKnobA ))/1024.0 );
}


float inputs_GetB()
{
  return( analogRead( kKnobB )/1024.0 );
}

