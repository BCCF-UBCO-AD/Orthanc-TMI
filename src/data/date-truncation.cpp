#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "date-truncation.h"
using namespace std;

int get_days_for_month(int month, int year);
bool isleap(int year);
std::string TruncateDate(std::string date, std::string format) {
    if (format.length() != 8 || date.length() != 8) {
        return date;
    }
    for (int i = 0; char c: format) {
        if (std::isdigit(c)) {
            date.replace(i, 1, 1, c);
        }
        i++;
    }
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(4, 2));
    int day = std::stoi(date.substr(6, 2));
    month = std::max(std::min(month, 12), 0);
    day = std::max(std::min(day, get_days_for_month(month, year)), 0);
    char truncated[24] = {0};
    sprintf(truncated,"%d%02d%02d",year,month,day);
    return truncated;
}

int get_days_for_month(int month, int year){
    bool leap = isleap(year);

    switch(month){
        case 1: //January
            return 31;
        case 2: //February
            return leap ? 29 : 28;
        case 3: //March
            return 31;
        case 4: //April
            return 30;
        case 5: //May
            return 31;
        case 6: //June
            return 30;
        case 7: //July
            return 31;
        case 8: //August
            return 31;
        case 9: //September
            return 30;
        case 10: //October
            return 31;
        case 11: //November
            return 30;
        case 12: //December
            return 31;
        default:
            return 0;
    }
}

bool isleap(int year) {
    if (year % 4 == 0) {
        if (year % 100 == 0) {
            if (year % 400 == 0) {
                return true;
            }
        }
    }
    return false;
}
