/* TODO:
- add NoMusic functions
- get LCD Screen for combineColors so ppl know what's going on
*/

//Libraries
#include <Adafruit_NeoPixel.h>  //Library to simplify interacting with the LED strand
#include <IRremote.h>
#ifdef __AVR__
#include <avr/power.h>   //Includes the library for power reduction registers if your chip supports them. 
#endif                   //More info: http://www.nongnu.org/avr-libc/user-manual/group__avr__power.htlm

//Constants (change these as necessary)
#define LED_PIN   A5  //Pin for the pixel strand. Does not have to be analog.
//#define LED_TOTAL 300  //Change this to the number of LEDs in your strand.
#define LED_TOTAL 100  //Change this to the number of LEDs in your strand.
#define LED_HALF  LED_TOTAL/2
#define AUDIO_PIN A0  //Pin for the envelope of the sound detector
//#define KNOB_PIN  A1  //Pin for the trimpot 10K
#define RECEIVER 7
#define RBTHRESHOLD 1529

//////////<Globals>
//  These values either need to be remembered from the last pass of loop() or 
//  need to be accessed by several functions in one pass, so they need to be global.

Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);  //LED strand objetc

uint16_t gradient = 0; //Used to iterate and loop through each color palette gradually

uint8_t volume = 0;    //Holds the volume level read from the sound detector.
uint8_t last = 0;      //Holds the value of volume from the previous loop() pass.

float maxVol = 15;     //Holds the largest volume recorded thus far to proportionally adjust the visual's responsiveness.
float avgVol = 0;      //Holds the "average" volume-level to proportionally adjust the visual experience.
float avgBump = 0;     //Holds the "average" volume-change to trigger a "bump."
float brightness = .50; //Multiplier to say how bright LED's should be

bool left = false;  //Determines the direction of iteration. Recycled in PaletteDance()
int8_t dotPos = 0;  //Holds which LED in the strand the dot is positioned at. Recycled in most other visuals.
float avgTime = 0;  //Holds the "average" amount of time between each "bump" (used for pacing the dot's movement).

bool bump = false;     //Used to pass if there was a "bump" in volume

//IMPORTANT:
//  This array holds the "threshold" of each color function (i.e. the largest number they take before repeating).
//  The values are in the same order as in ColorPalette()'s switch case (Rainbow() is first, etc). This is simply to
//  keep "gradient" from overflowing, the color functions themselves can take any positive value. For example, the
//  largest value Rainbow() takes before looping is 1529, so "gradient" should reset after 1529, as listed.
//     Make sure you add/remove values accordingly if you add/remove a color function in the switch-case in ColorPalette().
//uint16_t thresholds[] = {1529, 1019, 764, 764, 764, 1274};

IRrecv irrecv(RECEIVER);//connect the receiver to the irrecv object
decode_results results; // This will store our IR received codes

unsigned int setting = 999;
unsigned int lastSetting = 99;

bool on = false;

//////////</Globals>


//////////<Standard Functions>

void setup() {    //Like it's named, this gets ran before any other function.

  Serial.begin(9600); //Sets data rate for serial data transmission.

  irrecv.enableIRIn(); // Start the receiverfor irrecv

  strand.begin(); //Initialize the LED strand object.
  strand.show();  //Show a blank strand, just to get the LED's ready for use.
  strand.clear();
  Serial.println("Starting...");
}


