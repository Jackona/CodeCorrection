#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "xml-element.h"
#include "xml-generator.h"
#include "gridworld-model.h"

#include "gridworld-route.h"

namespace GPS::GridWorld
{

Route::Route(std::string routeString, Model gridModel)
  : routeString{routeString},
    gridModel{gridModel}
{
    if (isValidRouteString(routeString))
    {
        constructRoutePoints();
    }
    else
    {
        throw std::invalid_argument("Invalid point sequence, cannot construct Route.");
    }
}

void Route::constructRoutePoints()
{
    for (Model::PointName pointName : routeString)
    {
        routePoints.push_back( {gridModel[pointName], Model::pointToString(pointName)} );
    }
}

std::string Route::toGPX() const
{
    XML::Generator gpx;

    gpx.basicXMLDeclaration();
    gpx.openBasicGPXElement();

    gpx.openElement("rte",{});

    for (const RoutePoint& routePoint : routePoints)
    {
        XML::Attributes attribs =
          { {"lat", std::to_string(routePoint.position.latitude())},
            {"lon", std::to_string(routePoint.position.longitude())}
          };

        gpx.openElement("rtept",attribs);
        gpx.element("name",{},routePoint.name);
        gpx.element("ele",{},std::to_string(routePoint.position.elevation()));
        gpx.closeElement(); // "rtept"
    }

    return gpx.closeAllElementsAndExtractString();
}

std::string Route::toNMEA() const
{
    using namespace std;
    degrees d;
    int j, l;
    char c;
    string n, st;
    stringstream s; // easier than string
    Position p = Position(0,0,0); // dummy object as there's no public constructor in the Position class.
    l = routePoints.size();
    for (int i = 0; i < l; ++i) {
        if (i != 0) s << '\n';
        n = routePoints[i].name;
        p = routePoints[i].position;
        s << "$GPWPL,";

        for(int k = 0; k < 2; ++k){
            if(k == 0) {
                d = p.latitude();
                if (d < 0) {
                    c = 'S';
                    d = -d;
                } else c = 'N'; 
            }
            else {
                d = p.longitude();
                if (d < 0) {
                    c = 'W';
                    d = -d;
                } else c = 'E'; 
            }
            
            j = (int(d+0.5)) % 100; // degs
            s << setw(2) << setfill('0') << j;
            d = abs(d - j); // deg 100ths
            d = (d * 100) / 60; // mins
            j = (int(d+0.5)) % 100; // whole mins
            s << setw(2) << setfill('0') << j;
            j = d - j + 0.5; // min 100ths
            s << '.';
            s << setw(2) << setfill('0') << j << ',' << c << ',';
        }
        s << n;
        s << "*XX"; // no check yet
    }
    st = s.str();
    return st;
}

std::string Route::toString() const
{
    return routeString;
}

std::vector<RoutePoint> Route::toRoutePoints() const
{
    return routePoints;
}

bool Route::isValidRouteString(std::string routeStr)
{
    // To be valid, all chars must be in the range 'A'..'Y'.
    return std::all_of(routeStr.begin(), routeStr.end(),
                       [] (char point)->bool {return point >= 'A' && point <= 'Y';});
}

}