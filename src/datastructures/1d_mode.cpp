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
typedef struct { 
	ATy A; 
	APrimeTy APrime; 
	QTy Q;
	STy S;
	SPrimeTy SPrime;
	uint s;
} PreprocessedData;


namespace DS {
	static Mode get_mode_naive(ATy A, uint start, uint end) {
		// Simple counting approach; Could be made more efficient, but eh :shrug:
		auto num_colors = (uint)(*std::max_element(A.begin() + start, A.begin() + end));
		std::vector<uint> counts(num_colors + 1);

		for (uint i = start; i < end; i++) {
			counts[A[i]]++;
		}

		// then, find max index
		auto max = std::max_element(counts.begin(), counts.end());
		auto max_color = (uint)std::distance(counts.begin(), max);

		return { max_color, *max };
	}

	static ATy generate_colors(int count, int distinct_colors) {
		ATy Colors = ATy(count);
		unsigned int re_seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::uniform_real_distribution<double> rnd(0, distinct_colors);
		std::default_random_engine re(re_seed);

		for (int i = 0; i < count; i++) {
			Colors[i] = rnd(re);
		}

		return Colors;
	}

	static QTy generate_Q(ATy A) {
		auto count = (uint)(*std::max_element(A.begin(), A.end())) + 1;

		QTy Q = {};
		for (int i = 0; i < count; i++) {
			Q.push_back({});
		}

		uint n = (uint)A.size();
		for (uint i = 0; i < n; i++) {
			Q[(uint)A[i]].push_back(i);
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
		uint t = (uint)std::ceil((double)A.size() / (double)s);
		
		// Initialize S and SPrime with empty vectors
		STy S = STy(s);
		SPrimeTy SPrime = SPrimeTy(s);
		for (uint i = 0; i < s; i++) {
			S[i] = {};
			SPrime[i] = {};
			for (uint j = 0; j < s; j++) {
				S[i].push_back({});
				SPrime[i].push_back({});
			}
		}

		// Generate S
		for (uint i = 0; i < s; i++) {
			for (uint j = i; j < s; j++) {
				uint start = i * t; // no +1, arrays are 0 indexed;
				uint end = std::min((j + 1) * t, (uint)A.size()); // exclusive end index

				Mode mode = DS::get_mode_naive(A, start, end);
				S[i][j] = mode.color;
				SPrime[i][j] = mode.frequency;
			}
		}

		return { S, SPrime };
	}

	// inclusive start, exclusive end
	static Color get_mode(PreprocessedData* pre, uint start, uint end) {
		uint t = (uint)std::ceil((double)pre->A.size() / (double)pre->s);
		// no need for -1 in bi due to 0 indexing, no change to bj due to exclusive
		uint bi = (uint)std::ceil((double)start / (double)t);
		uint bj = (uint)std::floor((double)end / (double)t) - 1;
		
		// Span: A[bi*t : bj * t)
		// prefix: A[start : min(bi*t, end))
		// suffix: A[max((bj + 1)*t, start) : end)

		// Best color from span
		Color candidate_mode = (Color)pre->S[bi][bj];
		uint candidate_frequency = pre->SPrime[bi][bj];

		// Now check values from prefix/postfix by lemma 2
		// prefix
		for (uint i = start; i < std::min(bi * t, end); i++) {
			if (pre->APrime[i] > 1 && pre->Q[pre->A[i]][pre->APrime[i] - 1] >= start) {
				// entry was already counted
				continue;
			}
			// < instead of <= for end due to exclusive
			if (!(pre->APrime[i] + candidate_frequency - 1 < pre->Q[pre->A[i]].size() && pre->Q[pre->A[i]][pre->APrime[i] + candidate_frequency - 1] < end)) {
				// entry must have lower frequency than current candidate
				continue;
			}
			
			// linear scan in Q[A[i]] in order to find the new frequency
			uint y = pre->APrime[i] + candidate_frequency - 1;
			while (y < pre->Q[pre->A[i]].size() && pre->Q[pre->A[i]][y] < end) {
				y++;
			}
			candidate_mode = (Color)pre->A[i];
			candidate_frequency = y - pre->APrime[i];
		}

		// exact same for suffix; code is copy pasted for i am lazy
		for (uint i = std::max((bj + 1) * t, start); i < end; i++) {
			if (pre->APrime[i] > 1 && pre->Q[pre->A[i]][pre->APrime[i] - 1] >= start) {
				// entry was already counted
				continue;
			}
			// < instead of <= for end due to exclusive
			if (!(pre->APrime[i] + candidate_frequency - 1 < pre->Q[pre->A[i]].size() && pre->Q[pre->A[i]][pre->APrime[i] + candidate_frequency - 1] < end)) {
				// entry must have lower frequency than current candidate
				continue;
			}

			// linear scan in Q[A[i]] in order to find the new frequency
			uint y = pre->APrime[i] + candidate_frequency - 1;
			while (y < pre->Q[pre->A[i]].size() && pre->Q[pre->A[i]][y] < end) {
				y++;
			}
			candidate_mode = (Color)pre->A[i];
			candidate_frequency = y - pre->APrime[i];
		}

		// prefix and suffix done, therefore candidate must now be final
		return candidate_mode;
	}

	static PreprocessedData* preprocess(ATy A) {
		PreprocessedData* data = (PreprocessedData*)malloc(sizeof(PreprocessedData*));
		data = new PreprocessedData();

		uint s = (uint)std::sqrt(A.size());
		auto Q = generate_Q(A);
		auto APrime = generate_A_prime(A, Q);
		auto S_S_Prime = generate_S_S_Prime(A, s);

		data->A = A;
		data->APrime = APrime;
		data->Q = Q;
		data->S = S_S_Prime.S;
		data->SPrime = S_S_Prime.SPrime;
		data->s = s;

		return data;
	}

}