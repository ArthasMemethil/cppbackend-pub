#include "model.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <set>

namespace model {
    using namespace std::literals;
    // Road methods
    bool Road::IsYRoad() const noexcept {
        return start_.x == end_.x;
    }

    bool Road::IsXRoad() const noexcept {
        return start_.y == end_.y;
    }

    Point Road::GetStart() const noexcept {
        return start_;
    }

    Point Road::GetEnd() const noexcept {
        return end_;
    }

    const EdgeCoords& Road::GetEdgeCoords() const noexcept
    {
        return edge_;
    }

    void Road::CalculateEdgeCoords()
    {
        std::pair<double, double> x_edge{ 0, 0 };
        std::pair<double, double> y_edge{ 0, 0 };

        bool is_x_road = false;

        if (this->IsXRoad())
            is_x_road = true;

        if (is_x_road)
        {
            int start_x = start_.x;
            int end_x = end_.x;
            if (IsInvertedStartX())//описания начала дороги может начинаться НЕ из меньшей координаты!
            {
                int temp = std::move(start_x);
                start_x = std::move(end_.x);
                end_x = std::move(temp);
            }

            x_edge = { start_x - HALF_ROAD_WIDTH, end_x + HALF_ROAD_WIDTH }; //В рамки допустимых координат дороги также входит любое допустимое смещение по X или Y
            y_edge = { start_.y - HALF_ROAD_WIDTH, start_.y + HALF_ROAD_WIDTH };
        }
        else
        {
            int start_y = start_.y;
            int end_y = end_.y;
            if (IsInvertedStartY())//описания начала дороги может начинаться НЕ из меньшей координаты!
            {
                int temp = std::move(start_y);
                start_y = std::move(end_y);
                end_y = std::move(temp);
            }

            y_edge = { start_y - HALF_ROAD_WIDTH, end_y + HALF_ROAD_WIDTH }; //В рамки допустимых координат дороги также входит любое допустимое смещение по X или Y
            x_edge = { start_.x - HALF_ROAD_WIDTH, start_.x + HALF_ROAD_WIDTH };
        }
        edge_ = std::move(EdgeCoords{ x_edge, y_edge });
    }

    bool Road::IsInvertedStartX() const
    {
        return start_.x > end_.x;
    }

    bool Road::IsInvertedStartY() const
    {
        return start_.y > end_.y;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //Map methods
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map::Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& Map::GetId() const noexcept {
        return id_;
    }

    const std::string& Map::GetName() const noexcept {
        return name_;
    }

    const Buildings& Map::GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& Map::GetRoads() const noexcept {
        return roads_;
    }

    const Offices& Map::GetOffices() const noexcept {
        return offices_;
    }

    void Map::AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void Map::AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void Map::AddOffice(Office office) {
        if (warehouse_id_to_index_.contains(office.GetId())) {
            throw std::invalid_argument("Duplicate warehouse");
        }

        const size_t index = offices_.size();
        Office& o = offices_.emplace_back(std::move(office));
        try {
            warehouse_id_to_index_.emplace(o.GetId(), index);
        }
        catch (...) {
            // Удаляем офис из вектора, если не удалось вставить в unordered_map
            offices_.pop_back();
            throw;
        }
    }

    const DogSpeed& Map::GetDogSpeed() const noexcept
    {
        return default_map_speed_;
    }

    DogCoord Map::GetStartPosDog() const noexcept
    {
        Point p = roads_[0].GetStart();
        double d_x = static_cast<double>(p.x);
        double d_y = static_cast<double>(p.y);
        return DogCoord{ d_x, d_y };
    }

    DogCoord Map::GetRandomPosDog() const noexcept
    {
        int index = GetRandomRoadIndex();
        return GetRandomCoord(index);
    }
 
    int Map::GetRandomRoadIndex() const
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, static_cast<int>(roads_.size() - 1));

