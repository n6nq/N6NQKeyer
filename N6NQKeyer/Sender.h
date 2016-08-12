/*
 * Sender.h
 *
 * Created: 7/20/2016 8:12:36 PM
 *  Author: n6nq
 */ 


#ifndef SENDER_H_
#define SENDER_H_

#include <stdint.h>
#include <inttypes.h>

#define DIT_MAGIC  1200		// dit time in ms = magic / wpm



void sndr_init(uint8_t which);
int sndr_send(void);
int sndr_sendstrP(const uint_farptr_t pstr);



#endif /* SENDER_H_ */