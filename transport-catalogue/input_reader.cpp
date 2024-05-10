#include "input_reader.h"

#include <cassert>
#include <iterator>
#include <utility>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

std::vector<std::pair<std::string_view, Distance>> ParseDistance(std::string_view description) {
    auto char_m = description.find('m');
    if (char_m == description.npos) {
        return {};
    }
    std::vector<std::pair<std::string_view, Distance>> distances_to_stops;
    std::string_view to_first_m = description.substr(0, char_m);
    auto pos = to_first_m.find_last_of(' ');
    std::string_view stops_and_dist = Trim(description.substr(pos));
    std::vector<std::string_view> stops_and_dist_split = Split(stops_and_dist, ',');
    for (auto s : stops_and_dist_split) {
        Distance distance;
        distance.meters = std::stoi(std::string(s.substr(0, s.find('m'))));
        size_t pos = s.find('o');
        std::string_view stop_name = Trim(s.substr(++pos));
        distances_to_stops.push_back({stop_name, distance});
    }
    return distances_to_stops;
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    for (auto cmd : commands_) {
        if (cmd.command == "Stop") {
            catalogue.AddStop(cmd.id, ParseCoordinates(cmd.description));

        }
    }
    
    for (auto cmd : commands_) {
        if (cmd.command == "Stop") {
            auto to_stops = ParseDistance(cmd.description);
            if (!to_stops.empty()) {
                catalogue.AddStopDistance(cmd.id, to_stops);
            }
        }
    }

    for (auto cmd : commands_) {
        if (cmd.command == "Bus") {          
            catalogue.AddBus(cmd.id, ParseRoute(cmd.description));
        }  
    }
}