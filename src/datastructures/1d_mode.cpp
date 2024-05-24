#pragma once

#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <algorithm>

typedef unsigned int Color;
typedef unsigned int uint;
typedef std::vector<std::vector<uint>> QTy;
typedef std::vector<Color> ATy;
typedef std::vector<Color> APrimeTy;
typedef std::vector<std::vector<Color>> STy;
typedef std::vector<std::vector<uint>> SPrimeTy;
typedef struct { STy S; SPrimeTy SPrime; } S_S_Prime;
typedef struct { Color color; uint frequency; } Mode;

namespace DS {
	static ATy generate_Colors(int count, int distinct_Colors) {
		ATy Colors = ATy(count);
		unsigned int re_seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::uniform_real_distribution<double> rnd(0, distinct_Colors);
		std::default_random_engine re(re_seed);

		for (int i = 0; i < count; i++) {
			Colors[i] = rnd(re) + 1;
		}

		return Colors;
	}

	static QTy generate_Q(ATy A) {
		auto count = (uint)(*std::max_element(A.begin(), A.end()));

		QTy Q = {};
		for (int i = 0; i < count; i++) {
			Q.push_back({});
		}

		uint n = (uint)A.size();
		for (uint i = 0; i < n; i++) {
			Q[(uint)A[i] - 1].push_back(i);
		}

		return Q;
	}

	static APrimeTy generate_A_prime(ATy A, QTy Q) {
		APrimeTy APrime = APrimeTy(A.size());
		for (uint i = 0; i < Q.size(); i++) {
			for (int j = 0; j < Q[i].size(); j++) {
				APrime[Q[i][j]] = j;
			}
		}
		return APrime;
	}

	static S_S_Prime generate_S_S_Prime(ATy A, uint s) {
		uint t = std::ceil((double)A.size() / (double)s);
		
		// Initialize S and SPrime with empty vectors
		STy S = STy(s);
		SPrimeTy SPrime = SPrimeTy(s);
		for (int i = 0; i < s; i++) {
			S[i] = {};
			SPrime[i] = {};
		}

		// Generate S
		for (int i = 0; i < s; i++) {
			for (int j = i; j < s; j++) {
				uint start = i * t; // no +1, arrays are 0 indexed;
				uint end = std::min((j + 1) * t, (uint)A.size()); // exclusive end index

				Mode mode = get_mode_naive(A, start, end);
				S[i].push_back(mode.color);
				SPrime[i].push_back(mode.frequency);
			}
		}
	}

	static Mode get_mode_naive(ATy A, uint start, uint end) {
		// Simple counting approach; Could be made more efficient, but eh :shrug:
		auto num_colors = (uint)(*std::max_element(&A[start], &A[end]));
		uint* counts = new uint[num_colors];
		uint N = sizeof(counts) / sizeof(uint);

		for (int i = start; i < end; i++) {
			counts[A[i]]++;
		}
		
		// then, find max index
		auto max = std::max_element(counts, counts + N);
		auto max_color = (uint)std::distance(counts, max);

		return { max_color, *max };
	}
}