#include <Servo.h>

#define DIRECT_INPUT_FOLLOWS   0xFF

#define THROTTLE_PIN           7
#define THROTTLE_HOME          85    // In degrees
#define THROTTLE_MIN           60
#define THROTTLE_MAX           100
#define THROTTLE_STEP          2

#define STEERING_PIN           8
#define STEERING_HOME          90    // In degrees
#define STEERING_MIN           50
#define STEERING_MAX           155
#define STEERING_STEP          20

#define STATUS_MIN_DELAY       5000

#define KEYBOARD_REPEAT_DELAY  1000  // These values should match the repeat delay (ms) and repeat rate (chars/sec)
#define KEYBOARD_REPEAT_RATE   10    

Servo throttle, steering;

boolean display_enabled = true, return_steering = false, return_throttle = true;
int keyboard_repeat_delay = KEYBOARD_REPEAT_DELAY,
    keyboard_repeat_ms = 1000 / KEYBOARD_REPEAT_RATE;
int last_input = 0, consec_input = 0;
int last_throttle = 0, pos_throttle = THROTTLE_HOME;
int last_steering = 0, pos_steering = STEERING_HOME;
int last_status = 0;

void setup()
{
    Serial.begin( 9600 );

    throttle.attach( THROTTLE_PIN );
    steering.attach( STEERING_PIN );
}

void calibrate()
{
    int done, first_ms, second_ms, last_ms, n;
    
    steering.write( pos_steering = STEERING_HOME );
    throttle.write( pos_throttle = THROTTLE_HOME );
    
    Serial.print( "\r\nBeginning calibration.  Press and hold the 's' key\r\n" );
    Serial.flush();
    
    for ( done = 0; !done; )
    {
        switch ( Serial.read() )
        {
            case -1    : break;
            case 's'   : done = 1;                                     break;
            default    : Serial.print( "Calibration aborted\r\n" );    return;
        }
    }
    
    Serial.print( "." );
    first_ms     = millis();

    for ( done = 0; !done; )
    {
        switch ( Serial.read() )
        {
            case -1    : break;
            case 's'   : done = 1;                                     break;
            default    : Serial.print( "Calibration aborted\r\n" );    return;
        }
    }
    
    second_ms    = millis();
    for ( n = 0; n < 150; n++ )
    {
        if ( ( n % 10 ) == 0 )
        {
            Serial.print( "." );
        }
    
        for ( done = 0; !done; )
        {
            switch ( Serial.read() )
            {
                case -1    : break;
                case 's'   : done = 1;                                     break;
                default    : Serial.print( "Calibration aborted\r\n" );    return;
            }
        }
    }

    last_ms                  = millis();   
    keyboard_repeat_delay    = second_ms - first_ms;
    keyboard_repeat_ms       = ( last_ms - second_ms ) / n;
    
    Serial.print( " done.  Repeat Delay: " );
    Serial.print( keyboard_repeat_delay, DEC );
    Serial.print( "ms  Keys/sec: " );
    Serial.print( 1000 / keyboard_repeat_ms, DEC );
    Serial.print( "\r\n" );
}

