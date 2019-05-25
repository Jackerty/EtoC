#include"ABull.h"
#include<stdint.h>
#include<vector>

/***********************
* CAN I BE FULL TEXT.  *
***********************/
#define BULL 600

template<typename T>
struct X0 {
	T getValue(T arg) { return arg; }
};
/******************
* Description 1   *
******************/
template<int I>
struct X1;

template<int I>
struct X2;

template<int I>
struct X3;

template<template<int I> class>
struct X4;

template<template<long> class>
struct X5;
// Comment 1
template<typename>
struct X6;

// Comment 2

// Comment 3

extern X0<int> *x0i;
extern X0<long> *x0l;
extern X0<float> *x0r;

template<>
struct X0<char> {
	int member;
	char getValue(char ch) { return static_cast<char>(member); }
};

template<>
struct X0<wchar_t> {
	int member;
};

typedef X0<wchar_t> X0_wchar;

class UberX{
	public:
		std::vector<int> vecX;
		const static int QueerMultiplier=023;
	private:
		;
	protected:
		;
};
