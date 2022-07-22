#include "Utils.h"

#include <iostream>

void SuperUsefullFunction() {
	// create function that displays all combinations of three different digits
	// in ascending order.
	// 01 02 03 04 05 06 07 08 09
	// 12 13 14 15 16 17 18 19
	// 23 24 25 26 27 28 29
	// 34 35 36 37 38 39
	for (char a = '0'; a <= '9'; a++) {
		for (char b = a + 1; b <= '9'; b++) {
			for (char c = b + 1; c <= '9'; c++) {
				std::cout << a << b << c << ' ';
			}
		}
		std::cout << '\n';
	}
}