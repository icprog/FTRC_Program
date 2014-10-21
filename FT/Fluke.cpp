#include "StdAfx.h"
#include "Fluke.h"
#include "stdlib.h"
#include "decl-32.h"

CFluke::CFluke():m_fluke45(0)
{
}

CFluke::~CFluke(void)
{
}

void CFluke::fluke45_init (int address)
{
     m_fluke45 = ibdev(0,address,0,T10s,1,0);

//     ibren(fluke45);
     Sleep(200);
     ibwrt (m_fluke45,"*RST;*CLS;",8);
}

void CFluke::fluke45_set_filter (int filter)
{
	char buf[150];

	switch(filter) {
	  case -1 : strcpy_s(buf,150,"RATE S;");break;    /* -1  Slow (Default) */
	  case  0 : strcpy_s(buf,150,"RATE F;");break;    /*  0  Fast           */
	  case  1 : strcpy_s(buf,150,"RATE M;");break; }  /*  1  Bypass         */

	ibwrt (m_fluke45,buf,(long)strlen(buf));
}

void CFluke::fluke45_config (char *fun,int range,int trigger)
{
     char buf[150];

     if (strcmp(fun,"Z")==0) strcpy_s(buf,150,"OHMS;");
     else if (strcmp(fun,"V" )==0) strcpy_s(buf,150,"VDC;" );
     else if (strcmp(fun,"VA")==0) strcpy_s(buf,150,"VAC;" );
     else if (strcmp(fun,"I" )==0) strcpy_s(buf,150,"ADC;" );
     else if (strcmp(fun,"IA")==0) strcpy_s(buf,150,"AAC;" );
     else if (strcmp(fun,"F" )==0) strcpy_s(buf,150,"FREQ;");
     else if (strcmp(fun,"D" )==0) strcpy_s(buf,150,"DIODE;");

     switch(range) {
          case  1 : strcat_s(buf,150,"RANGE 1;");break;   /*  1  300mV   */
          case  2 : strcat_s(buf,150,"RANGE 2;");break;   /*  2     3V   */
          case  3 : strcat_s(buf,150,"RANGE 3;");break;   /*  3    30V   */
          case  4 : strcat_s(buf,150,"RANGE 4;");break;   /*  4   300V   */
          case  5 : strcat_s(buf,150,"RANGE 5;");break; } /*  5  1000V   */

     strcat_s(buf,150,"FIXED;");

	 switch(trigger) {
		  case -1 :
		  case  0 :
		  case  1 :
		  case  2 : strcat_s(buf,150,"TRIGGER 1;");break; }

	 ibwrt (m_fluke45,buf,(long)strlen(buf));
}


/*----- fluke45_measure -----*/
void CFluke::fluke45_measure (double &measure)
{
	char in_str[50] = "";
	measure=0.0;
	ibwrt (m_fluke45,"MEAS?",5);
	ibrd (m_fluke45,in_str,50);
	measure=atof(in_str);
}