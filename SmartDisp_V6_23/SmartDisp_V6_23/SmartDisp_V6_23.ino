/** @mainpage
* The SmartDisp.ino program implements an application that
* displays times, date, humidity and temperatures on a Matrix
*
* @author  Lukas Christian
* @version V6.23 2020/04/14
* @since   2019-09
* @date 2019/Sep/09 - 2020/Apr/17
*/

/* control leds*/
#include <Adafruit_NeoPixel.h>

/* RTC*/
#include <Wire.h>
#include <ds3231.h>

/* humanity and temperatures sensor*/
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/* NTP_time_client with wifi*/
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <string.h>

const char *ssid = "TP-LINK_AAB9";
const char *password = "08728425";

const long utcOffsetInSeconds = 3600 * 2;

char wday[7][4] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.at.pool.ntp.org", utcOffsetInSeconds);





/* Font*/
#include "italic5x5_font.h"


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN D6					///< On Trinket or Gemma, suggest changing this to 1

ts t;							///<ts is a struct findable in ds3231.h
const unsigned char _atoz = 48;		///<skipping the letters to get Numbers in "intalic5x5_font.h"

#define NUMPIXELS 128			///<Popular NeoPixel ring size
#define MAXCOLOR 255			///<Maximum Color brightness
#define MATRIX_DIMENSION_X 20	///<Maximum of rows 
#define MATRIX_DIMENSION_Y 7	///<Maximum of lines
#define MAX_RAINBOW_COLOR 255	///<Maximum LED Color brightness 
#define RAINBOW_MULTIPLIER 3	///<stepwidth of the rainbow color

#define DHTPIN D7				///< Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22		///< DHT 22 (AM2302)

/*! @brief create an object pixels from class Adafruit_NeoPixel
*
* If any argument is invalid, the function has no effect.
*
* @param NUMPIXELS The maximum number of
* @param PIN The Pinname from uC which is connected with data pin.
* @param NEO_GRB The way how the rgb color is transmitted
* @param NEO_KHZ800 The speed how fast the the data are transmitted
*/
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 0.1  ///< Time (in milliseconds) to pause between pixels
#define CLOCK_TIME 500 ///< Time (in milliseconds) to set clock frequenz


DHT_Unified dht(DHTPIN, DHTTYPE); ///<create object from class DHT_Unified in DHT_U.h
uint32_t delayMS;	///<delaytime from DHT22


