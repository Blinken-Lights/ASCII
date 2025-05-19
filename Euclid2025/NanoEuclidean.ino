//#include <U8x8lib.h>
#include <U8g2lib.h>
#include <SPI.h>

U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

byte switchPins[] = { 0, A5, 12 };
bool switchStates[] = { false, false, false };
unsigned long switchLastPressed = 0;
int switchDebounce = 125;

byte gateOutPins[] = { A2, A4, 3, 4, A3, 7, 6, 5 };

byte clockInPin = 2;

byte analoguePins[] = { A0, A1, A7, A6 };
int analogueReadings[] = { -1, -1, -1, -1 };
int jitterThreshold = 14;
byte readAnaloguePin = 0;
unsigned long knobLastChanged = 0;
int showNumbersFor = 2000;

byte selectedChannel = 0;
byte euclideanParameters[8][3] = {
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 },
  { 0, 0, 0 }
};
byte channelBeats[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// for printing numbers
enum {BufSize=9};
char buf[BufSize];

// for testing
byte switchPresses[] = { 0, 0, 0 };
bool testGateOuts = false;

const static byte euclid[33][33][4] PROGMEM = {
	{
		{0,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{3,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{3,	0,	0,	0,	},
		{7,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{5,	0,	0,	0,	},
		{7,	0,	0,	0,	},
		{15,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{5,	0,	0,	0,	},
		{21,	0,	0,	0,	},
		{15,	0,	0,	0,	},
		{31,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{9,	0,	0,	0,	},
		{21,	0,	0,	0,	},
		{45,	0,	0,	0,	},
		{31,	0,	0,	0,	},
		{63,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{9,	0,	0,	0,	},
		{21,	0,	0,	0,	},
		{85,	0,	0,	0,	},
		{109,	0,	0,	0,	},
		{63,	0,	0,	0,	},
		{127,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{17,	0,	0,	0,	},
		{73,	0,	0,	0,	},
		{85,	0,	0,	0,	},
		{109,	0,	0,	0,	},
		{221,	0,	0,	0,	},
		{127,	0,	0,	0,	},
		{255,	0,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{17,	0,	0,	0,	},
		{73,	0,	0,	0,	},
		{85,	0,	0,	0,	},
		{85,	1,	0,	0,	},
		{109,	1,	0,	0,	},
		{221,	1,	0,	0,	},
		{255,	0,	0,	0,	},
		{255,	1,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{33,	0,	0,	0,	},
		{73,	0,	0,	0,	},
		{41,	1,	0,	0,	},
		{85,	1,	0,	0,	},
		{173,	1,	0,	0,	},
		{109,	3,	0,	0,	},
		{189,	3,	0,	0,	},
		{255,	1,	0,	0,	},
		{255,	3,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{33,	0,	0,	0,	},
		{17,	1,	0,	0,	},
		{73,	2,	0,	0,	},
		{85,	1,	0,	0,	},
		{85,	5,	0,	0,	},
		{109,	3,	0,	0,	},
		{221,	5,	0,	0,	},
		{189,	7,	0,	0,	},
		{255,	3,	0,	0,	},
		{255,	7,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{65,	0,	0,	0,	},
		{17,	1,	0,	0,	},
		{73,	2,	0,	0,	},
		{41,	5,	0,	0,	},
		{85,	5,	0,	0,	},
		{173,	5,	0,	0,	},
		{109,	11,	0,	0,	},
		{221,	13,	0,	0,	},
		{125,	15,	0,	0,	},
		{255,	7,	0,	0,	},
		{255,	15,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{65,	0,	0,	0,	},
		{17,	1,	0,	0,	},
		{73,	2,	0,	0,	},
		{41,	5,	0,	0,	},
		{85,	5,	0,	0,	},
		{85,	21,	0,	0,	},
		{173,	21,	0,	0,	},
		{109,	27,	0,	0,	},
		{221,	29,	0,	0,	},
		{125,	31,	0,	0,	},
		{255,	15,	0,	0,	},
		{255,	31,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{129,	0,	0,	0,	},
		{33,	4,	0,	0,	},
		{145,	8,	0,	0,	},
		{73,	18,	0,	0,	},
		{169,	20,	0,	0,	},
		{85,	21,	0,	0,	},
		{173,	22,	0,	0,	},
		{109,	27,	0,	0,	},
		{221,	46,	0,	0,	},
		{189,	55,	0,	0,	},
		{253,	62,	0,	0,	},
		{255,	31,	0,	0,	},
		{255,	63,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{129,	0,	0,	0,	},
		{33,	4,	0,	0,	},
		{17,	17,	0,	0,	},
		{73,	18,	0,	0,	},
		{41,	37,	0,	0,	},
		{85,	21,	0,	0,	},
		{85,	85,	0,	0,	},
		{173,	53,	0,	0,	},
		{109,	91,	0,	0,	},
		{221,	93,	0,	0,	},
		{189,	119,	0,	0,	},
		{253,	126,	0,	0,	},
		{255,	63,	0,	0,	},
		{255,	127,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	1,	0,	0,	},
		{33,	4,	0,	0,	},
		{17,	17,	0,	0,	},
		{73,	18,	0,	0,	},
		{41,	41,	0,	0,	},
		{169,	84,	0,	0,	},
		{85,	85,	0,	0,	},
		{173,	86,	0,	0,	},
		{173,	173,	0,	0,	},
		{109,	219,	0,	0,	},
		{221,	221,	0,	0,	},
		{189,	247,	0,	0,	},
		{253,	253,	0,	0,	},
		{255,	127,	0,	0,	},
		{255,	255,	0,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	1,	0,	0,	},
		{65,	16,	0,	0,	},
		{17,	17,	0,	0,	},
		{145,	72,	0,	0,	},
		{73,	146,	0,	0,	},
		{41,	165,	0,	0,	},
		{85,	85,	0,	0,	},
		{85,	85,	1,	0,	},
		{173,	181,	0,	0,	},
		{109,	219,	0,	0,	},
		{221,	110,	1,	0,	},
		{221,	221,	1,	0,	},
		{125,	223,	1,	0,	},
		{253,	253,	1,	0,	},
		{255,	255,	0,	0,	},
		{255,	255,	1,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	2,	0,	0,	},
		{65,	16,	0,	0,	},
		{33,	66,	0,	0,	},
		{145,	72,	0,	0,	},
		{73,	146,	0,	0,	},
		{41,	165,	0,	0,	},
		{169,	82,	1,	0,	},
		{85,	85,	1,	0,	},
		{173,	90,	1,	0,	},
		{173,	181,	2,	0,	},
		{109,	219,	2,	0,	},
		{221,	110,	3,	0,	},
		{189,	123,	3,	0,	},
		{125,	223,	3,	0,	},
		{253,	251,	3,	0,	},
		{255,	255,	1,	0,	},
		{255,	255,	3,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	2,	0,	0,	},
		{65,	16,	0,	0,	},
		{33,	132,	0,	0,	},
		{17,	17,	1,	0,	},
		{73,	146,	0,	0,	},
		{41,	41,	1,	0,	},
		{169,	84,	2,	0,	},
		{85,	85,	1,	0,	},
		{85,	85,	5,	0,	},
		{173,	86,	3,	0,	},
		{173,	173,	5,	0,	},
		{109,	219,	6,	0,	},
		{221,	221,	5,	0,	},
		{189,	247,	6,	0,	},
		{125,	223,	7,	0,	},
		{253,	251,	7,	0,	},
		{255,	255,	3,	0,	},
		{255,	255,	7,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	4,	0,	0,	},
		{129,	64,	0,	0,	},
		{33,	132,	0,	0,	},
		{17,	17,	1,	0,	},
		{145,	68,	2,	0,	},
		{73,	146,	4,	0,	},
		{41,	165,	4,	0,	},
		{169,	82,	5,	0,	},
		{85,	85,	5,	0,	},
		{173,	90,	5,	0,	},
		{173,	181,	6,	0,	},
		{109,	219,	6,	0,	},
		{221,	118,	11,	0,	},
		{221,	221,	13,	0,	},
		{189,	247,	14,	0,	},
		{253,	126,	15,	0,	},
		{253,	247,	15,	0,	},
		{255,	255,	7,	0,	},
		{255,	255,	15,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	4,	0,	0,	},
		{129,	64,	0,	0,	},
		{33,	132,	0,	0,	},
		{17,	17,	1,	0,	},
		{145,	72,	4,	0,	},
		{73,	146,	4,	0,	},
		{41,	41,	9,	0,	},
		{169,	84,	10,	0,	},
		{85,	85,	5,	0,	},
		{85,	85,	21,	0,	},
		{173,	86,	11,	0,	},
		{173,	173,	13,	0,	},
		{109,	219,	22,	0,	},
		{221,	110,	23,	0,	},
		{221,	221,	29,	0,	},
		{189,	247,	30,	0,	},
		{253,	126,	31,	0,	},
		{253,	247,	31,	0,	},
		{255,	255,	15,	0,	},
		{255,	255,	31,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	8,	0,	0,	},
		{129,	64,	0,	0,	},
		{65,	8,	2,	0,	},
		{33,	66,	4,	0,	},
		{145,	136,	4,	0,	},
		{73,	146,	4,	0,	},
		{41,	73,	9,	0,	},
		{41,	165,	20,	0,	},
		{169,	74,	21,	0,	},
		{85,	85,	21,	0,	},
		{173,	106,	21,	0,	},
		{173,	181,	22,	0,	},
		{173,	109,	45,	0,	},
		{109,	219,	54,	0,	},
		{221,	238,	54,	0,	},
		{189,	123,	55,	0,	},
		{125,	239,	59,	0,	},
		{253,	126,	63,	0,	},
		{253,	239,	63,	0,	},
		{255,	255,	31,	0,	},
		{255,	255,	63,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	8,	0,	0,	},
		{1,	1,	1,	0,	},
		{65,	16,	4,	0,	},
		{33,	66,	4,	0,	},
		{17,	17,	17,	0,	},
		{145,	68,	18,	0,	},
		{73,	146,	36,	0,	},
		{41,	165,	20,	0,	},
		{169,	84,	42,	0,	},
		{85,	85,	21,	0,	},
		{85,	85,	85,	0,	},
		{173,	86,	43,	0,	},
		{173,	181,	86,	0,	},
		{109,	219,	54,	0,	},
		{221,	118,	91,	0,	},
		{221,	221,	93,	0,	},
		{189,	123,	119,	0,	},
		{125,	223,	119,	0,	},
		{253,	253,	125,	0,	},
		{253,	239,	127,	0,	},
		{255,	255,	63,	0,	},
		{255,	255,	127,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	16,	0,	0,	},
		{1,	1,	1,	0,	},
		{65,	16,	4,	0,	},
		{33,	132,	16,	0,	},
		{17,	17,	17,	0,	},
		{145,	72,	36,	0,	},
		{73,	146,	36,	0,	},
		{41,	41,	41,	0,	},
		{169,	148,	74,	0,	},
		{169,	74,	85,	0,	},
		{85,	85,	85,	0,	},
		{173,	106,	85,	0,	},
		{173,	214,	106,	0,	},
		{173,	173,	173,	0,	},
		{109,	219,	182,	0,	},
		{221,	110,	183,	0,	},
		{221,	221,	221,	0,	},
		{189,	247,	222,	0,	},
		{125,	223,	247,	0,	},
		{253,	253,	253,	0,	},
		{253,	223,	255,	0,	},
		{255,	255,	127,	0,	},
		{255,	255,	255,	0,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	16,	0,	0,	},
		{1,	1,	1,	0,	},
		{65,	16,	4,	0,	},
		{33,	132,	16,	0,	},
		{17,	17,	17,	0,	},
		{145,	72,	36,	0,	},
		{73,	146,	36,	0,	},
		{41,	73,	73,	0,	},
		{41,	165,	148,	0,	},
		{169,	82,	165,	0,	},
		{85,	85,	85,	0,	},
		{85,	85,	85,	1,	},
		{173,	90,	181,	0,	},
		{173,	181,	214,	0,	},
		{173,	109,	109,	1,	},
		{109,	219,	182,	1,	},
		{221,	110,	183,	1,	},
		{221,	221,	221,	1,	},
		{189,	247,	222,	1,	},
		{125,	223,	247,	1,	},
		{253,	253,	253,	1,	},
		{253,	223,	255,	1,	},
		{255,	255,	255,	0,	},
		{255,	255,	255,	1,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	32,	0,	0,	},
		{1,	2,	4,	0,	},
		{129,	32,	16,	0,	},
		{33,	132,	16,	0,	},
		{33,	34,	68,	0,	},
		{145,	136,	68,	0,	},
		{145,	36,	146,	0,	},
		{73,	146,	36,	1,	},
		{41,	41,	37,	1,	},
		{169,	84,	42,	1,	},
		{169,	42,	85,	1,	},
		{85,	85,	85,	1,	},
		{173,	170,	85,	1,	},
		{173,	86,	171,	1,	},
		{173,	173,	181,	1,	},
		{109,	219,	182,	1,	},
		{221,	182,	219,	2,	},
		{221,	238,	118,	3,	},
		{189,	187,	119,	3,	},
		{189,	247,	222,	3,	},
		{253,	190,	223,	3,	},
		{253,	251,	247,	3,	},
		{253,	191,	255,	3,	},
		{255,	255,	255,	1,	},
		{255,	255,	255,	3,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	32,	0,	0,	},
		{1,	2,	4,	0,	},
		{129,	64,	32,	0,	},
		{65,	8,	66,	0,	},
		{33,	66,	132,	0,	},
		{17,	17,	17,	1,	},
		{145,	68,	18,	1,	},
		{73,	146,	36,	1,	},
		{41,	41,	41,	1,	},
		{41,	165,	148,	2,	},
		{169,	82,	165,	2,	},
		{85,	85,	85,	1,	},
		{85,	85,	85,	5,	},
		{173,	90,	181,	2,	},
		{173,	181,	214,	2,	},
		{173,	173,	173,	5,	},
		{109,	219,	182,	5,	},
		{221,	118,	219,	5,	},
		{221,	221,	221,	5,	},
		{189,	123,	247,	6,	},
		{125,	239,	123,	7,	},
		{253,	126,	191,	7,	},
		{253,	251,	247,	7,	},
		{253,	191,	255,	7,	},
		{255,	255,	255,	3,	},
		{255,	255,	255,	7,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	64,	0,	0,	},
		{1,	2,	4,	0,	},
		{129,	64,	32,	0,	},
		{65,	8,	66,	0,	},
		{33,	66,	136,	0,	},
		{17,	17,	17,	1,	},
		{145,	72,	36,	2,	},
		{73,	146,	36,	1,	},
		{41,	73,	74,	2,	},
		{41,	165,	148,	2,	},
		{169,	84,	42,	5,	},
		{169,	42,	85,	5,	},
		{85,	85,	85,	5,	},
		{173,	170,	85,	5,	},
		{173,	86,	171,	5,	},
		{173,	181,	214,	10,	},
		{173,	109,	107,	11,	},
		{109,	219,	182,	13,	},
		{221,	110,	183,	11,	},
		{221,	221,	221,	13,	},
		{189,	123,	239,	14,	},
		{125,	239,	123,	15,	},
		{253,	126,	191,	15,	},
		{253,	251,	247,	15,	},
		{253,	127,	255,	15,	},
		{255,	255,	255,	7,	},
		{255,	255,	255,	15,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	64,	0,	0,	},
		{1,	4,	16,	0,	},
		{129,	64,	32,	0,	},
		{65,	16,	4,	1,	},
		{33,	132,	16,	2,	},
		{17,	17,	17,	1,	},
		{145,	136,	68,	4,	},
		{145,	36,	146,	4,	},
		{73,	146,	36,	9,	},
		{41,	41,	41,	9,	},
		{169,	148,	74,	9,	},
		{169,	82,	165,	10,	},
		{85,	85,	85,	5,	},
		{85,	85,	85,	21,	},
		{173,	90,	181,	10,	},
		{173,	214,	106,	13,	},
		{173,	173,	173,	13,	},
		{109,	219,	182,	13,	},
		{221,	182,	219,	22,	},
		{221,	238,	118,	23,	},
		{221,	221,	221,	29,	},
		{189,	247,	222,	27,	},
		{125,	223,	247,	29,	},
		{253,	126,	191,	31,	},
		{253,	247,	223,	31,	},
		{253,	127,	255,	31,	},
		{255,	255,	255,	15,	},
		{255,	255,	255,	31,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	128,	0,	0,	},
		{1,	4,	16,	0,	},
		{1,	129,	128,	0,	},
		{65,	16,	4,	1,	},
		{33,	132,	16,	2,	},
		{33,	34,	68,	4,	},
		{145,	136,	72,	4,	},
		{145,	68,	18,	9,	},
		{73,	146,	36,	9,	},
		{41,	73,	73,	10,	},
		{41,	165,	148,	18,	},
		{169,	84,	42,	21,	},
		{169,	170,	84,	21,	},
		{85,	85,	85,	21,	},
		{173,	170,	86,	21,	},
		{173,	86,	171,	21,	},
		{173,	181,	214,	26,	},
		{173,	109,	109,	43,	},
		{109,	219,	182,	45,	},
		{221,	118,	219,	45,	},
		{221,	238,	110,	55,	},
		{189,	187,	119,	55,	},
		{189,	247,	222,	59,	},
		{125,	223,	247,	61,	},
		{253,	253,	254,	62,	},
		{253,	247,	223,	63,	},
		{253,	255,	254,	63,	},
		{255,	255,	255,	31,	},
		{255,	255,	255,	63,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	128,	0,	0,	},
		{1,	4,	16,	0,	},
		{1,	1,	1,	1,	},
		{65,	16,	4,	1,	},
		{33,	132,	16,	2,	},
		{33,	66,	132,	8,	},
		{17,	17,	17,	17,	},
		{145,	72,	36,	18,	},
		{73,	146,	36,	9,	},
		{41,	73,	74,	18,	},
		{41,	41,	37,	37,	},
		{169,	148,	74,	41,	},
		{169,	74,	85,	42,	},
		{85,	85,	85,	21,	},
		{85,	85,	85,	85,	},
		{173,	106,	85,	43,	},
		{173,	214,	106,	45,	},
		{173,	173,	181,	53,	},
		{173,	109,	107,	91,	},
		{109,	219,	182,	109,	},
		{221,	110,	183,	91,	},
		{221,	221,	221,	93,	},
		{189,	123,	247,	110,	},
		{189,	247,	222,	123,	},
		{125,	223,	247,	125,	},
		{253,	253,	253,	125,	},
		{253,	247,	223,	127,	},
		{253,	255,	254,	127,	},
		{255,	255,	255,	63,	},
		{255,	255,	255,	127,	},
	},
	{
		{0,	0,	0,	0,	},
		{1,	0,	0,	0,	},
		{1,	0,	1,	0,	},
		{1,	8,	64,	0,	},
		{1,	1,	1,	1,	},
		{129,	32,	16,	4,	},
		{65,	8,	65,	8,	},
		{33,	66,	132,	8,	},
		{17,	17,	17,	17,	},
		{145,	72,	36,	18,	},
		{145,	36,	145,	36,	},
		{73,	146,	36,	73,	},
		{41,	41,	41,	41,	},
		{41,	165,	148,	82,	},
		{169,	82,	169,	82,	},
		{169,	170,	84,	85,	},
		{85,	85,	85,	85,	},
		{173,	170,	86,	85,	},
		{173,	90,	173,	90,	},
		{173,	181,	214,	90,	},
		{173,	173,	173,	173,	},
		{109,	219,	182,	109,	},
		{221,	182,	221,	182,	},
		{221,	110,	183,	219,	},
		{221,	221,	221,	221,	},
		{189,	123,	247,	238,	},
		{125,	239,	125,	239,	},
		{253,	190,	223,	247,	},
		{253,	253,	253,	253,	},
		{253,	239,	127,	255,	},
		{253,	255,	253,	255,	},
		{255,	255,	255,	127,	},
		{255,	255,	255,	255,	},
	},
};

const static float knobScales[33] = {
	0.000000f,
	0.000978f,
	0.001955f,
	0.002933f,
	0.003910f,
	0.004888f,
	0.005865f,
	0.006843f,
	0.007820f,
	0.008798f,
	0.009775f,
	0.010753f,
	0.011730f,
	0.012708f,
	0.013685f,
	0.014663f,
	0.015640f,
	0.016618f,
	0.017595f,
	0.018573f,
	0.019550f,
	0.020528f,
	0.021505f,
	0.022483f,
	0.023460f,
	0.024438f,
	0.025415f,
	0.026393f,
	0.027370f,
	0.028348f,
	0.029326f,
	0.030303f,
	0.031281f,
};

void setup() {
  randomSeed(analogRead(A5));

  u8g2.begin();

  Serial.begin(250000);

  for(int i = 0; i < 3; i++){
    pinMode(switchPins[i], INPUT_PULLUP);
  }

  for(int i = 0; i < 8; i++){
    pinMode(gateOutPins[i], OUTPUT);
  }

  pinMode(clockInPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(clockInPin), checkClock, CHANGE);

  for(int i = 0; i < 4; i++){
    analogueReadings[i] = analogRead(analoguePins[i]);
  }
  // gather values before calculating pattern, because all values
  // need to be known for the calculations to be correct
  for(int i = 0; i < 4; i++){
    analogueChanged(i);
  }

  randomiseParameters();
}

void loop() {
  redrawDisplay();

  int analogueReading = analogRead(analoguePins[readAnaloguePin]);
  if(abs(analogueReading - analogueReadings[readAnaloguePin]) > jitterThreshold){
    analogueReadings[readAnaloguePin] = analogueReading;
    analogueChanged(readAnaloguePin);
  }
  readAnaloguePin++;
  if(readAnaloguePin >= 4){
    readAnaloguePin = 0;
  }

  /*for(int i = 0; i < 4; i++){
    Serial.print(analogueReadings[i] >> 2);
    Serial.print("\t");
  }
  Serial.println();*/

  for(int i = 0; i < 3; i++){
    bool currentSwitchState = !opt_read(switchPins[i]);
    if(currentSwitchState && !switchStates[i] && millis() - switchLastPressed > switchDebounce){
      switchPressed(i);
      switchLastPressed = millis();
    }
    switchStates[i] = currentSwitchState;
  }
}

void checkClock(){
  bool clock = opt_read(clockInPin);
  if(clock){
    clockRise();
  }else{
    clockFall();
  }
}

void clockRise(){
  //Serial.println("clock rise");

  for(int i = 0; i < 8; i++){
    if(channelBeats[i] >= euclideanParameters[i][0]){
      channelBeats[i] = 0;
    }
    opt_write(gateOutPins[i], switchStates[1] || getEuclideanBeatForChannel(channelBeats[i], i));
    channelBeats[i]++;
  }
}

void clockFall(){
  for(int i = 0; i < 8; i++){
    opt_write(gateOutPins[i], LOW);
  }
}

void switchPressed(int switchNumber){
  switchPresses[switchNumber]++;

  if(switchNumber == 0){
    for(int i = 0; i < 8; i++){
      channelBeats[i] = 0;
    }
  }

  //Serial.print("switch pressed: ");
  //Serial.println(switchNumber);
}

void analogueChanged(int pinNumber){

  knobLastChanged = millis();

  if(pinNumber == 0){ // length
    if(selectedChannel <= 7){
      euclideanParameters[selectedChannel][0] = sanitiseKnobValue(round((float)analogueReadings[0] * 0.0312805474f));
    }else{
      //all
      for(int i = 0; i < 8; i++){
        euclideanParameters[i][0] = sanitiseKnobValue(round((float)analogueReadings[0] * 0.0312805474f));
      }
    }
  }else if(pinNumber == 1){ // events
    if(selectedChannel <= 7){
      euclideanParameters[selectedChannel][1] = sanitiseKnobValue(round((float)analogueReadings[1] * knobScales[euclideanParameters[selectedChannel][0]]));
    }else{
      //all
      for(int i = 0; i < 8; i++){
        euclideanParameters[i][1] = sanitiseKnobValue(round((float)analogueReadings[1] * knobScales[32]));
      }
    }
  }else if(pinNumber == 3){ // offset
    if(selectedChannel <= 7){
      euclideanParameters[selectedChannel][2] = sanitiseKnobValue(round((float)analogueReadings[3] * knobScales[euclideanParameters[selectedChannel][0]]));
    }else{
      //all
      for(int i = 0; i < 8; i++){
        euclideanParameters[i][2] = sanitiseKnobValue(round((float)analogueReadings[3] * knobScales[32]));
      }
    }
  }else if(pinNumber == 2){ // channel selection
    selectedChannel = round((float)analogueReadings[2] * 0.00782013685f);
  }

}

int sanitiseKnobValue(int scaledKnobValue){
  if(!switchStates[2] || scaledKnobValue == 0){
    return scaledKnobValue;
  }
  for(int i = 0; i < 8; i++){
    byte powerOfTwo = (1 << i);
    if(scaledKnobValue <= powerOfTwo){
      return powerOfTwo;
    }
  }
}

bool getEuclideanBeatForChannel(byte beat, byte channel){
  return getEuclideanBeat(beat, euclideanParameters[channel][0], euclideanParameters[channel][1], euclideanParameters[channel][2]);
}

bool getEuclideanBeat(byte beat, byte length, byte events, byte offset){

  if(events > length){
    events = length;
  }

  byte offsetBeat = (beat + offset) % length;
  byte byteAddress = (offsetBeat >> 3);
  byte bitAddress = offsetBeat % 8;

  byte addressedByte = pgm_read_byte(&euclid[length][events][byteAddress]);

  return bitRead(addressedByte, bitAddress);
}

void redrawDisplay(){
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tn);
    for(int i = 0; i < 8; i++){
      u8g2.drawLine(euclideanParameters[i][0] * 4, (i * 8), euclideanParameters[i][0] * 4, (i * 8) + 8);
      for(int j = 0; j < 32; j++){
        if(j >= euclideanParameters[i][0]){
          break;
        }
        if(getEuclideanBeatForChannel(j, i)){
          u8g2.drawPixel((j * 4) + 2, (i * 8) + 4);
        }
        if(j == channelBeats[i] && getEuclideanBeatForChannel(j, i)){
          u8g2.drawPixel((j * 4) + 2, (i * 8) + 5);
        }
        if(j == channelBeats[i]){
          u8g2.drawPixel((j * 4) + 2, (i * 8) + 6);
        }
      }
    }
    u8g2.drawLine(0, (selectedChannel * 8), 0, (selectedChannel * 8) + 8);

    if(millis() - knobLastChanged < showNumbersFor){
      int showSelectedChannel = selectedChannel;
      int numberLine = 64;
      if(selectedChannel > 5){
        numberLine = 8;
      }
      if(selectedChannel == 8){
        showSelectedChannel = 7;
      }else{
        itoa(selectedChannel, buf, 10);
        u8g2.drawStr(0, numberLine, buf);
      }
      itoa(euclideanParameters[showSelectedChannel][0], buf, 10);
      u8g2.drawStr(30, numberLine, buf);
      itoa(euclideanParameters[showSelectedChannel][1], buf, 10);
      u8g2.drawStr(60, numberLine, buf);
      itoa(euclideanParameters[showSelectedChannel][2], buf, 10);
      u8g2.drawStr(90, numberLine, buf);
    }
  } while ( u8g2.nextPage() );
}

void randomiseParameters(){

  for(int i = 0; i < 8; i++){

    int length = random(33);
    int events = random(length + 1);
    int offset = random(length + 1);

    euclideanParameters[i][0] = length;
    euclideanParameters[i][1] = events;
    euclideanParameters[i][2] = offset;

  }

}

bool opt_read(byte pin) {
  if (pin >= 14) {
    pin -= 14;
    return bitRead(PINC, pin);
  } else if (pin >= 8) {
    pin -= 8;
    return bitRead(PINB, pin);
  } else {
    return bitRead(PIND, pin);
  }
}

void opt_write(byte pin, bool val) {
  if (pin >= 14) {
    pin -= 14;
    bitWrite(PORTC, pin, val);
  } else if (pin >= 8) {
    pin -= 8;
    bitWrite(PORTB, pin, val);
  } else {
    bitWrite(PORTD, pin, val);
  }
}

void opt_mode(byte pin, byte val) {
  if (val == 2) {
    opt_write(pin, HIGH);
    val = 0;
  }
  if (pin >= 14) {
    pin -= 14;
    bitWrite(DDRC, pin, val);
  } else if (pin >= 8) {
    pin -= 8;
    bitWrite(DDRB, pin, val);
  } else {
    bitWrite(DDRD, pin, val);
  }
}
