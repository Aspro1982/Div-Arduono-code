0 
1 // Purchase hardware from these links
2 // https://www.aliexpress.com/item/NEW-Keyestudio-40-RGB-LED-WS2812-Pixel-Matrix-Shield-for-Arduino/32656163025.html?
3 // https://www.aliexpress.com/item/Best-prices-UNO-R3-MEGA328P-ATMEGA16U2-for-Arduino-Compatible-Free-Shipping-Dropshipping/2050129579.html?
4 // Library is included in this folder
5 
6 #include <Adafruit_NeoPixel.h>
7 #ifdef __AVR__
8 #include <avr/power.h>
9 #endif
10 
11 #define PIN 13
12 
13 // Parameter 1 = number of pixels in strip
14 // Parameter 2 = Arduino pin number (most are valid)
15 // Parameter 3 = pixel type flags, add together as needed:
16 //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
17 //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
18 //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
19 //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
20 Adafruit_NeoPixel strip = Adafruit_NeoPixel(40, PIN, NEO_GRB + NEO_KHZ800);
21 
22 // IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
23 // pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
24 // and minimize distance between Arduino and first pixel.  Avoid connecting
25 // on a live circuit...if you must, connect GND first.
26 
27 void setup() {
28   // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
29   #if defined (__AVR_ATtiny85__)
30     if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
31   #endif
32   // End of trinket special code
33 
34 
35   strip.begin();
36   strip.show(); // Initialize all pixels to 'off'
37 }
38 
39 void loop() {
40   // Some example procedures showing how to display to the pixels:
41   colorWipe(strip.Color(255, 0, 0), 50); // Red
42   colorWipe(strip.Color(0, 255, 0), 50); // Green
43   colorWipe(strip.Color(0, 0, 255), 50); // Blue
44   // Send a theater pixel chase in...
45   theaterChase(strip.Color(127, 127, 127), 50); // White
46   theaterChase(strip.Color(127, 0, 0), 50); // Red
47   theaterChase(strip.Color(0, 0, 127), 50); // Blue
48 
49   rainbow(40);
50   rainbowCycle(40);
51   theaterChaseRainbow(100);
52 }
53 
54 // Fill the dots one after the other with a color
55 void colorWipe(uint32_t c, uint8_t wait) {
56   for(uint16_t i=0; i<strip.numPixels(); i++) {
57     strip.setPixelColor(i, c);
58     strip.show();
59     delay(wait);
60   }
61 }
62 
63 void rainbow(uint8_t wait) {
64   uint16_t i, j;
65 
66   for(j=0; j<256; j++) {
67     for(i=0; i<strip.numPixels(); i++) {
68       strip.setPixelColor(i, Wheel((i+j) & 255));
69     }
70     strip.show();
71     delay(wait);
72   }
73 }
74 
75 // Slightly different, this makes the rainbow equally distributed throughout
76 void rainbowCycle(uint8_t wait) {
77   uint16_t i, j;
78 
79   for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
80     for(i=0; i< strip.numPixels(); i++) {
81       strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
82     }
83     strip.show();
84     delay(wait);
85   }
86 }
87 
88 //Theatre-style crawling lights.
89 void theaterChase(uint32_t c, uint8_t wait) {
90   for (int j=0; j<10; j++) {  //do 10 cycles of chasing
91     for (int q=0; q < 3; q++) {
92       for (int i=0; i < strip.numPixels(); i=i+3) {
93         strip.setPixelColor(i+q, c);    //turn every third pixel on
94       }
95       strip.show();
96 
97       delay(wait);
98 
99       for (int i=0; i < strip.numPixels(); i=i+3) {
100         strip.setPixelColor(i+q, 0);        //turn every third pixel off
101       }
102     }
103   }
104 }
105 
106 //Theatre-style crawling lights with rainbow effect
107 void theaterChaseRainbow(uint8_t wait) {
108   for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
109     for (int q=0; q < 3; q++) {
110       for (int i=0; i < strip.numPixels(); i=i+3) {
111         strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
112       }
113       strip.show();
114 
115       delay(wait);
116 
117       for (int i=0; i < strip.numPixels(); i=i+3) {
118         strip.setPixelColor(i+q, 0);        //turn every third pixel off
119       }
120     }
121   }
122 }
123 
124 // Input a value 0 to 255 to get a color value.
125 // The colours are a transition r - g - b - back to r.
126 uint32_t Wheel(byte WheelPos) {
127   WheelPos = 255 - WheelPos;
128   if(WheelPos < 85) {
129     return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
130   }
131   if(WheelPos < 170) {
132     WheelPos -= 85;
133     return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
134   }
135   WheelPos -= 170;
136   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
137 }
138 