//
//  main.c
//  BMP180calc
//
//  Created by Christoph Gommel on 04.01.15.
//  Copyright (c) 2015 Christoph Gommel. All rights reserved.
//

#include <stdio.h>
#include <math.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    
    int16_t AC1 =   408;
    int16_t AC2 =   -72;
    int16_t AC3 =-14383;
    uint16_t AC4=32741;
    uint16_t AC5=32757;
    uint16_t AC6=23153;
    int16_t B1=   6190;
    int16_t B2=      4;
    int16_t MB= -32768;
    int16_t MC=  -8711;
    int16_t MD=   2868;
    
    uint32_t UT=27898;
    int16_t oss=0;
    
    int32_t UP=23843;
    
    
    //calculate temperature
    
    int32_t X1;
    int32_t X2;
    int32_t X3;
    
    int32_t B3;
    int32_t B4;
    int32_t B5;
    int32_t B6;
    int32_t B7;
    int32_t T;
    int32_t p;
    
    
    X1=(UT-AC6)*AC5/(1<<15);
    X2=MC*(1<<11)/(X1+MD);
    B5=(X1+X2);
    T=(B5+8)/(1<<4);
    printf("Temperature is %i.%i°C\n",T/10,T%10);
    
    //calculate true pressure;
    
    B6=B5-4000;
    X1=(B2*(B6*B6/(1<<12)))/(1<<11);
    X2=AC2*B6/(1<<11);
    X3=X1+X2;
    B3=(((AC1*4+X3)<<oss)+2)/4;
    X1=AC3*B6/(1<<13);
    X2=(B1*(B6*B6/(1<<12)))/(1<<16);
    X3=((X1+X2)+2)/(1<<2);
    B4=AC4*(uint32_t)(X3+32768)/(1<<15);
    B7=((uint32_t)UP-B3)*(50000>>oss);
    // if(B7<0x80000000)
    //     p=(B7*2)/B4;
    // else
    p=(B7/B4)*2;
    X1=(p/(1<<8))*(p/(1<<8));
    X1=(X1*3038)/(1<<16);
    X2=(-7357*p)/(1<<16);
    p=p+(X1+X2+3791)/(1<<4);
    
    printf("Pressure is %i Pa\n",p);
    
    double p0;
    double alt=125;
    p0=((double)p)/pow(1-alt/44330.0,5.255);
    printf("Assuming we are at %.1f m elevation the Pressure at sea level is %0.1f  Pa\n",alt,p0);
    
    return 0;
}
