#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "phases.h"
#include "secret.h"
#include "additional.h"

const char* words[WORDCOUNT]={
"abendbrot",
"abendland",
"abenteuer",
"abfuehren",
"abgefeimt",
"abgelegen",
"abgeneigt",
"absondern",
"allgemein",
"anfaenger",
"anrichten",
"apfelsine",
"apotheker",
"artigkeit",
"astronaut",
"aufenhalt",
"ausbilden",
"ausdenken",
"auskramen",
"autogramm",
"baendchen",
"kloeppeln",
"knappheit",
"raeumlich",
"rhabarber",
"schiffbar",
"sichtlich",
"singspiel",
"sitzplatz"
};

char consume(unsigned char x)
{
    for (unsigned char i=0;i<x;++i)
    {
        (void)blackBoxC();
    }
    return '0';
}

void f3(int i);
void f4(int i);
void f5(int i);
void f6(int i);
void f7(int i);

void f3(int i)
{
    mute(1);
    char c=blackBoxC();
    appendChar(c);
}

void f4(int i)
{
    char c=blackBoxC();
    appendChar(c);
    mute(1);
}

void f5(int i)
{
    char c=blackBoxC();
    appendChar(c);
    c=blackBoxC();
    appendChar(c);
    mute(1);
}

void f6(int i)
{
    char c=blackBoxC();
    (void)c;
    mute(1);
}

void f7(int i)
{
    mute(1);
}

int f(int x, int y)
{
    if ((x==0) && (y==0))
    {
	return 0;
    }
    if (y==0)
    {
	if (x>0) 
	{
	    muteFlip();
	    return f(x-2,0);
	}
	return f(0,0);
    }
    if (y>x)
    {
	return f(y,x);
    }
    if (y%2)
    {
	return (x+y)+f(y,0);
    }
    return x+f(x-1,y);
}

bool demo_1(const char* guess)
{
    clearString();
    appendString("fish");
    appendString("custard");
    return stringMatches(guess);

}

bool demo_2(const char* guess)
{
    clearString();
    mute(true);
    appendInt(1);
    mute(false);
    appendInt(2);
    appendChar(' ');
    mute(true);
    appendInt(3);
    return stringMatches(guess);
}


bool demo_3(const char* guess)
{
    clearString();
    blackBoxC();
    appendString(words[(blackBoxC()-'0')%WORDCOUNT]);
    return stringMatches(guess);
}

bool demo_4(const char* guess)
{
    for (int i=0;i<8;++i) {
        int c=blackBoxC()%WORDCOUNT;
        appendChar(words[c][i]);
    }
    return stringMatches(guess);    
}

void A7(int i);
void A6(int i);
void A5(int i);
void A4(int i);
void A3(int i);
void A2(int i);
void A1(int i);



void A7(int i)
{
    if (i%29!=0) {
        mute(true);
    }
}

void A6(int i)
{
    if (i%2==0) {
        mute(true);
    }
}

void A5(int i)
{
    A3(i);
    if (i%11==0) {
        mute(true);
    }
}

void A4(int i)
{
    A5(i);
    A6(i);
    consume(1);
}

void A3(int i)
{
    if (i%7!=0) {
        mute(true);
    }
}

void A2(int i)
{
    if (i<500) {
        clearString();
    }    
    consume(1);    
}

void A1(int i) 
{
    A2(i);
    mute(false);
    A3(i);
    A4(i);
    A7(i);
}


bool F1(void)
{
    if (blackBoxC()%3==0) {
        if (blackBoxC()%2==0) {
            return false;
        }
	return true;
    } else {
        return true;
    }
}

bool phase_quandong(const char* guess) 
{
    char buffer[50];
    memset(buffer, 0, 50);
    strcpy(buffer, words[(blackBoxC()+2)%WORDCOUNT]);
    appendChar(buffer[2]);
    appendChar(buffer[1]);
    appendChar(buffer[0]);
    appendChar(buffer[5]);
    appendChar(buffer[3]);
    appendChar(buffer[0xA]);
    appendChar(buffer[8]);
    appendChar(buffer[7]);
    appendChar(buffer[4]);
    return stringMatches(guess);
}

