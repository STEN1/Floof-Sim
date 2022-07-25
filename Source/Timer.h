#pragma once
#include <chrono>

namespace FLOOF {
class Timer {
public:
	Timer() 
		: m_Creation{ std::chrono::high_resolution_clock::now() }
		, m_DeltaPoint{ m_Creation } {}

	double DeltaFromCreation() {
		return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - m_Creation).count();
	}

	double Delta() {
		auto now = std::chrono::high_resolution_clock::now();
		double delta = std::chrono::duration<double>(now - m_DeltaPoint).count();
		m_DeltaPoint = now;
		return delta;
	}
private:
	std::chrono::high_resolution_clock::time_point m_Creation;
	std::chrono::high_resolution_clock::time_point m_DeltaPoint;
};
}