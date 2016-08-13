/*
 * CircBuf.c
 *
 * Created: 7/22/2016 3:30:44 PM
 *  Author: n6nq
 */
#include <inttypes.h> 
#include "CircBuf.h"
#include <progmem.h>
#include <stddef.h>
#include <string.h>




//int CircBuf_available_space(CicBuf *buffer)
//{
              //1111
	//01234567890123	s  n   l  = r
	//s				    0  0  14  14 if s == n return l  case 1      
	//e				
	//s  e			    0  3  14  11 if n > s return l + s - n  case 2
	//  s   e			2  6  14  10    case 2
	//01234s6789012e    5 13  14   6    case 2
	//e	       s        9  0  14   9 if s > n return  14 - ((14 - 9) + 0) case 3
	//    e     s       10 4  14   6				  l - ((l-s) + n)
	//          e       10 10  14  14	case 1
	
	// case 2
	//register int_fast8_t length = buffer->length;
	//register int_fast8_t start = buffer->start;
	//register int_fast8_t next = buffer->next;
	//
	//if (next > start)
		//return (length + start) - next;
	//else if (start > next)
		//return length - ((length - start) + next);
	//else
		//return length;
//}

void CircBuf_init(CircBuf* pbuf)
{
	pbuf->length  = BUFSIZE;
	pbuf->start = 0;
	pbuf->next = 0;
	pbuf->data_avail = 0;
}

void CircBuf_make_empty(CircBuf* pbuf)
{
	pbuf->start = 0;
	pbuf->next = 0;
	pbuf->data_avail = 0;
}
void CircBuf_commit_read(CircBuf* pbuf, uint16_t length)
{
	pbuf->start = (pbuf->start + length) % BUFSIZE;
	pbuf->data_avail -= length;
}

void CircBuf_commit_write(CircBuf* pbuf, uint16_t length)
{
	 pbuf->next = (pbuf->next + length) % pbuf->length;
	 pbuf->data_avail += length;
}

//uint8_t circbuf_addP(CircBuf* buf, uint8_t* pStr)
//{
	//uint8_t len = strlen_PF(pStr);
	//
	//if (len > buf->available)
		//return -1;
		//
	//uint8_t free = circbuf_free(buf);
	//memcpy_PF(buf->buf, pStr, len);
	//return 0;
//}

int CircBuf_writeP(CircBuf *pbuf, const uint_farptr_t pdata)
{
	int bytes_to_end;
	int length = strlen_PF(pdata);
	int avail = CircBuf_available_space(pbuf);
	
	if (length > avail)
		return -1;		//TODO beep here

	bytes_to_end = CircBuf_bytes_at_end(pbuf);

	if (bytes_to_end >= length)
	{
		memcpy_PF(CircBuf_ends_at(pbuf), pdata, length);
	}
	else
	{
		memcpy_PF(CircBuf_ends_at(pbuf), pdata, bytes_to_end);
		memcpy_PF(pbuf->buffer, pdata+bytes_to_end, length-bytes_to_end);

	}
	
	//if (result == NULL)
	//	return -2;

	CircBuf_commit_write(pbuf, length);

	return length;
}

//int CircBuf_write(CircBuf *buffer, char *data, int length)
//{
////	if(CircBuf_available_data(buffer) == 0) {
////		buffer->start = buffer->next = 0;
////	}
//
	//if (length <= CircBuf_available_space(buffer))
		//goto error;
//
	//void *result = memcpy(CircBuf_ends_at(buffer), data, length);
	//check(result != NULL, "Failed to write data into buffer.");
//
	//CircBuf_commit_write(buffer, length);
//
	//return length;
	//error:
	//return -1;
//
////	if(CircBuf_available_data(buffer) == 0) {
////		buffer->start = buffer->next = 0;
////	}
////
////	check(length <= CircBuf_available_space(buffer),
////	"Not enough space: %d request, %d available",
////	CircBuf_available_data(buffer), length);
////
////	void *result = memcpy(CircBuf_ends_at(buffer), data, length);
////	check(result != NULL, "Failed to write data into buffer.");
////
////	CircBuf_commit_write(buffer, length);
////
////	return length;
////	error:
////	return -1;
//}
uint8_t CircBuf_read_char(CircBuf *buffer)
{
	uint8_t aChar;
	
	if (CircBuf_available_data(buffer) == 0)
	return 0;
	//	"Not enough in the buffer: has %d, needs %d",
	//	CircBuf_available_data(buffer), amount);

	aChar = *(uint8_t*)CircBuf_starts_at(buffer);

	CircBuf_commit_read(buffer, 1);

//	if(buffer->next == buffer->start) {
//		buffer->start = buffer->next = 0;
//	}

	return aChar;
}

int CircBuf_read(CircBuf *buffer, uint8_t *target, int amount)
{
	if (amount <= CircBuf_available_data(buffer))
		return -3;
//	"Not enough in the buffer: has %d, needs %d",
//	CircBuf_available_data(buffer), amount);

	void *result = memcpy(target, CircBuf_starts_at(buffer), amount);
	if (result != NULL)
		return -4;
	//, "Failed to write buffer into data.");

	CircBuf_commit_read(buffer, amount);

	if(buffer->next == buffer->start) {
		buffer->start = buffer->next = 0;
	}

	return amount;
//	error:
//	return -1;
}
//
//bstring CircBuf_gets(CircBuf *buffer, int amount)
//{
	//check(amount > 0, "Need more than 0 for gets, you gave: %d ", amount);
	//check_debug(amount <= CircBuf_available_data(buffer),
	//"Not enough in the buffer.");
//
	//bstring result = blk2bstr(CircBuf_starts_at(buffer), amount);
	//check(result != NULL, "Failed to create gets result.");
	//check(blength(result) == amount, "Wrong result length.");
//
	//CircBuf_commit_read(buffer, amount);
	//assert(CircBuf_available_data(buffer) >= 0 && "Error in read commit.");
//
	//return result;
	//error:
	//return NULL;
//}