void loop() {  //This is where the magic happens. This loop produces each frame of the visual.
  volume = analogRead(AUDIO_PIN);       //Record the volume level from the sound detector
  //knob = analogRead(KNOB_PIN) / 1023.0; //Record how far the trimpot is twisted
  avgVol = (avgVol + volume) / 2.0;     //Take our "average" of volumes.

  if (irrecv.decode(&results)) changeSettings();
  
  //Sets a threshold for volume.
  //  In practice I've found noise can get up to 15, so if it's lower, the visual thinks it's silent.
  //  Also if the volume is less than average volume / 2 (essentially an average with 0), it's considered silent.
  if (volume < avgVol / 2.0 || volume < 15) volume = 0;

  //If the current volume is larger than the loudest value recorded, overwrite
  if (volume > maxVol) maxVol = volume;

  //This is where "gradient" is reset to prevent overflow.
  if (gradient > 1529) {

    gradient %= 1530;

    //Everytime a palette gets completed is a good time to readjust "maxVol," just in case
    //  the song gets quieter; we also don't want to lose brightness intensity permanently 
    //  because of one stray loud sound.
    maxVol = (maxVol + volume) / 2.0;
  }

  //If there is a decent change in volume since the last pass, average it into "avgBump"
  if (volume - last > avgVol - last && avgVol - last > 0) avgBump = (avgBump + (volume - last)) / 2.0;

  //if there is a notable change in volume, trigger a "bump"
  bump = (volume - last) > avgBump;
  
  if (setting != lastSetting){
    if(on){
      switch(setting){
    case 0:
      Pulse();
      break;
    case 1:
      PalettePulse();
      break;
    case 2:
      PaletteDance();
      break;
    case 3:
      Snake();
      break;
    case 4: //setColor(RED)
      setColor(setting);
      break;
    case 5: //setColor(GREEN)
      setColor(setting);
      break;
    case 6: //setColor(BLUE)
      setColor(setting);
      break;
    case 7: //setColor(WHITE)
      setColor(setting);
      break;
    case 8: //setColor(YELLOW)
      setColor(setting);
      break;
    case 9: //setColor(ORANGE)
      setColor(setting);
      break;
    case 10: //setColor(CYAN)
      setColor(setting);
      break;
    case 11: //setColor(PURPLE)
      setColor(setting);
      break;
    case 17: // RED-ORANGE
      setColor(setting);
      break;
    case 18: // DARK GREEN
      setColor(setting);
      break;
    case 19: // DARK BLUE
      setColor(setting);
      break;
    case 20: // LIGHT GREEN
      setColor(setting);
      break;
    case 21: // YELLOW-ORANGE
      setColor(setting);
      break;
    case 22: // PURPLE-PINK
      setColor(setting);
      break;
    case 23: // SKY BLUE
      setColor(setting);
      break;
    case 24: // PINK
      setColor(setting);
      break;
    default:
      // all white
      setColor(999);
      break;
    }
    }
  }
  lastSetting=setting;
  
  gradient++;    //Increments gradient

  last = volume; //Records current volume for next pass

  delay(30);   //Paces visuals so they aren't too fast to be enjoyable
}

//////////</Standard Functions>


//////////<Display Functions>

//PULSE//Pulse from center of the strand
void Pulse() {
  strand.clear();

  fade(0.75);   //Listed below, this function simply dims the colors a little bit each pass of loop()

  //Advances the gradient to the next noticeable color if there is a "bump"
  if (bump) gradient += 64;

  //If it's silent, we want the fade effect to take over, hence this if-statement
  if (volume > 0) {
    uint32_t col = Rainbow(gradient); //Our retrieved 32-bit color

    //These variables determine where to start and end the pulse since it starts from the middle of the strand.
    //  The quantities are stored in variables so they only have to be computed once.
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    //Listed above, LED_HALF is simply half the number of LEDs on your strand. ↑ this part adjusts for an odd quantity.

    for (int i = start; i < finish; i++) {

      //"damp" creates the fade effect of being dimmer the farther the pixel is from the center of the strand.
      //  It returns a value between 0 and 1 that peaks at 1 at the center of the strand and 0 at the ends.
      float damp = float(
                     ((finish - start) / 2.0) -
                     abs((i - start) - ((finish - start) / 2.0))
                   )
                   / float((finish - start) / 2.0);

      //Sets the each pixel on the strand to the appropriate color and intensity
      //  strand.Color() takes 3 values between 0 & 255, and returns a 32-bit integer.
      //  Notice "knob" affecting the brightness, as in the rest of the visuals.
      //  Also notice split() being used to get the red, green, and blue values.
      strand.setPixelColor(i, strand.Color(
                             split(col, 0) * pow(damp, 2.0) * brightness, // changed knob to brightness
                             split(col, 1) * pow(damp, 2.0) * brightness,
                             split(col, 2) * pow(damp, 2.0) * brightness
                           ));
    }
    //Sets the max brightness of all LEDs. If it's loud, it's brighter.
    //  "knob" was not used here because it occasionally caused minor errors in color display.
    strand.setBrightness(255.0 * pow(volume / maxVol, 2)); // added a brightness multiplier
  }

  //This command actually shows the lights. If you make a new visualization, don't forget this!
  strand.show();
}


