
#include "application.h"


Application::Application(model::Game& game) :
	game_(game),
	players_(std::make_unique<Players>()),
	tokens_(std::make_unique<token::PlayersTokens>()){}

std::shared_ptr<Player> Application::FindPlayerByToken(Token token)
{
	return tokens_->FindPlayerByToken(token);
}

Token Application::SetTokenForPlayer(std::shared_ptr<Player> player)
{
	return tokens_->SetTokenForPlayer(player);
}

const std::vector<std::shared_ptr<Player>> Application::GetPlayersInSession(int session_id) const
{
	return players_->GetPlayersInSession(session_id);
}

void Application::AddPlayer(std::shared_ptr<Player> player)
{
	players_->Add(player);
}


model::Game& Application::GetGame()
{
	return game_;
}

void Application::RestoreToken(Token token, std::shared_ptr<Player> player)
{
	tokens_->AddTokenAndPlayer(player, token);
}