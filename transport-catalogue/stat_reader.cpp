
#include "stat_reader.h"

using ::std::literals::string_view_literals::operator""sv;

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    std::string_view request_type = request.substr(0, request.find(' '));
    if (request_type == "Bus"sv) {
        std::string_view bus = request.substr(request.find(' ') + 1);   
        if (transport_catalogue.FindBus(bus) == nullptr) {    
            output <<"Bus "sv << bus << ": not found"sv << std::endl;
        } else {
            output << "Bus "sv << bus << ": "sv;     
            output << transport_catalogue.GetRouteInfo(bus).stops <<  " stops on route, "sv;
            output << transport_catalogue.GetRouteInfo(bus).unique_stops <<  " unique stops, "sv;
            output << transport_catalogue.GetRouteInfo(bus).lenght <<  " route length"sv << std::endl;
        }
    }
    if (request_type == "Stop"sv) {
        std::string_view stop = request.substr(request.find(' ') + 1);
        if (transport_catalogue.FindStop(stop) == nullptr) {
            output <<"Stop "sv << stop << ": not found"sv << std::endl;
        } else {
        if (transport_catalogue.FindStopsToBuses(stop).empty()) {    
            output <<"Stop "sv << stop << ": no buses"sv << std::endl;
        } else {
                output << "Stop "sv << stop << ": buses"sv;   
                for (auto& stop : transport_catalogue.FindStopsToBuses(stop)) {
                    output << " "sv << stop;
                }
                output << std::endl;
            }
        }
    }    
}