//PALETTEPULSE
//Same as Pulse(), but colored the entire pallet instead of one solid color
void PalettePulse() {
  strand.clear();
  fade(0.75);
  if (bump) gradient += RBTHRESHOLD / 24;
  if (volume > 0) {
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    for (int i = start; i < finish; i++) {
      float damp = sin((i - start) * PI / float(finish - start));
      damp = pow(damp, 2.0);

      //This is the only difference from Pulse(). The color for each pixel isn't the same, but rather the
      //  entire gradient fitted to the spread of the pulse, with some shifting from "gradient".
      int val = RBTHRESHOLD * (i - start) / (finish - start);
      val += gradient;
      uint32_t col = Rainbow(gradient);

      uint32_t col2 = strand.getPixelColor(i);
      uint8_t colors[3];
      float avgCol = 0, avgCol2 = 0;
      for (int k = 0; k < 3; k++) {
        colors[k] = split(col, k) * damp * brightness * pow(volume / maxVol, 2);
        avgCol += colors[k];
        avgCol2 += split(col2, k);
      }
      avgCol /= 3.0, avgCol2 /= 3.0;
      if (avgCol > avgCol2) strand.setPixelColor(i, strand.Color(colors[0], colors[1], colors[2]));
    }
  }
  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show();
}


//SNAKE
//Dot sweeping back and forth to the beat
void Snake() {
  strand.clear();
  if (bump) {

    //Change color a little on a bump
    gradient += RBTHRESHOLD / 30;

    //Change the direction the dot is going to create the illusion of "dancing."
    left = !left;
  }

  fade(0.975); //Leave a trail behind the dot.

  uint32_t col = Rainbow(gradient);

  //The dot should only be moved if there's sound happening.
  //  Otherwise if noise starts and it's been moving, it'll appear to teleport.
  if (volume > 0) {

    //Sets the dot to appropriate color and intensity
    strand.setPixelColor(dotPos, strand.Color(
                           float(split(col, 0)) * pow(volume / maxVol, 1.5) * brightness, // changed knob to brightness
                           float(split(col, 1)) * pow(volume / maxVol, 1.5) * brightness,
                           float(split(col, 2)) * pow(volume / maxVol, 1.5) * brightness)
                        );

    //This is where "avgTime" comes into play.
    //  That variable is the "average" amount of time between each "bump" detected.
    //  So we can use that to determine how quickly the dot should move so it matches the tempo of the music.
    //  The dot moving at normal loop speed is pretty quick, so it's the max speed if avgTime < 0.15 seconds.
    //  Slowing it down causes the color to update, but only change position every other amount of loops.
    if (avgTime < 0.15)                                               dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.15 && avgTime < 0.5 && gradient % 2 == 0)   dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.5 && avgTime < 1.0 && gradient % 3 == 0)    dotPos += (left) ? -1 : 1;
    else if (gradient % 4 == 0)                                       dotPos += (left) ? -1 : 1;
  }
  
  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show(); // Display the lights

  //Check if dot position is out of bounds.
  if (dotPos < 0)    dotPos = strand.numPixels() - 1;
  else if (dotPos >= strand.numPixels())  dotPos = 0;
}


