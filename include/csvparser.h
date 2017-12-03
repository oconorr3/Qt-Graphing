#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <vector>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>

class CsvParser {
    private:
        CsvParser();
    public:
        static std::vector<std::vector<std::string>> parseCSV(char *csvPath);
};

#endif // CSVPARSER_H
