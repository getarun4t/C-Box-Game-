/**
 * @file asaphus_coding_challenge.cpp
 * @version 1.2
 * @copyright Copyright (c) 2022 Asaphus Vision GmbH
 *
 * The goal is to implement the mechanics of a simple game and write test cases for them.
 * The rules of the game are:
 * - There are two types of boxes, green and blue.
 * - Both can absorb tokens of a given weight, which they add to their own total weight.
 * - Both are initialized with a given initial weight.
 * - After a box absorbs a token weight, it outputs a score.
 * - Green and blue boxes calculate the score in different ways:
 * - A green box calculates the score as the square of the mean of the 3 weights that it most recently absorbed (square of mean of all absorbed weights if there are fewer than 3).
 * - A blue box calculates the score as Cantor's pairing function of the smallest and largest weight that it has absorbed so far, i.e. pairing(smallest, largest), where pairing(0, 1) = 2
 * - The game is played with two green boxes with initial weights 0.0 and 0.1, and two blue boxes with initial weights 0.2 and 0.3.
 * - There is a list of input token weights. Each gets used in one turn.
 * - There are two players, A and B. Both start with a score of 0.
 * - The players take turns alternatingly. Player A starts.
 * - In each turn, the current player selects one of the boxes with the currently smallest weight, and lets it absorb the next input token weight. Each input weight gets only used once.
 * - The result of the absorption gets added to the current player's score.
 * - When all input token weights have been used up, the game ends, and the player with highest score wins.
 *
 * Task:
 * - Create a git repo for the implementation. Use the git repository to have a commit history.
 * - Implement all missing parts, marked with "TODO", including the test cases.
 * - Split the work in reasonable commits (not just one commit).
 * - Make sure the test cases succeed.
 * - Produce clean, readable code.
 *
 * Notes:
 * - Building and running the executable: g++ --std=c++14 asaphus_coding_challenge.cpp -o challenge && ./challenge
 * - Feel free to add a build system like CMake, meson, etc.
 * - Feel free to add more test cases, if you would like to test more.
 * - This file includes the header-only test framework Catch v2.13.10.
 * - A main function is not required, as it is provided by the test framework.
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <numeric>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

class Box {
public:
    explicit Box(double initial_weight) : weight_(initial_weight), total_weight_(initial_weight) {}
    static std::unique_ptr<Box> makeGreenBox(double initial_weight);
    static std::unique_ptr<Box> makeBlueBox(double initial_weight);
    bool operator<(const Box& rhs) const { return weight_ < rhs.weight_; }

    double getScore() const { return calculateScore(); }

    void absorbWeight(double weight) {
        absorbed_weights_.push_back(weight);
        total_weight_ += weight;
    }
protected:
    double weight_;
    double total_weight_;
    std::vector<double> absorbed_weights_;

    virtual double calculateScore() const = 0;

    double meanOfLastThreeWeights() const {
        double mean = 0.0;
        int count = std::min(static_cast<int>(absorbed_weights_.size()), 3);
        if (count == 0) {
            return mean;
        }
        for (int i = 0; i < count; ++i) {
            mean += absorbed_weights_[absorbed_weights_.size() - 1 - i];
        }
        return mean / count;
    }

    double smallestWeight() const {
        if (absorbed_weights_.empty()) {
            return std::numeric_limits<double>::max();
        }
        return *std::min_element(absorbed_weights_.begin(), absorbed_weights_.end());
    }

    double largestWeight() const {
        if (absorbed_weights_.empty()) {
            return std::numeric_limits<double>::min();
        }
        return *std::max_element(absorbed_weights_.begin(), absorbed_weights_.end());
    }
};

class GreenBox : public Box {
public:
    using Box::Box;
protected:
    double calculateScore() const override {
        double mean = meanOfLastThreeWeights();
        return mean * mean;
    }
};

class BlueBox : public Box {
public:
    using Box::Box;
protected:
    double calculateScore() const override {
        double smallest = smallestWeight();
        double largest = largestWeight();
        return cantorsPairing(smallest, largest);
    }
private:
    double cantorsPairing(double x, double y) const {
        return 0.5 * (x + y) * (x + y + 1) + y;
    }
};

std::unique_ptr<Box> Box::makeGreenBox(double initial_weight) {
    return std::make_unique<GreenBox>(initial_weight);
}

std::unique_ptr<Box> Box::makeBlueBox(double initial_weight) {
    return std::make_unique<BlueBox>(initial_weight);
}

class Player {
public:
    Player() : score_(0.0) {}

    void takeTurn(uint32_t input_weight, std::vector<std::unique_ptr<Box>>& boxes) {
        auto min_box = std::min_element(boxes.begin(), boxes.end(), [](const auto& a, const auto& b) {
            return *a < *b;
            });
        (*min_box)->absorbWeight(static_cast<double>(input_weight));
        score_ += (*min_box)->getScore();
    }

    double getScore() const { return score_; }
private:
    double score_;
};

std::pair<double, double> play(const std::vector<uint32_t>& input_weights) {
    std::vector<std::unique_ptr<Box>> boxes;
    boxes.emplace_back(Box::makeGreenBox(0.0));
    boxes.emplace_back(Box::makeGreenBox(0.1));
    boxes.emplace_back(Box::makeBlueBox(0.2));
    boxes.emplace_back(Box::makeBlueBox(0.3));

    Player player_A;
    Player player_B;

    bool is_player_A_turn = true;

    for (uint32_t weight : input_weights) {
        if (is_player_A_turn) {
            player_A.takeTurn(weight, boxes);
        }
        else {
            player_B.takeTurn(weight, boxes);
        }

        is_player_A_turn = !is_player_A_turn;
    }

    std::cout << "Scores: player A " << player_A.getScore() << ", player B "
        << player_B.getScore() << std::endl;

    return std::make_pair(player_A.getScore(), player_B.getScore());
}

TEST_CASE("Final scores for first 4 Fibonacci numbers", "[fibonacci4]") {
    std::vector<uint32_t> inputs{ 1, 1, 2, 3 };
    auto result = play(inputs);
    REQUIRE(result.first == 13.0);
    REQUIRE(result.second == 25.0);
}

TEST_CASE("Final scores for first 8 Fibonacci numbers", "[fibonacci8]") {
    std::vector<uint32_t> inputs{ 1, 1, 2, 3, 5, 8, 13, 21 };
    auto result = play(inputs);
    REQUIRE(result.first == 155.0);
    REQUIRE(result.second == 366.25);
}

TEST_CASE("Test absorption of green box", "[green]") {
    std::unique_ptr<Box> green_box = Box::makeGreenBox(0.0);
    green_box->absorbWeight(5.0);
    REQUIRE(green_box->getScore() == 25.0);
}

TEST_CASE("Test absorption of blue box", "[blue]") {
    std::unique_ptr<Box> blue_box = Box::makeBlueBox(0.2);
    blue_box->absorbWeight(0.5);
    blue_box->absorbWeight(0.1);
    REQUIRE(blue_box->getScore() == 0.5);
}

TEST_CASE("Test absorption of green box with less than 3 weights", "[green]") {
    std::unique_ptr<Box> green_box = Box::makeGreenBox(0.0);
    green_box->absorbWeight(5.0);
    REQUIRE(green_box->getScore() == 25.0);
}

TEST_CASE("Test absorption of blue box with less than 3 weights", "[blue]") {
    std::unique_ptr<Box> blue_box = Box::makeBlueBox(0.2);
    blue_box->absorbWeight(0.5);
    REQUIRE(blue_box->getScore() == 0.5);
}

TEST_CASE("Test absorption of green box with equal weights", "[green]") {
    std::unique_ptr<Box> green_box = Box::makeGreenBox(0.0);
    green_box->absorbWeight(5.0);
    green_box->absorbWeight(5.0);
    REQUIRE(green_box->getScore() == 50.0);
}

TEST_CASE("Test absorption of blue box with equal weights", "[blue]") {
    std::unique_ptr<Box> blue_box = Box::makeBlueBox(0.2);
    blue_box->absorbWeight(0.5);
    blue_box->absorbWeight(0.5);
    REQUIRE(blue_box->getScore() == 1.0);
}

TEST_CASE("Test absorption of green box with empty weights vector", "[green]") {
    std::unique_ptr<Box> green_box = Box::makeGreenBox(0.0);
    REQUIRE(green_box->getScore() == 0.0);
}

TEST_CASE("Test absorption of blue box with empty weights vector", "[blue]") {
    std::unique_ptr<Box> blue_box = Box::makeBlueBox(0.2);
    REQUIRE(blue_box->getScore() == 0.0);
}

TEST_CASE("Test play() function with empty input weights vector", "[play]") {
    std::vector<uint32_t> inputs;
    auto result = play(inputs);
    REQUIRE(result.first == 0.0);
    REQUIRE(result.second == 0.0);
}