//PALETTEDANCE
//Projects a whole palette which oscillates to the beat, similar to the snake but a whole gradient instead of a dot
void PaletteDance() {
  //This is the most calculation-intensive visual, which is why it doesn't need delayed.
  strand.clear();

  if (bump) left = !left; //Change direction of iteration on bump

  //Only show if there's sound.
  if (volume > avgVol) {

    //This next part is convoluted, here's a summary of what's happening:
    //  First, a sin wave function is introduced to change the brightness of all the pixels (stored in "sinVal")
    //      This is to make the dancing effect more obvious. The trick is to shift the sin wave with the color so it all appears
    //      to be the same object, one "hump" of color. "dotPos" is added here to achieve this effect.
    //  Second, the entire current palette is proportionally fitted to the length of the LED strand (stored in "val" each pixel).
    //      This is done by multiplying the ratio of position and the total amount of LEDs to the palette's threshold.
    //  Third, the palette is then "shifted" (what color is displayed where) by adding "dotPos."
    //      "dotPos" is added to the position before dividing, so it's a mathematical shift. However, "dotPos"'s range is not
    //      the same as the range of position values, so the function map() is used. It's basically a built in proportion adjuster.
    //  Lastly, it's all multiplied together to get the right color, and intensity, in the correct spot.
    //      "gradient" is also added to slowly shift the colors over time.
    for (int i = 0; i < strand.numPixels(); i++) {

      float sinVal = abs(sin(
                           (i + dotPos) *
                           (PI / float(strand.numPixels() / 1.25) )
                         ));
      sinVal *= sinVal;
      sinVal *= volume / maxVol;
      sinVal *= brightness; // changed knob to brightness

      unsigned int val = float(RBTHRESHOLD + 1)
                         //map takes a value between -LED_TOTAL and +LED_TOTAL and returns one between 0 and LED_TOTAL
                         * (float(i + map(dotPos, -1 * (strand.numPixels() - 1), strand.numPixels() - 1, 0, strand.numPixels() - 1))
                            / float(strand.numPixels()))
                         + (gradient);

      val %= RBTHRESHOLD; //make sure "val" is within range of the palette

      uint32_t col = Rainbow(gradient); //get the color at "val"  

      strand.setPixelColor(i, strand.Color(
                             float(split(col, 0))*sinVal,
                             float(split(col, 1))*sinVal,
                             float(split(col, 2))*sinVal)
                          );
    }

    //After all that, appropriately reposition "dotPos."
    dotPos += (left) ? -1 : 1;
  }

  //If there's no sound, fade.
  else  fade(0.8);

  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show(); //Show lights.

  //Loop "dotPos" if it goes out of bounds.
  if (dotPos < 0) dotPos = strand.numPixels() - strand.numPixels() / 6;
  else if (dotPos >= strand.numPixels() - strand.numPixels() / 6)  dotPos = 0;
}


