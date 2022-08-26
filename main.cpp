#include "JSONDecoder.h"
#include "Memfile.h"
#include <string>

using namespace gnilk;

bool t_jsnew_basic() {
    std::string basic = "{ \"number\" : 10, \"myobject\" : { \"str\" : \"this is a string\" } }";
    Memfile mf(basic);
    auto callback = [](const char *object, const char *label, const char *value) {
        // The root object has length zero (0)
        if (strlen(object) > 0) {
            printf("%s : %s = '%s'\n", object, label, value);
        } else {
            printf("ROOT: '%s' = '%s'\n", label, value);
        }
    };

    JSONDecoder jsNew(&mf,callback);
    return jsNew.ProcessData();
}

int main(int argc, char **argv) {
    if (!t_jsnew_basic()) {
        printf("jsnew, failed\n");
    }
	return 0;
}