#include "json_support.h"


namespace json_support {

    namespace json = boost::json;


    boost::json::array GetJSONAllMaps(const model::Game::Maps& maps)
    {
        boost::json::array maps_arr;
        for (const auto& map : maps)
        {
            json::string str_id = json::string(*(map.GetId()));
            json::string str_name = json::string(map.GetName());
            boost::json::object json_map;
            json_map["id"] = str_id;
            json_map["name"] = str_name;
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

            json_building["x"] = pos.x;
            json_building["y"] = pos.y;
            json_building["w"] = size.width;
            json_building["h"] = size.height;

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

            json_office["id"] = json::string(id);
            json_office["x"] = pos.x;
            json_office["y"] = pos.y;
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

        json_map["id"] = json::string(id);
        json_map["name"] = json::string(name);
        json_map["roads"] = MakeJSONRoadsArr(roads);
        json_map["buildings"] = MakeJSONBuildingsArr(buildings);
        json_map["offices"] = MakeJSONOfficesArr(offices);

        return json_map;
    }

    json::object GetJSONNotFound()
    {
        json::object not_found;
        not_found["code"] = json::string("mapNotFound");
        not_found["messege"] = json::string("Map not found");
        return not_found;
    }

    json::object GetJSONBadRequest()
    {
        json::object bad_request;
        bad_request["code"] = json::string("badRequest");
        bad_request["message"] = json::string("Bad request");
        return bad_request;
    }

} //namespace json_support