/*
 * traffic_light.ino
 * Last Revision: 4/5/2017
 * Justin Schwausch
 * ECE 1305
 * Section 002
 * Worked With: No One
 * 
 * This program simulates a traffic light at an intersection using leds and pushbuttons and a photo-resistor.
 * The program has options for turning cars, crosswalks, and emergency vehicles.
 * The program also switches into a secondary light cycle during certain hours of the day.
 * This program is meant to be run on an Arduino Mega with: 
 * leds on digital pins 22-41, 46
 * buttons/momentary switches on digital pins 42-45
 * and a photo-resistor on analog pin A15
 * 
 * The street layout imitated by the lights is as follows:
 * 
 *The left bank of lights represents a large street heading east -> with the option to turn left (north) ^
 *The left crosswalk is for pedestrians planning to cross that road.
 *
 *The right bank of lights represent a street heading north ^ with the option to turn left (west) <-
 *The right crosswalk is for pedestrians planning to cross that road.
 *
 */
const int l_s_red = 22, l_s_yellow = 23, l_s_green = 24;
const int l_t_red = 25, l_t_yellow = 26, l_t_green = 27;
const int l_w_red = 28, l_w_white = 29;
const int l_t_blue = 30, l_w_green = 31;

const int r_s_red = 32, r_s_yellow = 33, r_s_green = 34;
const int r_t_red = 35, r_t_yellow = 36, r_t_green = 37;
const int r_w_red = 38, r_w_white = 39;
const int r_t_blue = 40, r_w_green = 41;

const int l_w_switch = 42, l_t_switch = 43;
const int r_w_switch = 44, r_t_switch = 45;

const int l_ind = 46, r_photo = A15;
/* 
 *  setup the names and pin-outs of every light, switch, and photo-resistor
 *  the naming convention is simple:
 *  a_b_c
 *  
 *  a -> l or r, the breadboard the component is on
 *  b -> s, t, or w, the purpose of the component, straight, turn, or walk
 *  c -> red, yellow, green, blue, or white, the color
 *  
 *  the only exceptions are the indicator led and the photo-resistor
*/

bool l_t_flag = 0, l_w_flag = 0;
bool r_t_flag = 0, r_w_flag = 0;
//setup button flags

bool priority = 0;
//priority for right turn and left walk

int photo_flag = 0;
//setup photo-resistor flag

int curr_time = 0;
//setup time for

int l_green_time = 10000;
int l_yellow_time = 2000;
int r_green_time = 3000;
int r_yellow_time = 1000;
//setup starting green and yellow light times

bool state = 0;
//setup state

unsigned int norm_light = 0;
//setup normal light level

unsigned long now = 0;
//ms value for current time

unsigned long next_time = 0;
//ms value for next transition

unsigned long done_time = 0;
//ms value for time done

bool emer_flag = 0;
//flag to skip remainder of cycle due to an emergency

void controller(bool &state, bool priority, bool &l_t_flag, bool &l_w_flag, bool &r_t_flag, bool &r_w_flag, int l_green_time, int l_yellow_time, int r_green_time, int r_yellow_time, bool &emer_flag);
//the controller is the main function it calls almost all of the other functions

void button_check(bool &l_t_flag, bool &l_w_flag, bool &r_t_flag, bool &r_w_flag, bool &emer_flag, bool state);
//this function checks the status of the buttons and the photo-resistor and potentially calls the emergency function

//---Light Sequence Functions---

void left_straight(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag);
//used only if the left set of lights needs to go straight

void left_turn(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag);
//used only when the left turn switch is set

void right_walk_turn(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag);
//used to let the left side go straight and turn while letting the right side walk, only used if those flags are set

void left_walk(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag);
//used only to let the left side walk and right side go straight

void right_straight(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag);
//used only to let the right side go straight

void right_turn(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag);
//used for the right side to go straight and turn

//------------------------------

void emergency(unsigned long &next_time, unsigned long now, unsigned int norm_light, bool &emer_flag);
//the emergency function, triggered by the light level inside the button check function
//turns all lights red and waits until two seconds after the light level has normalized

void night(unsigned long &next_time);
//used to enable a secondary light sequence during late night
//has the left side yellow lights, right side red lights, and all red walk lights flash
//(also calls button check function to check for emergency vehicles

