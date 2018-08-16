#include "Timer.hpp"

map<string, map<string, chrono::steady_clock::time_point>> Timer::mTable;
map<string, map<string, int>> Timer::mDuration;

void Timer::AddCategory(string str) {
	mTable[str] = map<string, chrono::steady_clock::time_point>();
	mDuration[str] = map<string, int>();
}

void Timer::StartTiming(const string cat, const string str) {
	if (!mTable.count(cat))
		AddCategory(cat);

	auto& iter = mTable[cat];
	iter[str] = chrono::steady_clock::now();
}

void Timer::StopTiming(const string cat, const string str) {
	if (!mTable.count(cat) || !mTable[cat].count(str))
		return;

	auto t = chrono::steady_clock::now();
	auto dt = chrono::duration_cast<chrono::microseconds>(t - mTable[cat][str]);

	mDuration[cat][str] = dt.count();
}

void Timer::PrintTiming() {

	int counter = 1;
	map<string, map<string, int>>::iterator iter = mDuration.begin();
	map<string, map<string, int>>::iterator lend = mDuration.end();

	cout << "------------------------------------------" << endl;
	cout << "Timer Readings: " << endl;
	for (; iter != lend; ++iter) {

		int total_time = 0;
		int inner_counter = 1;

		map<string, int>::iterator inner_iter = mDuration[iter->first].begin();
		map<string, int>::iterator inner_lend = mDuration[iter->first].end();
		for (; inner_iter != inner_lend; ++inner_iter) {
			total_time += inner_iter->second;
		}
		cout << counter++
			 << ". " << iter->first << " : "
			 << total_time << " us" << endl;

		inner_iter = mDuration[iter->first].begin();
		for (; inner_iter != inner_lend; ++inner_iter) {
			cout << "\t"
				 << inner_counter++ << ". "
				 << inner_iter->first
			     << " : "
			     << inner_iter->second
			     << " us" << endl;
		}
	}
}