//SETCOLOR
//Sets the whole string to a single color
void setColor(unsigned int col){
  strand.clear();
  uint32_t color;
  switch(col){
    case 4: //RED
      color = strand.Color(255, 0, 0);
      break;
    case 5: //GREEN
      color = strand.Color(0, 255, 0);
      break;
    case 6: //BLUE
      color = strand.Color(0, 0, 255);
      break;
    case 7: //WHITE
      color = strand.Color(255, 255, 255);
      break;
    case 8: //YELLOW
      color = strand.Color(255, 255, 0);
      break;
    case 9: //ORANGE
      color = strand.Color(255, 125, 0);
      break;
    case 10: //CYAN
      color = strand.Color(0, 255, 255);
      break;
    case 11: //PURPLE
      color = strand.Color(255, 0, 255);
      break;
    case 17: //RED-ORANGE
      color = strand.Color(255, 76, 0);
      break;
    case 18: //DARK GREEN
      color = strand.Color(2, 142, 0);
      break;
    case 19: //DARK BLUE
      color = strand.Color(7, 0, 117);
      break;
    case 20: //LIGHT GREEN
      color = strand.Color(99, 255, 127);
      break;
    case 21: //YELLOW-ORANGE
      color = strand.Color(255, 212, 0);
      break;
    case 22: //PURPLE-PINK
      color = strand.Color(255, 0, 187);
      break;
    case 23: //SKY BLUE
      color = strand.Color(0, 255, 250);
      break;
    case 24: //PINK
      color = strand.Color(255, 127, 197);
      break;
    default:
      color = strand.Color(0,0,0);
      break;
  }
  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.fill(color, 0, LED_TOTAL);
  strand.show();
}

