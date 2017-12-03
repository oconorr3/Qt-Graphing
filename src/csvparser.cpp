#include "../include/csvparser.h"

std::vector<std::vector<std::string>> CsvParser::parseCSV(char *csvPath) {

    std::ifstream file(csvPath);
    std::vector<std::vector<std::string>>   result;
    std::string line;

    //parse csv row by row
    while(std::getline(file,line))
    {
        std::vector<std::string> row;
        std::stringstream  lineStream(line);
        std::string        cell;

        while(std::getline(lineStream,cell,','))
        {
            row.push_back(cell);
        }
        result.push_back(row);
    }

    //print it
//    for (int i = 0; i < (int)result.size(); i++) {
//        for (int k = 0; k < (int)result[i].size(); k++) {
//            std::cout << result[i][k] << "\t";
//        }
//        std::cout << "\n";
//    }

    return result;
}
