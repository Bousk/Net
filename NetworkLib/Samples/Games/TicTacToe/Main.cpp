#include <Game.hpp>
#include <Net.hpp>

#include <Errors.hpp>
#include <Messages.hpp>
#include <Serialization/Deserializer.hpp>
#include <Sockets.hpp>
#include <UDP/UDPClient.hpp>
#include <UDP/Protocols/ReliableOrdered.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

void main_p2p();
void main_client();
void main_server();

int main(int argc, char* argv[])
{
	if (!Bousk::Network::Start())
	{
		std::cout << "Network lib initialisation error : " << Bousk::Network::Errors::Get();
		return -1;
	}

	main_p2p();

	Bousk::Network::Release();
	return 0;
}

void clearCin()
{
	if (!std::cin.good())
	{
		std::cin.ignore();
		std::cin.clear();
	}
}
bool askPort(Bousk::uint16& port)
{
	clearCin();
	std::cin >> port;
	return std::cin.good();
}
bool askYesNo(char& response)
{
	clearCin();
	std::cin >> response;
	if (response == 'y' || response == 'Y' || response == 'n' || response == 'N')
	{
		response = std::tolower(response);
		return true;
	}
	return false;
}
bool askPlay(TicTacToe::Net::Play& play)
{
	clearCin();
	std::string line;
	std::getline(std::cin, line);
	int x, y;
	if (std::sscanf(line.c_str(), "%d %d", &x, &y) == 2)
	{
		if (play.x.IsWithinRange(x) && play.y.IsWithinRange(y))
		{
			play.x = x;
			play.y = y;
			return true;
		}
	}
	return false;
}

