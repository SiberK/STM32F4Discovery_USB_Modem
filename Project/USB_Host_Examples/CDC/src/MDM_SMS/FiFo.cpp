//-------------------------------------------------
#include	"FiFo.h"
//-------------------------------------------------
void	TFiFo::In(char val)
{if(Buf && Size && Cnt<Size){ Cnt++	;
   Buf[Head] = val	; Head++	; if(Head>=Size) Head = 0	;}
 else{ Cnt=CntStr=Tail=Head=valPrevIn=0;}// если буфер переполнился, то сбрасываем его (на всякий случай)
 
 if(Buf && Size && CntStr <= Cnt && 
		(val=='\n' || (val == '>'&& !valPrevIn))) CntStr++	;// если пришел символ CR(ВК)
 valPrevIn = (val == '\n' || val == '\r') ? 0:val			;
}
//-------------------------------------------------
char	TFiFo::Out(void)
{char val=0	; 
 if(Buf && Size && Cnt>0){ Cnt--	; 
   val = Buf[Tail]	; Tail++		; 
   if(Tail>=Size) Tail = 0			;}
 if(!Cnt) Tail=Head=CntStr=0		;// если буфер опустошился, то сбрасываем его указатели(для ясности)
 return val	;}
//-------------------------------------------------
char*	TFiFo::GetS(char* buf,int lenBuf){					// взять одну строку, если есть
 int		ix=0	;
 char		val=0	;
 
 for(;ix<lenBuf && CntStr>0 && Cnt>0;){
    valPrevOut = (val == '\n' || val == '\r') ? 0:val	;
    val = Out()		;
	if(ix<lenBuf-1 && val != '\n' && val != '\r') buf[ix++] = val		;
   
   if(val == '\n' || (val == '>' && !valPrevOut)){
     buf[ix] = 0						;
	 if(CntStr>0) CntStr--				;
	 break	;
   }
 }
 
 if(!ix) buf = 0	;
 
 return	buf	;}
//-------------------------------------------------