//setup function prototypes

void setup() {

  Serial.begin(9600);
  Serial.println("<startup>");
  Serial.println("Serial Open");
  //open serial communications and show startup is running
  
  pinMode(l_s_red, OUTPUT);
  pinMode(l_s_yellow, OUTPUT);
  pinMode(l_s_green, OUTPUT);
  pinMode(l_t_red, OUTPUT);
  pinMode(l_t_yellow, OUTPUT);
  pinMode(l_t_green, OUTPUT);
  pinMode(l_w_red, OUTPUT);
  pinMode(l_w_white, OUTPUT);
  pinMode(l_t_blue, OUTPUT);
  pinMode(l_w_green, OUTPUT);

  pinMode(r_s_red, OUTPUT);
  pinMode(r_s_yellow, OUTPUT);
  pinMode(r_s_green, OUTPUT);
  pinMode(r_t_red, OUTPUT);
  pinMode(r_t_yellow, OUTPUT);
  pinMode(r_t_green, OUTPUT);
  pinMode(r_w_red, OUTPUT);
  pinMode(r_w_white, OUTPUT);
  pinMode(r_t_blue, OUTPUT);
  pinMode(r_w_green, OUTPUT);

  pinMode(l_w_switch, INPUT_PULLUP);
  pinMode(l_t_switch, INPUT_PULLUP);
  pinMode(r_w_switch, INPUT_PULLUP);
  pinMode(r_t_switch, INPUT_PULLUP);

  pinMode(l_ind, OUTPUT);
  //initialize all of the input and output pins
  
  Serial.println("Pins Defined");
  //indicate progress

  for (int n = 22; n <= 41; n++)
  {
   digitalWrite(n, LOW);
  }

  digitalWrite(l_ind, LOW);
  //set all digital output pins as off

  Serial.println("Pins Off");
  //indicate progress
  
  norm_light = analogRead(r_photo);
  Serial.print("Normal light level is: ");
  Serial.println(norm_light);
  //get the normal light level to compare to later

  now = millis( );
  Serial.print("Current time is: ");
  Serial.println(now);
  //get the current time

  digitalWrite(l_s_red, HIGH);
  digitalWrite(l_t_red, HIGH);
  digitalWrite(l_w_red, HIGH);
  digitalWrite(r_s_red, HIGH);
  digitalWrite(r_t_red, HIGH);
  digitalWrite(r_w_red, HIGH);
  Serial.println("Red Lights ON");
  //turn all red lights on

  Serial.println("</startup>");
  //print that startup is done, moving into main routine
}

void loop() {
  Serial.println("----------------------");
  Serial.println("In main loop!");
  Serial.println("----------------------");
  
  controller(state, priority, l_t_flag, l_w_flag, r_t_flag, r_w_flag, l_green_time, l_yellow_time, r_green_time, r_yellow_time, emer_flag);
  //run the controller

  curr_time ++;
  //count up the time
  
  Serial.print("Current time is: ");
  Serial.println(curr_time);
  //indicate the current time between cycles
  
  if (curr_time == 5)
  {
    Serial.println("It's night time!");
    l_t_flag = 0;
    l_w_flag = 0;
    r_t_flag = 0;
    r_w_flag = 0;
    //set all turn and walk flags off
    //traffic is so low it's a free for all

    state = 1;
    //indicate the night state for the button check function
    
    digitalWrite(l_t_blue, LOW);
    digitalWrite(l_w_green, LOW);
    digitalWrite(r_t_blue, LOW);
    digitalWrite(r_w_green, LOW);
    //turn off turn and walk indicator lights
    
    night(next_time);
    //calls the night function
    
    curr_time = 0;
    //set the time back to zero

    state = 0;
    //set the state back to 0
  }
  
}
//call the night function and reset the time

