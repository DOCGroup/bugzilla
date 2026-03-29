#include <ace/Time_Value.h>
#include <iostream>

class MyClass {};

int main (int argc, char* argv[])
{
	ACE_Time_Value tv (0);
	if (tv == 0)
		std::cout << "ACE_Time_Value(0) == 0" << std::endl;
	if (tv < 0)
		std::cout << "ACE_Time_Value(0) < 0" << std::endl;
	if (tv > 0)
	{
		// This is what gets printed.
		// if ((const timeval*) tv > (const timeval*) 0))
		std::cout << "ACE_Time_Value(0) > 0" << std::endl;
	}

	MyClass m;
	//if (m == 0)
	{
		/*
The comparing custom struct (correctly) does not compile.
However as a candidate operator Time_Value::operator== is listed.
test.cpp:22: error: no match for 'operator==' in 'm == 0'
opt/ace-5.8.2/ace/Time_Value.inl:322: note: candidates are: bool ACE_5_8_2::operator==(const ACE_5_8_2::ACE_Time_Value&, const ACE_5_8_2::ACE_Time_Value&)
		*/
	}
	return 0;
}
