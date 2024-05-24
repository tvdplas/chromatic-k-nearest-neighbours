// Based on DOI 10.1007/s00224-013-9455-2

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

		return { S, SPrime };
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

	// inclusive start, exclusive end
	static Mode get_mode(ATy A, APrimeTy APrime, QTy Q, STy S, SPrimeTy SPrime, uint start, uint end, uint s) {
		uint t = std::ceil((double)A.size() / (double)s);
		// no need for -1 in bi due to 0 indexing, no change to bj due to exclusive
		uint bi = std::ceil((double)start / (double)t);
		uint bj = std::floor((double)end / (double)t) - 1;
		
		// Span: A[bi*t : bj * t)
		// prefix: A[start : min(bi*t, end))
		// suffix: A[max((bj + 1)*t, start) : end)

		// Best color from span
		Color candidate_mode = S[bi][bj];
		uint candidate_frequency = SPrime[bi][bj];

		// Now check values from prefix/postfix by lemma 2
		// prefix
		for (uint i = start; i < std::min(bi * t, end); i++) {
			if (APrime[i] > 1 && Q[A[i]][APrime[i] - 1] >= start) {
				// entry was already counted
				continue;
			}
			// < instead of <= for end due to exclusive
			if (!(APrime[i] + candidate_frequency - 1 < Q[A[i]].size() && Q[A[i]][APrime[i] + candidate_frequency - 1] < end)) {
				// entry must have lower frequency than current candidate
				continue;
			}
			
			// linear scan in Q[A[i]] in order to find the new frequency
			uint y = APrime[i] + candidate_frequency - 1;
			while (y < Q[A[i]].size() && Q[A[i]][y] < end) {
				y++;
			}
			candidate_mode = A[i];
			candidate_frequency = y - APrime[i];
		}

		// exact same for suffix; code is copy pasted for i am lazy
		for (uint i = std::max((bj + 1) * t, start); i < end; i++) {
			if (APrime[i] > 1 && Q[A[i]][APrime[i] - 1] >= start) {
				// entry was already counted
				continue;
			}
			// < instead of <= for end due to exclusive
			if (!(APrime[i] + candidate_frequency - 1 < Q[A[i]].size() && Q[A[i]][APrime[i] + candidate_frequency - 1] < end)) {
				// entry must have lower frequency than current candidate
				continue;
			}

			// linear scan in Q[A[i]] in order to find the new frequency
			uint y = APrime[i] + candidate_frequency - 1;
			while (y < Q[A[i]].size() && Q[A[i]][y] < end) {
				y++;
			}
			candidate_mode = A[i];
			candidate_frequency = y - APrime[i];
		}

		// prefix and suffix done, therefore candidate must now be final
		return { candidate_mode, candidate_frequency };
	}
}