void loop()
{
    int c, direct_throttle, direct_steering;
    int ms = millis();
    boolean update_display = false;
    
    if ( Serial.available() )
    {
        c = Serial.read();
        if ( c == DIRECT_INPUT_FOLLOWS )
        {
            display_enabled    = false;
            return_throttle    = false;
            return_steering    = false;

            while ( Serial.available() < 2 )
            {
                ;
            }
            
            direct_throttle    = Serial.read();
            direct_steering    = Serial.read();
                                
            if ( direct_throttle == 128 )      pos_throttle    = THROTTLE_HOME;
            else if ( direct_throttle < 128 )  pos_throttle    = THROTTLE_MIN + ( ( ( THROTTLE_HOME - THROTTLE_MIN ) * direct_throttle ) / 128 );
            else                               pos_throttle    = THROTTLE_HOME + ( ( ( THROTTLE_MAX - THROTTLE_HOME ) * ( direct_throttle - 128 ) ) / 128 );

            if ( direct_steering == 128 )      pos_steering    = STEERING_HOME;
            else if ( direct_steering  < 128 ) pos_steering    = STEERING_MIN + ( ( ( STEERING_HOME - STEERING_MIN ) * direct_steering ) / 128 );
            else                               pos_steering    = STEERING_HOME + ( ( ( STEERING_MAX - STEERING_HOME ) * ( direct_steering - 128 ) ) / 128 );
        }
        else
        {
            display_enabled    = true;
            
            switch ( c )
            {
                case ' ' : calibrate();                                                        break;
                case '<' : return_steering = !return_steering;    update_display = true;       break;
                case '>' : return_throttle = !return_throttle;    update_display = true;       break;
                case 'Q' :
                case 'q' :
                case '7' : pos_steering += STEERING_STEP;    pos_throttle += THROTTLE_STEP;    break;
                case 'W' :
                case 'w' :
                case '8' :                                   pos_throttle += THROTTLE_STEP;    break;
                case 'E' :
                case 'e' :
                case '9' : pos_steering -= STEERING_STEP;    pos_throttle += THROTTLE_STEP;    break;
                case 'A' :
                case 'a' :
                case '4' : pos_steering += STEERING_STEP;                                      break;            
                case 'S' :
                case 's' :
                case '5' : pos_steering = STEERING_HOME;     pos_throttle = THROTTLE_HOME;     break;
                case 'D' :
                case 'd' :
                case '6' : pos_steering -= STEERING_STEP;                                      break;
                case 'Z' :
                case 'z' :
                case '1' : pos_steering += STEERING_STEP;    pos_throttle -= THROTTLE_STEP;    break;
                case 'X' :
                case 'x' :
                case '2' :                                   pos_throttle -= THROTTLE_STEP;    break;
                case 'C' :
                case 'c' :
                case '3' : pos_steering -= STEERING_STEP;    pos_throttle -= THROTTLE_STEP;    break;
            }
        }
    
        if ( pos_throttle < THROTTLE_MIN )        pos_throttle = THROTTLE_MIN;
        else if ( pos_throttle > THROTTLE_MAX )   pos_throttle = THROTTLE_MAX;
        
        if ( pos_steering < STEERING_MIN )        pos_steering = STEERING_MIN;
        else if ( pos_steering > STEERING_MAX )   pos_steering = STEERING_MAX;

        last_input      = ms;
        consec_input++;
    }
    else if ( ms > ( last_input + ( keyboard_repeat_ms * 2 ) + ( consec_input == 1 ? ( keyboard_repeat_delay + keyboard_repeat_ms ) : 0 ) ) )
    {
        if ( return_throttle )    pos_throttle    = THROTTLE_HOME;
        if ( return_steering )    pos_steering    = STEERING_HOME;

        last_input      = ms;
        consec_input    = 0;
    }

    if ( ( ms - last_status ) > STATUS_MIN_DELAY )    update_display = true;
    else if ( pos_throttle != last_throttle )         update_display = true;
    else if ( pos_steering != last_steering )         update_display = true;

    if ( display_enabled && update_display )
    {
        Serial.print( "S: " );
        Serial.print( pos_steering, DEC );
        Serial.print( "deg  T: " );
        Serial.print( pos_throttle, DEC );
        Serial.print( "deg  (space) Calibrate  (<) SR: "  );
        Serial.print( return_steering ? "enabled" : "disabled" );
        Serial.print( " (>) TR: " );
        Serial.print( return_throttle ? "enabled" : "disabled" );
        Serial.print( "      \r" );
        
        last_throttle    = pos_throttle;
        last_steering    = pos_steering;
        last_status      = ms;
    }
    
    throttle.write( pos_throttle );
    steering.write( pos_steering );
}
