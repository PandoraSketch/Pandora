#include "Pandora.hpp"
#include "adaptor.hpp"
#include <cstdlib>
#include <unordered_map>
#include <math.h>
#include <bits/stdc++.h>
#include <utility>
#include <iomanip>
#include "datatypes.hpp"
#include "util.h"

bool sortbysec(const std::pair<key_tp, val_tp> &a,
			   const std::pair<key_tp, val_tp> &b)
{
	return (a.second > b.second);
}

int main(int argc, char *argv[])
{

	int memory_size;
	std::cout << "Please enter the memory size (KB): ";
	std::cin >> memory_size;

	double aae = 0;

	int sketch_width = memory_size * 1024 / (13 * 4);

	int sumerror = 0;

	const char *filenames = "iptraces.txt";
	unsigned long long buf_size = 5000000000;

	double thresh = 0.5;

	// pandora sketch parameters
	int pandora_width = sketch_width;
	int pandora_depth = 4;

	// evaluation
	std::vector<std::pair<key_tp, val_tp>> results;
	int numfile = 0;
	double precision = 0, recall = 0, error = 0, throughput = 0, detectime = 0;
	double avpre = 0, avrec = 0, averr = 0, avthr = 0, avdet = 0, averaae = 0;

	std::ifstream tracefiles(filenames);
	if (!tracefiles.is_open())
	{
		std::cout << "Error opening file" << std::endl;
		return -1;
	}

	for (std::string file; getline(tracefiles, file);)
	{

		// load traces
		Adaptor *adaptor = new Adaptor(file, buf_size);
		std::cout << "[Dataset]: " << file << std::endl;
		std::cout << "[Message] Finish read data." << std::endl;

		adaptor->Reset();
		mymap ground;
		mymap ground2;
		val_tp sum = 0;
		val_tp epoch = 0;
		val_tp window_size = 1600;
		val_tp LENGTH = 0;
		tuple_t t;
		while (adaptor->GetNext(&t) == 1)
		{
			sum++;
		}
		std::cout << "[Message] Finish Insert hash table" << std::endl;
		LENGTH = ceil((sum - 1) / window_size);
		adaptor->Reset();
		memset(&t, 0, sizeof(tuple_t));
		while (adaptor->GetNext(&t) == 1)
		{
			key_tp key;
			memcpy(key.key, &(t.key), LGN);
			epoch = epoch + 1;
			if ((epoch) % LENGTH == 0)
			{
				for (auto &item : ground2)
				{
					ground[item.first] += 1;
				}
				ground2.clear();
			}

			else
			{
				ground2[key] = 1;
			}
		}
		val_tp threshold = thresh * window_size;

		// ground truth map to ground truth vector
		std::vector<std::pair<key_tp, val_tp>> v_ground;
		for (auto it = ground.begin(); it != ground.end(); it++)
		{
			std::pair<key_tp, val_tp> node;
			node.first = it->first;
			node.second = it->second;
			v_ground.push_back(node);
		}
		// sort gt vector according to persistence
		std::sort(v_ground.begin(), v_ground.end(), sortbysec);

		// Create sketch
		Pandora *pandora = new Pandora(pandora_depth, pandora_width, 8 * LGN);
		// Update sketch
		uint64_t t1 = 0, t2 = 0;
		adaptor->Reset();
		memset(&t, 0, sizeof(tuple_t));
		int number = 0;
		t1 = now_us();
		while (adaptor->GetNext(&t) == 1)
		{
			++number;
			if (number % LENGTH == 0)
			{
				pandora->Inactivity();
				pandora->NewWindow();
			}
			pandora->Update((unsigned char *)&(t.key), 1);
		}
		t2 = now_us();
		throughput = adaptor->GetDataSize() / (double)(t2 - t1) * 1000000000;
		std::cout << "time = " << (double)(t2 - t1) * 1000000000 << std::endl;

		// Query sketch
		results.clear();
		t1 = now_us();
		pandora->Query(threshold, results);
		t2 = now_us();
		detectime = (double)(t2 - t1) / 1000000000;

		int ae = 0, cnt = 0;
		float re = 0;
		for (auto it = v_ground.begin(); it != v_ground.end(); it++)
		{
			cnt++;
			ae += abs((int)pandora->QueryL2(it->first.key) - (int)it->second);
			re += (float)abs((int)pandora->QueryL2(it->first.key) - (int)it->second) / (float)it->second;
		}
		float aae = (float)ae / (float)cnt;
		float are = (float)re / (float)cnt;
		std::cout << "AAE: " << aae << std::endl;
		std::cout << "ARE: " << are << std::endl;
	}
}