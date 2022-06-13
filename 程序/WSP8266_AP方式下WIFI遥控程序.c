/********************************************************************/ 
#include <REGX51.H>
#define uint unsigned int 
#define uchar unsigned char   
sbit LED0=P2^0;  	//红灯
sbit LED1=P2^3;		//黄灯
sbit LED2=P2^6;	    //绿灯

/*****************相关变量**************/ 
uchar Receive,i,qj,yz,zz,ht;                              
uint n;  
uchar Recive_table[40];  		//用于接收wifi模块反馈到MCU上的数据  

/******************************************************************* 
名称：延时函数  作用：毫秒级延时，微妙级延时函数，为数据收发完成作等待.......  
********************************************************************/ 
void ms_delay(uint t) 
{  
	uint i,j;  
	for(i=t;i>0;i--)   
	for(j=110;j>0;j--); 
}   

void us_delay(uchar t) 
{  
	while(t--); 
}         

void Uart_Init()				//使用定时器1作为波特率发生器（STC89C52、STC89C51、AT89C51等均可） 
{  
    TMOD = 0x20;
    SCON = 0x50;		 		//设置串行方式
    TH1 = 0xFD;			 		//波特率9600
    TL1 = TH1;
    PCON = 0x00;
    EA = 1;						//总中断打开 
    ES = 1;						//开串口中断  
    TR1 = 1;					//启动定时器1
} 

/******************************************************************** 
名称：串口发送函数  功能：MCU向无线WIFI模块ESP8266发送数据  
********************************************************************/ 
void Send_Uart(uchar value) 
{  
	ES=0;  						//关闭串口中断  
	TI=0;   					//清发送完毕中断请求标志位   
	SBUF=value; 				//发送  
	while(TI==0); 				//等待发送完毕   
	TI=0;   					//清发送完毕中断请求标志位   
	ES=1;  						//允许串口中断  
}  

/******************************************************************** 
名称：WIFI模块设置函数  作用: 启动模块，以便可以实现无线接入和控制  
********************************************************************/ 

void ESP8266_Set(uchar *puf) 	// 数组指针*puf指向字符串数组                
{    
	while(*puf!='\0')    		//遇到空格跳出循环  
	{   
		Send_Uart(*puf);  		//向WIFI模块发送控制指令。   
		us_delay(5);   
		puf++;    
	}  
	us_delay(5);  
	Send_Uart('\r'); 			//回车  
	us_delay(5);  
	Send_Uart('\n');   			//换行  
} 

/******************************************************************** 
名称：主函数  作用：程序的执行入口  
********************************************************************/ 

void main() 
{  	
	Uart_Init();											//波特率发生器 
	ms_delay(2000);
	ESP8266_Set("AT+CWMODE=2"); 							//设置路由器模式1 station,模式2 AP,模式3 station+AP混合模式   
	ms_delay(2000);
//	ESP8266_Set("AT+RST"); 									//重新启动wifi模块            
//  ms_delay(2000);
	ESP8266_Set("AT+CWSAP=\"wifi_yuan\",\"123456789\",11,4");  	//AT+CWSAP="wifi_yuan","123456789",11,4  设置模块SSID:WIFI, PWD:密码 及安全类型加密模式（WPA2-PSK） 
	ms_delay(2000);
	ESP8266_Set("AT+CIPMUX=1");								//开启多连接模式，允许多个各客户端接入 
	ms_delay(2000);
	ESP8266_Set("AT+CIPSERVER=1,5000");  					//启动TCP/IP 实现基于网络//控制 	ESP8266_Set("AT+CIPSERVER=1,5000");  
	ms_delay(2000);
	ESP8266_Set("AT+CIPSTO=0"); 							//永远不超时
	ES=1;           										//允许串口中断 		
	qj=1;
	zz=1;
	yz=1;
	ht=1;
	LED0=1;
	LED1=1;
	LED2=1;
	while(1)   
	{    
		if((Recive_table[0]=='+')&&(Recive_table[1]=='I')&&(Recive_table[2]=='P'))//MCU接收到的数据为+IPD时进入判断控制0\1来使小灯亮与灭    
		{           
			if((Recive_table[9]=='G')&&(Recive_table[10]=='P'))            
			{      
				if(Recive_table[15]=='0')      
				{       
					LED0=0; 			//红灯亮			  
				}                                
				else 
				if (Recive_table[15]=='1')      
				{              
					LED0=1; 			//红灯灭
				} 
				else 
				if (Recive_table[15]=='2')      
				{              
                    LED1=0;            //黄灯亮           
				} 
				else 
				if (Recive_table[15]=='3')      
				{              
	                LED1=1; 		   //黄灯灭
				}
				else 
				if (Recive_table[15]=='4')      
				{              
	                LED2=0;       		//绿灯亮
				}  
				else 
				if (Recive_table[15]=='5')      
				{              
	                LED2=1;       		//绿灯灭
				}  								 											  
			}  
		}
  	}    
}    

/*********************************************************************  
名称：串行通讯中断  作用：发送或接收结束后进入该函数，对相应的标志位软件清0，实现模块对数据正常的收发。  
********************************************************************/ 

void Uart_Interrupt() interrupt 4         
{    
	static uchar i=0;  
	if(RI==1)  
	{   
		RI=0;   
		Receive=SBUF;        						//MCU接收wifi模块反馈回来的数据       
		Recive_table[i]=Receive;   
		if((Recive_table[i]=='\n'))
		{		     
			i=0;
		}    
		else i++;  										//遇到换行 重新装值  
	}   
	else TI=0;   
}  