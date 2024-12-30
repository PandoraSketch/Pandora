#include "Pandora.hpp"
#include "adaptor.hpp"
#include <unordered_map>
#include <math.h>
#include <bits/stdc++.h>
#include <utility>
#include <iomanip>
#include "datatypes.hpp"
#include "util.h"

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

		// Evaluate accuracy
		error = 0;
		int tp = 0, cnt = 0;
		aae = 0;
		for (auto it = ground.begin(); it != ground.end(); it++)
		{

			int flag = 0;
			if (it->second > threshold)
			{
				cnt++;
				for (auto res = results.begin(); res != results.end(); res++)
				{
					if (memcmp(it->first.key, res->first.key, sizeof(res->first.key)) == 0)
					{
						double hh = res->second > it->second ? res->second - it->second : it->second - res->second;
						flag = 1;
						sumerror += (int)hh;
						error = hh * 1.0 / it->second + error;
						aae += hh;

						tp++;
					}
				}
			}
		}

		precision = tp * 1.0 / results.size();
		recall = tp * 1.0 / cnt;
		error = error / tp;
		aae = aae * 1.0 / tp;

		avpre += precision;
		avrec += recall;
		averr += error;
		avthr += throughput;
		avdet += detectime;
		averaae += aae;
		delete pandora;
		delete adaptor;

		numfile++;
		std::cout << std::setfill(' ');
		std::cout << std::setw(20) << std::left << "Sketch" << std::setw(20) << std::left << "Precision"
				  << std::setw(20) << std::left << "Recall" << std::setw(20)
				  << std::left << "Relative Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
				  << std::left << "Detection Time" << std::endl;
		std::cout << std::setw(20) << "Pandora"
				  << std::setw(20) << std::left << precision << std::setw(20) << std::left << recall << std::setw(20)
				  << std::left << error << std::setw(20) << std::left << throughput << std::setw(20)
				  << std::left << detectime << std::endl;
	}

	// Output to standard output
	std::cout << "---------------------------------  summary ---------------------------------" << std::endl;
	std::cout << std::setfill(' ');
	std::cout << std::setw(20) << std::left << "Sketch" << std::setw(20) << std::left << "Precision"
			  << std::setw(20) << std::left << "Recall" << std::setw(20)
			  << std::left << "Relative Error" << std::setw(20) << std::left << "Throughput" << std::setw(20)
			  << std::left << "Detection Time" << std::endl;
	std::cout << std::setw(20) << "Pandora" << std::setw(20) << std::left << avpre / numfile << std::setw(20) << std::left << avrec / numfile << std::setw(20)
			  << std::left << averr / numfile << std::setw(20) << std::left << avthr / numfile << std::setw(20)
			  << std::left << avdet / numfile << std::endl;
}
