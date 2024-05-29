#include <iostream>
#include <fstream>
#include <sstream>

#include "json_reader.h"
#include "/C/LOGDURATION/log_duration.h"


using namespace std;
using namespace json;


int main() {
    
    LOG_DURATION("Speed"s);
    TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    RequestHandler handler(catalogue, renderer);
    JsonReader reader;
    reader.LoadHandler(handler);

    istream& input = cin;
    reader.LoadJson(input);
            
    reader.AddStopsDataToCatalogue();
    reader.AddBusesDataToCatalogue();
    reader.ParseRenderSettings(renderer);


    ostream& out = cout;
    reader.ParseAndPrintStat(handler, out);
    

}