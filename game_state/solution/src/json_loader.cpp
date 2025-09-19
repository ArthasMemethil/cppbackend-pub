
#include "json_loader.h"

namespace json_loader {

    std::string ReadJSONFromPath(const std::filesystem::path& json_path) {

        std::ifstream input;
        std::string path = json_path.string();
        input.open(json_path);

        if (!input.is_open()) {
            std::string err_mes = "Can't open the file: " + path;
            throw std::logic_error(err_mes);
        }

        std::string buffer;
        std::string text;
        while (getline(input, buffer)) {
            text += buffer;
        }
        return text;
    }

    std::string ReadJSONFromStream(std::stringstream& input) {
        std::string buffer;
        std::string text;
        while (std::getline(input, buffer)) {
            text += buffer;
        }
        return text;
    }


    void AddingRoadsToMap(const boost::json::array& roads, model::Map& map) {

        for (const auto road : roads) {
            model::Coord x = static_cast<int>(road.at("x0").get_int64());
            model::Coord y = static_cast<int>(road.at("y0").get_int64());
            model::Point start = { x, y };

            if (road.as_object().count("x1")) { 
                model::Coord end = static_cast<int>(road.at("x1").as_int64());
                model::Road hor_newroad(model::Road::HORIZONTAL, start, end);

                map.AddRoad(hor_newroad);
            }
            else if (road.as_object().count("y1")) {
                model::Coord end = static_cast<int>(road.at("y1").as_int64());
                model::Road vert_newroad(model::Road::VERTICAL, start, end);

                map.AddRoad(vert_newroad);
            }

        }
    }

    void AddingBuildingsToMap(const boost::json::array& buildings, model::Map& map) {
            
        for (auto building : buildings) {
            model::Coord x = static_cast<int>(building.at("x").get_int64());
            model::Coord y = static_cast<int>(building.at("y").get_int64());

            model::Dimension w = static_cast<int>(building.at("w").get_int64());
            model::Dimension h = static_cast<int>(building.at("h").get_int64());

            model::Rectangle rect(model::Point(x, y), model::Size(w, h));

            model::Building newbuild(std::move(rect));

            map.AddBuilding(newbuild);
        }
    }

    void AddingOfficesToMap(const boost::json::array& offices, model::Map& map) {
        for (auto office : offices) {

            model::Office::Id id(std::string(office.as_object().at("id").as_string()));

            model::Coord x = static_cast<int>(office.at("x").get_int64());
            model::Coord y = static_cast<int>(office.at("y").get_int64());

            model::Dimension dx = static_cast<int>(office.at("offsetX").get_int64());
            model::Dimension dy = static_cast<int>(office.at("offsetY").get_int64());

            model::Office of(id, model::Point(x, y), model::Offset(dx, dy));

            map.AddOffice(of);
        }
    }


model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
   
    std::string openedfile;
    openedfile = ReadJSONFromPath(json_path);

    // Распарсить строку как JSON, используя boost::json::parse

    auto js = boost::json::parse(openedfile);


    
    // Загрузить модель игры из файла
    auto maps = js.as_object().at("maps").get_array();

    model::Game game;

    for (const auto& map : maps) {
        std::string mapid(map.at("id").as_string());
        std::string mapname(map.at("name").as_string());

        model::Map::Id id{mapid};
        model::Map newmap(id, mapname);

        //добавляем на карту дороги
        auto roads = map.at("roads").get_array();
        AddingRoadsToMap(roads, newmap);

        //добавляем здания
        auto buildings = map.at("buildings").get_array();
        AddingBuildingsToMap(buildings, newmap);

        //добавляем офисы 
        auto offices = map.at("offices").get_array();
        AddingOfficesToMap(offices, newmap);

        

        game.AddMap(std::make_shared<model::Map>(newmap)); // сформировали карту добавили, повторили цикл
    }

    return game;
}

}  // namespace json_loader