bool phase_illawarra_plum(const char* guess)
{
    char* buffer="abcdfeghjiklmnpoqrstvuwxyz";
    if (strlen(guess)!=9) {
        return stringMatches("nope");
    }
    for (int i=0;i<9;++i) {
        if (guess[i]<'A' || guess[i]>'Z') {
            return stringMatches("NOzzzzzzzzzzzzzzzzzzz");
        }
        appendChar(buffer[guess[i]-'A']);
    }
    return stringMatches(words[blackBoxC()%WORDCOUNT]);
}


bool phase_desert_lime(const char* guess)
{
    if (strlen(guess)!=8) {
        return stringMatches("12345678");
    }
     if (guess[0]<'I' || guess[0]>'N') {
        appendChar('B');
    } else {
        appendChar('A');
    }
    if (!isdigit(guess[1])) {
        appendChar('B');
    } else {
        appendChar('A');
    }
    if (isxdigit(guess[2]) && isdigit(guess[2])) {
        appendChar('B');
    } else {
        appendChar('A');
    }    
    if (!ispunct(guess[3])) {
        appendChar('B');
    } else {
        appendChar('A');
    }
    if (!isxdigit(guess[4]) || isdigit(guess[4])) {
        appendChar('B');
    } else {
        appendChar('A');
    }     
    if (!isxdigit(guess[2])) {
        appendChar('B');
    } else {
        appendChar('A');
    }    
    if ((guess[6]+guess[7]<60) || (guess[6]+guess[7]>63)) {
        appendChar('B');
    } else {
        appendChar('A');
    }
    return stringMatches("AAAAAAA");
}

bool phase_riberry(const char* guess)
{
    consume(5);
    char word[12];
    strcpy(word, words[blackBoxC()%WORDCOUNT]);
    for (int i=0;i<strlen(word);++i) {
        word[i]=toupper(word[i]);
    }
    char buffer[12]="abcdefghijk";
    if (strlen(guess)!=11) {
        return stringMatches("abcdefghijk");
    }
    for (int i=0;i<11;++i) {
        char q=(guess[i]=='~')?(buffer[i]-10):(buffer[i]-guess[i]);
        appendChar(q);
    }
    return stringMatches(word);
}


bool phase_finger_lime(const char* guess) {
    consume(1);
    if (strlen(guess)!=7) {
	return stringMatches("nopenon");
    }
    for (int i=0; i<7; ++i) {
        if ((guess[i]+blackBoxC())%5==0) {
            appendChar('A');
	} else {
            appendChar('B');
	}
    }
    return stringMatches("AAAAAAA");
}

bool phase_kakadu_plum(const char* guess) {
    stringCap(6);
    for (int i=0;i<1000;++i) {
    	mute(F1());
    	char c=blackBoxC();
    	appendChar(c);
    }
    return stringMatches(guess);
}

bool phase_lemon_aspen(const char* guess) 
{
    consume(99);
    const char* word=words[blackBoxC()%WORDCOUNT];
    if (strlen(guess)!=strlen(word)) {
        return stringMatches("abcdefghijklmnopqrstuvwxyz"); // no chance
    }
    for (int i=0;i<strlen(guess);++i)
    {
        consume(guess[i]);
        appendChar(blackBoxC());
    }
    return stringMatches(word);
}

bool phase_pepper_berry(const char* guess)
{
    stringCap(8);
    mute(true);
    for (int i=5;i<10005;++i) {  
        char c=blackBoxC();
        appendChar(c);
        A1(i);
    }
    return stringMatches(guess);
}

bool phase_bush_tomato(const char* guess)
{
    char c;
    c=blackBoxC();
    appendChar(c);
    mute(true);
    c=blackBoxC();
    appendChar(c);
    c=blackBoxC();
    appendChar(c);
    mute(false);
    c=blackBoxC();
    appendChar(c);
    c=blackBoxC();
    appendChar(c);
    mute(false);
    c=blackBoxC();
    appendChar(c);
    c=blackBoxC();
    appendChar(c|3);
    c=blackBoxC();
    appendChar(c|5);
    c=blackBoxC();
    appendChar(c|9);
    return stringMatches(guess);
}

// Australian native foods list from --- ``Australian native fruits: Potential uses as functional food ingredients'' (Richmond, Bowyer, Vuong)
