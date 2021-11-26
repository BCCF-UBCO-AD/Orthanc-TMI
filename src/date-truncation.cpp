#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include "date-truncation.h"
#include "nlohmann/json.hpp"
using namespace std;

int get_days_for_month(int month, int year);
bool isleap(int year);
string DateTruncation(const nlm::json &config, string value){
    bool leap;
    string year, month, day;
    // todo: change configuration format to key with tags so that different dates can be truncated easily
    std::vector<string> format = config["DateTruncation"]["dateformat"].get<vector<string>>();
    year = value.substr(0,4);
    month = value.substr(4,2);
    day = value.substr(6, 2);

    if(!(format[0] == "YYYY")){
        year = format[0];
        value.erase(0, 4);
        value = year + value;
        return value;
    }
    if (!(format[1] == "MM")){
        if(!(format[2] == "DD")){
            month = format[1];
            if(get_days_for_month(stoi(year),stoi(month)) >= stoi(format[2])){
                day = format[2];
            }
            else{
                return value;
            }
            value.erase(0,8);
            value = year + month + day;
            return value;
        }
        month = format[1];
        value.erase(0, 6);
        value = year + month + value;
        return value;
    }
    if(!(format[2] == "DD")){
        if(get_days_for_month(stoi(month),stoi(year)) >= stoi(format[2])){
            day = format[2];
        }
        else{
            return value;
        }
        value.erase(0,8);
        value = year + month + day;
        return value;
    }
    return value;
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

bool isleap(int year)
{

    if (year % 4 == 0)
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0)
                return true;
            else
                return false;
        }
        else
           return true;
    }
    else
        return false;
}
