#include <iostream>
#include <fstream>
#include <sstream>

#include "json_reader.h"

using namespace std;
using namespace json;


int main() {
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