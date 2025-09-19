#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <random>
#include <iostream>
#include <chrono>

#include "tagged.h"


using namespace std::chrono_literals;

namespace model {

    constexpr double ITEM_WIDTH = 0.0;
    constexpr double HALF_DOG_WIDTH = 0.3;
    constexpr double HALF_ROAD_WIDTH = 0.4;
    constexpr double HALF_OFFICE_WIDTH = 0.25;

    using Dimension = int;
    using Coord = Dimension;
    enum class Direction : char
    {
        UP = 'U',
        DOWN = 'D',
        LEFT = 'L',
        RIGHT = 'R'
    };

    struct Point {
        Coord x, y;
    };

    struct Size {
        Dimension width, height;
    };

    struct Rectangle {
        Point position;
        Size size;
    };

    struct Offset {
        Dimension dx, dy;
    };


    struct EdgeCoords
    {
        std::pair<double, double> x_edge;
        std::pair<double, double> y_edge;
    };

    struct DogSpeed
    {
        double speed_x;
        double speed_y;
    };

    struct DogCoord
    {
        double x;
        double y;
    };

    class Road {
    private:
        struct HorizontalTag {
            explicit HorizontalTag() = default;
        };

        struct VerticalTag {
            explicit VerticalTag() = default;
        };
    public:
        constexpr static HorizontalTag HORIZONTAL{};
        constexpr static VerticalTag VERTICAL{};

        Road(HorizontalTag, Point start, Coord end_x) noexcept
            : start_{ start }
            , end_{ end_x, start.y }
        {
            CalculateEdgeCoords();
        }

        Road(VerticalTag, Point start, Coord end_y) noexcept
            : start_{ start }
            , end_{ start.x, end_y }
        {
            CalculateEdgeCoords();
        }

        bool IsYRoad() const noexcept;
        bool IsXRoad() const noexcept;
        Point GetStart() const noexcept;
        Point GetEnd() const noexcept;
        const EdgeCoords& GetEdgeCoords() const noexcept;
        bool IsInvertedStartX() const;
        bool IsInvertedStartY() const;

    private:
        Point start_;
        Point end_;
        EdgeCoords edge_;

        void CalculateEdgeCoords();
    };

    class Building {
    public:
        explicit Building(Rectangle bounds) noexcept
            : bounds_{ bounds } {
        }

        const Rectangle& GetBounds() const noexcept {
            return bounds_;
        }

    private:
        Rectangle bounds_;
    };

    class Office {
    public:
        using Id = util::Tagged<std::string, Office>;

        Office(Id id, Point position, Offset offset) noexcept
            : id_{ std::move(id) }
            , position_{ std::move(position) }
            , offset_{ std::move(offset) } {
        }

        const Id& GetId() const noexcept {
            return id_;
        }

        Point GetPosition() const noexcept {
            return position_;
        }

        Offset GetOffset() const noexcept {
            return offset_;
        }

    private:
        Id id_;
        Point position_;
        Offset offset_;
    };

  

    class Map {
    public:
        using Id = util::Tagged<std::string, Map>;
        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;

        Map(Id id, std::string name) noexcept;
        const Id& GetId() const noexcept;
        const std::string& GetName() const noexcept;
        const Buildings& GetBuildings() const noexcept;
        const Roads& GetRoads() const noexcept;
        const Offices& GetOffices() const noexcept;
        void AddRoad(const Road& road);
        void AddBuilding(const Building& building);
        void AddOffice(const Office office);

        const DogSpeed& GetDogSpeed() const noexcept;
        DogCoord GetStartPosDog() const noexcept;
        DogCoord GetRandomPosDog() const noexcept;

    private:
        using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

        Id id_;
        std::string name_;
        Roads roads_;
        Buildings buildings_;
        OfficeIdToIndex warehouse_id_to_index_;
        Offices offices_;
        DogSpeed default_map_speed_{ 0, 0 };

        int GetRandomRoadIndex() const;
        DogCoord GetRandomCoord(const int index) const;
  
    };



    class Dog
    {
        
    public:
        Dog(Direction dir, DogSpeed speed, DogCoord start_pos);
        const int GetGenerationId() const noexcept;
        const int GetObjectId() const noexcept;
        void SetCoords(DogCoord coord);
        const DogCoord& GetCoords() const noexcept;
        const DogSpeed& GetSpeed() const noexcept;
        const DogSpeed& GetDefaultSpeed() const noexcept;
        const Direction GetDirection() const noexcept;
        void StopDog();
        void SetInGameSpeed();
        DogCoord CalculateCoordinates(const std::chrono::milliseconds deltaTime) const;
        void SetInGameDirection(const Direction dir);

    private:
        static inline int generation_id_ = -1;
        int object_id_;
        Direction dir_;
        DogCoord coords_{ 0, 0 };
        DogSpeed speed_{ 0, 0 };
        DogSpeed default_speed_{ 0, 0 };
 
    };

    class GameSession
    {
    public:
        explicit GameSession(const std::shared_ptr<Map> map);
        void AddDog(std::shared_ptr<Dog> dog_ptr);
        const int& GetGenerationId() const noexcept;
        const int GetObjectId() const noexcept;
        const std::shared_ptr<Map> GetMap() const noexcept;
        const std::unordered_map<int, std::shared_ptr<Dog>>& GetDogs() const noexcept;
        void DeleteDog(int id);

    private:
        static inline int generation_id_ = -1;
        int object_id_;
        const std::shared_ptr<Map> map_;
        std::unordered_map<int, std::shared_ptr<Dog>> dogs_;
    };

    class Game {
    public:
        using Maps = std::vector<std::shared_ptr<Map>>;
        void AddMap(std::shared_ptr<Map> map);
        const Maps& GetMaps() const noexcept;
        const std::shared_ptr<Map> FindMap(const Map::Id& id) const noexcept;
        void AddSession(std::shared_ptr<GameSession> session, std::string map_id);
        void UpdateSession(std::shared_ptr<GameSession> session, std::string map_id);
        bool IsSessionExist(const std::string map_id) const;
        bool IsAnySession() const;
        const std::unordered_map<std::string, std::shared_ptr<GameSession>>& GetSessions() const noexcept;
        const std::shared_ptr<GameSession> GetSession(const std::string map_id) const noexcept;
     



        void DeleteSession(std::string map_id)
        {
            auto it = sessions_.find(map_id);

            if (it == sessions_.end())
                throw std::runtime_error("Can't delete uncreated session");

            sessions_.erase(it);
        }

   

        void DeleteEmptySessions()
        {
            if (sessions_.size() == 0)
                return;

            for (auto it = sessions_.begin(); it != sessions_.end();)
            {
                if (it->second->GetDogs().size() == 0)
                {
                    it = sessions_.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

    private:
        using MapIdHasher = util::TaggedHasher<Map::Id>;
        using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

        std::unordered_map<std::string, std::shared_ptr<GameSession>> sessions_;
        std::vector<std::shared_ptr<Map>> maps_;
        MapIdToIndex map_id_to_index_;

        

    };

}  // namespace model