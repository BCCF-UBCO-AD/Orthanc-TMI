#include <string>
using namespace std;

inline int get_days_for_month(int month, int year);
inline bool isleap(int year);

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

inline int get_days_for_month(int month, int year){
    static int months[] = {
            0,  //NULL
            31, //January
            28, //February
            31, //March
            30, //April
            31, //May
            30, //June
            31, //July
            31, //August
            30, //September
            31, //October
            30, //November
            31  //December
    };
    if(month < 0 || month > 12){
        return 0;
    }
    if(month == 2 && isleap(year)){
        // February in a leap year
        return 29;
    }
    return months[month];
}

inline bool isleap(int year) {
    if (year % 4 == 0) {
        if (year % 100 == 0) {
            if (year % 400 == 0) {
                return true;
            }
        }
    }
    return false;
}