struct RGB {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

RGB matrix[MATRIX_DIMENSION_X][MATRIX_DIMENSION_Y];  ///< x is 0 to 19, y is 0 to 6

/** @brief Prints color r, g, b on position n when color is set
*
* If any argument is invalid, the function has no effect.
*
* @param copyBlack The Value if the color is disabled.
* @param n The position of the pixel.
* @param r The intensity from green light is from 0 to 255.
* @param g The intensity from red light is from 0 to 255.
* @param b The intensity from blue light is from 0 to 255.
* @return void
* @addtogroup displayMatrix
*/
void overrideColorPixels(bool copy_black, uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
	if (copy_black) {
		pixels.setPixelColor(n, r, g, b);
	}
	else {
		if (r > 0 && b > 0 && g > 0) {
			pixels.setPixelColor(n, r, g, b);
		}
	}
}

/** @brief calculates the zeroth row
*
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*
*/
void rowZero(bool copy_black) {
	int startpix;
	startpix = 0;
	overrideColorPixels(copy_black, startpix, matrix[0][3].r, matrix[0][3].g, matrix[0][3].b);
}


/** @brief calculates the first row
*
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*/
void rowOne(bool copy_black) {
	int startpix;
	startpix = 1;
	for (int i = 0; i <= 4; i++) {
		overrideColorPixels(copy_black, startpix + i, matrix[1][5 - i].r, matrix[1][5 - i].g, matrix[1][5 - i].b);
	}
}


/** @brief calculates the last row
*
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*/
void lastRow(bool copy_black) {
	int startpix;
	startpix = 125;
	for (int i = 0; i <= 2; i++) {
		overrideColorPixels(copy_black, startpix + i, matrix[19][4 - i].r, matrix[19][4 - i].g, matrix[19][4 - i].b);
	}
}

/** @brief calculates the non inverted row
*
* @param row_nmb The number of the row
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*/
void copyRow(short row_nmb, bool copy_black) {
	int startpix;
	// 2 --> 6
	// 4 --> 20
	// 6 --> 34
	startpix = (row_nmb - 2) * 7 + 6;
	for (int i = 0; i <= 6; i++) {
		overrideColorPixels(copy_black, startpix + i, matrix[row_nmb][i].r, matrix[row_nmb][i].g, matrix[row_nmb][i].b);
	}
}

/** @brief calculates the inverted rows
*
* @param row_nmb The number of the row
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*/
void copyInvRow(short row_nmb, bool copy_black) {
	int startpix;
	// 3 --> 13
	// 5 --> 27
	// 7 --> 41
	startpix = (row_nmb - 2) * 7 + 6;

	for (int i = 0; i <= 6; i++) {
		overrideColorPixels(copy_black, startpix + i, matrix[row_nmb][6 - i].r, matrix[row_nmb][6 - i].g, matrix[row_nmb][6 - i].b);
	}
}

/** mapping the matrix to the real layout
*
* @param copy_black The unused pixels
* @return void
* @addtogroup displayMatrix
*/
void displ(bool copy_black) {
	for (int i = 0; i <= 19; i++) {
		if (i % 2 == 0 && i <= 18 && i >= 2) {
			copyRow(i, copy_black);
		}
		else if (i % 2 != 0 && i <= 17 && i >= 3) {
			copyInvRow(i, copy_black);
		}

		else if (i == 0) {
			rowZero(copy_black);
		}

		else if (i == 1) {
			rowOne(copy_black);
		}

		else if (i == 19) {
			lastRow(copy_black);
		}
	}
}
/** @brief
*
* @param copy_black The unused pixels
* @return void
* @addtogroup displayRainbow
*/
void writeRainbowToMatrix(unsigned char iteration) {
	unsigned char wheelPos; ///< variable used to calculate the colors
	char r;        ///< red color value
	char g;        ///< green color value
	char b;        ///< blue color value

	// iterate over all x and y values of the matrix
	for (int x = 0; x < MATRIX_DIMENSION_X; x++) {
		for (int y = 0; y < MATRIX_DIMENSION_Y; y++) {

			//in a previous example wheelPos was used to create a rainbow effect
			//this is the first try to adapt wheelPos because here the layout of the LEDs are differently aligned
			//wheelPos = (iteration + y * MATRIX_DIMENSION_Y + x) & 255;
			//alternative approach - let's try which one looks better
			unsigned char cycleCount = iteration;
			wheelPos = (cycleCount + y + x * MATRIX_DIMENSION_Y) & 255;

			if (wheelPos < 85) {
				r = wheelPos * RAINBOW_MULTIPLIER;
				g = MAX_RAINBOW_COLOR - wheelPos * RAINBOW_MULTIPLIER;
				b = 0;

			}
			else if (wheelPos < 170) {
				wheelPos -= 85;
				r = MAX_RAINBOW_COLOR - wheelPos * RAINBOW_MULTIPLIER;
				g = 0;
				b = wheelPos * RAINBOW_MULTIPLIER;

			}
			else {
				wheelPos -= 170;
				r = 0;
				g = wheelPos * RAINBOW_MULTIPLIER;
				b = MAX_RAINBOW_COLOR - wheelPos * RAINBOW_MULTIPLIER;
			}
			//write the r, g, b values to the matrix according the current x and y value
			matrix[x][y] = { r, g, b };
		}
	}
}

/** @brief
* Color all pixels in the matrix in one Color
*
* @param r The red light intensity from 0-255
* @param g The green light intensity from 0-255
* @param b The blue light intensity from 0-255
* @return void
* @addtogroup application
*/
void fillMatrix(unsigned char r, unsigned char g, unsigned char b) {
	for (int x = 0; x < 20; x++) {
		for (int y = 0; y < 7; y++) {
			matrix[x][y] = { r, g, b };
		}
	}
	displ(true);
	pixels.show();
}

/** @brief
* printing column after column in a color and
* then print row after row in a differen color
* @return void
*
* @addtogroup application
*/
void testMatrix(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2) {
	for (int x = 0; x < 20; x++) {
		for (int y = 0; y < 7; y++) {
			matrix[x][y] = { r1, g1, b1 };
		}
		displ(true);
		pixels.show();
		delay(500);
	}
	delay(1000);

	for (int y = 0; y < 7; y++) {
		for (int x = 0; x < 20; x++) {
			matrix[x][y] = { r2, g2, b2 };
		}
		displ(true);
		pixels.show();
		delay(500);
	}
}

/** @brief
* @param c The character you choose from "italic5x5_font.h"
* @param startColumn The column where the character starts
* @param startBaseline The row where the character starts
* @param r The red light intensity from 0-255
* @param g The green light intensity from 0-255
* @param b The blue light intensity from 0-255
* @return void
*
* @addtogroup displayCharacter
*/
void writeCharToMatrix(char c, unsigned char startColumn, unsigned char startBaseline, char r, char g, char b)
{
	int x;
	int y;
	int linecount;
	unsigned char bitvalue;
	linecount = 0;
	x = startColumn;
	y = startBaseline;
	for (x; x <= startColumn + 4; x++)
	{
		if (x < MATRIX_DIMENSION_X)
		{
			bitvalue = font[c][linecount];
			y = startBaseline;

			for (; y <= MATRIX_DIMENSION_Y; y++)
			{
				if (bitvalue % 2)
				{
					matrix[x][y] = { r, g, b };
				}
				bitvalue = bitvalue >> 1;
			}
			linecount++;
		}
	}
}






/** @brief
* printig the clock time to the matrix with Rainbow background
* @return void
*
* @addtogroup application
*/
void RTCToMatrix(unsigned char runs)
{
	char CR = 255;
	char CG = 255;
	char CB = 255;

	unsigned char hour_tenth;
	unsigned char hour_ones;
	unsigned char min_tenth;
	unsigned char min_ones;

	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++) {


		DS3231_get(&t); //get the value and pass to the function the pointer to t, that make an lower memory fingerprint and faster execution than use return
		//DS3231_get() will use the pointer of t to directly change t value (faster, lower memory used)

		//Clock Time
		hour_tenth = t.hour / 10;
		hour_ones = t.hour % 10;
		min_tenth = t.min / 10;
		min_ones = t.min % 10;

		if (colorIterator % 64 >= 32)
		{
			writeRainbowToMatrix(colorIterator);

			writeCharToMatrix(hour_tenth + _atoz, 2, 1, CR, CG, CB);
			writeCharToMatrix(hour_ones + _atoz, 6, 1, CR, CG, CB);
			writeCharToMatrix(min_tenth + _atoz, 12, 1, CR, CG, CB);
			writeCharToMatrix(min_ones + _atoz, 16, 1, CR, CG, CB);
			writeCharToMatrix(58, 10, 1, CR, CG, CB);
		}
		else
		{
			writeCharToMatrix(58, 10, 1, NULL, NULL, NULL);

			writeRainbowToMatrix(colorIterator);
			writeCharToMatrix(hour_tenth + _atoz, 2, 1, CR, CG, CB);
			writeCharToMatrix(hour_ones + _atoz, 6, 1, CR, CG, CB);
			writeCharToMatrix(min_tenth + _atoz, 12, 1, CR, CG, CB);
			writeCharToMatrix(min_ones + _atoz, 16, 1, CR, CG, CB);
		}
		displ(true);
		pixels.show();
		delay(30);
	}
}