void main_p2p()
{
	char createGame;
	do
	{
		std::cout << "Do you want to create a game ? (Y/N) ";
	} while (!askYesNo(createGame));

	const bool isHost = (createGame == 'y');

	Bousk::uint16 port = 0;
	Bousk::Network::UDP::Client client;
	client.registerChannel<Bousk::Network::UDP::Protocols::ReliableOrdered>();
	if (isHost)
	{
		// If I'm not host my port doesn't matter, let's skip it then
		do
		{
			std::cout << "Enter the port to use for your local client : ";
		} while (!askPort(port));
	}

	if (!client.init(port))
	{
		std::cout << "Client initialisation error : " << Bousk::Network::Errors::Get();
		return;
	}

	Bousk::Network::Address opponent;
	if (!isHost)
	{
		Bousk::uint16 hostPort;
		// Ask the host post to connect to
		do
		{
			std::cout << "Enter host port to connect to : ";
		} while (!askPort(hostPort));
		const Bousk::Network::Address host = Bousk::Network::Address::Loopback(Bousk::Network::Address::Type::IPv4, hostPort);
		opponent = host;
		client.connect(host);
	}
	else
	{
		std::cout << "Waiting for opponent..." << std::endl;
	}

	enum class State {
		WaitingOpponent,
		WaitingConnection,
		MyTurn,
		OpponentTurn,
		Finished,
	};
	// Host wait for opponent to connect while guest connect to host right away
	State state = isHost ? State::WaitingOpponent : State::WaitingConnection;
	// Create the grid. Host will be the first to play.
	TicTacToe::Grid grid;
	// Host plays X, guest plays O
	const TicTacToe::Case localPlayerSymbol = isHost ? TicTacToe::Case::X : TicTacToe::Case::O;
	// Save my opponent symbol too
	const TicTacToe::Case opponentSymbol = (localPlayerSymbol == TicTacToe::Case::X) ? TicTacToe::Case::O : TicTacToe::Case::X;
	bool exit = false;
	std::thread networkThread([&]()
	{
		while (!exit)
		{
			// Receive network data
			client.receive();
			// Send network data
			client.processSend();
			// Don't loop like crazy...
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});
	while (!exit)
	{
		auto messages = client.poll();
		for (auto&& message : messages)
		{
			if (message->is<Bousk::Network::Messages::IncomingConnection>())
			{
				if (isHost)
				{
					if (state == State::WaitingOpponent)
					{
						std::cout << "Incoming connection from [" << message->emitter().toString() << "]..." << std::endl;
						std::cout << "Accepting connection..." << std::endl;
						client.connect(message->emitter());
						state = State::WaitingConnection;
					}
					else
					{
						std::cout << "Incoming connection from [" << message->emitter().toString() << "]..." << std::endl;
						std::cout << "Game is starting: refusing connection." << std::endl;
					}
				}
			}
			else if (message->is<Bousk::Network::Messages::Connection>())
			{
				if (message->as<Bousk::Network::Messages::Connection>()->result == Bousk::Network::Messages::Connection::Result::Success)
				{
					std::cout << "Starting game !" << std::endl;
					state = isHost ? State::MyTurn : State::OpponentTurn;
					opponent = message->emitter();
				}
				else
				{
					std::cout << "Connection failed !" << std::endl;
					std::cout << "Exiting game." << std::endl;
					exit = true;
				}
			}
			else if (message->is<Bousk::Network::Messages::UserData>())
			{
				if (state != State::OpponentTurn)
				{
					std::cout << "Receiving data but it's not their turn..." << std::endl;
					std::cout << "Exiting game." << std::endl;
					exit = true;
				}
				else
				{
					const Bousk::Network::Messages::UserData* userData = message->as<Bousk::Network::Messages::UserData>();
					TicTacToe::Net::Play playedAction;
					Bousk::Serialization::Deserializer deserializer(userData->data.data(), userData->data.size());
					if (!playedAction.read(deserializer))
					{
						std::cout << "Error deserializing data..." << std::endl;
						std::cout << "Exiting game." << std::endl;
						exit = true;
					}
					else
					{
						std::cout << "Opponent plays (" << static_cast<int>(playedAction.x) << "," << static_cast<int>(playedAction.y) << ")" << std::endl;
						grid.play(playedAction.x, playedAction.y, opponentSymbol);
						state = grid.isFinished() ? State::Finished : State::MyTurn;
					}
				}
			}
			else if (message->is<Bousk::Network::Messages::Disconnection>())
			{
				std::cout << "Opponent disconnected..." << std::endl;
				std::cout << "Exiting game." << std::endl;
				exit = true;
			}
		}

		switch (state)
		{
			case State::WaitingOpponent:
			case State::WaitingConnection:
			case State::OpponentTurn:
			{
				// Nothing to do
			} break;
			case State::MyTurn:
			{
				grid.display();
				TicTacToe::Net::Play play;
				do
				{
					std::cout << "Enter coordinates to play (x y) : ";
				} while (!askPlay(play) || !grid.play(play.x, play.y, localPlayerSymbol));
				Bousk::Serialization::Serializer serializer;
				if (!play.write(serializer))
				{
					std::cout << "Error serializing data..." << std::endl;
					std::cout << "Exiting game." << std::endl;
					exit = true;
				}
				else
				{
					client.sendTo(opponent, serializer.buffer(), serializer.bufferSize(), 0);
					if (!grid.isFinished())
					{
						grid.display();
						std::cout << "Opponent turn" << std::endl;
						state = State::OpponentTurn;
					}
					else
					{
						state = State::Finished;
					}
				}
			} break;
			case State::Finished:
			{
				const TicTacToe::Case winner = grid.winner();
				if (winner == TicTacToe::Case::Empty)
					std::cout << "That's a draw." << std::endl;
				else if (winner == localPlayerSymbol)
					std::cout << "You WIN !" << std::endl;
				else
					std::cout << "You lose." << std::endl;

				std::cout << "Press any key to quit...";
				char quit;
				std::cin >> quit;
				exit = true;
			} break;
		}
	}
	networkThread.join();
	client.release();
}
void main_client()
{
	;
}
void main_server()
{
	;
}