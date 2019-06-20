﻿/*
 * ztest.c
 *
 * Created: 2012-3-17 13:05:08
 *  Author: Administrator
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

//#include <math.h>

#define uint unsigned int


#define  beep_on  PORTC|=(1<<2)                   //电平不发声
#define  beep_off PORTC&=~(1<<2)                 //发声

#define E_set PORTD|=(1<<7) //液晶使能
#define E_clear PORTD &=~(1<<7)

#define RW_set PORTD|=(1<<6) //液晶读写
#define RW_clear PORTD &=~(1<<6)

#define RS_set PORTD|=(1<<5) //液晶数据
#define RS_clear PORTD &=~(1<<5) //液晶指令

#define Sw_on PORTD |=1 //加热

#define Sw_off PORTD &=0XFE //不加热

//unsigned char OVERI=4;
//unsigned char sswint=0;
unsigned char currenttimer[3]={40,1,0};      //电容  currenttimer[0]秒，【1】分
//unsigned char currenttimer[3]={59,1,0};   //实践加热 currenttimer[0]分，【1】时  【2】秒	
unsigned char currentemper[2]={50,50} ;    //currentemper[0]当前温度

unsigned char  Kkey[8]={1,1,1,1,};	 
unsigned char SWPower=0;

int Printnm(unsigned char x,unsigned char y,unsigned char n,unsigned char f);
void outChinese(unsigned char place,unsigned char unit,unsigned char *point);

//========================================================
//        外部中断0，用于过流中断
//=========================================================
ISR(INT0_vect)    //过流中断
{
 Sw_off;   //关闭电源
}


//===========================================================
//定时器1溢出中断 1秒一次   用电容模拟热水器时的定时中断  基本单位秒
//============================================================
ISR(TIMER1_OVF_vect)  //定时中断
{
  TCNT1H = 0xE1;  //设置 TC1 的 计数寄存器 高8位值，基于8.M晶振
  TCNT1L = 0x7B;  //设置 TC1 的 计数寄存器 低8位值，基于8.M晶振

	if(!(( currenttimer[0]==0) && ( currenttimer[1]==0) ))
	{	
	currenttimer[0]--;                         //currenttimer[0]  秒
	if (currenttimer[0]>=60)
	{
	currenttimer[0]=59;
	if(currenttimer[1]!=0) currenttimer[1]--;  // currenttimer[1]分
	}	  
  }
		
}

//===========================================================
//定时器1溢出中断 1秒一次   用电容模拟热水器时的定时中断  基本单位秒
//以下为实际热水器的定时中断          基本单位分
//==========================================================
//ISR(TIMER1_OVF_vect)  //定时中断
//{
// TCNT1H = 0xE1;  //设置 TC1 的 计数寄存器 高8位值，基于8.M晶振
// TCNT1L = 0x7B;  //设置 TC1 的 计数寄存器 低8位值，基于8.M晶振
//   FShown++;	
//	if(!(( currenttimer[0]==0) && ( currenttimer[1]==0) ))
//	{	
//    currenttimer[2]++;
//    if (currenttimer[2]>=60)    //currenttimer[2]  秒
//    {
//   currenttimer[2]=0;  
//	currenttimer[0]--;             //currenttimer[0] 分
//	if (currenttimer[0]>=60)
//	{
//	currenttimer[0]=59;
//	if(currenttimer[1]!=0) currenttimer[1]--;//时
//	}	  
//  }
//	}	
//}

void ttimer1_init(void)
{
 TCCR1B = 0x00;  //stop timer

 TCNT1H = 0xE1;  //设置 TC1 的 计数寄存器 高8位值，基于8M晶振
 TCNT1L = 0x7B;  //设置 TC1 的 计数寄存器 低8位值，基于8M晶振 
 
 TCCR1A = 0x00;
 TCCR1B = 0x05;  //设置TC1 为 CLK/1024分频，启动TC1

 MCUCR = 0x00;  //设置 MCU 的 控制寄存器
 GICR  = 0x00;  //设置 中断控制寄存器
 TIMSK = 0x00;  //设置 定时计数器 的 屏蔽寄存器
}

unsigned char str00[8]={0xB5,0xB1,0xC7,0xB0,0xCE,0xC2,0xB6,0xC8};//当前温度
unsigned char str01[8]={0xCE,0xC2,0xB6,0xC8,0xC9,0xE8,0xD6,0xC3};//温度设置
unsigned char str02[4]={0xB6,0xA8,0xCA,0xB1};//定时
unsigned char str03[8]={0xC9,0xE8,0xB6,0xA8,0xCE,0xC2,0xB6,0xC8};//设定温度	
unsigned char str04[4]={0xD3,0xE0,0xCA,0xB1};//余时
unsigned char str05[8]={0xB9,0xFD,0xC1,0xF7,0xA1,0xA3,0xA1,0xA3};//过流。。
unsigned char str06[8]={0xBC,0xD3,0xCE,0xC2,0xD6,0xD0,0xA1,0xA3};//加温中。
unsigned char str07[8]={0xB1,0xA3,0xCE,0xC2,0xD6,0xD0,0xA1,0xA3};//保温中。
unsigned char str08[12]={0xD5,0xC5,0xB6,0xAB,0xB1,0xCF,0xD2,0xB5,0xC9,0xE8,0xBC,0xC6};//张东毕业设计
unsigned char str09[8]={0xCE,0xC2,0xB6,0xC8,0xC9,0xE8,0xB6,0xA8};//温度设定	
unsigned char str0A[8]={0xB6,0xA8,0xCA,0xB1,0xC9,0xE8,0xB6,0xA8};//定时设定
unsigned char str0B[8]={0xB5,0xC8,0xB4,0xFD,0xBC,0xD3,0xCE,0xC2};//等待加温

unsigned char sz[385] = { //0X00,0X01,0X10,0X00,0X10,0X00,                     0--9 数码字模 / ：/ C0
//const unsigned char gImage_Y0[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X2F,0XE8,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X20,0X08,
0X00,0X00,0X20,0X08,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,0X1F,0XF0,
//};
//const unsigned char gImage_Y1[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X00,0X00,0X00,0X08,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X08,
0X00,0X00,0X00,0X08,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X08,0X00,0X00,
//};
//const unsigned char gImage_Y2[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X0F,0XE8,
0X1F,0XF0,0X2F,0XE0,0X30,0X00,0X30,0X00,0X30,0X00,0X30,0X00,0X2F,0XE0,0X1F,0XF0,
//};
//const unsigned char gImage_Y3[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X0F,0XE8,
0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X0F,0XE8,0X1F,0XF0,
//};
//const unsigned char gImage_Y4[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X00,0X00,0X20,0X08,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,
0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X08,0X00,0X00,
//};
//const unsigned char gImage_Y5[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X2F,0XE0,0X30,0X00,0X30,0X00,0X30,0X00,0X30,0X00,0X2F,0XE0,
0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X0F,0XE8,0X1F,0XF0,
//const unsigned char gImage_Y6[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X2F,0XE0,0X30,0X00,0X30,0X00,0X30,0X00,0X30,0X00,0X2F,0XE0,
0X1F,0XF0,0X2F,0XE8,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,0X1F,0XF0,
//};
//const unsigned char gImage_Y7[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X08,
0X00,0X00,0X00,0X08,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X08,0X00,0X00,
//};
//const unsigned char gImage_Y8[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X2F,0XE8,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,
0X1F,0XF0,0X2F,0XE8,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,0X1F,0XF0,
//};
//const unsigned char gImage_Y9[38] = { 0X00,0X01,0X10,0X00,0X10,0X00,
0X00,0X00,0X1F,0XF0,0X2F,0XE8,0X30,0X18,0X30,0X18,0X30,0X18,0X30,0X18,0X2F,0XE8,
0X1F,0XF0,0X0F,0XE8,0X00,0X18,0X00,0X18,0X00,0X18,0X00,0X18,0X0F,0XE8,0X1F,0XF0,
//};
//const unsigned char gImage_Y10[38] = { //:
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X06,0X00,0X0F,0X00,
0X0F,0X00,0X06,0X00,0X00,0X00,0X06,0X00,0X0F,0X00,0X0F,0X00,0X06,0X00,0X00,0X00,
//const unsigned char gImage_Y11[38] = {  //C0
0X00,0X00,0X00,0X0E,0X1F,0X11,0X39,0XB1,0X71,0XB1,0X61,0XB1,0X60,0X1E,0X60,0X0E,
0X60,0X00,0X60,0X00,0X60,0X00,0X70,0X00,0X70,0X80,0X3F,0X80,0X1F,0X00,0X00,0X00,
};


void delayms(unsigned int z)
{
	unsigned int x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}
void delayus(unsigned int t)
{
	while(--t);

}


void wr_com(unsigned char value)//写指令，写指令时必须为RS=0;RW=0;
{
	E_clear;
	RS_clear;
	RW_clear;
	delayus(1);
	PORTB=value;
	E_set;
	delayus(4); //如果没有延时就必须要加查忙指令
	E_clear;
	delayus(1);
}

void wr_data(unsigned char sj)//写数据，写数据时必须为 RS=1;RW=0;
{

	E_clear;
	RS_set;
	RW_clear;
	delayus(2);
	
	PORTB=sj;
	E_set;
	delayus(4);//如果没有延时就必须要加查忙指令
	E_clear;
	delayus(2);
}

void reset (void)
{
	wr_com(0x18);
	delayms(1);
	wr_com(0x18);
	delayms(1);
    wr_com(0x30);//功能设置，一次送8位数据，基本指令集
	delayms(1);
	wr_com(0x01);//0000,1100  整体显示，游标off，游标位置off
	delayms(1);
	wr_com(0x05);//0000,0001 清DDRAM   清屏
	delayms(1);


}

void Craw1616(unsigned char a1,unsigned char b1)       //清除A1列 B1行
    {
     unsigned char i;
     wr_com(0x34);      //8unsigned char扩充指令集,即使是36H也要写两次
	 wr_com(0x36); 
  for(i=0;i<16;i++)
  { 		  
	  if((b1+i)<32)
	     {
			   wr_com(0x80|(b1+i));      //输入行地址
               wr_com(0x80|a1);      //输入列地址	 
		 }
		 else
		 {
			 		wr_com(0x80|(b1+i-32));     //行位置
                    wr_com(0x80|(a1+8));      //列位置
		 }		
         wr_data(0); //输入高位字节数据
		 wr_data(0);  //输入低位字节数据 
         }
 	 wr_com(0x36);      //绘图ON,基本指令集里面36H不能开绘图
	 wr_com(0x30); //恢复至基本指令集	
	}		
	
//画一个16*16的字模
void Draw1616(unsigned char a1,unsigned char b1,unsigned char n,const unsigned char *cpoint,unsigned char f)
{
     unsigned int x;
     unsigned char i;
	 x=n*32;
     wr_com(0x34);      //8unsigned char扩充指令集,即使是36H也要写两次
	 wr_com(0x36); 
   
  for(i=0;i<16;i++)
  { 		  
	  if((b1+i)<32)
	     {
			   wr_com(0x80|(b1+i));      //输入行地址
               wr_com(0x80|a1);      //输入列地址	 
		 }
		 else
		 {
			 		wr_com(0x80|(b1+i-32));     //行位置
                    wr_com(0x80|(a1+8));      //列位置
		 }		
		 if(f==1)
		 {
         wr_data(cpoint[x++]); //输入高位字节数据
		 wr_data(cpoint[x++]);  //输入低位字节数据 
		 }
		 else
		 {
		 wr_data(~cpoint[x++]); //输入高位字节数据
		 wr_data(~cpoint[x++]);  //输入低位字节数据	 
		 }		 
   }
 	 wr_com(0x36);      //绘图ON,基本指令集里面36H不能开绘图
	 wr_com(0x30);     //恢复至基本指令集	
};

int Printnm(unsigned char x,unsigned char y,unsigned char data,unsigned char f)
    {
	 unsigned char k,j;
	 k=data/10;      //K 十位     f: 反 / 正
	 j=data%10;      //  个位
	 Draw1616(x,y,k,sz,f);
	 Draw1616(x+1,y,j,sz,f);  		
	return  k;
	}

void  InKey()    //去抖动读键盘
      {
	   char key;
	   key=PINC;
	   Kkey[0]=key;
	   key=key & Kkey[1];
       Kkey[2]=key;
	   key=Kkey[3];
	   key=~key;
	   key=key & Kkey[2];
	   Kkey[4]=key | Kkey[4];
	   Kkey[3]=Kkey[2];
	   Kkey[1]=Kkey[0];
	  }

void int0_init()    //初始化外部INT0
{	
	MCUCR=0X0A; //0#,1#外中断下降沿	
	GICR=0X40;//开通0#外中断
};	


void SetTimer(void)   //时间设定
  {
	  unsigned char  Inset=1;
               
				   
				   outChinese(0x88,2,str02);  //定时	
				   Printnm(3,32,currenttimer[1],0);
				   Draw1616(5,32,10,sz,1);
				   Printnm(6,32,currenttimer[0],0);
				   
				   while(Inset)
				      {
					   InKey();  
					   delayms(500);
										   
					   switch (Kkey[2])
					   {
					    case 216:    //k2
						   currenttimer[1]++;
						   if(currenttimer[1]>=12)currenttimer[1]=0;
						   Printnm(3,32,currenttimer[1],0);
						   Kkey[0]=0;
						   Kkey[1]=0;
						   Kkey[2]=0;
						   Kkey[3]=0;
						   delayms(500);
					   	break;
					    case 184:    //k3 
						   currenttimer[0]+=5;
						   if(currenttimer[0]>=60)currenttimer[0]=0;
						    Printnm(6,32,currenttimer[0],0);
						   Kkey[0]=0;
						   Kkey[1]=0;
						   Kkey[2]=0;
						   Kkey[3]=0;
						   delayms(500);						
					    break;
						case 120:    //k4
 							   outChinese(0x88,2,str04);////余时	
				               Printnm(3,32,currenttimer[1],1);
				               Printnm(6,32,currenttimer[0],1);
							   Inset=0;
						break;
					    default: break; 
					   }
					  
					  }		
					 	  
  };


void SetTemperture(void)  //温度设定
  {
    unsigned char  Inset=1;
              
				   outChinese(0x98,4,str01);//温度设置
				   Draw1616(7,48,11,sz,1);	
				   Printnm(5,48,currentemper[0],0);
				  				   
				   while(Inset)
				      {
					   InKey();  
					   delayms(500);
										   
					   switch (Kkey[2])
					   {
					    case 216:    //k2
						   currentemper[0]++;
						   if(currentemper[0]>=81)currentemper[0]=30;
						   Printnm(5,48,currentemper[0],0);
						   Kkey[0]=0;
						   Kkey[1]=0;
						   Kkey[2]=0;
						   Kkey[3]=0;
						   delayms(500);
					   	break;
					    case 184:    //k3 
						   currentemper[0]--;
						   if(currentemper[0]<=29)currentemper[0]=80;
						    Printnm(5,48,currentemper[0],0);
						   Kkey[0]=0;
						   Kkey[1]=0;
						   Kkey[2]=0;
						   Kkey[3]=0;
						   delayms(500);						
					    break;
						case 120:    //k4
						       currentemper[1]=currentemper[0];
                               outChinese(0x98,4,str03);//设定温度
							   Printnm(5,48,currentemper[0],1);
						
						
						Inset=0;
						break;
					    default: break; 
					   }
					  
					  }		
					
  };   				  

void ClearSreenLine(unsigned char L)  //清行 L=0,16,32,48==>1,2,3,4行
     {
					 Craw1616(0,L);
					 Craw1616(1,L);
					 Craw1616(2,L);
					 Craw1616(3,L);
					 Craw1616(4,L);
					 Craw1616(5,L);
					 Craw1616(6,L);
					 Craw1616(7,L);
	 }      

void ShownAllData()
       {
        
        Printnm(3,32,currenttimer[1],1);
		Printnm(6,32,currenttimer[0],1); 
		Printnm(5,48,currentemper[0],1); 
		 
	   }		   
	   

void ShownIover()
    {
	unsigned char	int_I;
	unsigned char FLAGSW=1;

	while(FLAGSW!=7)
	 {
	 int_I=PIND;
	 int_I &= 0x04;     //PIND.2=0  过流。。		 	 
	
	 if (int_I==0){                  
		             Sw_off;  
					if (FLAGSW==1)
					 {
					  ClearSreenLine(0);
		              FLAGSW=4;
					  
					 outChinese(0x82,4,str05);//过流。。		 	 

	                 } 			  
	                }
	   else 
	               {
					if (FLAGSW==4)ClearSreenLine(0);
	                FLAGSW=7;

				   }					
					
	            			  		  		 	
	   }		
					 
	}
		
void Shownmen()
    {          
					outChinese(0x90,4,str00);/////当前温度
					outChinese(0x88,2,str04);////余时
					outChinese(0x98,4,str03);//设定温度
					outChinese(0x81,6,str08);//张东
					
                    Draw1616(5,32,10,sz,1);   //C0
					Draw1616(7,48,11,sz,1);   //:
					Draw1616(7,16,11,sz,1);   //:
	 }					
	

uint M16ADC()    
    {
	uint addata;
	DDRA&=~(1<<0); //PA0口设置为输入，无上垃电阻，高阻态
	PORTA&=~(1<<0);
	SFIOR =0; //连续转换模式
	ADMUX=0X40; //AVCC，AREF引脚外加滤波电容
	//右对齐，ADC0单端输入
	ADCSRA =0X80; //启动ADC转换
	ADCSRA |=(1<<ADSC); //开始转换
	while(!(ADCSRA &(1<<ADIF))); //等待转换结束，ADIF置1
	addata=ADCL;
	addata=addata+ADCH*256; //把ADCL,ADCH转换成10进制
	ADCSRA |=(1<<ADIF); //对ADIF写”1”，ADIF清0
	ADCSRA =0X00; //关闭ADC转换
	return addata;	
    }
//输出内部汉字码
void outChinese(unsigned char place,unsigned char unit,unsigned char *point)
{
	
	unsigned char i,progdata;
	
	wr_com(place);
	for(i=0;i<unit*2;i++)//一个汉字为两个字符
	{
	progdata=*(point+i);
	wr_data(progdata);
	}
}

float lgg(float x)   //近似计算自然对数LOG（X）
     {
		int	i;
		float y=0,yy,yyy,j=0.0,sg=-1.0;
		yy=x-1;
		yyy=1.0;
		
		for (i=0;i<300;i++)
		{
		j=j+1.0;
		sg=-1.0*sg;
		yyy=yy*yyy;
		y=y+sg*yyy/j;
		}	
		return y;
	 }
	 
float the_heating_time1(float currentt0) 
  //电容模拟加热时间 根据三要素法   
    {
	
	   float x,y;
		x=(100.0-currentemper[0])/(100.0-currentt0);  // 、、 currentemper[0]
		y=-50.0*lgg(x)+4.0;   // 单位秒
		//50=RC;
		return y;    //返回充电时间
	}		

/*
double the_heating_time2(double currentt0) 
//实际水加温时间函数                currentemper[0]设定温度     currentt0当前温度
//时间=N*C.m.(t2-t1)/(P*3600)
//N效率
//加热时间：小时
//C：每公斤水的比热，4200焦耳/公斤。度
//M：水的重量，公斤
//t1、t2：加温前后的温度
//P：加热器功率，瓦
    {
	    double c=4200.0,m=40.0,p=3000.0,N=0.8,y;		
		y=N*c*m*(currentemper[0]-currentt0)/(p*60);    // 单位分
		return y;       //返加热时间
	}	

*/

	 	  	
//================================================================
int main( void )
{
	
	uint  ITempture,dtloop;
	uint	STUS=0;
	unsigned char CTempture;
	float FTemptureK=0.0976;      //2~10=1024  满刻度为100度时对应系数
	float dv;
	uint kloop=0,beeploop=0,dataloop=1;
	uint lt,dlt=1;
     
	SWPower=0;
	
	
	currenttimer[0]=55;
	currenttimer[1]=1;
    currentemper[0]=50;
	
	
	cli();   //关总中断 
	
	DDRD=0XF1;
	DDRB=0XFF;
    DDRC=0X07;
	
    Sw_off;           //关加热  
   
	ttimer1_init(); //定时器1设置
	int0_init(); //外中断0设置
	
	delayms(400);
	reset();      //LCD设置
	wr_com(0x01);  //LCD清除显示
	wr_com(0x30);  //LCD        8位控制界面

    Shownmen();    //菜单
	
	sei();     //开总中断

  

     while(1)
     {


//===============================================		   
//   过流在外部中断0 ISR(INT0_vect) 检测，检测到立马关闭电源 
//    ShownIover() 检测过流，并显示“过流”并保持电源关闭状态直到故障去除
//===============================================
		ShownIover();
		
		
//===============================================		   
//    读按键，设定定时、加热温度     
//===============================================		   
		InKey();
		switch(Kkey[2])
		   {
		   case 240:    //k0    时间设置
				TIMSK = 0x00;
		        ClearSreenLine(0);
				outChinese(0x82,4,str0A);//设定保温
				STUS=0;
				Sw_off;
		        SetTimer();   
				break;
		   case 232:    //k1
				TIMSK = 0x00;
		        ClearSreenLine(0);
			    outChinese(0x82,4,str09);//定时设定
			    STUS=0;
			    Sw_off;
		        SetTemperture();
			    break;
		   case 120:    //k4		   
				TIMSK = 0x04;
				ClearSreenLine(0);
				STUS=0;
			  outChinese(0x82,4,str0B);
		        break;
		   default: break;
		   }
		   
//==========================================================		   
//    以下ADC采集温度加以显示,     dataloop为降低采集速率     
//==========================================================		   
	    dataloop++;
		if(dataloop>20){
	       ITempture=M16ADC();        //AD 当前温度
		   dv=FTemptureK * ITempture; //当前温度归一化    10位AD，5V对应满刻度水温为100度。
		   CTempture=(int) (dv);      //当前温度取整
		   Printnm(5,16,CTempture,1); //显示温度
		   ShownAllData();	          //显示时间等
		   }	     
		   
//==========================================================
//     根据水量、当前温度、设定温度等确定加热时间：dlt=(int) the_heating_time2；
//         模拟调试用电容的的充电时间函数 dt=（int）the_heating_time1(dv)    
//         dtloop  ;为减少计算次数
//============================================================
		dtloop++;
		if(dtloop>=300)
		{
		dtloop=0;	
		dlt=(int ) the_heating_time1(dv);   //所需加热时间 （电容模拟加热时间计算函数the_heating_time1 单位：秒））
		                           //实际应为水加温时间计算函数应为the_heating_time2 单位：分）
		}		
		
//============================================================
//     //计算设定时间   剩余时间化为秒（电容模拟为秒）；实际应调整为分     
//=============================================================
		lt=60*currenttimer[1]+currenttimer[0]; 

 
//============================================================
//     //等待   控制加温、保温等   STUS=0         
//=============================================================		
switch(STUS)
		   {

	   case 0:    //    等待
 
 		    if ((lt<=dlt))
		     {
		       ClearSreenLine(0);
		       outChinese(0x82,4,str06);//加温中
		       STUS=1;
		       Sw_on;	  
		     }

				   break;
		   case 1:    //    加温
         kloop++;	
		     if((kloop>=10))
		        {
		           if (CTempture>=currentemper[0])
			         {Sw_off;
			           outChinese(0x82,4,str07);//保温中
			           STUS=2;
			         }
		        }			
			     break;
		   case 2:     //    保温
		   	
		   	    kloop++;beeploop++;	
	        	if((kloop>=20))
	        	{
	             kloop=0;
	        	   if (CTempture>=currentemper[0])
	        		 Sw_off;
//        			 outChinese(0x82,4,str07);//保温中	
	        		else
	        		 Sw_on;	 		//开加热  			
	        
	        	  if (beeploop>=20){beeploop=0; beep_off;}//保温时喇叭响
	        	  if (beeploop>=10)beep_on;	       		   
          
             }
		   	
		       break;
		        
		   default: break;
		   	
		  }			
		  
}
//===============================================================
  return 0;

}