        return dist(rng);
    }

    DogCoord Map::GetRandomCoord(const int index) const
    {
        Road road = roads_[index];
        std::random_device dev;
        std::mt19937 rng(dev());

        double start_x = static_cast<double>(road.GetStart().x - HALF_ROAD_WIDTH);
        double end_x = static_cast<double>(road.GetEnd().x + HALF_ROAD_WIDTH);

        double start_y = static_cast<double>(road.GetStart().y - HALF_ROAD_WIDTH);
        double end_y = static_cast<double>(road.GetEnd().y + HALF_ROAD_WIDTH);

        //Проверки необходимы, поскольку uniform_real_distribution строго требует a <= b в конструкторе.
        if (road.IsInvertedStartX())
        {
            double temp = std::move(start_x);
            start_x = std::move(end_x);
            end_x = std::move(temp);
        }
        if (road.IsInvertedStartY())
        {
            double temp = std::move(start_y);
            start_y = std::move(end_y);
            end_y = std::move(temp);
        }

        std::uniform_real_distribution<double> dist(start_x, end_x);
        std::uniform_real_distribution<double> dist2(start_y, end_y);

        return { dist(rng), dist2(rng) };
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    //DogMethods

    Dog::Dog(Direction dir, DogSpeed speed, DogCoord start_pos) : coords_(std::move(start_pos)), default_speed_(std::move(speed)), dir_(std::move(dir))
    {
        ++generation_id_;
        object_id_ = generation_id_;
    }

    const int Dog::GetGenerationId() const noexcept {
        return generation_id_;
    }

    const int Dog::GetObjectId() const noexcept
    {
        return object_id_;
    }

    const DogCoord& Dog::GetCoords() const noexcept
    {
        return coords_;
    }

    const DogSpeed& Dog::GetSpeed() const noexcept
    {
        return speed_;
    }

    const DogSpeed& Dog::GetDefaultSpeed() const noexcept
    {
        return default_speed_;
    }

    const Direction Dog::GetDirection() const noexcept
    {
        return dir_;
    }

    void Dog::SetCoords(DogCoord coord)
    {
        coords_ = std::move(coord);
    }

    void Dog::StopDog()
    {
        speed_.speed_x = 0;
        speed_.speed_y = 0;
    }

    void Dog::SetInGameSpeed()
    {
        switch (dir_)
        {
        case Direction::UP:
        {
            if (speed_.speed_x != 0)
            {
                speed_.speed_x = 0;
                speed_.speed_y = default_speed_.speed_y * -1;
            }
            else
                speed_.speed_y = default_speed_.speed_y * -1;
            break;
        }
        case Direction::DOWN:
        {
            if (speed_.speed_x != 0)
            {
                speed_.speed_x = 0;
                speed_.speed_y = default_speed_.speed_y;
            }
            else
                speed_.speed_y = default_speed_.speed_y;
            break;
        }
        case Direction::LEFT:
        {
            if (speed_.speed_y != 0)
            {
                speed_.speed_y = 0;
                speed_.speed_x = default_speed_.speed_x * -1;
            }
            else
                speed_.speed_x = default_speed_.speed_x * -1;
            break;
        }
        case Direction::RIGHT:
        {
            if (speed_.speed_y != 0)
            {
                speed_.speed_y = 0;
                speed_.speed_x = default_speed_.speed_x;
            }
            else
                speed_.speed_x = default_speed_.speed_x;
            break;
        }
        }
    }

    DogCoord Dog::CalculateCoordinates(const std::chrono::milliseconds deltaTime) const
    {
        DogCoord coords = coords_;
        double temp = static_cast<double>((deltaTime.count()));
        double time = temp / 1000;
        double multiply_x = speed_.speed_x * time;
        double multiply_y = speed_.speed_y * time;

        if (dir_ == Direction::LEFT || dir_ == Direction::RIGHT)
            coords.x += multiply_x;
        else
            coords.y += multiply_y;

        return coords;
    }

    void Dog::SetInGameDirection(Direction dir)
    {
        dir_ = std::move(dir);
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    //Game methods


    using Maps = std::vector<std::shared_ptr<Map>>;

    const Maps& Game::GetMaps() const noexcept {
        return maps_;
    }

    const std::shared_ptr<Map> Game::FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return maps_.at(it->second);
        }
        return nullptr;
    }

    void Game::AddSession(std::shared_ptr<GameSession> session, std::string map_id)
    {
        sessions_[map_id] = std::move(session);
    }

    void Game::UpdateSession(std::shared_ptr<GameSession> session, std::string map_id)
    {
        Game::AddSession(session, map_id);
    }

    bool Game::IsSessionExist(std::string map_id) const
    {
        if (sessions_.contains(map_id))
            return true;
        return false;
    }

    bool Game::IsAnySession() const
    {
        return sessions_.size() != 0;
    }

    const std::shared_ptr<GameSession> Game::GetSession(std::string map_id) const noexcept
    {
        if (sessions_.contains(map_id))
            return sessions_.at(map_id);
        return nullptr;
    }


    const std::unordered_map<std::string, std::shared_ptr<GameSession>>& Game::GetSessions() const noexcept
    {
        return sessions_;
    }

    void Game::AddMap(std::shared_ptr<Map> map) {
        const size_t index = maps_.size();
        if (auto [it, inserted] = map_id_to_index_.emplace(map->GetId(), index); !inserted) {
            throw std::invalid_argument("Map with id "s + *map->GetId() + " already exists"s);
        }
        else {
            try {
                maps_.emplace_back(std::move(map));
            }
            catch (...) {
                map_id_to_index_.erase(it);
                throw;
            }
        }
    }
    ////////////////////////////////////////////
    //GameSessionMethods
    GameSession::GameSession(const std::shared_ptr<Map> map) : map_(map)
    {
        ++generation_id_;
        object_id_ = generation_id_;
    }

    void GameSession::AddDog(std::shared_ptr<Dog> dog_ptr)
    {
        int id = dog_ptr->GetObjectId();
        dogs_.insert({ id, dog_ptr });
    }

    const int& GameSession::GetGenerationId() const noexcept
    {
        return generation_id_;
    }

    const int GameSession::GetObjectId() const noexcept
    {
        return object_id_;
    }

    const std::shared_ptr<Map> GameSession::GetMap() const noexcept
    {
        return map_;
    }

    const std::unordered_map<int, std::shared_ptr<Dog>>& GameSession::GetDogs() const noexcept
    {
        return dogs_;
    }

    void GameSession::DeleteDog(int id)
    {
        auto it = dogs_.find(id);

        if (it == dogs_.end())
            throw std::runtime_error("Can't delete uncreated dog");
        dogs_.erase(it);
    }

}  // namespace model