/*
//PULSE NO MUSIC
//Pulse from center of the strand
void PulseNoMusic() {

  fade(0.75);   //Listed below, this function simply dims the colors a little bit each pass of loop()

  //Advances the gradient to the next noticeable color if there is a "bump"
  if (bump) gradient += 64;

  if (1) {
    uint32_t col = Rainbow(gradient); //Our retrieved 32-bit color

    //These variables determine where to start and end the pulse since it starts from the middle of the strand.
    //  The quantities are stored in variables so they only have to be computed once.
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    //Listed above, LED_HALF is simply half the number of LEDs on your strand. ↑ this part adjusts for an odd quantity.

    for (int i = start; i < finish; i++) {

      //"damp" creates the fade effect of being dimmer the farther the pixel is from the center of the strand.
      //  It returns a value between 0 and 1 that peaks at 1 at the center of the strand and 0 at the ends.
      float damp = float(
                     ((finish - start) / 2.0) -
                     abs((i - start) - ((finish - start) / 2.0))
                   )
                   / float((finish - start) / 2.0);

      //Sets the each pixel on the strand to the appropriate color and intensity
      //  strand.Color() takes 3 values between 0 & 255, and returns a 32-bit integer.
      //  Notice "knob" affecting the brightness, as in the rest of the visuals.
      //  Also notice split() being used to get the red, green, and blue values.
      strand.setPixelColor(i, strand.Color(
                             split(col, 0) * pow(damp, 2.0) * brightness, // changed knob to brightness
                             split(col, 1) * pow(damp, 2.0) * brightness,
                             split(col, 2) * pow(damp, 2.0) * brightness
                           ));
    }
    //Sets the max brightness of all LEDs. If it's loud, it's brighter.
    //  "knob" was not used here because it occasionally caused minor errors in color display.
    strand.setBrightness(255.0 * pow(volume / maxVol, 2)); // added a brightness multiplier
  }

  //This command actually shows the lights. If you make a new visualization, don't forget this!
  strand.show();
}


//PALETTEPULSE NO MUSIC
//Same as Pulse(), but colored the entire pallet instead of one solid color
void PalettePulseNoMusic() {
  fade(0.75);
  if (bump) gradient += RBTHRESHOLD / 24;
  if (1) {
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    for (int i = start; i < finish; i++) {
      float damp = sin((i - start) * PI / float(finish - start));
      damp = pow(damp, 2.0);

      //This is the only difference from Pulse(). The color for each pixel isn't the same, but rather the
      //  entire gradient fitted to the spread of the pulse, with some shifting from "gradient".
      int val = RBTHRESHOLD * (i - start) / (finish - start);
      val += gradient;
      uint32_t col = Rainbow(gradient);

      uint32_t col2 = strand.getPixelColor(i);
      uint8_t colors[3];
      float avgCol = 0, avgCol2 = 0;
      for (int k = 0; k < 3; k++) {
        colors[k] = split(col, k) * damp * brightness * pow(volume / maxVol, 2);
        avgCol += colors[k];
        avgCol2 += split(col2, k);
      }
      avgCol /= 3.0, avgCol2 /= 3.0;
      if (avgCol > avgCol2) strand.setPixelColor(i, strand.Color(colors[0], colors[1], colors[2]));
    }
  }
  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show();
}


//SNAKE NO MUSIC
//Dot sweeping back and forth to the beat
void SnakeNoMusic() {
  if (bump) {

    //Change color a little on a bump
    gradient += RBTHRESHOLD / 30;

    //Change the direction the dot is going to create the illusion of "dancing."
    left = !left;
  }

  fade(0.975); //Leave a trail behind the dot.

  uint32_t col = Rainbow(gradient);

  //The dot should only be moved if there's sound happening.
  //  Otherwise if noise starts and it's been moving, it'll appear to teleport.
  if (1) {

    //Sets the dot to appropriate color and intensity
    strand.setPixelColor(dotPos, strand.Color(
                           float(split(col, 0)) * pow(volume / maxVol, 1.5) * brightness, // changed knob to brightness
                           float(split(col, 1)) * pow(volume / maxVol, 1.5) * brightness,
                           float(split(col, 2)) * pow(volume / maxVol, 1.5) * brightness)
                        );

    //This is where "avgTime" comes into play.
    //  That variable is the "average" amount of time between each "bump" detected.
    //  So we can use that to determine how quickly the dot should move so it matches the tempo of the music.
    //  The dot moving at normal loop speed is pretty quick, so it's the max speed if avgTime < 0.15 seconds.
    //  Slowing it down causes the color to update, but only change position every other amount of loops.
    if (avgTime < 0.15)                                               dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.15 && avgTime < 0.5 && gradient % 2 == 0)   dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.5 && avgTime < 1.0 && gradient % 3 == 0)    dotPos += (left) ? -1 : 1;
    else if (gradient % 4 == 0)                                       dotPos += (left) ? -1 : 1;
  }
  
  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show(); // Display the lights

  //Check if dot position is out of bounds.
  if (dotPos < 0)    dotPos = strand.numPixels() - 1;
  else if (dotPos >= strand.numPixels())  dotPos = 0;
}


//PALETTEDANCE NO MUSIC
//Projects a whole palette which oscillates to the beat, similar to the snake but a whole gradient instead of a dot
void PaletteDanceNoMusic() {
  //This is the most calculation-intensive visual, which is why it doesn't need delayed.

  if (bump) left = !left; //Change direction of iteration on bump

  //Only show if there's sound.
  if (1) {

    //This next part is convoluted, here's a summary of what's happening:
    //  First, a sin wave function is introduced to change the brightness of all the pixels (stored in "sinVal")
    //      This is to make the dancing effect more obvious. The trick is to shift the sin wave with the color so it all appears
    //      to be the same object, one "hump" of color. "dotPos" is added here to achieve this effect.
    //  Second, the entire current palette is proportionally fitted to the length of the LED strand (stored in "val" each pixel).
    //      This is done by multiplying the ratio of position and the total amount of LEDs to the palette's threshold.
    //  Third, the palette is then "shifted" (what color is displayed where) by adding "dotPos."
    //      "dotPos" is added to the position before dividing, so it's a mathematical shift. However, "dotPos"'s range is not
    //      the same as the range of position values, so the function map() is used. It's basically a built in proportion adjuster.
    //  Lastly, it's all multiplied together to get the right color, and intensity, in the correct spot.
    //      "gradient" is also added to slowly shift the colors over time.
    for (int i = 0; i < strand.numPixels(); i++) {

      float sinVal = abs(sin(
                           (i + dotPos) *
                           (PI / float(strand.numPixels() / 1.25) )
                         ));
      sinVal *= sinVal;
      sinVal *= volume / maxVol;
      sinVal *= brightness; // changed knob to brightness

      unsigned int val = float(RBTHRESHOLD + 1)
                         //map takes a value between -LED_TOTAL and +LED_TOTAL and returns one between 0 and LED_TOTAL
                         * (float(i + map(dotPos, -1 * (strand.numPixels() - 1), strand.numPixels() - 1, 0, strand.numPixels() - 1))
                            / float(strand.numPixels()))
                         + (gradient);

      val %= RBTHRESHOLD; //make sure "val" is within range of the palette

      uint32_t col = Rainbow(gradient); //get the color at "val"  

      strand.setPixelColor(i, strand.Color(
                             float(split(col, 0))*sinVal,
                             float(split(col, 1))*sinVal,
                             float(split(col, 2))*sinVal)
                          );
    }

    //After all that, appropriately reposition "dotPos."
    dotPos += (left) ? -1 : 1;
  }

  //If there's no sound, fade.
  else  fade(0.8);

  //strand.setBrightness(255.0 * brightness); // added a brightness multiplier
  strand.show(); //Show lights.

  //Loop "dotPos" if it goes out of bounds.
  if (dotPos < 0) dotPos = strand.numPixels() - strand.numPixels() / 6;
  else if (dotPos >= strand.numPixels() - strand.numPixels() / 6)  dotPos = 0;
}
*/
//////////</Display Functions>

