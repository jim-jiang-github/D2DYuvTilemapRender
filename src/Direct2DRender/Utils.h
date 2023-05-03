#pragma once
#include <any>
#include <string>

class Utils {
public:
    static bool anyEqual(const std::any& a1, const std::any& a2)
    {
        if (a1.type() != a2.type()) {
            return false;
        }

        if (a1.type() == typeid(int)) {
            return std::any_cast<int>(a1) == std::any_cast<int>(a2);
        }
        else if (a1.type() == typeid(float)) {
            return std::any_cast<float>(a1) == std::any_cast<float>(a2);
        }
        else if (a1.type() == typeid(double)) {
            return std::any_cast<double>(a1) == std::any_cast<double>(a2);
        }
        else if (a1.type() == typeid(std::string)) {
            return std::any_cast<std::string>(a1) == std::any_cast<std::string>(a2);
        }
        return false;
    }

private:
    Utils() = delete;
    ~Utils() = delete;
};
