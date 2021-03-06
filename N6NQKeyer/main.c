/*
 * N6NQKeyer.c
 *
 * Created: 7/20/2016 7:51:53 PM
 * Author : n6nq
 */ 

#include <avr/io.h>
#include "Sender.h"
#include "tc_timeout.h"
#include <progmem.h>

#define DIT_TIMER		0
#define BEACON_TIMER	1
#define SYS_TIMER		2

#define BEACON_TIMEOUT	20

// The ditdah table. First byte is a map of the dits and dahs to be sent for this char. 
// A zero signifies a dit and a one indicates a dah. The most significant bit is sent first.
// The second byte indicates how many bits in the map are valid or part of the character,
// except for the first row. There are not 0x85 dits and dahs in a SPACE. There are 5 dit times in
// a space but they are silent. So the 0x80 bit means this character is silent and the 5 means
// that there five silent dits. The third member in each row is a character that serves a 
// convienence for the programmer. It simply reminds the programmer what character this row is for.
// The fourth member is a string that reminds the programmer what this dits and dahs are for this 
// character, again a convienence. The last member is a hex/ASCII code for this character. This
// is what the program expects to receive from the keyboard. The value received from the keyboard
// is used to index into the table after a little math is done on the keyboard value. If we receive
// a 0x20(space) from the keyboard and take 0x20 away from all received characters, we'lll end up 
// with the 0 index we need to find the data we need for output a space. Also understand that 
// DITDAHS is a macro ans does not define any data for the program. Instead this macro defines
// a long sequence of other macros named ASSOC. Below the table, you'll see the defines of the ASSOC()
// macro. Looking close at those defines, you will see that they define/use only one member of the 
// of the input parameters. When DITDAHS is referred to in each following data declaration, only one
// member of each row is going into each row of that that declared character array. So the first member
// of each row goes in the the first array and the second member goes into the second array. The
// third, fourth and fifth members are never referred to and therefore never go into the programs
// data or program space. They only live here in the source as a convienence for the programmer.
// Of course, you could use this trick to define as many parallel arrays as you wish.    
// 
#define DITDAHS \
ASSOC(0b00000000, 0x85, ' ',".....",  0x20) \
ASSOC(0b10101100, 6, '!', "-.-.--",0x21) \
ASSOC(0b01001000, 6, '"', ".-..-.",0x22) \
ASSOC(0b00000000, 0, '#', "      ",0x23) \
ASSOC(0b00010010, 7, '$', "...-..-",0x24) \
ASSOC(0b00000000, 0, '%', "      ",0x25) \
ASSOC(0b01000000, 5, '&', ".-...", 0x26) \
ASSOC(0b01111000, 6, '\'', ".----.",0x27) \
ASSOC(0b10110000, 5, '(', "-.--.", 0x28) \
ASSOC(0b10110100, 6, ')', "-.--.-",0x29) \
ASSOC(0b00000000, 0, '*', "      ",0x2a) \
ASSOC(0b01010000, 5, '+', ".-.-.", 0x2b) \
ASSOC(0b11001100, 6, ',', "--..--",0x2c) \
ASSOC(0b10000100, 6, '-', "-....-",0x2d) \
ASSOC(0b01010100, 6, '.', ".-.-.-",0x2e) \
ASSOC(0b10010000, 5, '/', "-..-.", 0x2f) \
ASSOC(0b11111000, 5, '0', "-----", 0x30) \
ASSOC(0b01111000, 5, '1', ".----", 0x31) \
ASSOC(0b00111000, 5, '2', "..---", 0x32) \
ASSOC(0b00011000, 5, '3', "...--", 0x33) \
ASSOC(0b00001000, 5, '4', "....-", 0x34) \
ASSOC(0b00000000, 5, '5', ".....", 0x35) \
ASSOC(0b10000000, 5, '6', "-....", 0x36) \
ASSOC(0b11000000, 5, '7', "--...", 0x37) \
ASSOC(0b11100000, 5, '8', "---..", 0x38) \
ASSOC(0b11110000, 5, '9', "----.", 0x39) \
ASSOC(0b11100000, 6, ':', "---...",0x3a) \
ASSOC(0b10101000, 6, ';', "-.-.-.",0x3b) \
ASSOC(0b00000000, 0, '<', "      ",0x3c) \
ASSOC(0b10001000, 5, '=', "-...-", 0x3d) \
ASSOC(0b00000000, 0, '>', "      ",0x3e) \
ASSOC(0b00110000, 6, '?', "..--..",0x3f) \
ASSOC(0b01101000, 6, '@', ".--.-.",0x40) \
ASSOC(0b01000000, 2, 'A', ".-",    0x41) \
ASSOC(0b10000000, 4, 'B', "-...",  0x42) \
ASSOC(0b10100000, 4, 'C', "-.-.",  0x43) \
ASSOC(0b10000000, 3, 'D', "-..",   0x44) \
ASSOC(0b00000000, 1, 'E', ".",     0x45) \
ASSOC(0b00100000, 4, 'F', "..-.",  0x46) \
ASSOC(0b11000000, 3, 'G', "--.",   0x47) \
ASSOC(0b00000000, 4, 'H', "....",  0x48) \
ASSOC(0b00000000, 2, 'I', "..",    0x49) \
ASSOC(0b01110000, 4, 'J', ".---",  0x4a) \
ASSOC(0b10100000, 3, 'K', "-.-",   0x4b) \
ASSOC(0b01000000, 4, 'L', ".-..",  0x4c) \
ASSOC(0b11000000, 2, 'M', "--",    0x4d) \
ASSOC(0b10000000, 2, 'N', "-.",    0x4e) \
ASSOC(0b11100000, 3, 'O', "---",   0x4f) \
ASSOC(0b01100000, 4, 'P', ".--.",  0x50) \
ASSOC(0b11010000, 4, 'Q', "--.-",  0x51) \
ASSOC(0b01010000, 3, 'R', ".-. ",  0x52) \
ASSOC(0b00000000, 3, 'S', "...",   0x53) \
ASSOC(0b10000000, 1, 'T', "-",     0x54) \
ASSOC(0b00100000, 3, 'U', "..-",   0x55) \
ASSOC(0b00010000, 4, 'V', "...-",  0x56) \
ASSOC(0b01100000, 3, 'W', ".--",   0x57) \
ASSOC(0b10010000, 4, 'X', "-..-",  0x58) \
ASSOC(0b10110000, 4, 'Y', "-.--",  0x59) \
ASSOC(0b11000000, 4, 'Z', "--..",  0x5a) \
ASSOC(0b00000000, 0, '[', "--..",  0x5b) \
ASSOC(0b00000000, 0, '\\', "--..",  0x5c) \
ASSOC(0b00000000, 0, ']', "--..",  0x5d) \
ASSOC(0b00000000, 0, '^', "--..",  0x5e) \
ASSOC(0b00110100, 6, '_', "..--.-",0x5f) \
ASSOC(0b00010100, 6, "eow","...-.-",000) \
ASSOC(0b00000000, 8, "error","........",000) \
ASSOC(0b10101000, 5, "starting","-.-.-",000) \
ASSOC(0b01010000, 5, "new page",".-.-.",000) \
ASSOC(0b00010000, 5, "understood","...-.",000) \
ASSOC(0b01000000, 5, "wait",".-...",000) \