void button_check(bool &l_t_flag, bool &l_w_flag, bool &r_t_flag, bool &r_w_flag, bool &emer_flag, bool state)
//checks the status of the buttons and photo-resistor and flags if a change is detected
//uses the state flag to disable checking the buttons during the night function
{
  
  int button_change = 0;
  int emer = 0;
  unsigned int curr_light = 0;
  //set flags to detect changes in inputs

  if (digitalRead(l_t_switch) == LOW && l_t_flag == 0 && state == 0)
  {
    l_t_flag = 1;
    button_change = 1;
    digitalWrite(l_t_blue, HIGH);
  }
  //check the status of the left turn switch

  if (digitalRead(l_w_switch) == LOW && l_w_flag == 0 && state == 0)
  {
    l_w_flag = 1;
    button_change = 1;
    digitalWrite(l_w_green, HIGH);
  }
  //check the status of the left walk switch

  if (digitalRead(r_t_switch) == LOW && r_t_flag == 0 && state == 0)
  {
    r_t_flag = 1;
    button_change = 1;
    digitalWrite(r_t_blue, HIGH);
  }
  //check the status of the right turn switch

  if (digitalRead(r_w_switch) == LOW && r_w_flag == 0 && state == 0)
  {
    r_w_flag = 1;
    button_change = 1;
    digitalWrite(r_w_green, HIGH);
  }
  //check the status of the right walk switch

  curr_light = analogRead(r_photo);
  
  if (curr_light < .50*norm_light || curr_light > 1.3*norm_light)
  {
    emer = 1;
    
  }
  //check the status of the photo-resistor, if too high or too low, declare a state of emergency

  if (button_change == 1)
  {
    Serial.println("----------------------");
    Serial.println("Button change detected:");
    Serial.print("Left Turn: ");
    Serial.println(l_t_flag);
    Serial.print("Left Ped: ");
    Serial.println(l_w_flag);
    Serial.print("Right Turn: ");
    Serial.println(r_t_flag);
    Serial.print("Right Ped: ");
    Serial.println(r_w_flag);
    Serial.println("----------------------");   
  }
  //output the updated button flags

  if (emer == 1)
  {
    Serial.println("Emergency Detected");
    Serial.print("Light level of: ");
    Serial.println(curr_light);
    emer_flag = 1;
    emergency(next_time, now, norm_light, emer_flag);
  }
  //indicate the emergency and call the emergency function
  
}

void controller(bool &state, bool priority, bool &l_t_flag, bool &l_w_flag, bool &r_t_flag, bool &r_w_flag, int l_green_time, int l_yellow_time, int r_green_time, int r_yellow_time, bool &emer_flag)
{
  Serial.println("In controller");
  if (l_t_flag == 1 && r_w_flag == 1)
  {
    right_walk_turn(next_time, now, l_green_time, l_yellow_time, emer_flag);
    Serial.println("Cont. Right Walk Turn");
    l_t_flag = 0;
    digitalWrite(l_t_blue, LOW);
    r_w_flag = 0;
    digitalWrite(r_w_green, LOW);
    Serial.println("l_t_blue and r_w_green set to 0");
  }
  //call the function to let the left set turn and go straight and to have the right side walk
  //then set the turn and walk flags back to zero and turn off the indicator lights
  
   else if (l_t_flag == 1){
   Serial.println("Cont. Left Turn");
   left_turn(next_time, now, l_green_time, l_yellow_time, emer_flag);
   l_t_flag = 0;
   digitalWrite(l_t_blue, LOW);
   Serial.println("l_t_green set to low");
   }
   //if only the left turn flag is set, call the left turn function

   else if (r_w_flag == 1){
    Serial.println("Cont. Right Walk");
    right_walk(next_time, now, l_green_time, l_yellow_time, emer_flag);
    r_w_flag = 0;
    digitalWrite(r_w_green, LOW);
    Serial.println("r_w_green set to low");
   }
   //if only the right walk flag is set, call the right walk function
    
   else {
    Serial.println("Cont. Left Straight");
    left_straight(next_time, now, l_green_time, l_yellow_time, emer_flag);
    }
    //if neither the left turn or right walk flags are set, just call the left straight function

 if (r_t_flag == 1 && l_w_flag == 1)
  {
    if (priority == 1){
      Serial.println("Cont. Priority Right Turn");
      right_turn(next_time, now, r_green_time, r_yellow_time, emer_flag);
      r_t_flag = 0;
      digitalWrite(r_t_blue, LOW);
      priority = 0;
      Serial.println("r_t_blue and priority set to 0");
      }
    else if (priority == 0){
      Serial.println("Cont. Priority Left Walk");
      left_walk(next_time, now, r_green_time, r_yellow_time, emer_flag);
      l_w_flag = 0;
      digitalWrite(l_w_green, LOW);
      priority = 1;
      Serial.println("l_w_flag set to 0 and priority set to 1");
    }
    //since right turn and left walk cannot happen at the same time,
    //alternate between them if they both are set, the priority value
    //is used to determine which one went last when they were tied
      
  }
  
  else if (r_t_flag == 1){
   Serial.println("Cont. Right Turn");
   right_turn(next_time, now, r_green_time, r_yellow_time, emer_flag);
   r_t_flag = 0;
   digitalWrite(r_t_blue, LOW);
   Serial.println("r_t_flag set to 0");
   }
   //if the right turn flag is set and not the left walk, call the right turn
   
  else if (l_w_flag == 1){
    Serial.println("Cont. Left Walk");
    left_walk(next_time, now, r_green_time, r_yellow_time, emer_flag);
    l_w_flag = 0;
    digitalWrite(l_w_green, LOW);
    Serial.println("l_w_flag set to 0");
  }
  //if the left walk flag is set and not the right turn, call the left walk
  
  else if (r_t_flag == 0 && l_w_flag == 0){
    Serial.println("Cont. Right Straight");
    right_straight(next_time, now, r_green_time, r_yellow_time, emer_flag);
    } 
    //if none of them are set, just call the right straight function
}

