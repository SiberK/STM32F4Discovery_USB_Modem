#ifndef	FIFO_H
#define	FIFO_H
//-------------------------------------------------
typedef	void(*callbackIfCR)(class TFiFo* fifo,int cnt)	;
//-------------------------------------------------
class	TFiFo{
private:
 char*				Buf							;
 short				Size,Tail,Head,Cnt,flCR		;
 char				valPrevIn,valPrevOut		;
 callbackIfCR		FnIfCR						;
 
 
public:
 const char*		strEndS						;// набор символов, которые означают 'конец строки' 
 int				CntStr						;// количество собранных строк
   
   TFiFo(char* buf=0,int size=0,callbackIfCR fnIfCR=0)
 { Set(buf,size,fnIfCR)	;}
   
 void	Set(char* buf,int size,callbackIfCR fnIfCR,const char* endS=0)
 {FnIfCR=fnIfCR		; Buf = buf	; Size=size			; 
  valPrevIn=valPrevOut=0		;
  Tail=Head=Cnt=CntStr=flCR=0	; strEndS = endS	;}
  
 void	Reset(void){ Tail=Head=Cnt=CntStr=flCR=0	;}
 void	In(char val);
 char	Out(void)	;
 int	Empty(void){ return Cnt >  0    ? 0:1		;}
 int	Full(void) { return Cnt <= Size ? 0:1		;}
 char*	GetS(char* buf,int lenBuf)					;// взять одну строку, если есть
 char*	GetBuf(void){ return Buf					;}// дай буфер
 int	GetLen(void){ return Cnt					;}// сколько данных в буфере?
};
//-------------------------------------------------
//void	xputs_F(const char* str);
//-------------------------------------------------
//-------------------------------------------------
#endif
