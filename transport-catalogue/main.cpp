#include <iostream>
#include <fstream>
#include <sstream>

#include "json_reader.h"

using namespace std;
using namespace json;

int main() {
    TransportCatalogue catalogue;
    renderer::MapRenderer renderer;
    transport_router::TransportRouter router(catalogue);
    RequestHandler handler(catalogue, renderer, router);
    JsonReader reader;
    reader.LoadHandler(handler);

    istream& input = cin;
    reader.LoadJson(input);

    reader.AddStopsDataToCatalogue();
    reader.AddBusesDataToCatalogue();
    reader.AddRoutingSetting();
    reader.ParseRenderSettings(renderer);

    ostream& out = cout;
    reader.ParseAndPrintStat(handler, out);   
}