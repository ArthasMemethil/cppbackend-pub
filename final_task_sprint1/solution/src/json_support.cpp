#include "json_support.h"


namespace json_support {

    namespace json = boost::json;

    //keys
    char const* X = "x";
    char const* Y = "y";
    char const* W = "w";
    char const* H = "h";
    char const* ID = "id";
    char const* NAME = "name";
    char const* CODE = "code";
    char const* MESSAGE = "message";

    boost::json::array GetJSONAllMaps(const model::Game::Maps& maps) 
    {
        boost::json::array maps_arr;
        for (const auto& map : maps)
        {
            json::string str_id = json::string(*(map.GetId()));
            json::string str_name = json::string(map.GetName());
            boost::json::object json_map;
            json_map[ID] = str_id;
            json_map[NAME] = str_name;
            maps_arr.push_back(json_map);
        }
        return maps_arr;
    }

    json::array MakeJSONRoadsArr(const std::vector<model::Road>& roads)
    {
        json::array road_arr;
        for (const auto& road : roads)
        {
            json::object json_road;

            model::Point coords = road.GetStart();
            model::Dimension x = coords.x;
            model::Dimension y = coords.y;
            model::Point end = road.GetEnd();
            bool is_x_road = road.IsXRoad();

            json_road["x0"] = x;
            json_road["y0"] = y;
            if (is_x_road)
                json_road["x1"] = end.x;
            else
                json_road["y1"] = end.y;

            road_arr.push_back(json_road);
        }
        return road_arr;
    }

    json::array MakeJSONBuildingsArr(const std::vector<model::Building>& buildings)
    {
        json::array building_arr;

        for (const auto& building : buildings)
        {
            json::object json_building;
            model::Rectangle figure = building.GetBounds();
            model::Point pos = figure.position;
            model::Size size = figure.size;

            json_building[X] = pos.x;
            json_building[Y] = pos.y;
            json_building[W] = size.width;
            json_building[H] = size.height;

            building_arr.push_back(json_building);
        }

        return building_arr;
    }
    json::array MakeJSONOfficesArr(const std::vector<model::Office>& offices)
    {
        json::array offices_arr;

        for (const auto& office : offices)
        {
            json::object json_office;

            std::string id = *(office.GetId());
            model::Point pos = office.GetPosition();
            model::Offset offset = office.GetOffset();

            json_office[ID] = json::string(id);
            json_office[X] = pos.x;
            json_office[Y] = pos.y;
            json_office["offsetX"] = offset.dx;
            json_office["offsetY"] = offset.dy;

            offices_arr.push_back(json_office);
        }

        return offices_arr;
    }

    json::object GetJSONRequiredMap(const model::Map& map)
    {
        boost::json::object json_map;
        std::string id = *(map.GetId());
        std::string name = map.GetName();
        std::vector<model::Road> roads = map.GetRoads();
        std::vector<model::Building> buildings = map.GetBuildings();
        std::vector<model::Office> offices = map.GetOffices();

        json_map[ID] = json::string(id);
        json_map[NAME] = json::string(name);
        json_map["roads"] = MakeJSONRoadsArr(roads);
        json_map["buildings"] = MakeJSONBuildingsArr(buildings);
        json_map["offices"] = MakeJSONOfficesArr(offices);

        return json_map;
    }

    json::object GetJSONNotFound()
    {
        json::object not_found;
        not_found[CODE] = json::string("mapNotFound");
        not_found[MESSAGE] = json::string("Map not found");
        return not_found;
    }

    json::object GetJSONBadRequest()
    {
        json::object bad_request;
        bad_request[CODE] = json::string("badRequest");
        bad_request[MESSAGE] = json::string("Bad request");
        return bad_request;
    }

} //namespace json_support