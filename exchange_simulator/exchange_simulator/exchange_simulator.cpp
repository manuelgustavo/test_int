// exchangesimulator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "IExchange.hpp"
#include "Exchange.hpp"

#include <memory>

using namespace std;

int main()
{
	unique_ptr<IExchange> exch = make_unique<Exchange>();
	return 0;
}