void left_straight(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag)
{
  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
  //while waiting for the next change time, check the buttons
    
  digitalWrite(l_s_red, LOW);
  digitalWrite(l_s_green, HIGH);
  next_time = millis( ) + l_green_time;
  Serial.println("l_s_green");
  Serial.print("Next time is: ");
  Serial.println(next_time);
  //change lights, set the next change time

  while (millis( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
  //wait while checking buttons

  if (emer_flag == 0)
  {
  //if the emergency flag is set AKA (an emergency occurred during a previous part of the function)
  //skip this part so the cycle doesn't resume in the middle of a yellow light
    digitalWrite(l_s_green, LOW);
    digitalWrite(l_s_yellow, HIGH);
    next_time = millis( ) + l_yellow_time;
    Serial.println("l_s_yellow");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  //change lights, set the next change time

  while (millis ( ) <= next_time){
     button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
  //wait while checking buttons

  if (emer_flag == 0)
  {
    digitalWrite(l_s_yellow, LOW);
    digitalWrite(l_s_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("l_s_red");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  emer_flag = 0;
  //set emergency flag back to zero so next cycle will go normally
}

void left_turn(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag)
{
  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
  
  digitalWrite(l_s_red, LOW);
  digitalWrite(l_t_red, LOW);
  digitalWrite(l_s_green, HIGH);
  digitalWrite(l_t_green, HIGH);
  next_time = millis( ) + l_green_time;
  Serial.println("l_t_green");

  while (millis( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_green, LOW);
    digitalWrite(l_t_green, LOW);
    digitalWrite(l_s_yellow, HIGH);
    digitalWrite(l_t_yellow, HIGH);
    next_time = millis( ) + l_yellow_time;
    Serial.println("l_t_yellow");
  }

  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_yellow, LOW);
    digitalWrite(l_t_yellow, LOW);
    digitalWrite(l_s_red, HIGH);
    digitalWrite(l_t_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("l_t_red");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  emer_flag = 0;

}

void right_walk_turn(unsigned long &next_time, unsigned long &now, int l_green_time, int l_yellow_time, bool &emer_flag)
{
    while (millis ( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

   digitalWrite(l_s_red, LOW);
   digitalWrite(l_t_red, LOW);
   digitalWrite(r_w_red, LOW);
   digitalWrite(l_s_green, HIGH);
   digitalWrite(l_t_green, HIGH);
   digitalWrite(r_w_white, HIGH);
   next_time = millis( ) + l_green_time + 2000;
   Serial.println("r_w_t_green");

   while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_green, LOW);
    digitalWrite(l_t_green, LOW);
    digitalWrite(l_s_yellow, HIGH);
    digitalWrite(l_t_yellow, HIGH);
    next_time = millis( ) + l_yellow_time;
    Serial.println("r_w_t_yellow");
  }

    while (millis ( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_yellow, LOW);
    digitalWrite(l_t_yellow, LOW);
    digitalWrite(r_w_white, LOW);
    digitalWrite(l_s_red, HIGH);
    digitalWrite(l_t_red, HIGH);
    digitalWrite(r_w_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("r_w_t_red");
  }
  emer_flag = 0;
  
}

  void left_walk(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag)
  {
    while (millis ( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

    digitalWrite(r_s_red, LOW);
    digitalWrite(l_w_red, LOW);
    digitalWrite(r_s_green, HIGH);
    digitalWrite(l_w_white, HIGH);
    next_time = millis( ) + r_green_time + 4000;
    Serial.println("r_s_green");
    Serial.print("Next time is: ");
    Serial.println(next_time);

    while (millis( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

    if (emer_flag == 0)
    {
      digitalWrite(r_s_green, LOW);
      digitalWrite(r_s_yellow, HIGH);
      next_time = millis( ) + r_yellow_time;
      Serial.println("r_s_yellow");
      Serial.print("Next time is: ");
      Serial.println(next_time);
    }

    while (millis ( ) <= next_time){
     button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

    if (emer_flag == 0)
    {
      digitalWrite(r_s_yellow, LOW);
      digitalWrite(l_w_white, LOW);
      digitalWrite(r_s_red, HIGH);
      digitalWrite(l_w_red, HIGH);
      next_time = millis( ) + 1000;
      Serial.println("r_s_red");
      Serial.print("Next time is: ");
      Serial.println(next_time);
    }
    emer_flag = 0;
    
  }

void right_straight(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag)
{
  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
     
  digitalWrite(r_s_red, LOW);
  digitalWrite(r_s_green, HIGH);
  next_time = millis( ) + r_green_time;
  Serial.println("r_s_green");
  Serial.print("Next time is: ");
  Serial.println(next_time);

  while (millis( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(r_s_green, LOW);
    digitalWrite(r_s_yellow, HIGH);
    next_time = millis( ) + r_yellow_time;
    Serial.println("r_s_yellow");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }

  while (millis ( ) <= next_time){
     button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(r_s_yellow, LOW);
    digitalWrite(r_s_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("r_s_red");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  emer_flag = 0;
}
  

void right_turn(unsigned long &next_time, unsigned long &now, int r_green_time, int r_yellow_time, bool &emer_flag)
{
  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
  
  digitalWrite(r_s_red, LOW);
  digitalWrite(r_t_red, LOW);
  digitalWrite(r_s_green, HIGH);
  digitalWrite(r_t_green, HIGH);
  next_time = millis( ) + r_green_time;
  Serial.println("r_t_green");

  while (millis( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(r_s_green, LOW);
    digitalWrite(r_t_green, LOW);
    digitalWrite(r_s_yellow, HIGH);
    digitalWrite(r_t_yellow, HIGH);
    next_time = millis( ) + r_yellow_time;
    Serial.println("r_t_yellow");
  }

  while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(r_s_yellow, LOW);
    digitalWrite(r_t_yellow, LOW);
    digitalWrite(r_s_red, HIGH);
    digitalWrite(r_t_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("r_t_red");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  emer_flag = 0;
}

void right_walk(unsigned long next_time, unsigned long now, int l_green_time, int l_yellow_time, bool &emer_flag)
{
  while (millis ( ) <= next_time){
  button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
    
  digitalWrite(l_s_red, LOW);
  digitalWrite(r_w_red, LOW);
  digitalWrite(l_s_green, HIGH);
  digitalWrite(r_w_white, HIGH);
  next_time = millis( ) + l_green_time;
  Serial.println("r_w_s_green");
  Serial.print("Next time is: ");
  Serial.println(next_time);

  while (millis( ) <= next_time){
      button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_green, LOW);
    digitalWrite(l_s_yellow, HIGH);
    next_time = millis( ) + l_yellow_time;
    Serial.println("r_w_s_yellow");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }

  while (millis ( ) <= next_time){
     button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}

  if (emer_flag == 0)
  {
    digitalWrite(l_s_yellow, LOW);
    digitalWrite(r_w_white, LOW);
    digitalWrite(l_s_red, HIGH);
    digitalWrite(r_w_red, HIGH);
    next_time = millis( ) + 1000;
    Serial.println("r_w_s_red");
    Serial.print("Next time is: ");
    Serial.println(next_time);
  }
  emer_flag = 0;
  //set the emergency flag back to zero so normal operations can resume
}

void emergency(unsigned long &next_time, unsigned long now, unsigned int norm_light, bool &emer_flag)
{
  bool done = 0;
  Serial.println("----------------------");
  Serial.println("In Emergency");
  
  for (int n = 22; n <= 29; n++){
     digitalWrite(n, LOW);}

  for (int n = 32; n <= 39; n++){
     digitalWrite(n, LOW);}  
  //turn off all of the lights   
     
  digitalWrite(l_s_red, HIGH);
  digitalWrite(l_t_red, HIGH);
  digitalWrite(r_s_red, HIGH);
  digitalWrite(r_t_red, HIGH);
  digitalWrite(l_w_red, HIGH);
  digitalWrite(r_w_red, HIGH);
  digitalWrite(l_ind, HIGH);
  //turn on all of the red lights

  Serial.println("Emergency Lights Set");

  next_time = millis( ) + 2000;
  //set the next time to two seconds in advance
  
  while(done == 0)
  {
  //while the emergency vehicle has been visible in the last two seconds:
   unsigned int curr_light;
      curr_light = analogRead(r_photo);
  
  if (curr_light < .55*norm_light || curr_light > 1.3*norm_light)
  {
    next_time = millis( ) + 2000;
    //if the emergency vehicle is still visible keep resetting the time
    //until two seconds after the vehicle has passed
  }
  else if (millis( ) > next_time)
  {
    Serial.println("Emergency Over");
    Serial.println("----------------------");
    digitalWrite(l_ind, LOW);
    done = 1;
    //systems back to normal, reset
  }
  }
}

void night(unsigned long &next_time)
{
  Serial.println("----------------------");
  Serial.println("In night function.");
  digitalWrite(l_s_red, LOW);
  digitalWrite(l_t_red, LOW);
  digitalWrite(l_ind, HIGH);
  //turn off left red lights and turn on night indicator

  digitalWrite(l_s_yellow, HIGH);
  digitalWrite(l_t_yellow, HIGH);
  digitalWrite(l_w_red, HIGH);
  digitalWrite(r_s_red, HIGH);
  digitalWrite(r_t_red, HIGH);
  digitalWrite(r_w_red, HIGH);
  //turn on all of the lights before jumping into the loop
  //(the extra Write statements are there due to delay problems
  //observed while debugging
  
  for (int n = 0; n <=8; n++)
  {
  //loop till morning
    Serial.println("ON");
    digitalWrite(l_s_yellow, HIGH);
    digitalWrite(l_t_yellow, HIGH);
    digitalWrite(l_w_red, HIGH);
    digitalWrite(r_s_red, HIGH);
    digitalWrite(r_t_red, HIGH);
    digitalWrite(r_w_red, HIGH);
    //turn all of the lights on
    
    next_time = millis( ) + 500;
    //set the time to half a second in advance

    while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
    //continually check the status of the photo-resistor but use state 1 to disable
    //the button checking until morning, its a free for all that late at night    

    Serial.println("OFF");
    digitalWrite(l_s_yellow, LOW);
    digitalWrite(l_t_yellow, LOW);
    digitalWrite(l_s_red, LOW);
    digitalWrite(l_t_red, LOW);
    digitalWrite(l_w_red, LOW);
    digitalWrite(r_s_red, LOW);
    digitalWrite(r_t_red, LOW);
    digitalWrite(r_w_red, LOW);
    //turn all of the lights off

    next_time = millis( ) + 500;
    //set the time to half a second in advance

    while (millis ( ) <= next_time){
    button_check(l_t_flag, l_w_flag, r_t_flag, r_w_flag, emer_flag, state);}
    //continually check the status of the photo-resistor but use state 1 to disable
    //the button checking until morning, its a free for all that late at night
    
  }

  digitalWrite(l_s_red, HIGH);
  digitalWrite(l_t_red, HIGH);
  digitalWrite(l_w_red, HIGH);
  digitalWrite(r_s_red, HIGH);
  digitalWrite(r_t_red, HIGH);
  digitalWrite(r_w_red, HIGH);
  digitalWrite(l_ind, LOW);
  Serial.println("----------------------");
  //reset lights for the normal cycle
}

