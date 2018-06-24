#include "stdafx.h"
#include <iostream>
#include <utility>
using Point2D = std::pair<int, int>;

int main() {
	Point2D p1;
	using namespace std;
	cout << sizeof(p1) << endl;
	system("pause");
}