#define NUM_SYMBOLS  70
#define ASSOC(a, b, c, d, e)  a,
const uint8_t dit_maps[NUM_SYMBOLS] PROGMEM = {DITDAHS};

#undef ASSOC
#define ASSOC(a, b, c, d, e) b,

const uint8_t dit_lengths[NUM_SYMBOLS] PROGMEM = {DITDAHS};
	
const char string1[] PROGMEM = "CQ CQ CQ CQ ";
const char string2[] PROGMEM = "N6NQ N6KZ N6JZ ";
const char string3[] PROGMEM = "QTH QRZ QRP QRO ";
const char beaconstr[] PROGMEM = "CQ CQ CQ N6NQ N6NQ K   ";

uint8_t beacon_timer;
uint16_t beacon_seconds;

void beacon_init(uint8_t which)
{
	beacon_timer = which;
	beacon_seconds = 0;
	tc_timeout_start_periodic(which, 1000);
}

void beacon_send()
{
	if (!tc_timeout_test_and_clear_expired(beacon_timer))
		return;
	
	
	if ((beacon_seconds % BEACON_TIMEOUT) == 0)
	{
		sndr_sendstrP((uint_farptr_t) beaconstr);
	//	beacon_seconds = 0;
	}
	
	beacon_seconds++;	
}
void setup(void)
{
	// Setup a blinker. Clone UNO has PORTB pin5 as an LED
	DDRB = (1 << DDB5);
	
	tc_timeout_init();
	sei();
	sndr_init(DIT_TIMER);
	beacon_init(BEACON_TIMER);	
}

int main(void)
{
	
	setup();
    //sndr_sendstrP((uint_farptr_t) string1);
    //sndr_sendstrP((uint_farptr_t) string2);
    //sndr_sendstrP((uint_farptr_t) string3);
    //sndr_sendstrP((uint_farptr_t) beaconstr);
    //sndr_sendstrP((uint_farptr_t) string1);
    //sndr_sendstrP((uint_farptr_t) string2);

    while (1) 
    {
		sndr_send();
		beacon_send();
    }
}