//////////<Helper Functions>

//Fades lights by multiplying them by a value between 0 and 1 each pass of loop().
void fade(float damper) {
  damper *= brightness;
  //"damper" must be between 0 and 1, or else you'll end up brightening the lights or doing nothing.
  if (damper >= 1) damper = 0.99;

  for (int i = 0; i < strand.numPixels(); i++) {

    //Retrieve the color at the current position.
    uint32_t col = (strand.getPixelColor(i)) ? strand.getPixelColor(i) : strand.Color(0, 0, 0);

    //If it's black, you can't fade that any further.
    if (col == 0) continue;

    float colors[3]; //Array of the three RGB values

    //Multiply each value by "damper"
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;

    //Set the dampened colors back to their spot.
    strand.setPixelColor(i, strand.Color(colors[0] , colors[1], colors[2]));
  }
}


uint8_t split(uint32_t color, uint8_t i ) {

  //0 = Red, 1 = Green, 2 = Blue

  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}


//This function simply take a value and returns a gradient color
//  in the form of an unsigned 32-bit integer

//The gradient returns a different, changing color for each multiple of 255
//  This is because the max value of any of the 3 LEDs is 255, so it's
//  an intuitive cutoff for the next color to start appearing.
//  Gradients should also loop back to their starting color so there's no jumps in color.

uint32_t Rainbow(unsigned int i) {
  if (i > 1529) return Rainbow(i % 1530);
  if (i > 1274) return strand.Color(255, 0, 255 - (i % 255));   //violet -> red
  if (i > 1019) return strand.Color((i % 255), 0, 255);         //blue -> violet
  if (i > 764) return strand.Color(0, 255 - (i % 255), 255);    //aqua -> blue
  if (i > 509) return strand.Color(0, 255, (i % 255));          //green -> aqua
  if (i > 255) return strand.Color(255 - (i % 255), 255, 0);    //yellow -> green
  return strand.Color(255, i, 0);                               //red -> yellow
}

