#include "testlib.h"
using namespace std;
#include<iostream>
#include<iomanip>
#include<vector>

namespace logger {
inline void json_() {}
template<typename Key, typename Value, typename... Rest>
void json_(const Key& key, const Value& value, const Rest&... rest)
{
  std::cout << "\"" << key << "\":\"" << value << "\"";
  if (sizeof...(Rest) > 0) std::cout << ",";
  json_(rest...);
}

// example : json("key1", "foo", "key2", 3, "key", 4.0);
// {"key1":"foo","key2":"3","key":"4"}
template<typename... Args>
void json(const Args&... args)
{
  std::cout << "{"; json_(args...); std::cout << "}\n";
}
} // namespace logger

int main(int argc, char* argv[])
{
	setName("marathon match 115");
	registerTestlibCmd(argc, argv);

	try {
		int N = inf.readInt();
		inf.readEoln();
		double C = inf.readDouble();
		inf.readEoln();
		int K = inf.readInt();
		inf.readEoln();
		int NumPaths = inf.readInt();
		inf.readEoln();
		for (int i = 0; i < NumPaths; ++i) {
			string path = inf.readString();
		}
		inf.readEof();

		int N_ = ouf.readInt();
		ouf.readEoln();
		vector<vector<int> > mout(N, vector<int>(N));
		for (int i = 0; i < N; i++) {
			string path = ouf.readString();
			for (int j = 0; j < N; ++j) {
				mout[i][j] = path[j]-'0';
			}
		}

		N_ = ans.readInt();
		ans.readEoln();
		vector<vector<int> > mans(N, vector<int>(N));
		for (int i = 0; i < N; i++) {
			string path = ans.readString();
			for (int j = 0; j < N; ++j) {
				mans[i][j] = path[j]-'0';
			}
		}

		int TP = 0, FP = 0, FN = 0, TN = 0;
		for (int i = 0; i < N; i++) {
			for (int j = i+1; j < N; ++j) {
				if (mout[i][j]) {
					if (mans[i][j]) TP++;
					else FP++;
				}
				else {
					if (mans[i][j]) FN++;
					else TN++;
				}
			}
		}
    double Precision = (TP+FP==0 ? 0 : TP*1.0/(TP+FP));
    double Recall = (TP+FN==0 ? 0 : TP*1.0/(TP+FN));
    //compute the F1 score
    double Score = (Precision + Recall < 1e-9 ? 0 : (2 * Precision * Recall)/(Precision + Recall));
		cout << setprecision(10);
		logger::json("status", "AC", "score", Score, "TP", TP, "FP", FP, "Precision", Precision, "Recall", Recall);

		quitf(_ok, "Correct");
	}
	catch (char* str) {
		logger::json("status", "WA", "message", str, "score", "-1");
		// cerr << "error: " << str << endl;
		quitf(_wa, "Something error occured.");
	}
}