/** @brief
* printig the date to the matrix with Rainbow background
* @return void
*
* @addtogroup application
*/
void datetime(unsigned char runs)
{
	unsigned char CR = 255;
	unsigned char CG = 255;
	unsigned char CB = 255;

	int day_tenth;
	int day_ones;
	int month_tenth;
	int month_ones;

	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++) {

		DS3231_get(&t); //get the value and pass to the function the pointer to t, that make an lower memory fingerprint and faster execution than use return
		//DS3231_get() will use the pointer of t to directly change t value (faster, lower memory used)
		//Date Time
		day_tenth = t.mday / 10;
		day_ones = t.mday % 10;
		month_tenth = t.mon / 10;
		month_ones = t.mon % 10;


		if (colorIterator % 64 >= 32)
		{
			writeRainbowToMatrix(colorIterator);

			writeCharToMatrix(day_tenth + _atoz, 2, 1, CR, CG, CB);
			writeCharToMatrix(day_ones + _atoz, 6, 1, CR, CG, CB);
			writeCharToMatrix(month_tenth + _atoz, 11, 1, CR, CG, CB);
			writeCharToMatrix(month_ones + _atoz, 15, 1, CR, CG, CB);
			writeCharToMatrix(46, 9, 1, CR, CG, CB);
			writeCharToMatrix(46, 18, 1, CR, CG, CB);
		}
		else
		{
			writeCharToMatrix(46, 9, 1, NULL, NULL, NULL);
			writeCharToMatrix(46, 18, 1, NULL, NULL, NULL);

			writeRainbowToMatrix(colorIterator);
			writeCharToMatrix(day_tenth + _atoz, 2, 1, CR, CG, CB);
			writeCharToMatrix(day_ones + _atoz, 6, 1, CR, CG, CB);
			writeCharToMatrix(month_tenth + _atoz, 11, 1, CR, CG, CB);
			writeCharToMatrix(month_ones + _atoz, 15, 1, CR, CG, CB);
		}

		displ(true);
		pixels.show();
		delay(30);

	}
}

