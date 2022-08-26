#include "JSONDecoder.h"
#include "Memfile.h"
#include <string>

using namespace gnilk;

bool t_jsnew_basic() {
    std::string basic = "{ \"number\" : 10 }";
    Memfile mf(basic);
    JSONDecoder jsNew(&mf,nullptr);
    jsNew.ProcessData();
    return true;
}

int main(int argc, char **argv) {
    if (!t_jsnew_basic()) {
        printf("jsnew, failed\n");
    }
	return 0;
}