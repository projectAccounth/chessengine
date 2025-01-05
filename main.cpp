#include <chesscpp.h>
#include <unordered_map>

using squ = std::optional<std::tuple<square, pieceSymbol, color>>;

int getPieceValue(const pieceSymbol& p) {
	std::map<pieceSymbol, int> pieceValue = {
		{PAWN, 1},
		{BISHOP, 3},
		{KNIGHT, 3},
		{QUEEN, 9},
		{ROOK, 5},
		{KING, 200}
	};
	return pieceValue.at(p);
}

int getRank(const square& sq) {
	return static_cast<int>(sq) / 8;
}

int getFile(const square& sq) {
	return static_cast<int>(sq) % 8;
}

std::optional<square> createSquare(const int& rank, const int& file) {
	if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
		return static_cast<square>(rank * 8 + file);
	}
	return std::nullopt;
}

std::vector<square> getSurroundingSquares(const square& sq) {
	std::vector<square> surrounding;
	int rank = getRank(sq);
	int file = getFile(sq);

	const int offsets[8][2] = {
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1},          {0, 1},
		{1, -1}, {1, 0}, {1, 1}
	};

	for (const auto& offset : offsets) {
		auto neighbor = createSquare(rank + offset[0], file + offset[1]);
		if (neighbor) {
			surrounding.push_back(*neighbor);
		}
	}
	return surrounding;
}


std::optional<std::pair<pieceSymbol, color>> pieceOnSquare(Chess& game, const square& sq) {
	auto board = game.board();
	for (auto& row : board) {
		for (auto& p : row) {
			if (!p) continue;
			if (std::get<square>(p.value()) == sq) return std::make_pair(std::get<pieceSymbol>(p.value()), std::get<color>(p.value()));
		}
	}
	return std::nullopt;
};

auto evaluatePawnShield = [](Chess& game, const square& kingPos, const bool& isWhite) {
	int shieldBonus = 0;
	int direction = isWhite ? 1 : -1;
	std::string strKingPos = squareToString(kingPos);
	std::vector<square> shieldSquares = {
		stringToSquare(std::to_string(strKingPos[1] + direction) + std::to_string(strKingPos[0] - 1)),
		stringToSquare(std::to_string(strKingPos[1] + direction) + std::to_string(strKingPos[0])),
		stringToSquare(std::to_string(strKingPos[1] + direction) + std::to_string(strKingPos[0] + 1))
	};

	for (const auto& sq : shieldSquares) {
		if (pieceOnSquare(game, sq) == std::make_pair(PAWN, WHITE)) shieldBonus++;
	}

	return shieldBonus;
};

auto evaluateEnemyAttacks = [](Chess& game, const square& kingPos, const bool& isWhite) {
	double attackScore = 0.0;
	auto safetyZone = getSurroundingSquares(kingPos);

	color opponentSide = isWhite ? BLACK : WHITE;


	for (auto& sq : safetyZone) {
		auto attackingPieces = game.getAttackingPieces(opponentSide, sq);
		if (attackingPieces.empty() || attackingPieces[0] == std::nullopt) continue;
		for (auto& attacker : attackingPieces)
			if (game.isAttacked(sq, opponentSide)) attackScore += getPieceValue(attacker.value()) / 10;
	}

	return attackScore;
};

auto evaluateExposedKing = [](Chess& game, const square& kingPos, const bool& isWhite) {
	int exposedPenalty = 0;

	std::vector<square> squaresAroundKing = getSurroundingSquares(kingPos);

	color currentColor = isWhite ? WHITE : BLACK;
	color opponentSide = currentColor == WHITE ? BLACK : WHITE;

	for (auto& sq : squaresAroundKing) {
		bool isPieceOpponent = pieceOnSquare(game, sq).value().second != currentColor;
		bool isSquareAttacked = game.isAttacked(sq, opponentSide);
		if (isPieceOpponent || isSquareAttacked) exposedPenalty++;
	}

	return exposedPenalty;
};

auto evaluateKingMobility = [](Chess& game, const square& kingPos) {
	return static_cast<int>(game.moves(squareToString(kingPos), pieceOnSquare(game, kingPos).value().first).size());
};

auto evaluateKingPositioning = [](Chess& game, const square& kingPos, const bool& isWhite) {
	if (game.)
};

double evaluateKingSafety(Chess& game) {
	bool isWhite = game.turn() == WHITE;
	auto board = game.board();

	const double pawnShieldWeight = 1.0;
	const double exposedKingWeight = -0.5;
	const double enemyAttacksWeight = -0.2;
	const double kingPositioningWeight = -0.8;
	const double openFilesWeight = -1.0;
	const double mobilityWeight = 0.1;

	square kingPosition;

	for (auto& row : board) {
		for (auto& p : row) {
			if (!p) continue;
			if (std::get<pieceSymbol>(p.value()) != KING) continue;
			if (std::get<color>(p.value()) == WHITE && isWhite) { kingPosition = std::get<square>(p.value()); break; }
			if (std::get<color>(p.value()) == BLACK && !isWhite) { kingPosition = std::get<square>(p.value()); break; }
		}
	}

	double safetyScore = 0;

	safetyScore += pawnShieldWeight * evaluatePawnShield(game, kingPosition, isWhite);
	safetyScore += exposedKingWeight * evaluateExposedKing(game, kingPosition, isWhite);
	safetyScore += enemyAttacksWeight * evaluateEnemyAttacks(game, kingPosition, isWhite);
	safetyScore += kingPositioningWeight * evaluatePositioning(game, kingPosition, isWhite);
	safetyScore += mobilityWeight * evaluateKingMobility(game, kingPosition);
	safetyScore += openFilesWeight * evaluateOpenFiles(game, kingPosition, isWhite);

	return safetyScore;
}