void showYear(unsigned char runs)
{
	unsigned char CR = 255;
	unsigned char CG = 255;
	unsigned char CB = 255;

	int year_tenth;
	int year_ones;
	int year_hun;
	int month_thou;

	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++) {

		DS3231_get(&t); //get the value and pass to the function the pointer to t, that make an lower memory fingerprint and faster execution than use return
		//DS3231_get() will use the pointer of t to directly change t value (faster, lower memory used)
		//Date Time
		month_thou = t.year / 1000;
		year_hun = (t.year % 1000) / 100;
		year_ones = t.year % 10;
		year_tenth = (t.year % 100) / 10;

		writeRainbowToMatrix(colorIterator);

		writeCharToMatrix(month_thou + _atoz, 3, 1, CR, CG, CB);
		writeCharToMatrix(year_hun + _atoz, 7, 1, CR, CG, CB);
		writeCharToMatrix(year_tenth + _atoz, 11, 1, CR, CG, CB);
		writeCharToMatrix(year_ones + _atoz, 15, 1, CR, CG, CB);

		displ(true);
		pixels.show();
		delay(30);
	}
}

void weekDays(unsigned char runs)
{

	int i;
	unsigned char CR = 255;
	unsigned char CG = 255;
	unsigned char CB = 255;
	int week_day;
	week_day = t.wday;


	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++) {

		DS3231_get(&t); //get the value and pass to the function the pointer to t, that make an lower memory fingerprint and faster execution than use return
		//DS3231_get() will use the pointer of t to directly change t value (faster, lower memory used)
		//Date Time

		writeRainbowToMatrix(colorIterator);
		for (i = 0; i <= 2; i++)
		{
			//printf("%d - ", wday[6][i]);
			if (i == 0)
				writeCharToMatrix(wday[week_day][i], 3, 1, CR, CG, CB);
			if (i == 1)
				writeCharToMatrix(wday[week_day][i], 7, 1, CR, CG, CB);
			if (i == 2)
				writeCharToMatrix(wday[week_day][i], 12, 1, CR, CG, CB);
		}
		displ(true);
		pixels.show();
		delay(30);
	}
}

/** @brief
* printig the humidity time to the matrix with Rainbow background
* @return void
*
* @addtogroup application
*/
void humidity(unsigned char runs)
{
	unsigned char CR = 255;
	unsigned char CG = 255;
	unsigned char CB = 255;

	int hum_tenth;
	int hum_ones;

	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++) {
		// Get temperature event and print its value.
		sensors_event_t event;

		writeRainbowToMatrix(colorIterator);

		dht.humidity().getEvent(&event);
		//humidity Digits
		if (isnan(event.relative_humidity)) {}
		else {
			hum_tenth = (int)event.relative_humidity / 10;
			hum_ones = (int)event.relative_humidity % 10;
			//delay(delayMS);
			writeRainbowToMatrix(colorIterator);
			writeCharToMatrix(hum_tenth + _atoz, 4, 1, CR, CG, CB);
			writeCharToMatrix(hum_ones + _atoz, 8, 1, CR, CG, CB);
			writeCharToMatrix('%', 13, 1, CR, CG, CB);
		}
		displ(true);
		pixels.show();
		delay(30);
	}

}
/** @brief
* printig the temperatures to the matrix with Rainbow background
* @return void
*
* @addtogroup application
*/
void printTemperature(unsigned char runs)
{
	unsigned char CR = 255;
	unsigned char CG = 255;
	unsigned char CB = 255;

	int temp_tenth;
	int temp_ones;

	for (int colorIterator = 0; colorIterator < 256 * runs; colorIterator++)
	{
		//delay(delayMS);

		// Get temperature event and print its value.
		sensors_event_t event;

		writeRainbowToMatrix(colorIterator);
		dht.temperature().getEvent(&event);
		if (isnan(event.temperature)) {}
		else {
			temp_tenth = (int)event.temperature / 10;
			temp_ones = (int)event.temperature % 10;
			writeCharToMatrix(temp_tenth + _atoz, 2, 1, CR, CG, CB);
			writeCharToMatrix(temp_ones + _atoz, 6, 1, CR, CG, CB);
			writeCharToMatrix(128, 10, 1, CR, CG, CB);
			writeCharToMatrix('C', 13, 1, CR, CG, CB);
		}
		displ(true);
		pixels.show();
		delay(30);
	}
}