void changeSettings(){
  int resultCode = (results.value);
  Serial.println(results.value);
  uint32_t lastColor;
  switch(results.value & 0xFFFF){
    case 0x38C7: //On
      Serial.println("ON");
      on = true;
      setColor(17);
      delay(1000); 
      irrecv.resume();
      break;
    case 0xA857: //Off
      Serial.println("OFF");
      on = false;
      strand.clear();
      strand.show();
      delay(1000); 
      irrecv.resume();
      break;
    case 0xF807: //Brigthness 100%
      brightness = 1.0;
      lastColor = strand.getPixelColor(0);
      strand.clear();
      strand.setBrightness(255.0 * brightness);
      strand.fill(lastColor, 0, LED_TOTAL);
      strand.show();
      delay(1000);
      irrecv.resume();
      break;
    case 0xD827: //Brightness 75%
      brightness = .75;
      lastColor = strand.getPixelColor(0);
      strand.clear();
      strand.setBrightness(255.0 * brightness);
      strand.fill(lastColor, 0, LED_TOTAL);
      strand.show();
      delay(1000);
      irrecv.resume();
      break;
    case 0x7887: //Brigthness 50%
      brightness = .5;
      lastColor = strand.getPixelColor(0);
      strand.clear();
      strand.setBrightness(255.0 * brightness);
      strand.fill(lastColor, 0, LED_TOTAL);
      strand.show();
      delay(1000); 
      irrecv.resume();
      break;
    case 0x58A7: //Brightness 25%
      brightness = .25;
      lastColor = strand.getPixelColor(0);
      strand.clear();
      strand.setBrightness(255.0 * brightness);
      strand.fill(lastColor, 0, LED_TOTAL);
      strand.show();
      delay(1000);
      irrecv.resume();
      break;
    case 0x8877: //Pulse
      setting = 0;
      delay(1000);
      irrecv.resume();
      break;
    case 0xF00F: //PalettePulse
      setting = 1;
      delay(1000);
      irrecv.resume();
      break;
    case 0xC837: //Snake
      setting = 2;
      delay(1000);
      irrecv.resume();
      break;
    case 0x08F7: //PaletteDance
      setting = 3;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x807F: //All Red
      setting = 4;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x40BF: //All Green
      setting = 5;
      delay(1000); 
      irrecv.resume();
      break;
    case 0xC03F: //All Blue
      setting = 6;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x9867: //All White
      setting = 7;
      delay(1000); 
      irrecv.resume();
      break;
    case 0xB04F: //All Yellow
      setting = 8;
      delay(1000);
      irrecv.resume();
      break;
    case 0xE01F: //All Orange
      setting = 9;
      delay(1000);
      irrecv.resume();
      break;
    case 0x00FF: //All Cyan
      setting = 10;
      delay(1000);
      irrecv.resume();
      break;
    case 0x906F: //All Purple
      setting = 11;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x20DF: //All Red-Orange
      setting = 17;
      delay(1000); 
      irrecv.resume();
      break;
    case 0xA05F: //All Dark Green
      setting = 18;
      delay(1000);
      irrecv.resume();
      break;
    case 0x609F: //All Dark Blue
      setting = 19;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x10EF: //All Light Green
      setting = 20;
      delay(1000);
      irrecv.resume();
      break;
    case 0x50AF: //All Yellow-Orange
      setting = 21;
      delay(1000); 
      irrecv.resume();
      break;
    case 0xE817: //All Purple-Pink
      setting = 22;
      delay(1000); 
      irrecv.resume();
      break;
    case 0x708F: //All Sky Blue
      setting = 23;
      delay(1000); 
      irrecv.resume();
      break;
    case 0xB847: //All Pink
      setting = 24;
      delay(1000); 
      irrecv.resume();
      break;
    default:
      setting = 999;
      delay(1000);
      irrecv.resume();
  }
}

/*
0x38C7  on
0xA857  off
0x8877  flash
0xF00F  strobe
0xC837  fade
0x08F7  smooth
0x807F  red
0x40BF  green
0xC03F  blue
0x58A7  25
0x20DF  red-orange
0xA05F  dark green
0x609F  dark blue
0x7887  50
0xE01F  orange
0x10EF  light green
0x906F  purple
0xD827  75
0x50AF  yellow-orange
0x00FF  cyan
0xE817  purple-pink
0xF807  100
0xB04F  yellow
0x708F  sky-blue
0xB847  pink
0x9867  white
 */

//////////</Helper Functions>
