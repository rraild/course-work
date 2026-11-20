#include <array>
#include <iomanip>
#include <iostream>
#include <random>
#include <string_view>

struct SessionResult {
    bool success;
    int attempts;
};

// nextAttempt возвращает результат одной попытки. Лимит по условию положительный.
template <typename NextAttempt>
SessionResult runSession(int limit, NextAttempt nextAttempt) {
    for (int attempt = 1; attempt <= limit; ++attempt) {
        if (nextAttempt()) {
            return {true, attempt};
        }
    }
    return {false, limit};
}

struct TestCase {
    std::string_view sequence;
    int limit;
    SessionResult expected;
};

bool checkCounter() {
    // '+' обозначает правильный ввод, '-' обозначает ошибку.
    const std::array<TestCase, 5> tests{{
        {"+--",         3,  {true, 1}},
        {"-+-",         3,  {true, 2}},
        {"--+",         3,  {true, 3}},
        {"-----+",      5,  {false, 5}},
        {"----------+", 10, {false, 10}},
    }};

    bool allPassed = true;
    for (const auto& test : tests) {
        int consumed = 0;
        const auto result = runSession(test.limit, [&]() {
            return test.sequence.at(consumed++) == '+';
        });
        const bool passed = result.success == test.expected.success
                         && result.attempts == test.expected.attempts
                         && consumed == test.expected.attempts;
        std::cerr << test.sequence << " | лимит " << test.limit << " | "
                  << (result.success ? "успех" : "блокировка")
                  << " | попыток: " << result.attempts
                  << " | " << (passed ? "ОК" : "ОШИБКА") << '\n';
        allPassed = allPassed && passed;
    }
    return allPassed;
}

struct Statistics {
    int successes = 0;
    int totalAttempts = 0;
};

Statistics simulate(int sessions, int limit, double probability, std::mt19937& generator) {
    std::bernoulli_distribution attempt(probability);
    Statistics stats;
    for (int session = 0; session < sessions; ++session) {
        const auto result = runSession(limit, [&]() { return attempt(generator); });
        if (result.success) {
            ++stats.successes;
        }
        stats.totalAttempts += result.attempts;
    }
    return stats;
}

int main() {
    if (!checkCounter()) {
        return 1;
    }

    constexpr int ownerSessions = 100'000;
    constexpr int guessSessions = 1'000'000;
    std::mt19937 generator(42);
    std::cout << "Цифр\tЛимит\tТочность владельца, %\tВходы, %\tПодборы, %"
                 "\tПопыток владельца в среднем\tПопыток подбора в среднем\n";
    std::cout << std::fixed << std::setprecision(6);
    for (int digits : {4, 6}) {
        const double guessProbability = digits == 4 ? 1.0 / 10'000 : 1.0 / 1'000'000;
        for (int limit : {3, 5, 10}) {
            for (int accuracy : {80, 90}) {
                const auto owner = simulate(ownerSessions, limit, accuracy / 100.0, generator);
                const auto guess = simulate(guessSessions, limit, guessProbability, generator);
                std::cout << digits << '\t' << limit << '\t' << accuracy << '\t'
                          << 100.0 * owner.successes / ownerSessions << '\t'
                          << 100.0 * guess.successes / guessSessions << '\t'
                          << static_cast<double>(owner.totalAttempts) / ownerSessions << '\t'
                          << static_cast<double>(guess.totalAttempts) / guessSessions << '\n';
            }
        }
    }
}
