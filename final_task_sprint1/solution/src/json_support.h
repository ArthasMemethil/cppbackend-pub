#pragma once

#include <boost/json/value.hpp>
#include <boost/json/object.hpp>
#include <boost/json.hpp>
#include <boost/json/string.hpp>
#include <boost/json/array.hpp>

#include "model.h"

namespace json_support{



    namespace json = boost::json;

    boost::json::array GetJSONAllMaps(const model::Game::Maps& maps);
    json::array MakeJSONRoadsArr(const std::vector<model::Road>& roads);
    json::array MakeJSONBuildingsArr(const std::vector<model::Building>& buildings);
    json::array MakeJSONOfficesArr(const std::vector<model::Office>& offices);
    json::object GetJSONRequiredMap(const model::Map& map);
    json::object GetJSONNotFound();
    json::object GetJSONBadRequest();

    template<typename json>
    std::string GetFormattedJSONStr(const json json_value)
    {
        std::stringstream read;
        std::string buffer;
        std::string body;
        read << json_value << std::endl;
        while (std::getline(read, buffer)) {
            body += buffer;
        }
        return body;
    }

}//namespace json_support