
#include "44b.h"
#include "timer.h"

int tmr_set_prescaler(int p, int  value)
{
	int offset = p*8;
	value &= 0xFF;

	if (p < 0 || p > 3)
		return -1;

	
	rTCFG0 |= (0xFF << offset);
	rTCFG0 &= (value << offset);

	return 0;
}

int tmr_set_divider(int d, enum tmr_div div)
{
	int ret = 0;
	int pos = d*4;

	if ((d < 0 || d > 5) ||
			(div == D1_32 && d > 3) ||
			(div == EXTCLK && d != 5) ||
			(div == TCLK && d != 4))
		return -1;

	if (div == EXTCLK || div == TCLK)
		div = 4;

	
	rTCFG1 &= ~(0xF << pos);
	rTCFG1 |= (div << pos);

	return 0;
}

int tmr_set_count(enum tmr_timer t, int count, int cmp)
{
	int err = 0;
	switch (t) {
		case TIMER0:

			rTCON |= (0x1 << 1);
			rTCNTB0 = count;
			rTCMPB0 = cmp;
			rTCON &= ~(0x1 << 1);
			 break;
		case TIMER1:

			rTCON |= (0x1 << 9);
			rTCNTB1 = count;
			rTCMPB1 = cmp;
			rTCON &= ~(0x1 << 9);
			 break;
		case TIMER2:

			rTCON |= (0x1 << 13);
			rTCNTB2 = count;
			rTCMPB2 = cmp;
			rTCON &= ~(0x1 << 13);
			break;
		case TIMER3:

			rTCON |= (0x1 << 17);
			rTCNTB3 = count;
			rTCMPB3 = cmp;
			rTCON &= ~(0x1 << 17);
			break;
		case TIMER4:

			rTCON |= (0x1 << 21);
			rTCNTB4 = count;
			rTCMPB4 = cmp;
			rTCON &= ~(0x1 << 21);
			 break;
		case TIMER5:

			rTCON |= (0x1 << 25);
			rTCNTB5 = count;
			rTCON &= ~(0x1 << 25);
			break;
		default:
			err = -1;
	}

	return err;
}

int tmr_update(enum tmr_timer t)
{
	int pos = t*4;
	if (t > 0)
		pos += 4;

	if (t < 0 || t > 5)
		return -1;


	rTCON |= (0x1 << pos);
	rTCON &= ~(0x1 << pos);

	return 0;
}

int tmr_set_mode(enum tmr_timer t, enum tmr_mode mode)
{
	int err = 0;
	int pos = t*4;
	if (t > 0)
		pos += 4;

	if (t < 0 || t > 5)
		return -1;

	if (mode == ONE_SHOT)

		rTCON &= ~(0x8 << (pos));
	else if (mode == RELOAD)

		rTCON |= (0x8 << (pos));
	else
		err = -1;

	return err;
}

int tmr_start(enum tmr_timer t)
{
	int pos = t*4;
	if (t > 0)
		pos += 4;

	if (t < 0 || t > 5)
		return -1;


	rTCON |= (0x1 << pos);

	return 0;
}

int tmr_stop(enum tmr_timer t)
{
	int pos = t*4;
	if (t > 0)
		pos += 4;

	if (t < 0 || t > 5)
		return -1;

	rTCON &= ~(0x1 << pos);

	return 0;
}

int tmr_isrunning(enum tmr_timer t)
{
	int ret = 0;
	int pos = t*4;
	if (t > 0)
		pos += 4;

	if ((t >= 0) && (t <= 5) 
			&& (rTCON & (0x1 << pos)))
		ret = 1;

	return ret;
}

