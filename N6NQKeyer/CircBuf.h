/*
 * CircBuf.h
 *
 * Created: 7/22/2016 3:31:30 PM
 *  Author: n6nq
 */ 


#ifndef CIRCBUF_H_
#define CIRCBUF_H_

#include "stdint.h"

#define BUFSIZE 64

typedef struct tagCircBuf {
	char buffer[BUFSIZE];  // limited to 128 or index 127 (signed 8bit int)
	uint16_t length;
	uint16_t start; 
	uint16_t next;
	uint16_t data_avail;
} CircBuf;

void CircBuf_init(CircBuf* pbuf);

//void CircBuf_destroy(CircBuf *pbuf);
void CircBuf_make_empty(CircBuf* pbuf);

int CircBuf_read(CircBuf *buffer, uint8_t *target, int amount);

uint8_t CircBuf_read_char(CircBuf *buffer);

int CircBuf_write(CircBuf *buffer, char *data, int length);

int CircBuf_writeP(CircBuf *buffer, const uint_farptr_t pdata);

int CircBuf_full(CircBuf *buffer);

//int CircBuf_available_data(CircBuf *buffer);

//int CircBuf_available_space(CircBuf *buffer);

char* CircBuf_gets(CircBuf *buffer, int amount);

void CircBuf_commit_read(CircBuf* pbuf, uint16_t length);

void CircBuf_commit_write(CircBuf* pbuf, uint16_t length);

#define CircBuf_available_space(B) (BUFSIZE - (B)->data_avail)

#define CircBuf_available_data(B) ((B)->data_avail)

#define CircBuf_bytes_at_end(B) ((B)->length - (B)->next)

#define CircBuf_ends_at(B) ((B)->buffer + (B)->next)

#define CircBuf_test_empty(B) ((B)->start == (B)->next)

//#define CircBuf_commit_read(B, A) ((B)->start = ({(B)->start + (A)) % (B)->length;
		//		 (B)->data_avail -= (A)})

//#define CircBuf_commit_write(B, A) ((B)->next = ({(B)->next + (A)) % (B)->length;
//	 (B)->data_avail += (A)})

#define CircBuf_starts_at(B) ((B)->buffer + (B)->start)

#endif /* CIRCBUF_H_ */