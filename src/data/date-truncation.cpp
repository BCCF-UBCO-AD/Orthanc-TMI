#include <fstream>
#include <string>
#include <vector>
#include "date-truncation.h"
using namespace std;

int get_days_for_month(int month, int year);
bool isleap(int year);
string TruncateDate(string date, const char* config){
    if(date.length() != 8){
        return date;
    }
    string year, month, day;
    string year_config,month_config,day_config;

    // todo: change configuration format to key with tags so that different dates can be truncated easily

    year = date.substr(0, 4);
    month = date.substr(4, 2);
    day = date.substr(6, 2);

    year_config = basic_string(config).substr(0,4);
    month_config = basic_string(config).substr(4,2);
    day_config = basic_string(config).substr(6, 2);

    if(!(year_config == "YYYY")){
        year = year_config;
        date.erase(0, 4);
        date = year + date;
        return date;
    }
    if (!(month_config == "MM")){
        if(!(day_config == "DD")){
            month = month_config;
            if(get_days_for_month(stoi(year),stoi(month)) >= stoi(day_config)){
                day = day_config;
            }
            else{
                return date;
            }
            date.erase(0, 8);
            date = year + month + day;
            return date;
        }
        month = month_config;
        date.erase(0, 6);
        date = year + month + date;
        return date;
    }
    if(!(day_config == "DD")){
        if(get_days_for_month(stoi(month),stoi(year)) >= stoi(day_config)){
            day = day_config;
        }
        else{
            return date;
        }
        date.erase(0, 8);
        date = year + month + day;
        return date;
    }
    return date;
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
