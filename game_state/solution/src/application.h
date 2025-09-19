#pragma once

#include "model.h"
#include "players.h"
#include "players_tokens.h"


struct GameSettings
{
	bool is_auto_tick;
	bool is_save_mode;
	bool is_auto_save_mode;
	bool randomize_spawn_points;
	uint32_t save_period;
	uint32_t retirement_time;
};

class Application
{
public:
	Application(model::Game& game);
	std::shared_ptr<Player> FindPlayerByToken(Token token);
	Token SetTokenForPlayer(std::shared_ptr<Player> player);
	const std::vector<std::shared_ptr<Player>> GetPlayersInSession(int session_id) const;
	void AddPlayer(std::shared_ptr<Player> player);

	model::Game& GetGame();
	void RestoreToken(Token token, std::shared_ptr<Player> player);
	

private:
	model::Game& game_;
	std::unique_ptr<Players> players_;
	std::unique_ptr<token::PlayersTokens> tokens_;

};