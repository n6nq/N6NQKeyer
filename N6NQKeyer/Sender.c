/*
 * Sender.c
 *
 * Created: 7/20/2016 8:11:58 PM
 *  Author: n6nq
 */ 


#include "Sender.h"
#include "tc_timeout.h"
#include "CircBuf.h"
#include <progmem.h>

#define NOTSENDING 0
#define SENDING    4
#define NOBITS     0
#define BITS	   2
#define EMPTY	   0
#define NOTEMPTY   1

#define START	 0
#define SILENT	0x80

uint8_t sndr_speed;
uint8_t sndr_which;
uint8_t sndr_char;
uint8_t sndr_state;
uint8_t sndr_nbits;
uint8_t sndr_running;
uint_fast32_t sndr_bits;

CircBuf sndr_buf;



void sndr_init(uint8_t which)  // which timeout
{
	sndr_bits = 0;
	sndr_speed = 20;  // The default speed is 20wpm
	sndr_which = which;
	sndr_bits = 0;
	sndr_running = 0;
	sndr_state = START;
	
	CircBuf_init(&sndr_buf);
}

extern const uint8_t dit_maps[] PROGMEM;
extern const uint8_t dit_lengths[] PROGMEM;

void ouch(uint8_t errnum)
{
	
}

void sndr_key_close(bool bit)
{
	#define LED0_MASK (1 << PORTB5)
	if (bit)
		PORTB |= (1 << PORTB5);
	else
		PORTB &= ~(1 << PORTB5); // Setting pin PD1 low in order to turn on LED1.
}

uint8_t sndr_special_char(uint8_t aChar)
{
	return 0;
}

uint8_t sndr_bitify(uint8_t aChar)
{
	uint8_t nIndex;
	
	sndr_bits = 0;
	sndr_nbits = 0;
	
	if (aChar >= ' ')
		nIndex = aChar - ' ';
	if (aChar > 0x5f)
		return sndr_special_char(aChar);
		
	uint8_t template = pgm_read_byte(&dit_maps[nIndex]);
	uint8_t templen = pgm_read_byte(&dit_lengths[nIndex]);

	if (templen & SILENT)
	{
		return (templen & ~SILENT);
	}
	else if (templen > 0)
	{					// TODO add farnesworth here
		sndr_nbits = 2;	// character spacing = 3 = 2+1 silent dit times, 2 here and 1 one front of each sound
		
		while (templen)
		{
			if (template & 0x80)	// send a dah
			{
				sndr_bits = sndr_bits << 4;		//1 dit space + dah = 1+3 dit times
				sndr_bits |= 7;
				sndr_nbits += 4;
			}
			else
			{
				sndr_bits = sndr_bits << 2;		//1 dit space + dit = 1+1 dit times
				sndr_bits |= 1;
				sndr_nbits += 2;
			}
			template = template << 1;
			templen--;
		}
		sndr_bits = sndr_bits << (32 - sndr_nbits);
		return sndr_nbits;	
	}
	return 0;
}

int sndr_send()
{
	bool bit;
	//Scenarios
	//0) Power On
	//	Initial: !sending, nbits = 0, empty				000 NOTSENDING | NOBITS | EMPTY
	//	Action:		if !sending and empty then return
	//	Finish:  same
	//1) First time
	//	Initial: !sending, nbits = 0, !empty			011
	//	Action: 	if !empty and !sending
	//					turn on timer
	//					get character -> empty
	//					nbits = bitify(character)
	//					sending = true
	//					eat/send first bit -> nbits
	//	Finish: sending, nbits > 0, !empty
	//2) Mid-character
	//	Initial: sending, nbits > 0						11?
	//	Action:		eat/send next bit -> nbits
	//	Finish:   sending, nbits > 0
	//3) End of character
	//	Initial: sending, nbits = 0, !empty				101
	//	Action:		get next character -> empty
	//				bitify(character) -> nbits
	//				eat/send first bit
	//	Finish: sending, nbits > 0, !empty
	//4) Last character
	//	Initial: sending, nbits = 0, empty				100
	//	Action:		set !sending
	//	Finish: !sending, nbits = 0, empty

	if (!tc_timeout_test_and_clear_expired(sndr_which))
		return 0;

	sndr_state = sndr_state & SENDING;
	sndr_state |= (CircBuf_test_empty(&sndr_buf) ? EMPTY : NOTEMPTY);
	sndr_state |= ((sndr_nbits > 0) ? BITS : NOBITS);
	
	switch (sndr_state)							//0) Power On
	{
	case  NOTSENDING | NOBITS | EMPTY:			//	Initial: !sending, nbits = 0, empty				000
		return 0;								//	Action:		if !sending and empty then return
												//	Finish:  same
												//1) First time
												//new
	case NOTSENDING | NOBITS | NOTEMPTY:		//	Initial: !sending, nbits = 0, !empty			011
												//Action: turn on timer
		sndr_state |= (((sndr_char = CircBuf_read_char(&sndr_buf)) > 0) ? NOTEMPTY : EMPTY);	// get character -> empty
		sndr_nbits = sndr_bitify(sndr_char);	// nbits = bitify(character)
		sndr_state |= SENDING;					// sending = true
		sndr_bits = sndr_bits << 1;
		sndr_nbits--;							// eat/send first bit -> nbits
		break;									// Finish: sending, nbits > 0, !empty
												//2) Mid-character
	case SENDING | BITS:
	case SENDING | BITS | NOTEMPTY:						// Initial: sending, nbits > 0						11?
		bit = (sndr_bits & 0x80000000);			// Action:		eat/send next bit -> nbits
		sndr_key_close(bit);					// Finish:   sending, nbits > 0
		sndr_bits = sndr_bits << 1;
		sndr_nbits--;
		break;
												//3) End of character
	case SENDING | NOBITS | NOTEMPTY:			// Initial: sending, nbits = 0, !empty				101
		sndr_state |= (((sndr_char = CircBuf_read_char(&sndr_buf)) > 0) ? NOTEMPTY : EMPTY);	//Action: get next character -> empty
		sndr_nbits = sndr_bitify(sndr_char);	// bitify(character) -> nbits
		sndr_bits = sndr_bits << 1;
		sndr_nbits--;							// eat/send first bit -> nbits
		break;									// Finish: sending, nbits > 0, !empty

												// 4) Last character
	case SENDING | NOBITS | EMPTY:				// Initial: sending, nbits = 0, empty				100
		sndr_key_close(0);					// Finish:   sending, nbits > 0
		tc_timeout_stop(sndr_which);
		sndr_running = 0;
		sndr_state &= ~SENDING;;				// Action: set !sending
		break;									//	Finish: !sending, nbits = 0, empty
		
	default:
		ouch(1);
		return 0;
	}

	return 1;
}

int sndr_sendstrP(const uint_farptr_t pstr)
{
	uint8_t rv = CircBuf_writeP(&sndr_buf, pstr);
	if (!sndr_running) {
			tc_timeout_start_periodic(sndr_which, 1200/sndr_speed);
			sndr_running = 1;
	}
	return rv;
}
