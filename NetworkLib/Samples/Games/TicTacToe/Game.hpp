#pragma once

#include <array>

namespace TicTacToe
{
	enum class Case
	{
		Empty,
		X,
		O,
	};
	class Grid
	{
	public:
		Grid() = default;
		~Grid() = default;

		// Play a move from given player in given case. Return true if it's valid, false otherwise.
		bool play(unsigned int x, unsigned int y, Case player);
		// Return true if the game is over, false otherwise
		bool isFinished() const { return mFinished; }
		// Return winner, if any, Case::Empty otherwise
		Case winner() const { return mWinner; }

		// Display the board in console
		void display() const;

	private:
		// Check if the grid is full
		bool isGridFull() const;

	private:
		std::array<std::array<Case, 3>, 3> mGrid{ Case::Empty, Case::Empty, Case::Empty, Case::Empty, Case::Empty, Case::Empty, Case::Empty, Case::Empty, Case::Empty };
		Case mWinner{ Case::Empty };
		bool mFinished{ false };
	};
}