/** @brief
* printig a text which moves from right to left
* @return void
*
* @addtogroup application
*/
void shiftTextV1()
{
	unsigned char x;

	x = 15;
	while (x + 14 >= 1)
	{
		writeCharToMatrix(1, x + 1, 1, 255, 255, 255);
		writeCharToMatrix(0, x + 5, 1, 255, 255, 255);
		writeCharToMatrix(20, x + 9, 1, 255, 255, 255);
		writeCharToMatrix(12, x + 14, 1, 255, 255, 255);

		x--;
		displ(true);
		pixels.setBrightness(60);
		pixels.show();
		delay(200);
		writeCharToMatrix(1, x + 2, 1, NULL, NULL, NULL);
		writeCharToMatrix(0, x + 6, 1, NULL, NULL, NULL);
		writeCharToMatrix(20, x + 10, 1, NULL, NULL, NULL);
		writeCharToMatrix(12, x + 15, 1, NULL, NULL, NULL);
	}
}

/** @brief
* printig a text which moves from right to left
* @return void
*
* @addtogroup application
*/
void shiftTextV2(char CAPSLK_text[])
{
	int length;

	length = strlen(CAPSLK_text);

	for (int j = 0; j < length; j++) {
		fillMatrix(0, 120, 20);
		for (int i = 0; i < 5 && i + j < length; i++) {
			writeCharToMatrix(CAPSLK_text[i + j], (i * 5), 1, 150, 150, 150);
		}
		displ(true);
		pixels.show();
		delay(1000);
	}

}


/** @brief
* startup function
* @retrun void
* @addtogroup mainFunction
*
*/
void setup() {

	sensor_t sensor;
	dht.temperature().getSensor(&sensor);
	dht.humidity().getSensor(&sensor);

	// END of Trinket-specific code.
	Wire.begin(); //start i2c (required for connection)
	DS3231_init(DS3231_INTCN); //register the ds3231 (DS3231_INTCN is the default address of ds3231, this is set by macro for no performance loss)

	pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)

	dht.begin();

	Serial.begin(115200);


	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	timeClient.begin();
	timeClient.forceUpdate();
	timeClient.update();
	t.hour = (uint8_t)timeClient.getHours();
	t.min = (uint8_t)timeClient.getMinutes();
	t.sec = (uint8_t)timeClient.getSeconds();
	t.mon = (uint8_t)timeClient.getMonth();
	t.mday = (uint8_t)timeClient.getDate();
	t.year = (uint8_t)timeClient.getYear();
	t.wday = (uint8_t)timeClient.getDay();
	DS3231_set(t);

	delayMS = sensor.min_delay / 1000;
	timeClient.end();
}


/** @brief
*  loop function which is used after the starup function
* @return void
* @addtogroup mainFunction
*
*/
void loop() {
	 pixels.clear(); // Set all pixel colors to 'off'

	// The first NeoPixel in a strand is #0, second is 1, all the way up
	// to the count of pixels minus one.

	//dateAndClock();
	//fillMatrix(100, 0, 10); delay(3000);
	//testMatrix(0, 50, 100, 10, 100, 10);
	unsigned char runs = 1;

	//shiftTextV2("Hallo Bulme �");
	printTemperature(runs);
	RTCToMatrix(runs);
	weekDays(runs);
	datetime(runs);
	showYear(runs);
	humidity(runs);

	

	/*for (int i = 0; i < 256; i++)
	{
		for (int p = 0; p < numpixels; p++)
		{
			pixels.setpixelcolor(p, i + p & 255, 255 - i + p & 255, 0);
			pixels.show();

		}

	}
	for (int i = 0; i < 256; i++)
	{
		for (int p = 0; p < numpixels; p++)
		{
			pixels.setpixelcolor(p, 255 - i + p & 255, 0, i + p & 255);
			pixels.show();

		}


	}
	for (int i = 0; i < 256; i++)
	{
		for (int p = 0; p < numpixels; p++)
		{
			pixels.setpixelcolor(p, 0, i + p & 255, 255 - i + p & 255);
			pixels.show();

		}

	}*/


}