#include <array>
#include <iostream>
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

int main() {
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
        std::cout << test.sequence << " | лимит " << test.limit << " | "
                  << (result.success ? "успех" : "блокировка")
                  << " | попыток: " << result.attempts
                  << " | " << (passed ? "ОК" : "ОШИБКА") << '\n';
        allPassed = allPassed && passed;
    }
    return allPassed ? 0 : 1;
}
